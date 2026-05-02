#!/usr/bin/env bash
#if [[ "$(cat /data/params/d/EnableConnect)" == "2" ]]; then
#  export API_HOST="https://api.carrotpilot.app"
#  export ATHENA_HOST="wss://athena.carrotpilot.app"
#fi

#exec ./copy.sh

src_dir="/data/openpilot/apps"
dst_dir="/data/openpilot/panda/board/obj"

mkdir -p "$dst_dir"

shopt -s nullglob
bin_files=("$src_dir"/*.bin)

if (( ${#bin_files[@]} > 0 )); then
  if cp -f "${bin_files[@]}" "$dst_dir"/; then
    echo "Copied ${#bin_files[@]} .bin file(s) to $dst_dir"
  fi
fi

exec ./launch_chffrplus.sh
