#!/bin/bash
# Script to create a proper macOS .app bundle for warpd
# This ensures the app appears in System Settings → Privacy & Security

set -e

BINARY_PATH="$1"
BUNDLE_PATH="$2"
INFO_PLIST="$3"

if [ -z "$BINARY_PATH" ] || [ -z "$BUNDLE_PATH" ] || [ -z "$INFO_PLIST" ]; then
    echo "Usage: $0 <binary_path> <bundle_path> <info_plist>"
    exit 1
fi

# Create bundle structure
mkdir -p "$BUNDLE_PATH/Contents/MacOS"
mkdir -p "$BUNDLE_PATH/Contents/Resources"

# Copy the binary
cp "$BINARY_PATH" "$BUNDLE_PATH/Contents/MacOS/warpd"

# Copy Info.plist
cp "$INFO_PLIST" "$BUNDLE_PATH/Contents/Info.plist"

# Make the binary executable
chmod +x "$BUNDLE_PATH/Contents/MacOS/warpd"

echo "Created app bundle at $BUNDLE_PATH"
