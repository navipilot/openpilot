#!/usr/bin/env bash
set -e

SUDO=""

# Use sudo if not root
if [[ ! $(id -u) -eq 0 ]]; then
  if [[ -z $(which sudo) ]]; then
    echo "Please install sudo or run as root"
    exit 1
  fi
  SUDO="sudo"
fi

# Check if stdin is open
if [ -t 0 ]; then
  INTERACTIVE=1
fi

# Install common packages
function install_ubuntu_common_requirements() {
  $SUDO apt-get update

  # normal stuff, mostly for the bare docker image
  $SUDO apt-get install -y --no-install-recommends \
    ca-certificates \
    clang \
    build-essential \
    curl \
    libssl-dev \
    libcurl4-openssl-dev \
    locales \
    git \
    git-lfs \
    xvfb

  # TODO: vendor the rest of these in third_party/
  $SUDO apt-get install -y --no-install-recommends \
    gcc-arm-none-eabi \
    capnproto \
    libcapnp-dev \
    ffmpeg \
    libavformat-dev \
    libavcodec-dev \
    libavdevice-dev \
    libavutil-dev \
    libavfilter-dev \
    libbz2-dev \
    libeigen3-dev \
    libffi-dev \
    libgles2-mesa-dev \
    libglfw3-dev \
    libglib2.0-0 \
    libjpeg-dev \
    libqt5charts5-dev \
    libncurses5-dev \
    libusb-1.0-0-dev \
    libzmq3-dev \
    libzstd-dev \
    libsqlite3-dev \
    opencl-headers \
    ocl-icd-libopencl1 \
    ocl-icd-opencl-dev \
    portaudio19-dev \
    qttools5-dev-tools \
    libqt5svg5-dev \
    libqt5x11extras5-dev \
    libqt5opengl5-dev \
    gettext
}

# Newer Ubuntu releases (>= 25.x) dropped Qt5SerialBus from repos while keeping
# the rest of Qt5 5.15.x. The noble (24.04) debs are ABI-compatible; the only
# blocker is a virtual-package version guard, which --force-depends safely skips.
function ensure_qt5serialbus() {
  if dpkg -s libqt5serialbus5-dev >/dev/null 2>&1; then
    return 0
  fi
  if apt-cache show libqt5serialbus5-dev >/dev/null 2>&1; then
    $SUDO apt-get install -y --no-install-recommends libqt5serialbus5-dev
    return 0
  fi

  echo "libqt5serialbus5-dev missing from repos — installing noble (24.04) debs..."
  local tmpdir=$(mktemp -d)
  local base="http://archive.ubuntu.com/ubuntu/pool/universe/q/qtserialbus-everywhere-src"
  local ver="5.15.13-1"
  for pkg in libqt5serialbus5 libqt5serialbus5-dev libqt5serialbus5-plugins; do
    wget -q -P "$tmpdir" "${base}/${pkg}_${ver}_amd64.deb"
  done
  $SUDO dpkg --force-depends -i "$tmpdir"/*.deb
  rm -rf "$tmpdir"
}

# Install Ubuntu 24.04+ packages
function install_ubuntu_lts_latest_requirements() {
  install_ubuntu_common_requirements

  $SUDO apt-get install -y --no-install-recommends \
    g++-12 \
    qtbase5-dev \
    qtbase5-dev-tools \
    python3-dev \
    python3-venv

  ensure_qt5serialbus
}

# Detect OS using /etc/os-release file
if [ -f "/etc/os-release" ]; then
  source /etc/os-release
  case "$VERSION_CODENAME" in
    "jammy" | "kinetic" | "noble" | "questing")
      install_ubuntu_lts_latest_requirements
      ;;
    *)
      echo "$ID $VERSION_ID is unsupported. This setup script is written for Ubuntu 24.04."
      read -p "Would you like to attempt installation anyway? " -n 1 -r
      echo ""
      if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        exit 1
      fi
      install_ubuntu_lts_latest_requirements
  esac

  if [[ -d "/etc/udev/rules.d/" ]]; then
    # Setup jungle udev rules
    $SUDO tee /etc/udev/rules.d/12-panda_jungle.rules > /dev/null <<EOF
SUBSYSTEM=="usb", ATTRS{idVendor}=="3801", ATTRS{idProduct}=="ddcf", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="3801", ATTRS{idProduct}=="ddef", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="bbaa", ATTRS{idProduct}=="ddcf", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="bbaa", ATTRS{idProduct}=="ddef", MODE="0666"

EOF

    # Setup panda udev rules
    $SUDO tee /etc/udev/rules.d/11-panda.rules > /dev/null <<EOF
SUBSYSTEM=="usb", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="df11", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="3801", ATTRS{idProduct}=="ddcc", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="3801", ATTRS{idProduct}=="ddee", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="bbaa", ATTRS{idProduct}=="ddcc", MODE="0666"
SUBSYSTEM=="usb", ATTRS{idVendor}=="bbaa", ATTRS{idProduct}=="ddee", MODE="0666"
EOF

    # Setup adb udev rules
    $SUDO tee /etc/udev/rules.d/50-comma-adb.rules > /dev/null <<EOF
SUBSYSTEM=="usb", ATTR{idVendor}=="04d8", ATTR{idProduct}=="1234", ENV{adb_user}="yes"
EOF

    $SUDO udevadm control --reload-rules && $SUDO udevadm trigger || true
  fi

else
  echo "No /etc/os-release in the system. Make sure you're running on Ubuntu, or similar."
  exit 1
fi
