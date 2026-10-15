#!/usr/bin/env bash
set -e
set -x

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null && pwd)"
cd "$DIR"

BUILD_DIR="${BUILD_DIR:-/data/openpilot}"
SOURCE_DIR="$(git rev-parse --show-toplevel)"

# On-device: if BUILD_DIR == SOURCE_DIR, isolate to prevent self-deletion
if [ "$BUILD_DIR" = "$SOURCE_DIR" ]; then
  echo "[-] BUILD_DIR == SOURCE_DIR, isolating to ${BUILD_DIR}_isolated"
  BUILD_DIR="${BUILD_DIR}_isolated"
fi

# The release worktree may have been created by sudo during a previous run.
# Fix ownership before Git accesses worktree metadata or the build directory.
for repo_dir in /data/openpilot /data/openpilot_src; do
  if [ -e "$repo_dir" ]; then
    sudo chown -R comma:comma "$repo_dir"
  fi
done

export PYTHONPATH="$BUILD_DIR:$BUILD_DIR/msgq_repo:$BUILD_DIR/opendbc_repo:$BUILD_DIR/rednose_repo:$BUILD_DIR/teleoprtc_repo:$BUILD_DIR/tinygrad_repo"

# Set up writable home/cache for root (sudo resets HOME to /root which is read-only on comma3)
if [ "$(id -u)" = "0" ]; then
  _comma_home="$(getent passwd comma 2>/dev/null | cut -d: -f6)" || true
  if [ -n "$_comma_home" ]; then
    export HOME="$_comma_home"
    export XDG_CACHE_HOME="${HOME}/.cache"
    export TMPDIR="${HOME}/.tmp"
    mkdir -p "$XDG_CACHE_HOME" "$TMPDIR"
  fi
fi

# Ensure scons is in PATH for sudo (sudo resets PATH, drops /usr/local/venv/bin)
if ! command -v scons &>/dev/null && [ -f /usr/local/venv/bin/scons ]; then
  export PATH="/usr/local/venv/bin:$PATH"
fi

# Check GitHub SSH authentication before replacing the release worktree.
mkdir -p "$HOME/.ssh"
sudo chown -R comma:comma "$HOME/.ssh"
chmod 700 "$HOME/.ssh"
cat > "$HOME/.ssh/config" <<'SSH_CONFIG'
Host github.com
  HostName ssh.github.com
  Port 443
  User git
  UserKnownHostsFile /data/ssh/known_hosts
  StrictHostKeyChecking no
  IdentityFile /data/ssh/id_ed25519
SSH_CONFIG
chown comma:comma "$HOME/.ssh/config"
chmod 600 "$HOME/.ssh/config"

env | grep -i proxy || true
cat /etc/environment 2>/dev/null | grep -i proxy || true
cat "$HOME/.profile" 2>/dev/null | grep -i proxy || true
ssh -vT -o HostKeyAlias=github.com -p 443 git@ssh.github.com 2>&1 || true
unset http_proxy https_proxy HTTP_PROXY HTTPS_PROXY all_proxy ALL_PROXY

# Run the same command as the manual verification as comma. The release script
# itself runs as root, and changing HOME alone does not change SSH's user context.
ssh_auth_output="$(sudo -u comma -H env -u http_proxy -u https_proxy -u HTTP_PROXY -u HTTPS_PROXY -u all_proxy -u ALL_PROXY ssh -T git@github.com 2>&1 || true)"
printf '%s\n' "$ssh_auth_output"
if ! printf '%s\n' "$ssh_auth_output" | grep -Fq "You've successfully authenticated"; then
  echo "[!] GitHub SSH authentication failed; aborting release build" >&2
  exit 1
fi

BUILD_BRANCH="XL-jetlink-tici"


# set git identity
source "$DIR/identity.sh"

echo "[-] Setting up repo T=$SECONDS"
if ! git -C "$SOURCE_DIR" worktree remove --force "$BUILD_DIR" 2>/dev/null; then
  rm -rf "$BUILD_DIR"
fi
git -C "$SOURCE_DIR" worktree prune
git -C "$SOURCE_DIR" worktree add --detach --no-checkout "$BUILD_DIR"
cd "$BUILD_DIR"
git update-ref -d "refs/heads/$BUILD_BRANCH"
git symbolic-ref HEAD "refs/heads/$BUILD_BRANCH"
git read-tree --empty

# do the files copy
echo "[-] copying files T=$SECONDS"
cd "$SOURCE_DIR"
./tools/release/release_files.py | xargs -0 cp -pR --parents -t "$BUILD_DIR" --

# in the directory
cd "$BUILD_DIR"

# use the full CPU available for speeding up the build.
# openpilot resets the CPU frequencies when test_onroad.py runs below.
for policy in /sys/devices/system/cpu/cpufreq/policy*; do
  [ -d "$policy" ] || continue
  hardware_max="$(cat "$policy/cpuinfo_max_freq")"
  echo "$hardware_max" | sudo tee "$policy/scaling_max_freq" >/dev/null || true
done

