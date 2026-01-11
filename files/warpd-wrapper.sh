#!/bin/sh
# Wrapper script to launch warpd with proper app bundle context
# This ensures macOS TCC (Transparency, Consent, and Control) properly
# recognizes the app for accessibility permissions.

# The bundle must be run from its canonical path, not via symlink,
# so that [NSBundle mainBundle] correctly identifies the bundle.
exec /Applications/warpd.app/Contents/MacOS/warpd "$@"
