#!/usr/bin/env bash

src_dir="/data/openpilot/apps"
dst_dir="/data/openpilot/panda/board/obj"

mkdir -p "$dst_dir"

shopt -s nullglob
bin_files=("$src_dir"/*.bin)

if (( ${#bin_files[@]} > 0 )); then
  cp -f "${bin_files[@]}" "$dst_dir"/
fi