# Inject HOME/XDG_CACHE_HOME into SConstruct's ENV for tinygrad cache (root/sudo)
# SConstruct 用硬编码的 ENV={...} 跑所有 scons 子命令，tinygrad 收不到 $HOME → 掉到 /root → 只读 FS 崩溃
if [ -f SConstruct ]; then
  cp SConstruct .SConstruct.bak
  sed -i 's|"PYTHONPATH": os\.pathsep\.join(submodule_python_paths),|"PYTHONPATH": os.pathsep.join(submodule_python_paths),\n    "HOME": os.environ.get("HOME", "/home/comma"),\n    "XDG_CACHE_HOME": os.environ.get("XDG_CACHE_HOME", os.environ.get("HOME", "/home/comma") + "/.cache"),|' SConstruct
fi

# taskset -c N fails when target CPU is offline (AGNOS hotplugs big cores when idle).
# Provide a tolerant wrapper that drops taskset args and runs the command directly.
mkdir -p /tmp/fakebin
cat > /tmp/fakebin/taskset <<'TASKSET_WRAPPER'
#!/bin/bash
while [ $# -gt 0 ]; do
  case "$1" in
    -c) shift 2 ;;
    -p) shift 2 ;;
    *)  break ;;
  esac
done
exec "$@"
TASKSET_WRAPPER
chmod +x /tmp/fakebin/taskset
export PATH="/tmp/fakebin:$PATH"

scons

if [ -z "$PANDA_DEBUG_BUILD" ]; then
  # release panda fw
  CERT=/data/pandaextra/certs/release RELEASE=1 scons panda/
else
  # build with ALLOW_DEBUG=1 to enable features like experimental longitudinal
  scons panda/
fi

# Restore SConstruct after build (undo HOME/XDG_CACHE_HOME injection, avoid committing source changes)
if [ -f .SConstruct.bak ]; then
  cp .SConstruct.bak SConstruct
  rm -f .SConstruct.bak
fi

find openpilot/selfdrive/modeld/models -name '*.pkl' -size +95M -exec ./openpilot/common/file_chunker.py {} \;
if [ -n "$INCLUDE_BIG_MODEL" ]; then
  test -f openpilot/selfdrive/modeld/models/big_driving_tinygrad.pkl.chunkmanifest
fi

# Ensure no submodules in release
if test "$(git submodule--helper list | wc -l)" -gt "0"; then
  echo "submodules found:"
  git submodule--helper list
  exit 1
fi
git submodule status

# Cleanup
find . -name '*.a' -delete
find . -name '*.o' -delete
find . -name '*.os' -delete
find . -name '*.pyc' -delete
find . -name '__pycache__' -delete
rm -rf .sconsign.dblite Jenkinsfile tools/release/
rm -f openpilot/selfdrive/modeld/models/*.onnx*
rm -f openpilot/sunnypilot/modeld*/models/*.onnx*

find openpilot/third_party/ -name '*x86*' -exec rm -r {} +
find openpilot/third_party/ -name '*Darwin*' -exec rm -r {} +


# Restore third_party from the source repository. The release worktree has an
# intentionally empty index, so checkout cannot resolve this path there.
git -C "$SOURCE_DIR" archive HEAD openpilot/third_party/ | tar -x -C "$BUILD_DIR"

# Mark as prebuilt release
touch prebuilt

VERSION=$(cat openpilot/sunnypilot/common/version.h | awk -F[\"-]  '{print $2}')
# Add built files to git
# writing larger objects is faster than compressing them on-device
git -c core.compression=0 add -f .
git -c core.compression=0 -c gc.auto=0 commit -m "openpilot v$VERSION"

# Leave the generated worktree usable by the comma user if a later test or push
# fails and manual recovery is needed.
for repo_dir in /data/openpilot /data/openpilot_src; do
  if [ -e "$repo_dir" ]; then
    sudo chown -R comma:comma "$repo_dir"
  fi
done

# Run tests (SKIP_TEST_ONROAD=1 to skip on-car test)
cd "$BUILD_DIR"
if [ -z "$SKIP_TEST_ONROAD" ]; then
  RELEASE=1 ./openpilot/selfdrive/test/test_onroad.py "$@"
fi
#tools/test_runner.py openpilot/selfdrive/car/tests/test_car_interfaces.py

echo "[-] pushing release T=$SECONDS"
# uploading the larger pack is faster than spending CPU to optimize it
# GitHub SSH on port 22 is intercepted by the device network. Use the SSH-over-
# HTTPS endpoint explicitly; do not depend on root's ~/.ssh/config.
if [ -z "$GIT_SSH_COMMAND" ]; then
  GIT_SSH_COMMAND="ssh -p 443 -o HostName=ssh.github.com -o ServerAliveInterval=60 -o ServerAliveCountMax=3"
  if [ -f /data/ssh/id_ed25519 ]; then
    GIT_SSH_COMMAND="$GIT_SSH_COMMAND -i /data/ssh/id_ed25519"
  fi
  export GIT_SSH_COMMAND
fi
git -c safe.directory="$BUILD_DIR" \
    -c pack.window=0 -c pack.depth=0 -c pack.compression=0 \
    push -f origin "$BUILD_BRANCH:$BUILD_BRANCH"

echo "[-] done T=$SECONDS"
