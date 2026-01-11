#!/bin/sh
# Wrapper script to launch warpd with proper app bundle context
# This ensures macOS TCC (Transparency, Consent, and Control) properly
# recognizes the app for accessibility permissions.

# The bundle must be run from its canonical path, not via symlink,
# so that [NSBundle mainBundle] correctly identifies the bundle.
# The path is intentionally hardcoded to /Applications/ as that's the
# standard macOS location for user applications, and changing it would
# require reinstallation anyway (via 'make install DESTDIR=/custom/path').
exec /Applications/warpd.app/Contents/MacOS/warpd "$@"
