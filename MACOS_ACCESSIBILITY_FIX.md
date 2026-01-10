# macOS Accessibility Permissions Fix

## Problem
On macOS 10.15 (Catalina) and later, applications need to be properly bundled as `.app` bundles to appear in:
- System Settings → Privacy & Security → Accessibility
- System Settings → Privacy & Security → Input Monitoring

Simply embedding an `Info.plist` into a standalone binary using linker flags is no longer sufficient.

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
   - Symlink created at `bin/warpd` for convenience
   - Install target copies the `.app` bundle to `/Applications/`
   - Symlink created at `/usr/local/bin/warpd` pointing to the bundle

2. **Updated `files/com.warpd.warpd.plist`**:
   - LaunchAgent now points to `/Applications/warpd.app/Contents/MacOS/warpd`

3. **Updated `codesign/sign.sh`**:
   - Made more robust to handle signing errors gracefully
   - Accepts binary path as argument
   - Falls back to ad-hoc signing if certificate not available

4. **Updated `README.md`**:
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
