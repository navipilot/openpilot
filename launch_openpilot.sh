#!/usr/bin/env bash
if [[ "$(cat /data/params/d/EnableConnect)" == "2" ]]; then
  export API_HOST="https://api.carrotpilot.app"
  export ATHENA_HOST="wss://athena.carrotpilot.app"
fi


export FINGERPRINT="CHANGAN_Z6_IDD"
export SKIP_FW_QUERY="1"

bash ./copy.sh
exec ./launch_chffrplus.sh
