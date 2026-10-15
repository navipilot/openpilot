#!/usr/bin/env bash
# Compiles a converted model with Google's ahead-of-time compiler for Tensor,
# to see on a PC whether it compiles for a Tensor NPU at all: the phone's own
# compiler, which the app uses, runs only on the phone.
#
#   tensor-compile-check.sh SDK_TARBALL MODEL.tflite [SOC...]
#
# SDK_TARBALL is the Google Tensor SDK beta's litert_plugin_compiler.tar.gz.
# MODEL.tflite is the model.tflite of a LiteRT artifact. SOC is Tensor_G3,
# Tensor_G4, Tensor_G5 or Tensor_G6; all four by default. Prints LiteRT's
# compilation report, how many ops each SoC takes, and keeps nothing else:
# the compiler writes beside its input, so it gets a copy.
#
# The compiler is x86-64 Linux only, so it runs in Docker as linux/amd64
# (emulated on an Apple silicon Mac). Its Python environment is kept in
# $JETLINK_TENSOR_ENV (~/.cache/jetlink-tensor by default) between runs, on
# the host's disk rather than Docker's. The compiler needs about 11 times the
# model's weights in memory: give Docker 10 GB or more for a big model. A
# failed compile's log is in $ENV_DIR/tmp, which the report calls /env/tmp.
set -euo pipefail

SDK=${1:?usage: tensor-compile-check.sh SDK_TARBALL MODEL.tflite [SOC...]}
MODEL=${2:?usage: tensor-compile-check.sh SDK_TARBALL MODEL.tflite [SOC...]}
shift 2
SOCS=("$@")
[[ ${#SOCS[@]} -gt 0 ]] || SOCS=(Tensor_G3 Tensor_G4 Tensor_G5 Tensor_G6)

# The release the app runs, and the SDK package Google's notebook pairs with it.
LITERT=2.2.0
SDK_PACKAGE=2.1.5
ENV_DIR=${JETLINK_TENSOR_ENV:-$HOME/.cache/jetlink-tensor}
mkdir -p "$ENV_DIR/tmp"
SDK_DIR=$(cd "$(dirname "$SDK")" && pwd)
WORK="$ENV_DIR/tmp/check"
rm -rf "$WORK"
mkdir -p "$WORK"
trap 'rm -rf "$WORK"' EXIT
ln "$MODEL" "$WORK/model.tflite" 2>/dev/null || cp "$MODEL" "$WORK/model.tflite"

run() {
  docker run --rm -i --platform linux/amd64 \
    -v "$ENV_DIR":/env -v "$SDK_DIR":/sdk:ro \
    -e TMPDIR=/env/tmp -e GOOGLE_TENSOR_SDK_BETA="/sdk/$(basename "$SDK")" \
    python:3.11-slim bash -c "$1"
}

# The venv's python links to the container's, so the host checks its marker.
if [[ ! -f "$ENV_DIR/venv/installed" ]]; then
  echo "installing the compiler into $ENV_DIR (once)"
  run "python -m venv --clear /env/venv && /env/venv/bin/python -m pip install -q --no-cache-dir \
    ai-edge-litert-sdk-google-tensor==$SDK_PACKAGE ai-edge-litert==$LITERT && touch /env/venv/installed"
fi

# One after another, as aot_compile runs them: two at once would need twice
# the memory.
run "cd /env/tmp && /env/venv/bin/python - /env/tmp/check/model.tflite ${SOCS[*]}" <<'PYTHON'
import sys
from ai_edge_litert.aot import aot_compile
from ai_edge_litert.aot.vendors.google_tensor import target

targets = [target.Target(target.SocModel(soc)) for soc in sys.argv[2:]]
print(aot_compile.aot_compile(sys.argv[1], target=targets, keep_going=True).compilation_report())
PYTHON
