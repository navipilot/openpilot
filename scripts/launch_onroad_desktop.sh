#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "${ROOT_DIR}"

if [[ ! -f .venv/bin/activate ]]; then
  echo "Missing .venv. Run tools/install_python_dependencies.sh first."
  exit 1
fi

default_jobs() {
  if command -v nproc >/dev/null 2>&1; then
    nproc
  elif command -v sysctl >/dev/null 2>&1; then
    sysctl -n hw.ncpu
  else
    echo 8
  fi
}

env_var_truthy() {
  [[ "${1:-}" =~ ^(1|true|yes|on)$ ]]
}

usage() {
  cat <<'EOF'
Usage:
  ./onroad [jobs] (--c3 | --c4 | --raybig | --all | --replay-only) [--prefix name] <route-or-replay-args...>

Examples:
  ./onroad --c3 f08912a233c1584f/2022-08-11--18-02-41/1
  ./onroad --c4 f08912a233c1584f/2022-08-11--18-02-41/1 --start 30
  ./onroad --all f08912a233c1584f/2022-08-11--18-02-41/1
  ./onroad --replay-only --demo --no-vipc --no-loop

Notes:
  - This is host/dev only. It uses the isolated host worktree and does not touch the device path.
  - A private comma connect route still requires tools/lib/auth.py before replay can download it.
  - Use multiple UI flags together if you want more than one desktop UI at once.
EOF
}

jobs="$(default_jobs)"
if [[ "${1:-}" =~ ^[0-9]+$ ]]; then
  jobs="$1"
  shift || true
fi

PREFIX_ARG=""
REPLAY_ARGS=()
UI_TARGETS=()
LEGACY_UI_SELECTION=""
REPLAY_ONLY=0
REPLAY_PID=""
UI_PIDS=()

parse_args() {
  while [[ $# -gt 0 ]]; do
    case "$1" in
      -h|--help)
        usage
        exit 0
        ;;
      --c3)
        UI_TARGETS+=(c3)
        shift
        ;;
      --c4)
        UI_TARGETS+=(c4)
        shift
        ;;
      --raybig)
        UI_TARGETS+=(raybig)
        shift
        ;;
      --all)
        UI_TARGETS+=(c3 c4 raybig)
        shift
        ;;
      --replay-only)
        REPLAY_ONLY=1
        shift
        ;;
      --ui)
        if [[ $# -lt 2 ]]; then
          echo "Missing value for --ui" >&2
          exit 1
        fi
        LEGACY_UI_SELECTION="$2"
        shift 2
        ;;
      --ui=*)
        LEGACY_UI_SELECTION="${1#*=}"
        shift
        ;;
      -p|--prefix)
        if [[ $# -lt 2 ]]; then
          echo "Missing value for $1" >&2
          exit 1
        fi
        PREFIX_ARG="$2"
        shift 2
        ;;
      --prefix=*)
        PREFIX_ARG="${1#*=}"
        shift
        ;;
      --)
        shift
        REPLAY_ARGS+=("$@")
        break
        ;;
      *)
        REPLAY_ARGS+=("$1")
        shift
        ;;
    esac
  done
}

expand_ui_targets() {
  local selection="${1// /}"

  case "${selection,,}" in
    all|"")
      UI_TARGETS=(c3 c4 raybig)
      return
      ;;
    none)
      UI_TARGETS=()
      return
      ;;
  esac

  local raw_targets=()
  IFS=',' read -r -a raw_targets <<< "${selection}"

  local normalized=()
  local raw=""
  for raw in "${raw_targets[@]}"; do
    case "${raw,,}" in
      c3|c4|raybig)
        normalized+=("${raw,,}")
        ;;
      *)
        echo "Unknown UI target in --ui: ${raw}" >&2
        echo "Valid values: all, none, c3, c4, raybig" >&2
        exit 1
        ;;
    esac
  done

  local ordered_targets=(c3 c4 raybig)
  local target=""
  for target in "${ordered_targets[@]}"; do
    local candidate=""
    for candidate in "${normalized[@]}"; do
      if [[ "${candidate}" == "${target}" ]]; then
        UI_TARGETS+=("${target}")
        break
      fi
    done
  done
}

