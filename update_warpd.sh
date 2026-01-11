#!/bin/bash

# Usage check
if [ $# -ne 1 ]; then
  echo "Usage: $0 <warped source directory path>"
  exit 1
fi

TARGET_DIR="$1"

# Check if directory exists
if [ ! -d "$TARGET_DIR" ]; then
  echo "Error: Directory does not exist → $TARGET_DIR"
  exit 1
fi

cd "$TARGET_DIR" || exit 1

echo "Starting warpd reinstall..."

# Important! This asks for password only once (cached for 5 minutes usually)
sudo -v || {
  echo "sudo authentication failed"
  exit 1
}

echo "→ Changed to directory: $(pwd)"

# From here, most sudo commands won't ask for password again
launchctl unload /Library/LaunchAgents/com.warpd.warpd.plist 2>/dev/null || true

echo "→ Unloaded existing launch agent"

sudo make uninstall && echo "→ make uninstall completed"

make clean && echo "→ make clean completed" # Usually safer to run

make && echo "→ Build completed"

sudo make install && echo "→ make install completed"

launchctl load /Library/LaunchAgents/com.warpd.warpd.plist && echo "→ Relaunched launch agent"

echo ""
echo "warpd has been successfully reinstalled! 🎉"
