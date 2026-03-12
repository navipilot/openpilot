FROM ghcr.io/commaai/openpilot-base-aarch64:latest

ENV PYTHONUNBUFFERED=1
ENV UV_LINK_MODE=copy
WORKDIR /work

# Keep this image minimal; dependency resolution is done by uv inside the workspace.
# rsync/ssh-client are needed for optional sysroot sync from a comma device.
RUN apt-get update && apt-get install -y --no-install-recommends \
    rsync \
    openssh-client \
    e2fsprogs \
    android-sdk-libsparse-utils \
    qemu-user-static \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    qtbase5-private-dev \
    python3-pip \
    xz-utils \
    git \
  && rm -rf /var/lib/apt/lists/*

RUN /usr/bin/python3 -m pip install --no-cache-dir --break-system-packages uv

# Fix for "dubious ownership" git errors when mounting folders on Linux hosts.
RUN git config --global --add safe.directory /work