dedupe_ui_targets() {
  local ordered_targets=(c3 c4 raybig)
  local deduped=()
  local target=""
  for target in "${ordered_targets[@]}"; do
    local candidate=""
    for candidate in "${UI_TARGETS[@]-}"; do
      if [[ "${candidate}" == "${target}" ]]; then
        deduped+=("${target}")
        break
      fi
    done
  done
  UI_TARGETS=("${deduped[@]-}")
}

cleanup() {
  local exit_code=$?
  trap - EXIT INT TERM

  local pid=""
  for pid in "${UI_PIDS[@]-}"; do
    if [[ -n "${pid}" ]]; then
      kill "${pid}" >/dev/null 2>&1 || true
    fi
  done
  if [[ -n "${REPLAY_PID}" ]]; then
    kill "${REPLAY_PID}" >/dev/null 2>&1 || true
  fi

  for pid in "${UI_PIDS[@]-}"; do
    if [[ -n "${pid}" ]]; then
      wait "${pid}" >/dev/null 2>&1 || true
    fi
  done
  if [[ -n "${REPLAY_PID}" ]]; then
    wait "${REPLAY_PID}" >/dev/null 2>&1 || true
  fi

  if [[ -n "${OPENPILOT_PREFIX:-}" && "${OPENPILOT_PREFIX}" == desktop-onroad-* ]]; then
    echo "Cleaning up temporary prefix environment (${OPENPILOT_PREFIX})..."
    rm -rf "/dev/shm/msgq_${OPENPILOT_PREFIX}"
    rm -rf "/tmp/comma_download_cache${OPENPILOT_PREFIX}"
    rm -rf "${HOME}/.comma${OPENPILOT_PREFIX}"
  fi

  exit "${exit_code}"
}

