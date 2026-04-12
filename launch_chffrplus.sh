#!/usr/bin/env bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"

source "$DIR/launch_env.sh"

function agnos_init {
  # TODO: move this to agnos
  sudo rm -f /data/etc/NetworkManager/system-connections/*.nmmeta

  # set success flag for current boot slot
  sudo abctl --set_success

  # Restore SSH access after a user-triggered reset if keys were backed up to /cache.
  SSH_BACKUP_DIR="/cache/reset_backup"
  if [ -d "$SSH_BACKUP_DIR" ]; then
    sudo mkdir -p /data/params/d
    for key in GithubSshKeys SshEnabled; do
      if [ -f "$SSH_BACKUP_DIR/$key" ]; then
        sudo cp "$SSH_BACKUP_DIR/$key" "/data/params/d/$key"
      fi
    done
    sudo chown comma:comma /data/params/d/GithubSshKeys /data/params/d/SshEnabled 2>/dev/null || true
    sudo chmod 600 /data/params/d/GithubSshKeys /data/params/d/SshEnabled 2>/dev/null || true
    sudo rm -rf "$SSH_BACKUP_DIR"
  fi

  # TODO: do this without udev in AGNOS
  # udev does this, but sometimes we startup faster
  sudo chgrp gpu /dev/adsprpc-smd /dev/ion /dev/kgsl-3d0
  sudo chmod 660 /dev/adsprpc-smd /dev/ion /dev/kgsl-3d0

  # StarPilot variables
  sudo chmod 0777 /cache

  # Weston loads display color correction from /data/misc/display/color_cal/color_cal.
  # Prefer a factory /persist/comma/color_cal blob when present. Otherwise, derive a
  # Weston-compatible calibration blob from the device's legacy dwo gamma tables.
  COLOR_CAL_SRC="/persist/comma/color_cal"
  DWO_GAMMA_SRC="/persist/comma/dwo_gamma_curves"
  COLOR_CAL_DST_DIR="/data/misc/display/color_cal"
  COLOR_CAL_DST="${COLOR_CAL_DST_DIR}/color_cal"
  COLOR_CAL_HASH_PATH="${COLOR_CAL_DST_DIR}/source.sha256"
  DWO_REFERENCE="$DIR/tools/reference_dwo_gamma_curves.txt"
  DWO_GENERATOR="$DIR/tools/generate_color_cal_from_dwo.py"
  COLOR_CAL_UPDATED=0
  COLOR_CAL_TMP=""
  DWO_SOURCE_HASH=""

  if [ -f "$COLOR_CAL_SRC" ]; then
    sudo mkdir -p "$COLOR_CAL_DST_DIR"
    if [ ! -f "$COLOR_CAL_DST" ] || ! cmp -s "$COLOR_CAL_SRC" "$COLOR_CAL_DST"; then
      sudo cp "$COLOR_CAL_SRC" "$COLOR_CAL_DST"
      sudo chown -R comma:comma "$COLOR_CAL_DST_DIR"
      sudo chmod 664 "$COLOR_CAL_DST"
      sudo rm -f "$COLOR_CAL_HASH_PATH"
      COLOR_CAL_UPDATED=1
    fi
  elif [ -f "$DWO_GAMMA_SRC" ] && [ -f "$DWO_REFERENCE" ] && [ -f "$DWO_GENERATOR" ]; then
    DWO_SOURCE_HASH="$(cat "$DWO_GAMMA_SRC" "$DWO_REFERENCE" "$DWO_GENERATOR" | sha256sum | awk '{print $1}')"
    if [ ! -f "$COLOR_CAL_DST" ] || [ ! -f "$COLOR_CAL_HASH_PATH" ] || [ "$(cat "$COLOR_CAL_HASH_PATH" 2>/dev/null)" != "$DWO_SOURCE_HASH" ]; then
      COLOR_CAL_TMP="$(mktemp)"
      if python3 "$DWO_GENERATOR" --reference "$DWO_REFERENCE" --input "$DWO_GAMMA_SRC" --output "$COLOR_CAL_TMP"; then
        sudo mkdir -p "$COLOR_CAL_DST_DIR"
        if [ ! -f "$COLOR_CAL_DST" ] || ! cmp -s "$COLOR_CAL_TMP" "$COLOR_CAL_DST"; then
          sudo cp "$COLOR_CAL_TMP" "$COLOR_CAL_DST"
          COLOR_CAL_UPDATED=1
        fi
        printf '%s' "$DWO_SOURCE_HASH" | sudo tee "$COLOR_CAL_HASH_PATH" >/dev/null
        sudo chown -R comma:comma "$COLOR_CAL_DST_DIR"
        sudo chmod 664 "$COLOR_CAL_DST" "$COLOR_CAL_HASH_PATH"
      fi
      rm -f "$COLOR_CAL_TMP"
    fi
  fi

  if [ "$COLOR_CAL_UPDATED" = "1" ] && systemctl is-active --quiet weston.service; then
    sudo systemctl restart weston.service
  fi

  # Check if AGNOS update is required
  if [ $(< /VERSION) != "$AGNOS_VERSION" ]; then
    AGNOS_PY="$DIR/system/hardware/tici/agnos.py"
    MANIFEST="$DIR/system/hardware/tici/agnos.json"
    if $AGNOS_PY --verify $MANIFEST; then
      sudo reboot
    fi
    $DIR/system/hardware/tici/updater $AGNOS_PY $MANIFEST
  fi
}

function launch {
  # Remove orphaned git lock if it exists on boot
  [ -f "$DIR/.git/index.lock" ] && rm -f $DIR/.git/index.lock

  # Check to see if there's a valid overlay-based update available. Conditions
  # are as follows:
  #
  # 1. The DIR init file has to exist, with a newer modtime than anything in
  #    the DIR Git repo. This checks for local development work or the user
  #    switching branches/forks, which should not be overwritten.
  # 2. The FINALIZED consistent file has to exist, indicating there's an update
  #    that completed successfully and synced to disk.

  if [ -f "${DIR}/.overlay_init" ]; then
    find ${DIR}/.git -newer ${DIR}/.overlay_init | grep -q '.' 2> /dev/null
    if [ $? -eq 0 ]; then
      echo "${DIR} has been modified, skipping overlay update installation"
    else
      if [ -f "${STAGING_ROOT}/finalized/.overlay_consistent" ]; then
        if [ ! -d /data/safe_staging/old_openpilot ]; then
          echo "Valid overlay update found, installing"
          LAUNCHER_LOCATION="${BASH_SOURCE[0]}"

          mv $DIR /data/safe_staging/old_openpilot
          mv "${STAGING_ROOT}/finalized" $DIR
          cd $DIR

          echo "Restarting launch script ${LAUNCHER_LOCATION}"
          unset AGNOS_VERSION
          exec "${LAUNCHER_LOCATION}"
        else
          echo "openpilot backup found, not updating"
          # TODO: restore backup? This means the updater didn't start after swapping
        fi
      fi
    fi
  fi

  # handle pythonpath
  ln -sfn $(pwd) /data/pythonpath
  export BASEDIR="$DIR"
  export PYTHONPATH="$DIR/starpilot/third_party:$PWD"

  # hardware specific init
  if [ -f /AGNOS ]; then
    agnos_init
  fi

  # write tmux scrollback to a file
  tmux capture-pane -pq -S-1000 > /tmp/launch_log

  # start manager
  cd system/manager

  if ! python3 ./launch_param_migrations.py; then
    echo "Launch param migrations failed; continuing boot."
  fi

  # Bootstrap runtime (e.g. /usr/comma after reset/uninstall) must go straight
  # to manager/setup flow. Do not run StarPilot prebuilt checks/builds here.
  if [ "$DIR" = "/usr/comma" ] || [ ! -d "$DIR/.git" ]; then
    ./manager.py
    while true; do sleep 1; done
  fi

  function prebuilt_runtime_compatible {
    python3 - <<'PY'
import importlib
from pathlib import Path
import sys

mods = [
  "openpilot.common.params_pyx",
  "msgq.ipc_pyx",
  "msgq.visionipc.visionipc_pyx",
  "openpilot.common.transformations.transformations",
  "openpilot.selfdrive.modeld.models.commonmodel_pyx",
  "openpilot.selfdrive.pandad.pandad_api_impl",
  "openpilot.selfdrive.controls.lib.lateral_mpc_lib.c_generated_code.acados_ocp_solver_pyx",
  "openpilot.selfdrive.controls.lib.longitudinal_mpc_lib.c_generated_code.acados_ocp_solver_pyx",
]

for mod in mods:
  try:
    importlib.import_module(mod)
  except Exception as e:
    print(f"Prebuilt compatibility failure in {mod}: {e}", file=sys.stderr)
    raise

repo_root = Path.cwd().parents[1]
required_files = [
  repo_root / "selfdrive/modeld/models/driving_vision_metadata.pkl",
  repo_root / "selfdrive/modeld/models/driving_policy_metadata.pkl",
  repo_root / "selfdrive/modeld/models/driving_vision_tinygrad.pkl",
  repo_root / "selfdrive/modeld/models/driving_policy_tinygrad.pkl",
  repo_root / "selfdrive/modeld/models/dmonitoring_model_metadata.pkl",
  repo_root / "selfdrive/modeld/models/dmonitoring_model_tinygrad.pkl",
  repo_root / "selfdrive/pandad/pandad_api_impl.so",
  repo_root / "selfdrive/controls/lib/lateral_mpc_lib/c_generated_code/acados_ocp_solver_pyx.so",
  repo_root / "selfdrive/controls/lib/lateral_mpc_lib/c_generated_code/libacados_ocp_solver_lat.so",
  repo_root / "selfdrive/controls/lib/longitudinal_mpc_lib/c_generated_code/acados_ocp_solver_pyx.so",
  repo_root / "selfdrive/controls/lib/longitudinal_mpc_lib/c_generated_code/libacados_ocp_solver_long.so",
  repo_root / "opendbc_repo/opendbc/dbc/gm_global_a_powertrain_generated.dbc",
]

for path in required_files:
  if not path.is_file():
    raise FileNotFoundError(f"Missing prebuilt runtime artifact: {path}")
PY
  }

  USE_PREBUILT=1
  if [ -f /data/params/d/UsePrebuilt ]; then
    USE_PREBUILT=$(tr -d '\n' < /data/params/d/UsePrebuilt)
  fi

  if [ "$USE_PREBUILT" = "1" ] && [ -f $DIR/prebuilt ] && ! prebuilt_runtime_compatible; then
    echo "Prebuilt runtime artifacts are incompatible on this device; rebuilding locally."
    USE_PREBUILT=0
  fi

  if [ "$USE_PREBUILT" != "1" ] || [ ! -f $DIR/prebuilt ]; then
    ./build.py
  fi
  ./manager.py

  # if broken, keep on screen error
  while true; do sleep 1; done
}

launch
