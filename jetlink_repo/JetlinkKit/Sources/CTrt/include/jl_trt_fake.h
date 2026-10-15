// Controls for the fake shim, jl_trt_fake.c: jl_trt.h over host memory, with
// no GPU and no TensorRT, so every Swift path runs in tests on a Mac and in a
// plain Linux container. It is compiled only when TensorRT's headers are not
// supplied, and its jl_trt_open always returns JL_TRT_UNAVAILABLE: a binary
// that shipped it by mistake could never serve with it. Tests get a handle
// from jl_trt_fake_open instead.
//
// Device memory is host memory filled with 0xff (a float read before anything
// wrote it is NaN), so a jl_trt_dptr is a host address; streams, events and
// graphs run synchronously; a capture records what is queued and a launch
// replays it. It keeps CUDA's and TensorRT's rules, so a Swift path that
// breaks one fails here first; a rule broken in a call that returns nothing
// (a destroy out of order, a double free) latches the sticky flag, and the
// next call fails with a "fake:" message saying why.
//
// A plan is text:
//
//   jl_trt_fake_plan 1
//   built 10.3.0.30
//   input x float16 1 8
//   input state float16 1 8
//   output y float32 1 8
//   output next_state float16 1 8 from state
//
// `built` is optional: a plan that has it loads only on a fake of that exact
// version, as TensorRT refuses a plan from another build. Types are float32
// and float16; a dim of -1 is dynamic. An enqueue computes, in double,
// output[j] = input[j] + 1 for an output `from` an input (a next_state), and
// otherwise output[j] = the sum over every input k of input_k[j mod count_k];
// then converts to the output's type. So a looped state that is never copied
// back still shows in the other outputs: after a reset, y = x, then x + 1,
// x + 2... Each enqueue adds 1 ms to the clock timing events read.
//
// A build writes the plan set by jl_trt_fake_set_build (by default the one
// above), with `built` and a `settings` line that records what the build was
// given: fp16=0|1 optimization_level=N workspace=BYTES
// timing_cache=none|cold|warm. It
// reports a root phase "fake build" with a step per layer, each with a nested
// phase "fake tactics" of two steps, and logs one warning. A timing cache is
// the line "jl_trt_fake_timing <major>.<minor>.<patch>.<build> <builds>";
// one of another version fails to attach, and each build adds one.
#ifndef JL_TRT_FAKE_H
#define JL_TRT_FAKE_H

#include "jl_trt.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  // What jl_trt_get_info reports.
  int major, minor, patch, build;
  int strongly_typed;
  int cuda_driver;
  int plugins;
  const char *device_name;
  int cc_major, cc_minor;
} jl_trt_fake_config;

// TensorRT 10.3.0.30 on an "Orin", sm87, 8 GB, CUDA 12.6, weakly typed, with
// plugins.
void jl_trt_fake_defaults(jl_trt_fake_config *config);

// A fake jl_trt; config NULL takes the defaults. device_name is copied.
int jl_trt_fake_open(const jl_trt_fake_config *config, jl_trt **out, char *err, size_t errlen);

// Makes the nth next call of `call` fail, 1 being the very next one. `call`
// is the function's name without "jl_trt_": "graph_launch",
// "context_enqueue", "engine_deserialize", "build_write_plan", "mem_alloc".
// It returns `code`, with err reading "<call>: <message>", and does nothing
// else; JL_TRT_CUDA_STICKY latches as the real shim's does. A JL_TRT_ERROR
// injected into "engine_deserialize" whose message names an allocation
// failure comes back JL_TRT_CUDA_ERROR, as the real shim reads TensorRT's
// log. Calls that return nothing cannot fail.
void jl_trt_fake_fail(jl_trt *trt, const char *call, int nth, int code, const char *message);

// The input and output lines the next build writes into its plan, and the
// layer count jl_trt_build_layers reports once parsed.
void jl_trt_fake_set_build(jl_trt *trt, const char *tensor_lines, int layers);

typedef struct {
  // Stream-ordered work done, direct or replayed by a graph; engine runs.
  uint64_t h2d, d2h, d2d, memsets, enqueues, graph_launches;
  // Live objects: all 0 once everything is destroyed.
  int64_t device_allocs, host_allocs, streams, events, graphs, graph_execs, engines, contexts, builds;
} jl_trt_fake_stats;

void jl_trt_fake_get_stats(jl_trt *trt, jl_trt_fake_stats *out);

#ifdef __cplusplus
}
#endif

#endif
