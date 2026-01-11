# macOS Accessibility Permissions Fix

## Problem
On macOS 10.15 (Catalina) and later, applications need to be properly bundled as `.app` bundles to appear in:
- System Settings → Privacy & Security → Accessibility
- System Settings → Privacy & Security → Input Monitoring

Simply embedding an `Info.plist` into a standalone binary using linker flags is no longer sufficient.

### Specific Issue Fixed
When running `warpd -f` (foreground mode) via a symlink at `/usr/local/bin/warpd`, macOS TCC (Transparency, Consent, and Control) fails to properly associate the running process with the app bundle at `/Applications/warpd.app`. This causes:
1. The accessibility permission prompt to appear correctly
2. **But** the app fails to show up in the System Settings → Privacy & Security → Accessibility list

**Root Cause**: When a binary is executed via a symlink, `[NSBundle mainBundle]` cannot properly identify the enclosing app bundle. macOS TCC requires the bundle to be correctly identified to register the app in the Privacy & Security settings. Running via symlink causes the system to track the app by binary path instead of bundle identifier, which prevents it from appearing in the UI.

This issue was resolved by:
- Replacing the symlink with a wrapper shell script that executes the binary from its canonical path within the bundle
- Signing the entire app bundle (not just the binary)
- Explicitly activating the app when running in foreground mode
- Verifying the bundle identifier at startup

## Solution
The build system has been updated to create a proper macOS application bundle structure:

```
warpd.app/
├── Contents/
    ├── Info.plist
    ├── MacOS/
    │   └── warpd (binary)
    └── Resources/
```

## Changes Made

1. **Updated `mk/macos.mk`**:
   - Build process now creates `bin/warpd.app` with proper bundle structure
   - Binary is placed in `warpd.app/Contents/MacOS/`
   - `Info.plist` is copied to `warpd.app/Contents/`
   - **The entire app bundle is now code-signed** (not just the binary) for proper TCC recognition
   - **Wrapper script created at `bin/warpd` for local testing** that properly executes the binary from within the bundle
   - Install target copies the `.app` bundle to `/Applications/`
   - **Wrapper script installed at `/usr/local/bin/warpd`** (instead of symlink) to preserve bundle context for TCC

2. **Created `files/warpd-wrapper.sh`**:
   - Shell wrapper script that executes the binary from its canonical bundle path
   - Ensures `[NSBundle mainBundle]` correctly identifies the bundle for TCC recognition
   - Passes all command-line arguments through to the binary

3. **Updated `files/com.warpd.warpd.plist`**:
   - LaunchAgent now points to `/Applications/warpd.app/Contents/MacOS/warpd`

4. **Updated `codesign/sign.sh`**:
   - Made more robust to handle signing errors gracefully
   - Accepts binary or bundle path as argument
   - **Now signs the entire app bundle for proper TCC database registration**
   - Falls back to ad-hoc signing if certificate not available

5. **Updated `src/platform/macos/macos.m`**:
   - Added bundle identifier verification at startup
   - **When running in foreground mode (`-f`), explicitly activates the app** to ensure TCC visibility
   - Logs bundle identifier for debugging purposes

6. **Updated `README.md`**:
   - Updated installation and uninstallation instructions
   - Added note about the app bundle location
   - Clarified permission requirements

## Installation

After building, the app will be installed to `/Applications/warpd.app`. The LaunchAgent will automatically start the application on login, and macOS will prompt for accessibility permissions on first run.

## Testing

To test that the fix works:

1. Build and install warpd: `make && sudo make install`
2. Load the LaunchAgent: `launchctl load /Library/LaunchAgents/com.warpd.warpd.plist`
3. Open System Settings → Privacy & Security → Accessibility
4. Look for "warpd" in the list of applications
5. Enable the toggle if not already enabled

## Upgrading from Previous Versions

If upgrading from a version that installed warpd as a standalone binary:

1. Unload the old LaunchAgent: `launchctl unload /Library/LaunchAgents/com.warpd.warpd.plist`
2. Reset accessibility permissions (optional but recommended): `sudo tccutil reset Accessibility`
3. Remove old binary: `sudo rm /usr/local/bin/warpd`
4. Install new version: `make && sudo make install`
5. Load new LaunchAgent: `launchctl load /Library/LaunchAgents/com.warpd.warpd.plist`
6. Grant accessibility permissions when prompted
