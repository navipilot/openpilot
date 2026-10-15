// A few plain C calls over LiteRT's C API, so Swift never holds LiteRT's
// handles or writes its accelerators' options. Only what the server needs: the
// process's environment, a model compiled for the CPU, the GPU or an NPU, its
// inputs and outputs, tensor buffers over caller memory or the accelerator's
// own, and runs.
//
// LiteRT is opened at run time everywhere, from a directory the caller names
// or else the loader's path: on a Mac the ai-edge-litert wheel's
// libLiteRt.dylib, on Android the LiteRT AAR's libLiteRt.so from the app's
// nativeLibraryDir. LiteRT then opens its GPU accelerator from the same
// directory (libLiteRtMetalAccelerator.dylib, libLiteRtClGlAccelerator.so),
// and for an NPU the vendor's dispatch library and compiler plugin
// (libLiteRtDispatch_GoogleTensor.so, libLiteRtCompilerPlugin_google_tensor.so).
// The build needs only the 2.2.0 headers in vendor/.
//
// Every call that can fail returns NULL on success, or a message the caller
// frees with jl_litert_free.
#ifndef JL_LITERT_H
#define JL_LITERT_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// LiteRtHwAccelerators' bits.
enum {
  JL_LITERT_CPU = 1,
  JL_LITERT_GPU = 2,
  JL_LITERT_NPU = 4,
};

typedef struct jl_litert_model jl_litert_model;
typedef struct jl_litert_buffer jl_litert_buffer;

typedef struct {
  // The GPU alone, in fp16 (1), so an op it cannot run fails the compile
  // rather than run on the CPU; or the CPU alone (0).
  int gpu;
  // XNNPACK's pool on the CPU; 0 leaves LiteRT's default.
  int cpu_threads;
  // Where the GPU keeps the programs it compiled between runs, under a key
  // naming the model; NULL for no cache.
  const char *cache_dir;
  const char *cache_key;
  // The NPU first, when set: an existing directory where LiteRT keeps the
  // model it compiled for the NPU, which the next load with the same
  // directory reads back rather than compile again. LiteRT hands the model
  // to the vendor's compiler plugin, which hands it to the compiler in the
  // phone's system; where the plugin cannot take it, LiteRT runs it on what
  // `gpu` names instead. The model gets an environment of its own, as this
  // directory is an environment's option.
  const char *npu_cache_dir;
} jl_litert_options;

// Opens LiteRT from `directory`, NULL or "" for the loader's path, and makes
// the process's environment. The first call that succeeds decides; later
// calls return at once.
char *jl_litert_open(const char *directory);

// The hardware the environment's accelerators drive (JL_LITERT_CPU,
// JL_LITERT_GPU), and their names, comma separated and truncated to
// names_cap. The GPU's is missing when its library did not load.
char *jl_litert_accelerators(int *hardware, char *names, size_t names_cap);

// Loads a .tflite and compiles it with `options`.
char *jl_litert_model_create(const char *path, const jl_litert_options *options, jl_litert_model **out);
void jl_litert_model_release(jl_litert_model *model);

// Whether the accelerators asked for run every op, none left to LiteRT's own
// CPU kernels.
char *jl_litert_model_fully_accelerated(const jl_litert_model *model, int *fully);



// output 0 counts the first signature's inputs, 1 its outputs.
size_t jl_litert_model_io_count(const jl_litert_model *model, int output);

// One input's or output's name (NUL terminated, truncated to name_cap),
// element type numbered as ONNX numbers them (1 float, 2 uint8, 10 float16)
// and shape. *rank is the true rank; at most dims_cap dims are written. A
// dynamic dimension is -1.
char *jl_litert_model_io_info(const jl_litert_model *model, int output, size_t index, char *name, size_t name_cap,
                              int *elem_type, int64_t *dims, size_t dims_cap, size_t *rank);

// Whose memory the accelerator reads input `index` from best: JL_LITERT_CPU
// for the host's, where a buffer over the caller's memory costs no copy;
// JL_LITERT_GPU for a GPU's; JL_LITERT_NPU for memory shared with an NPU (an
// AHardwareBuffer, DMA-BUF, ION or FastRPC buffer). For input 0, what runs
// the model: an NPU compile that worked, or one LiteRT fell back from.
char *jl_litert_model_input_hardware(const jl_litert_model *model, size_t index, int *hardware);

// A buffer for an input or output over `nbytes` of caller memory, 64-byte
// aligned, which must outlive it.
char *jl_litert_buffer_wrap(jl_litert_model *model, int output, size_t index, void *data, size_t nbytes,
                            jl_litert_buffer **out);

// A buffer of the kind the accelerator works in for input or output `index`
// (GPU memory on a GPU, an AHardwareBuffer on an NPU), zeroed: the looped
// state's, which an output writes too, and on an NPU every input's and
// output's.
char *jl_litert_buffer_create(jl_litert_model *model, int output, size_t index, jl_litert_buffer **out);

// Copies `nbytes` into or out of a buffer. Writing NULL zeroes it.
char *jl_litert_buffer_write(jl_litert_buffer *buffer, const void *data, size_t nbytes);
char *jl_litert_buffer_read(jl_litert_buffer *buffer, void *data, size_t nbytes);
void jl_litert_buffer_release(jl_litert_buffer *buffer);

// One run of the first signature, its buffers in signature order.
char *jl_litert_run(jl_litert_model *model, jl_litert_buffer *const *inputs, size_t n_inputs,
                    jl_litert_buffer *const *outputs, size_t n_outputs);

void jl_litert_free(void *p);

#ifdef __cplusplus
}
#endif

#endif