prepare_env() {
  source .venv/bin/activate

  if [[ -d /opt/homebrew/bin ]]; then
    export PATH="/opt/homebrew/bin:${PATH}"
  fi

  export PATH="${ROOT_DIR}/.venv/bin:${PATH}"
  export PYTHONPATH="${ROOT_DIR}:${ROOT_DIR}/starpilot/third_party"
  local repo_dir=""
  for repo_dir in "${ROOT_DIR}"/*_repo; do
    [[ -d "${repo_dir}" ]] && export PYTHONPATH="${PYTHONPATH}:${repo_dir}"
  done
  [[ -d "${ROOT_DIR}/third_party/acados" ]] && export PYTHONPATH="${PYTHONPATH}:${ROOT_DIR}/third_party/acados"

  export BASEDIR="${ROOT_DIR}"
  export NOBOARD=1
  export SIMULATION=1
  export SKIP_FW_QUERY=1
  export USE_WEBCAM=1
  export SP_C3_FAKE_WIFI=0
  export SP_C4_FAKE_WIFI=0
  export SP_RAYBIG_FAKE_WIFI=0
  export SP_ALLOW_DESKTOP_FAKE_WIFI=0

  if [[ "$(uname -s)" == "Darwin" ]] || env_var_truthy "${ZMQ:-0}"; then
    if [[ -n "${PREFIX_ARG}" || -n "${OPENPILOT_PREFIX:-}" ]]; then
      echo "Ignoring OPENPILOT_PREFIX because the ZMQ backend does not support prefixes." >&2
    fi
    unset OPENPILOT_PREFIX
  else
    export OPENPILOT_PREFIX="${PREFIX_ARG:-${OPENPILOT_PREFIX:-desktop-onroad-$$}}"
    mkdir -p "/dev/shm/msgq_${OPENPILOT_PREFIX}"
  fi
}

seed_params() {
  "${ROOT_DIR}/.venv/bin/python3" - <<'PY'
from openpilot.common.params import Params
from openpilot.system.version import terms_version, training_version

params = Params()
params.put("HasAcceptedTerms", terms_version)
params.put("CompletedTrainingVersion", training_version)
params.put_bool("OpenpilotEnabledToggle", True)
params.put_bool("IsDriverViewEnabled", False)
params.put_bool("ForceOnroad", False)
params.put_bool("ForceOffroad", False)
PY
}

seed_starpilot_theme() {
  "${ROOT_DIR}/.venv/bin/python3" - <<'PY'
from openpilot.starpilot.common.starpilot_functions import seed_desktop_theme_assets

seed_desktop_theme_assets()
PY
}

build_replay() {
  SP_DISABLE_AUTO_DEVICE_SCONS=1 "${ROOT_DIR}/.venv/bin/scons" --extras -j"${jobs}" tools/replay/replay
}

prepare_c3_runtime() {
  SP_C3_COMPILE_ONLY=1 "${ROOT_DIR}/scripts/launch_ui_desktop.sh" "${jobs}"
}

prepare_python_ui_runtime() {
  SP_KEEP_DESKTOP_RUNTIME_ARTIFACTS=1 SP_C4_COMPILE_ONLY=1 "${ROOT_DIR}/scripts/launch_ui_c4_desktop.sh" "${jobs}"
}

launch_replay() {
  local replay_cmd=(
    "${ROOT_DIR}/tools/replay/replay"
    --headless
    --dcam
    --ecam
  )
  replay_cmd+=("${REPLAY_ARGS[@]}")

  "${replay_cmd[@]}" &
  REPLAY_PID=$!

  sleep 1
  if ! kill -0 "${REPLAY_PID}" >/dev/null 2>&1; then
    wait "${REPLAY_PID}"
    return 1
  fi
}

launch_c3_ui() {
  local os_ext="linux"
  if [[ "$(uname -s)" == "Darwin" ]]; then
    os_ext="macos"
  fi

  local host_ui="${ROOT_DIR}/selfdrive/ui/ui.${os_ext}"
  if [[ ! -x "${host_ui}" ]]; then
    echo "Missing ${host_ui}. C3 build did not produce the desktop binary." >&2
    return 1
  fi

  "${host_ui}" &
  UI_PIDS+=("$!")
}

launch_python_ui() {
  local big="$1"
  (
    export BIG="${big}"
    exec "${ROOT_DIR}/.venv/bin/python3" "${ROOT_DIR}/selfdrive/ui/ui.py"
  ) &
  UI_PIDS+=("$!")
}

parse_args "$@"

if [[ -n "${LEGACY_UI_SELECTION}" ]]; then
  expand_ui_targets "${LEGACY_UI_SELECTION}"
fi

if [[ "${REPLAY_ONLY}" == "1" ]]; then
  UI_TARGETS=()
else
  dedupe_ui_targets
fi

if [[ ${#REPLAY_ARGS[@]} -eq 0 ]]; then
  usage >&2
  exit 1
fi

if [[ "${REPLAY_ONLY}" != "1" && ${#UI_TARGETS[@]} -eq 0 ]]; then
  echo "Select at least one UI with --c3, --c4, --raybig, or use --replay-only." >&2
  exit 1
fi

prepare_env
trap cleanup EXIT INT TERM

echo "Using OPENPILOT_PREFIX=${OPENPILOT_PREFIX:-<default>}"
echo "Preparing replay and desktop UI runtime..."

build_replay

case " ${UI_TARGETS[*]-} " in
  *" c3 "*)
    prepare_c3_runtime
    ;;
esac

case " ${UI_TARGETS[*]-} " in
  *" c4 "*|*" raybig "*)
    prepare_python_ui_runtime
    ;;
esac

seed_params
seed_starpilot_theme

echo "Starting replay: ${REPLAY_ARGS[*]}"
launch_replay

if [[ ${#UI_TARGETS[@]} -eq 0 ]]; then
  echo "Replay is running without UI windows. Press Ctrl-C to stop."
  wait "${REPLAY_PID}"
  exit 0
fi

echo "Launching UIs: ${UI_TARGETS[*]}"

local_target=""
for local_target in "${UI_TARGETS[@]}"; do
  case "${local_target}" in
    c3)
      launch_c3_ui
      ;;
    c4)
      launch_python_ui 0
      ;;
    raybig)
      launch_python_ui 1
      ;;
  esac
done

wait "${UI_PIDS[@]}"
