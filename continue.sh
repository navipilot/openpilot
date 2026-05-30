#!/usr/bin/env bash

# 1. Start CAN bridge in the background
if ! pgrep -f "bridge can" >/dev/null; then
    (
        sleep 15
        cd /data/openpilot/cereal/messaging
        nohup ./bridge can > /dev/null 2>&1 &
    ) &
fi

# 2. Wi-Fi Connection Function (Backgrounded)
connect_wifi() {
    SSID="HotSpot SSID"
    PASSWORD="HotSpot Password"
    
    ip link set wlan0 up
    
    # Infinite loop that runs in the background
    while true; do
        # Check if already connected to the correct SSID
        CURRENT_SSID=$(nmcli -t -f active,ssid dev wifi | grep '^yes' | cut -d: -f2)
        
        if [ "$CURRENT_SSID" != "$SSID" ]; then
            # Attempt connection. Note: double quotes are vital for the apostrophe in the SSID
            nmcli device wifi connect "$SSID" password "$PASSWORD" ifname wlan0 > /dev/null 2>&1
        fi
        
        # Check connection status every 30 seconds to keep it alive
        sleep 30
    done
}

# Run the Wi-Fi function in the background so boot doesn't hang
connect_wifi &

# 3. Launch Openpilot
cd /data/openpilot
exec ./launch_openpilot.sh
