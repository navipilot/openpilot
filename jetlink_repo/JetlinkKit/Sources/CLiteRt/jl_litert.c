#include "jl_litert.h"

#include <dlfcn.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "litert/c/internal/litert_accelerator.h"
#include "litert/c/internal/litert_logging.h"
#include "litert/c/litert_common.h"
#include "litert/c/litert_compiled_model.h"
#include "litert/c/litert_environment.h"
#include "litert/c/litert_model.h"
#include "litert/c/litert_opaque_options.h"
#include "litert/c/litert_options.h"
#include "litert/c/litert_tensor_buffer.h"
#include "litert/c/litert_tensor_buffer_requirements.h"

#ifdef __APPLE__
#define JL_LITERT_LIBRARY "libLiteRt.dylib"
#else
#define JL_LITERT_LIBRARY "libLiteRt.so"
#endif

// Every entry point the shim calls. Each is found by name in the library and
// called through a pointer of the vendored header's type, so a prototype that
// differs from the header does not compile.
#define JL_LITERT_FUNCTIONS(X)                                \
  X(LiteRtGetStatusString)                                    \
  X(LiteRtGetDefaultLogger)                                   \
  X(LiteRtSetMinLoggerSeverity)                               \
  X(LiteRtCreateEnvironment)                                  \
  X(LiteRtDestroyEnvironment)                                 \
  X(LiteRtGetNumAccelerators)                                 \
  X(LiteRtGetAccelerator)                                     \
  X(LiteRtGetAcceleratorName)                                 \
  X(LiteRtGetAcceleratorHardwareSupport)                      \
  X(LiteRtCreateModelFromFile)                                \
  X(LiteRtDestroyModel)                                       \
  X(LiteRtGetNumModelSignatures)                              \
  X(LiteRtGetModelSignature)                                  \
  X(LiteRtGetNumSignatureInputs)                              \
  X(LiteRtGetSignatureInputName)                              \
  X(LiteRtGetSignatureInputTensorByIndex)                     \
  X(LiteRtGetNumSignatureOutputs)                             \
  X(LiteRtGetSignatureOutputName)                             \
  X(LiteRtGetSignatureOutputTensorByIndex)                    \
  X(LiteRtGetTensorTypeId)                                    \
  X(LiteRtGetRankedTensorType)                                \
  X(LiteRtCreateOptions)                                      \
  X(LiteRtDestroyOptions)                                     \
  X(LiteRtSetOptionsHardwareAccelerators)                     \
  X(LiteRtCreateOpaqueOptions)                                \
  X(LiteRtDestroyOpaqueOptions)                               \
  X(LiteRtAddOpaqueOptions)                                   \
  X(LiteRtCreateCompiledModel)                                \
  X(LiteRtDestroyCompiledModel)                               \
  X(LiteRtCompiledModelIsFullyAccelerated)                    \
  X(LiteRtGetCompiledModelInputBufferRequirements)            \
  X(LiteRtGetCompiledModelOutputBufferRequirements)           \
  X(LiteRtGetNumTensorBufferRequirementsSupportedBufferTypes) \
  X(LiteRtGetTensorBufferRequirementsSupportedTensorBufferType) \
  X(LiteRtCreateTensorBufferFromHostMemory)                   \
  X(LiteRtCreateManagedTensorBufferFromRequirements)          \
  X(LiteRtGetTensorBufferPackedSize)                          \
  X(LiteRtLockTensorBuffer)                                   \
  X(LiteRtUnlockTensorBuffer)                                 \
  X(LiteRtDestroyTensorBuffer)                                \
  X(LiteRtRunCompiledModel)

static struct {
#define JL_FIELD(name) __typeof__(&name) name;
  JL_LITERT_FUNCTIONS(JL_FIELD)
#undef JL_FIELD
} lrt;

static pthread_mutex_t opening = PTHREAD_MUTEX_INITIALIZER;
static LiteRtEnvironment environment;
// The directory LiteRT was opened from, NULL for the loader's path: kept for
// the life of the process, as the environment is.
static char *runtime_dir;

