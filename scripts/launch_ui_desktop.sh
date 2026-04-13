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

jobs="$(default_jobs)"
if [[ "${1:-}" =~ ^[0-9]+$ ]]; then
  jobs="$1"
  shift || true
fi

OS_EXT="linux"
if [[ "$(uname -s)" == "Darwin" ]]; then
  OS_EXT="macos"
fi
HOST_UI="${ROOT_DIR}/selfdrive/ui/ui.${OS_EXT}"
PRE_TRACKED_DIRTY="$(mktemp /tmp/starpilot_c3_pretracked.XXXXXX)"
POST_TRACKED_DIRTY="$(mktemp /tmp/starpilot_c3_posttracked.XXXXXX)"
BACKUP_DIR="$(mktemp -d /tmp/starpilot_c3_backup.XXXXXX)"
BACKUP_MANIFEST="${BACKUP_DIR}/.artifact_manifest"
FAKE_WIFI_PID=""

runtime_artifacts=(
  "selfdrive/ui/ui"
  "common/libcommon.a"
  "common/transformations/libtransformations.a"
  "cereal/libsocketmaster.a"
  "msgq_repo/libmsgq.a"
  "msgq_repo/libvisionipc.a"
  "common/params_pyx.so"
  "common/transformations/transformations.so"
  "msgq_repo/msgq/ipc_pyx.so"
  "msgq_repo/msgq/visionipc/visionipc_pyx.so"
)

collect_tracked_dirty() {
  if ! command -v git >/dev/null 2>&1; then
    return 1
  fi
  if ! git -C "${ROOT_DIR}" rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    return 1
  fi
  git -C "${ROOT_DIR}" status --porcelain --untracked-files=no | sed -E 's/^.. //'
}

backup_artifact() {
  local rel="$1"
  local src="${ROOT_DIR}/${rel}"
  echo "${rel}" >> "${BACKUP_MANIFEST}"
  if [[ -f "${src}" ]]; then
    mkdir -p "${BACKUP_DIR}/$(dirname "${rel}")"
    cp -f "${src}" "${BACKUP_DIR}/${rel}"
  fi
}

restore_runtime_artifacts() {
  if [[ ! -d "${BACKUP_DIR}" ]]; then
    return
  fi

  if [[ -f "${BACKUP_MANIFEST}" ]]; then
    while IFS= read -r rel; do
      [[ -z "${rel}" ]] && continue
      if [[ -f "${BACKUP_DIR}/${rel}" ]]; then
        cp -f "${BACKUP_DIR}/${rel}" "${ROOT_DIR}/${rel}"
      else
        rm -f "${ROOT_DIR}/${rel}"
      fi
    done < "${BACKUP_MANIFEST}"
  fi

  rm -rf "${BACKUP_DIR}"
}

restore_new_tracked_changes() {
  if ! collect_tracked_dirty > "${POST_TRACKED_DIRTY}"; then
    return
  fi

  while IFS= read -r path; do
    [[ -z "${path}" ]] && continue
    if ! grep -Fxq "${path}" "${PRE_TRACKED_DIRTY}"; then
      git -C "${ROOT_DIR}" checkout -- "${path}" >/dev/null 2>&1 || true
    fi
  done < "${POST_TRACKED_DIRTY}"
}

cleanup() {
  restore_runtime_artifacts
  restore_new_tracked_changes
  stop_fake_wifi
  rm -f "${PRE_TRACKED_DIRTY}" "${POST_TRACKED_DIRTY}"
}

start_fake_wifi() {
  if [[ ! "${SP_C3_FAKE_WIFI:-1}" =~ ^(1|true|yes|on)$ ]]; then
    export SP_ALLOW_DESKTOP_FAKE_WIFI=0
    return
  fi

  export SP_ALLOW_DESKTOP_FAKE_WIFI=1
  "${ROOT_DIR}/.venv/bin/python3" selfdrive/debug/fake_wifi.py --network wifi --strength great --interval 0.2 &
  FAKE_WIFI_PID=$!
}

seed_starpilot_theme() {
  "${ROOT_DIR}/.venv/bin/python3" - <<'PY'
from openpilot.starpilot.common.starpilot_functions import seed_desktop_theme_assets

seed_desktop_theme_assets()
PY
}

stop_fake_wifi() {
  if [[ -n "${FAKE_WIFI_PID}" ]]; then
    kill "${FAKE_WIFI_PID}" >/dev/null 2>&1 || true
    FAKE_WIFI_PID=""
  fi
}

