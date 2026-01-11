#!/bin/bash

PLIST="/Library/LaunchAgents/com.warpd.warpd.plist"

# 존재 여부 확인 (선택사항이지만 추천)
if [ ! -f "$PLIST" ]; then
  echo "Error: plist file not found → $PLIST"
  exit 1
fi

echo "Reloading warpd..."

# unload 실패해도 무시 (이미 안 올라가 있는 경우 등)
launchctl unload "$PLIST" 2>/dev/null || true

# 실제 reload
launchctl load "$PLIST"

if launchctl list | grep -q "com.warpd.warpd"; then
  echo "→ Successfully reloaded"
else
  echo "→ Load failed (check logs or permissions)"
  exit 1
fi