struct jl_litert_model {
  // A model for an NPU's own environment, NULL for the process's.
  LiteRtEnvironment environment;
  // The directory that environment keeps the NPU's compiled model in, which
  // outlives it.
  char *npu_cache_dir;
  LiteRtModel model;
  LiteRtOptions options;
  LiteRtCompiledModel compiled;
  size_t n_inputs;
  size_t n_outputs;
  // signature order, inputs then outputs
  const char **names;
  LiteRtRankedTensorType *types;
};

// The handle the caller holds is LiteRT's own.
static LiteRtTensorBuffer handle(jl_litert_buffer *buffer) {
  return (LiteRtTensorBuffer)buffer;
}

static char *copy(const char *s) {
  size_t n = strlen(s) + 1;
  char *out = malloc(n);
  if (out != NULL) {
    memcpy(out, s, n);
  }
  return out;
}

static char *format(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  int n = vsnprintf(NULL, 0, fmt, args);
  va_end(args);
  char *out = n >= 0 ? malloc((size_t)n + 1) : NULL;
  if (out != NULL) {
    va_start(args, fmt);
    vsnprintf(out, (size_t)n + 1, fmt, args);
    va_end(args);
  }
  return out;
}

// NULL for kLiteRtStatusOk, else what failed and LiteRT's word for why. The
// details are in LiteRT's own log (stderr, or logcat on Android).
static char *take(LiteRtStatus status, const char *call) {
  if (status == kLiteRtStatusOk) {
    return NULL;
  }
  char *message = format("%s failed: %s", call, lrt.LiteRtGetStatusString(status));
  return message != NULL ? message : copy("LiteRT failed");
}

