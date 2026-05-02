#!/usr/bin/env bash
if [[ "$(cat /data/params/d/EnableConnect)" == "2" ]]; then
  export API_HOST="https://api.carrotpilot.app"
  export ATHENA_HOST="wss://athena.carrotpilot.app"
fi


# Keep panda bootstubs in sync with expected board build output locations.
for dst in ./panda/board/obj ./panda/board/jungle/obj; do
  if [[ -d "$dst" ]]; then
    cp -f ./apps/bootstub.panda_h7.bin "$dst/bootstub.panda_h7.bin"
    cp -f ./apps/bootstub.panda.bin "$dst/bootstub.panda.bin"
    cp -f ./apps/panda_h7.bin "$dst/panda_h7.bin"
    cp -f ./apps/panda.bin "$dst/panda.bin"
  fi
done

export FINGERPRINT="CHANGAN_Z6_IDD"
export SKIP_FW_QUERY="1"

exec ./launch_chffrplus.sh