archive_is_aarch64_elf() {
  local path="$1"
  [[ -f "${path}" ]] || return 1
  if command -v readelf >/dev/null 2>&1; then
    readelf -h "${path}" 2>/dev/null | grep -iq "aarch64"
  else
    objdump -a "${path}" 2>/dev/null | head -n 12 | grep -iqE "elf64-littleaarch64|aarch64"
  fi
}

remove_if_elf() {
  local rel="$1"
  local path="${ROOT_DIR}/${rel}"
  if [[ -f "${path}" ]] && file "${path}" | grep -q "ELF"; then
    rm -f "${path}"
  fi
}

purge_objects() {
  local root="$1"
  if [[ -d "${root}" ]]; then
    find "${root}" -type f \( -name '*.o' -o -name '*.os' \) -delete
  fi
}

prepare_host_artifacts() {
  if archive_is_aarch64_elf "${ROOT_DIR}/common/libcommon.a"; then
    rm -f "${ROOT_DIR}/common/libcommon.a" "${ROOT_DIR}/common/params_pyx.o"
    purge_objects "${ROOT_DIR}/common"
  fi

  if archive_is_aarch64_elf "${ROOT_DIR}/cereal/libsocketmaster.a"; then
    rm -f "${ROOT_DIR}/cereal/libsocketmaster.a"
    purge_objects "${ROOT_DIR}/cereal/messaging"
  fi

  if archive_is_aarch64_elf "${ROOT_DIR}/common/transformations/libtransformations.a"; then
    rm -f "${ROOT_DIR}/common/transformations/libtransformations.a" \
      "${ROOT_DIR}/common/transformations/transformations.o"
    purge_objects "${ROOT_DIR}/common/transformations"
  fi

  if archive_is_aarch64_elf "${ROOT_DIR}/msgq_repo/libmsgq.a" || archive_is_aarch64_elf "${ROOT_DIR}/msgq_repo/libvisionipc.a"; then
    rm -f "${ROOT_DIR}/msgq_repo/libmsgq.a" "${ROOT_DIR}/msgq_repo/libvisionipc.a"
    purge_objects "${ROOT_DIR}/msgq_repo/msgq"
    purge_objects "${ROOT_DIR}/msgq_repo/msgq/visionipc"
  fi

  remove_if_elf "common/params_pyx.so"
  remove_if_elf "common/transformations/transformations.so"
  remove_if_elf "msgq_repo/msgq/ipc_pyx.so"
  remove_if_elf "msgq_repo/msgq/visionipc/visionipc_pyx.so"

  purge_objects "${ROOT_DIR}/selfdrive/ui"
  purge_objects "${ROOT_DIR}/starpilot/ui"
  rm -f "${ROOT_DIR}/selfdrive/ui/libqt_widgets.a" "${ROOT_DIR}/selfdrive/ui/libqt_util.a"
  rm -f "${ROOT_DIR}/selfdrive/ui/ui"
}

collect_tracked_dirty > "${PRE_TRACKED_DIRTY}" || true
trap cleanup EXIT

for rel in "${runtime_artifacts[@]}"; do
  backup_artifact "${rel}"
done

source .venv/bin/activate
if [[ -d /opt/homebrew/bin ]]; then
  export PATH="/opt/homebrew/bin:${PATH}"
fi

if [[ "$(uname -s)" == "Darwin" ]]; then
  # Keep desktop host builds on Apple toolchain even if shell exports Homebrew llvm.
  export CC="/usr/bin/clang"
  export CXX="/usr/bin/clang++"
  export AR="/usr/bin/ar"
  export RANLIB="/usr/bin/ranlib"
fi
unset CPATH C_INCLUDE_PATH CPLUS_INCLUDE_PATH CPPFLAGS CFLAGS CXXFLAGS LDFLAGS

prepare_host_artifacts

export PYTHONPATH="${ROOT_DIR}:${ROOT_DIR}/starpilot/third_party"
for d in "${ROOT_DIR}"/*_repo; do [[ -d "$d" ]] && export PYTHONPATH="${PYTHONPATH}:$d"; done
[[ -d "${ROOT_DIR}/third_party/acados" ]] && export PYTHONPATH="${PYTHONPATH}:${ROOT_DIR}/third_party/acados"

export SP_DISABLE_AUTO_DEVICE_SCONS=1
scons -j"${jobs}" selfdrive/ui/ui

cp -f "${ROOT_DIR}/selfdrive/ui/ui" "${HOST_UI}"
cleanup
trap - EXIT

if [[ "${SP_C3_COMPILE_ONLY:-0}" == "1" ]]; then
  echo "C3 UI binary prepared."
  exit 0
fi

seed_starpilot_theme
start_fake_wifi
trap stop_fake_wifi EXIT
"${HOST_UI}" "$@"
rc=$?
stop_fake_wifi
trap - EXIT
exit "${rc}"