// Calls LiteRT's `name` and returns what went wrong, if anything.
#define TRY(name, ...)                                     \
  do {                                                     \
    char *error_ = take(lrt.name(__VA_ARGS__), #name);     \
    if (error_ != NULL) {                                  \
      return error_;                                       \
    }                                                      \
  } while (0)

void jl_litert_free(void *p) {
  free(p);
}

char *jl_litert_open(const char *directory) {
  pthread_mutex_lock(&opening);
  if (environment != NULL) {
    pthread_mutex_unlock(&opening);
    return NULL;
  }
  bool named = directory != NULL && directory[0] != '\0';
  char *path = named ? format("%s/%s", directory, JL_LITERT_LIBRARY) : copy(JL_LITERT_LIBRARY);
  void *library = path != NULL ? dlopen(path, RTLD_NOW | RTLD_LOCAL) : NULL;
  if (library == NULL) {
    const char *why = dlerror();
    char *error = format("cannot open %s: %s", path != NULL ? path : JL_LITERT_LIBRARY, why != NULL ? why : "not found");
    free(path);
    pthread_mutex_unlock(&opening);
    return error;
  }
  const char *missing = NULL;
#define JL_LOAD(name)                                         \
  if (missing == NULL) {                                      \
    lrt.name = (__typeof__(&name))dlsym(library, #name);      \
    if (lrt.name == NULL) {                                   \
      missing = #name;                                        \
    }                                                         \
  }
  JL_LITERT_FUNCTIONS(JL_LOAD)
#undef JL_LOAD
  if (missing != NULL) {
    char *error = format("%s has no %s: not the LiteRT 2.2.0 jetlink was built against", path, missing);
    free(path);
    dlclose(library);
    memset(&lrt, 0, sizeof(lrt));
    pthread_mutex_unlock(&opening);
    return error;
  }
  free(path);

  // LiteRT's info lines are a few per model load; warnings and errors stay.
  lrt.LiteRtSetMinLoggerSeverity(lrt.LiteRtGetDefaultLogger(), kLiteRtLogSeverityWarning);

  // The GPU accelerator is a library of its own, which LiteRT opens from the
  // runtime library directory. The CPU and the GPU only: a model for an NPU
  // gets an environment of its own (npu_environment).
  LiteRtEnvOption options[2];
  int n_options = 0;
  options[n_options].tag = kLiteRtEnvOptionTagAutoRegisterAccelerators;
  options[n_options].value.type = kLiteRtAnyTypeInt;
  options[n_options].value.int_value = kLiteRtHwAcceleratorCpu | kLiteRtHwAcceleratorGpu;
  n_options++;
  if (named) {
    runtime_dir = copy(directory);
    options[n_options].tag = kLiteRtEnvOptionTagRuntimeLibraryDir;
    options[n_options].value.type = kLiteRtAnyTypeString;
    options[n_options].value.str_value = runtime_dir;
    n_options++;
  }
  char *error = take(lrt.LiteRtCreateEnvironment(n_options, options, &environment), "LiteRtCreateEnvironment");
  if (error != NULL) {
    environment = NULL;
  }
  pthread_mutex_unlock(&opening);
  return error;
}

char *jl_litert_accelerators(int *hardware, char *names, size_t names_cap) {
  *hardware = 0;
  if (names_cap > 0) {
    names[0] = '\0';
  }
  if (environment == NULL) {
    return copy("LiteRT is not open");
  }
  LiteRtParamIndex count = 0;
  TRY(LiteRtGetNumAccelerators, environment, &count);
  for (LiteRtParamIndex i = 0; i < count; i++) {
    LiteRtAccelerator accelerator = NULL;
    TRY(LiteRtGetAccelerator, environment, i, &accelerator);
    LiteRtHwAcceleratorSet supported = 0;
    TRY(LiteRtGetAcceleratorHardwareSupport, accelerator, &supported);
    *hardware |= supported;
    const char *name = NULL;
    TRY(LiteRtGetAcceleratorName, accelerator, &name);
    size_t used = strlen(names);
    if (name != NULL && used + 2 < names_cap) {
      snprintf(names + used, names_cap - used, "%s%s", used > 0 ? ", " : "", name);
    }
  }
  return NULL;
}

// Opaque options carry an accelerator's settings as TOML text, which the
// accelerator parses (LiteRT's litert/c/options/litert_gpu_options.cc and
// litert_cpu_options.cc write the same). `options` takes the text.
static char *add_toml(LiteRtOptions options, const char *identifier, char *toml) {
  if (toml == NULL) {
    return copy("out of memory");
  }
  LiteRtOpaqueOptions opaque = NULL;
  char *error = take(lrt.LiteRtCreateOpaqueOptions(identifier, toml, free, &opaque), "LiteRtCreateOpaqueOptions");
  if (error != NULL) {
    free(toml);
    return error;
  }
  error = take(lrt.LiteRtAddOpaqueOptions(options, opaque), "LiteRtAddOpaqueOptions");
  if (error != NULL) {
    lrt.LiteRtDestroyOpaqueOptions(opaque);
  }
  return error;
}

// `s` as a TOML basic string's contents: quotes and backslashes escaped.
static char *escaped(const char *s) {
  char *out = malloc(2 * strlen(s) + 1);
  if (out != NULL) {
    char *p = out;
    for (; *s != '\0'; s++) {
      if (*s == '"' || *s == '\\') {
        *p++ = '\\';
      }
      *p++ = *s;
    }
    *p = '\0';
  }
  return out;
}

// On Android the GPU is OpenCL's alone. Left to choose, LiteRT falls back to
// OpenGL where there is no OpenCL, a backend its own header calls
// experimental; pinned, a phone without OpenCL fails the compile instead.
// Metal is a Mac's only backend.
#ifdef __ANDROID__
_Static_assert(kLiteRtGpuBackendOpenCl == 1, "the backend line below");
static const char gpu_backend[] = "backend = 1\n";
#else
static const char gpu_backend[] = "";
#endif

// fp16 arithmetic where the GPU can, and its compiled programs cached alone:
// a phone has no room for a second copy of the weights in the GPU's layout,
// which on an M1 Pro made the whole cache of Cinque Terre V3 772 MB (a 2 s
// load against a 7 s compile). The programs are what an OpenCL GPU compiles
// slowly; Metal writes no such cache.
static char *gpu_toml(const jl_litert_options *o) {
  if (o->cache_dir == NULL || o->cache_key == NULL) {
    return format("precision = %d\n%s", kLiteRtDelegatePrecisionFp16, gpu_backend);
  }
  char *dir = escaped(o->cache_dir);
  char *key = escaped(o->cache_key);
  char *toml = NULL;
  if (dir != NULL && key != NULL) {
    toml = format(
        "precision = %d\n%sserialization_dir = \"%s\"\nmodel_cache_key = \"%s\"\nserialize_program_cache = true\n"
        "cache_only_compiled_programs = true\n",
        kLiteRtDelegatePrecisionFp16, gpu_backend, dir, key);
  }
  free(dir);
  free(key);
  return toml;
}

static char *cpu_toml(int threads) {
  return format("num_threads = %d\n", threads);
}

// Google Tensor's NPU held at high performance, the mode for low-latency
// interactive work (litert_google_tensor_options_type.h); its runtime ignores
// it before the phone's southbound API 0.18. The plugin's compile options are
// not set: the compiler in the phone's system takes none from an app.
static char *google_tensor_toml(void) {
  return format("performance_mode = %d\n", 3);
}

// An environment for one model on an NPU: LiteRT finds the vendor's dispatch
// library and compiler plugin in the directory it was opened from, and keeps
// the model it compiles in `cache_dir`, where the next load finds it. LiteRT
// keys that copy by the model's bytes, the options and the phone's build
// fingerprint, so a system update compiles again.
static char *npu_environment(const char *cache_dir, LiteRtEnvironment *out) {
  if (runtime_dir == NULL) {
    return copy("an NPU's libraries are looked for in the directory LiteRT was opened from, and it was opened from the loader's path");
  }
  LiteRtEnvOption options[] = {
      {kLiteRtEnvOptionTagAutoRegisterAccelerators, {.type = kLiteRtAnyTypeInt}},
      {kLiteRtEnvOptionTagRuntimeLibraryDir, {.type = kLiteRtAnyTypeString, .str_value = runtime_dir}},
      {kLiteRtEnvOptionTagDispatchLibraryDir, {.type = kLiteRtAnyTypeString, .str_value = runtime_dir}},
      {kLiteRtEnvOptionTagCompilerPluginLibraryDir, {.type = kLiteRtAnyTypeString, .str_value = runtime_dir}},
      {kLiteRtEnvOptionTagCompilerCacheDir, {.type = kLiteRtAnyTypeString, .str_value = cache_dir}},
  };
  options[0].value.int_value = kLiteRtHwAcceleratorCpu | kLiteRtHwAcceleratorGpu | kLiteRtHwAcceleratorNpu;
  TRY(LiteRtCreateEnvironment, (int)(sizeof(options) / sizeof(options[0])), options, out);
  return NULL;
}

// The environment `model` runs in.
static LiteRtEnvironment environment_of(const jl_litert_model *model) {
  return model->environment != NULL ? model->environment : environment;
}

// ONNX's numbering, which the Swift side's ElementType uses; 0 for a type it
// does not stage.
static int onnx_type(LiteRtElementType type) {
  switch (type) {
    case kLiteRtElementTypeFloat32: return 1;
    case kLiteRtElementTypeUInt8: return 2;
    case kLiteRtElementTypeInt8: return 3;
    case kLiteRtElementTypeUInt16: return 4;
    case kLiteRtElementTypeInt16: return 5;
    case kLiteRtElementTypeInt32: return 6;
    case kLiteRtElementTypeInt64: return 7;
    case kLiteRtElementTypeBool: return 9;
    case kLiteRtElementTypeFloat16: return 10;
    case kLiteRtElementTypeFloat64: return 11;
    default: return 0;
  }
}

void jl_litert_model_release(jl_litert_model *model) {
  if (model == NULL) {
    return;
  }
  // the compiled model reads the model and its options
  if (model->compiled != NULL) {
    lrt.LiteRtDestroyCompiledModel(model->compiled);
  }
  if (model->options != NULL) {
    lrt.LiteRtDestroyOptions(model->options);
  }
  if (model->model != NULL) {
    lrt.LiteRtDestroyModel(model->model);
  }
  if (model->environment != NULL) {
    lrt.LiteRtDestroyEnvironment(model->environment);
  }
  free(model->npu_cache_dir);
  free(model->names);
  free(model->types);
  free(model);
}

// The first signature's inputs and outputs: names and types, which the model
// owns and which live as long as it does.
static char *describe(jl_litert_model *m) {
  LiteRtParamIndex signatures = 0;
  TRY(LiteRtGetNumModelSignatures, m->model, &signatures);
  if (signatures == 0) {
    return copy("the model has no signature");
  }
  LiteRtSignature signature = NULL;
  TRY(LiteRtGetModelSignature, m->model, 0, &signature);
  LiteRtParamIndex n_in = 0, n_out = 0;
  TRY(LiteRtGetNumSignatureInputs, signature, &n_in);
  TRY(LiteRtGetNumSignatureOutputs, signature, &n_out);
  m->n_inputs = n_in;
  m->n_outputs = n_out;
  m->names = calloc(n_in + n_out + 1, sizeof(char *));
  m->types = calloc(n_in + n_out + 1, sizeof(LiteRtRankedTensorType));
  if (m->names == NULL || m->types == NULL) {
    return copy("out of memory");
  }
  for (size_t i = 0; i < n_in + n_out; i++) {
    bool output = i >= n_in;
    LiteRtParamIndex index = output ? i - n_in : i;
    LiteRtTensor tensor = NULL;
    if (output) {
      TRY(LiteRtGetSignatureOutputName, signature, index, &m->names[i]);
      TRY(LiteRtGetSignatureOutputTensorByIndex, signature, index, &tensor);
    } else {
      TRY(LiteRtGetSignatureInputName, signature, index, &m->names[i]);
      TRY(LiteRtGetSignatureInputTensorByIndex, signature, index, &tensor);
    }
    LiteRtTensorTypeId id = kLiteRtUnrankedTensorType;
    TRY(LiteRtGetTensorTypeId, tensor, &id);
    if (id != kLiteRtRankedTensorType) {
      return format("%s has no fixed rank; jetlink builds fixed-shape engines", m->names[i]);
    }
    TRY(LiteRtGetRankedTensorType, tensor, &m->types[i]);
  }
  return NULL;
}

static char *compile(jl_litert_model *m, const char *path, const jl_litert_options *o) {
  char *error = NULL;
  if (o->npu_cache_dir != NULL) {
    m->npu_cache_dir = copy(o->npu_cache_dir);
    error = m->npu_cache_dir != NULL ? npu_environment(m->npu_cache_dir, &m->environment) : copy("out of memory");
    if (error != NULL) {
      return error;
    }
  }
  TRY(LiteRtCreateModelFromFile, environment_of(m), path, &m->model);
  error = describe(m);
  if (error != NULL) {
    return error;
  }
  TRY(LiteRtCreateOptions, &m->options);
  LiteRtHwAcceleratorSet hardware = o->gpu ? kLiteRtHwAcceleratorGpu : kLiteRtHwAcceleratorCpu;
  if (o->npu_cache_dir != NULL) {
    hardware |= kLiteRtHwAcceleratorNpu;
  }
  TRY(LiteRtSetOptionsHardwareAccelerators, m->options, hardware);
  if (o->gpu) {
    // LrtGetGpuOptionsIdentifier()
    error = add_toml(m->options, "gpu_options", gpu_toml(o));
  } else if (o->cpu_threads > 0) {
    // LrtGetCpuOptionsIdentifier()
    error = add_toml(m->options, "xnnpack", cpu_toml(o->cpu_threads));
  }
  if (error == NULL && o->npu_cache_dir != NULL) {
    // LrtGoogleTensorOptionsGetIdentifier()
    error = add_toml(m->options, "google_tensor", google_tensor_toml());
  }
  if (error != NULL) {
    return error;
  }
  // The log is the only word of why an NPU's plugin could not take a model,
  // so a compile for one says what it does there, partitions and all.
  bool npu = o->npu_cache_dir != NULL;
  if (npu) {
    lrt.LiteRtSetMinLoggerSeverity(lrt.LiteRtGetDefaultLogger(), kLiteRtLogSeverityInfo);
  }
  LiteRtStatus status = lrt.LiteRtCreateCompiledModel(environment_of(m), m->model, m->options, &m->compiled);
  if (npu) {
    lrt.LiteRtSetMinLoggerSeverity(lrt.LiteRtGetDefaultLogger(), kLiteRtLogSeverityWarning);
  }
  return take(status, "LiteRtCreateCompiledModel");
}

char *jl_litert_model_create(const char *path, const jl_litert_options *options, jl_litert_model **out) {
  *out = NULL;
  if (environment == NULL) {
    return copy("LiteRT is not open");
  }
  jl_litert_model *model = calloc(1, sizeof(jl_litert_model));
  if (model == NULL) {
    return copy("out of memory");
  }
  char *error = compile(model, path, options);
  if (error != NULL) {
    jl_litert_model_release(model);
    return error;
  }
  *out = model;
  return NULL;
}

char *jl_litert_model_fully_accelerated(const jl_litert_model *model, int *fully) {
  bool value = false;
  TRY(LiteRtCompiledModelIsFullyAccelerated, model->compiled, &value);
  *fully = value ? 1 : 0;
  return NULL;
}

size_t jl_litert_model_io_count(const jl_litert_model *model, int output) {
  return output ? model->n_outputs : model->n_inputs;
}

// Where input or output `index` is in the model's arrays, or -1.
static ptrdiff_t slot(const jl_litert_model *model, int output, size_t index) {
  if (index >= (output ? model->n_outputs : model->n_inputs)) {
    return -1;
  }
  return (ptrdiff_t)(output ? model->n_inputs + index : index);
}

char *jl_litert_model_io_info(const jl_litert_model *model, int output, size_t index, char *name, size_t name_cap,
                              int *elem_type, int64_t *dims, size_t dims_cap, size_t *rank) {
  ptrdiff_t i = slot(model, output, index);
  if (i < 0) {
    return copy("no such input or output");
  }
  if (name_cap > 0) {
    strncpy(name, model->names[i], name_cap - 1);
    name[name_cap - 1] = '\0';
  }
  const LiteRtRankedTensorType *type = &model->types[i];
  *elem_type = onnx_type(type->element_type);
  *rank = type->layout.rank;
  for (size_t d = 0; d < type->layout.rank && d < dims_cap; d++) {
    dims[d] = type->layout.dimensions[d] < 0 ? -1 : type->layout.dimensions[d];
  }
  return NULL;
}

// The requirements of input or output `index`, which the compiled model owns.
static char *requirements(const jl_litert_model *model, int output, size_t index, LiteRtTensorBufferRequirements *out) {
  if (output) {
    TRY(LiteRtGetCompiledModelOutputBufferRequirements, model->compiled, 0, index, out);
  } else {
    TRY(LiteRtGetCompiledModelInputBufferRequirements, model->compiled, 0, index, out);
  }
  return NULL;
}

char *jl_litert_model_input_hardware(const jl_litert_model *model, size_t index, int *hardware) {
  *hardware = JL_LITERT_CPU;
  if (slot(model, 0, index) < 0) {
    return copy("no such input");
  }
  LiteRtTensorBufferRequirements req = NULL;
  char *error = requirements(model, 0, index, &req);
  if (error != NULL) {
    return error;
  }
  // The first type the accelerator lists is the one it reads best; with
  // none, LiteRT treats the input as the CPU's.
  int n = 0;
  TRY(LiteRtGetNumTensorBufferRequirementsSupportedBufferTypes, req, &n);
  LiteRtTensorBufferType type = kLiteRtTensorBufferTypeHostMemory;
  if (n > 0) {
    TRY(LiteRtGetTensorBufferRequirementsSupportedTensorBufferType, req, 0, &type);
  }
  switch (type) {
    case kLiteRtTensorBufferTypeAhwb:
    case kLiteRtTensorBufferTypeIon:
    case kLiteRtTensorBufferTypeDmaBuf:
    case kLiteRtTensorBufferTypeFastRpc:
      *hardware = JL_LITERT_NPU;
      break;
    case kLiteRtTensorBufferTypeHostMemory:
    case kLiteRtTensorBufferTypeUnknown:
      *hardware = JL_LITERT_CPU;
      break;
    default:
      *hardware = JL_LITERT_GPU;
      break;
  }
  return NULL;
}

char *jl_litert_buffer_wrap(jl_litert_model *model, int output, size_t index, void *data, size_t nbytes,
                            jl_litert_buffer **out) {
  *out = NULL;
  ptrdiff_t i = slot(model, output, index);
  if (i < 0) {
    return copy("no such input or output");
  }
  LiteRtTensorBuffer buffer = NULL;
  TRY(LiteRtCreateTensorBufferFromHostMemory, &model->types[i], data, nbytes, NULL, &buffer);
  *out = (jl_litert_buffer *)buffer;
  return NULL;
}

char *jl_litert_buffer_create(jl_litert_model *model, int output, size_t index, jl_litert_buffer **out) {
  *out = NULL;
  ptrdiff_t i = slot(model, output, index);
  if (i < 0) {
    return copy("no such input or output");
  }
  LiteRtTensorBufferRequirements req = NULL;
  char *error = requirements(model, output, index, &req);
  if (error != NULL) {
    return error;
  }
  LiteRtTensorBuffer buffer = NULL;
  TRY(LiteRtCreateManagedTensorBufferFromRequirements, environment_of(model), &model->types[i], req, &buffer);
  error = jl_litert_buffer_write((jl_litert_buffer *)buffer, NULL, 0);
  if (error != NULL) {
    lrt.LiteRtDestroyTensorBuffer(buffer);
    return error;
  }
  *out = (jl_litert_buffer *)buffer;
  return NULL;
}

// Maps a buffer to host memory, checking it holds `nbytes` (0 for whatever
// it holds), and says how many bytes that is.
static char *lock(LiteRtTensorBuffer buffer, LiteRtTensorBufferLockMode mode, size_t nbytes, void **host, size_t *size) {
  TRY(LiteRtGetTensorBufferPackedSize, buffer, size);
  if (nbytes != 0 && nbytes != *size) {
    return format("a copy of %zu bytes for a tensor of %zu", nbytes, *size);
  }
  TRY(LiteRtLockTensorBuffer, buffer, host, mode);
  return NULL;
}

char *jl_litert_buffer_write(jl_litert_buffer *buffer, const void *data, size_t nbytes) {
  void *host = NULL;
  size_t size = 0;
  char *error = lock(handle(buffer), kLiteRtTensorBufferLockModeWrite, data != NULL ? nbytes : 0, &host, &size);
  if (error != NULL) {
    return error;
  }
  if (data != NULL) {
    memcpy(host, data, size);
  } else {
    memset(host, 0, size);
  }
  TRY(LiteRtUnlockTensorBuffer, handle(buffer));
  return NULL;
}

char *jl_litert_buffer_read(jl_litert_buffer *buffer, void *data, size_t nbytes) {
  void *host = NULL;
  size_t size = 0;
  char *error = lock(handle(buffer), kLiteRtTensorBufferLockModeRead, nbytes, &host, &size);
  if (error != NULL) {
    return error;
  }
  memcpy(data, host, size);
  TRY(LiteRtUnlockTensorBuffer, handle(buffer));
  return NULL;
}

void jl_litert_buffer_release(jl_litert_buffer *buffer) {
  if (buffer != NULL) {
    lrt.LiteRtDestroyTensorBuffer(handle(buffer));
  }
}

char *jl_litert_run(jl_litert_model *model, jl_litert_buffer *const *inputs, size_t n_inputs,
                    jl_litert_buffer *const *outputs, size_t n_outputs) {
  TRY(LiteRtRunCompiledModel, model->compiled, 0, n_inputs, (LiteRtTensorBuffer *)inputs, n_outputs,
                                 (LiteRtTensorBuffer *)outputs);
  return NULL;
}
