// What the shim and the fake share. Beside the sources rather than in
// include/, so Swift never imports it.
#ifndef JL_TRT_UTIL_H
#define JL_TRT_UTIL_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Writes why into err (errlen bytes at most, NUL terminated; err may be
// NULL) and returns code: every failure jl_trt.h describes goes through it.
static inline int say(char *err, size_t errlen, int code, const char *fmt, ...) __attribute__((format(printf, 4, 5)));

static inline int say(char *err, size_t errlen, int code, const char *fmt, ...) {
  if (err != NULL && errlen > 0) {
    va_list args;
    va_start(args, fmt);
    vsnprintf(err, errlen, fmt, args);
    va_end(args);
  }
  return code;
}

// Nonzero when TensorRT's reason for a refusal is memory it could not get
// ("out of memory", "OutOfMemory", "could not be allocated"), in any case: a
// deserialize that fails so says nothing about the plan, which must not be
// deleted for it.
static inline int is_allocation_failure(const char *reason) {
  static const char *const needles[] = {"out of memory", "outofmemory", "allocat"};
  char lower[1024];
  size_t n = 0;
  for (; reason != NULL && reason[n] != '\0' && n + 1 < sizeof lower; n++) {
    char c = reason[n];
    lower[n] = c >= 'A' && c <= 'Z' ? (char)(c - 'A' + 'a') : c;
  }
  lower[n] = '\0';
  for (size_t i = 0; i < sizeof needles / sizeof needles[0]; i++) {
    if (strstr(lower, needles[i]) != NULL) {
      return 1;
    }
  }
  return 0;
}

// Nonzero once all of data is in path and the file closed cleanly.
static inline int write_file(const char *path, const void *data, size_t size) {
  FILE *f = fopen(path, "wb");
  int ok = f != NULL && fwrite(data, 1, size, f) == size;
  return f != NULL && fclose(f) == 0 && ok;
}

// Round to nearest even, as a GPU converts.
static inline uint16_t float_to_half(float value) {
  uint32_t x;
  memcpy(&x, &value, sizeof x);
  uint32_t sign = (x >> 16) & 0x8000u, mag = x & 0x7fffffffu;
  if (mag >= 0x7f800000u) {
    return (uint16_t)(sign | 0x7c00u | (mag > 0x7f800000u ? 0x200u : 0));
  }
  if (mag >= 0x477ff000u) {
    return (uint16_t)(sign | 0x7c00u);
  }
  if (mag < 0x33000000u) {
    return (uint16_t)sign;
  }
  // a normal half drops 13 mantissa bits; a subnormal one drops more
  uint32_t shift = mag >= 0x38800000u ? 13 : 126u - (mag >> 23);
  uint32_t m = mag >= 0x38800000u ? mag - 0x38000000u : (mag & 0x7fffffu) | 0x800000u;
  uint32_t h = m >> shift, rest = m & ((1u << shift) - 1u), half = 1u << (shift - 1u);
  if (rest > half || (rest == half && (h & 1u))) {
    h++;
  }
  return (uint16_t)(sign | h);
}

#endif
