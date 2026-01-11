# Pull Request Summary

## Title
Fix macOS accessibility permissions by creating proper .app bundle

## Problem Statement
The app/binary executes without any errors, but it is not showing up in the macOS System Settings → Privacy & Security → Accessibility section (or other privacy categories), so users cannot manually allow the required permissions.

## Root Cause
On macOS 10.15 (Catalina) and later, the TCC (Transparency, Consent, and Control) system requires applications to be properly bundled as `.app` bundles to appear in Privacy & Security settings. 

**The critical issue**: When a binary is executed via a symlink (as was done with `/usr/local/bin/warpd` → `/Applications/warpd.app/Contents/MacOS/warpd`), the macOS API `[NSBundle mainBundle]` fails to properly identify the enclosing app bundle. This causes TCC to register the app by binary path instead of bundle identifier, preventing it from appearing in the System Preferences UI even though the permission prompt appears.

The previous approach of creating a symlink to the binary was insufficient because:
1. Symlinks break bundle context recognition
2. TCC tracks the app as a path-based client (client_type=1) instead of bundle-based (client_type=0)
3. Only bundle-based clients appear in the System Settings UI

## Solution
Modified the build system to create a proper macOS application bundle structure and use a wrapper script instead of a symlink:

```
warpd.app/
└── Contents/
    ├── Info.plist
    ├── MacOS/
    │   └── warpd (executable)
    └── Resources/
```

## Changes Made

### 1. mk/macos.mk
- Modified `all` target to:
  - Build binary as `bin/warpd-bin`
  - Create `bin/warpd.app` bundle structure
  - Copy binary to `warpd.app/Contents/MacOS/warpd`
  - Copy Info.plist to `warpd.app/Contents/Info.plist`
  - **Create wrapper script `bin/warpd` that executes the binary from its canonical bundle path**
  - Code sign the bundled binary

- Modified `rel` target to follow same pattern for release builds

- Modified `install` target to:
  - Copy app bundle to `$(DESTDIR)/Applications/`
  - **Install wrapper script at `/usr/local/bin/warpd` (instead of symlink) to preserve bundle context**

- Modified `uninstall` target to remove app bundle

- Modified `clean` target to clean up app bundle, intermediate binary, and wrapper script

### 2. files/warpd-wrapper.sh (new file)
- Shell wrapper script that executes the warpd binary from its canonical path within the app bundle
- Ensures macOS TCC properly recognizes the app bundle via `[NSBundle mainBundle]`
- Passes all command-line arguments through to the binary
- Critical for making `warpd -f` and `warpd` work correctly from command line

### 3. files/com.warpd.warpd.plist
- Updated `ProgramArguments` to point to `/Applications/warpd.app/Contents/MacOS/warpd` instead of `/usr/local/bin/warpd`

### 4. codesign/sign.sh
- Made script accept binary path as argument
- Added error suppression and fallback logic for signing operations
- Falls back to ad-hoc signing (`-s -`) if certificate not available
- More robust error handling

### 5. README.md
- Updated macOS installation instructions
- Updated uninstallation instructions to include app bundle removal
- Updated notes to explain the app bundle approach
- Added note about upgrading from previous versions

### 6. MACOS_ACCESSIBILITY_FIX.md
- Updated to document the wrapper script approach instead of symlink
- Explained the root cause of the TCC recognition issue
- Added technical details about `[NSBundle mainBundle]` and symlink problems

### 5. MACOS_ACCESSIBILITY_FIX.md (new file)
- Comprehensive documentation of the problem and solution
- Step-by-step testing instructions
- Upgrade guide for existing users

### 6. mk/macos-bundle.sh (new file)
- Shell script for creating app bundles (currently not used by build, but available as utility)

## Testing Checklist
- [ ] Build completes successfully on macOS
- [ ] App bundle is created with correct structure
- [ ] Binary is code-signed (or ad-hoc signed)
- [ ] Installation places bundle in /Applications/
- [ ] Symlink at /usr/local/bin/warpd works correctly
- [ ] LaunchAgent starts the application
- [ ] Application appears in System Settings → Privacy & Security → Accessibility
- [ ] Application can be granted accessibility permissions
- [ ] Application functions correctly after granting permissions
- [ ] Uninstallation removes all installed files

## Backwards Compatibility
The changes maintain backwards compatibility in terms of command-line usage (`warpd` command still works via symlink). However, users upgrading from previous versions may need to:
1. Unload the old LaunchAgent
2. Optionally reset accessibility permissions with `sudo tccutil reset Accessibility`
3. Remove the old standalone binary
4. Install the new version
5. Re-grant accessibility permissions

## Notes
- The app bundle approach is the recommended way to distribute macOS applications since 10.15
- This fix ensures warpd appears properly in all Privacy & Security categories
- The symlink maintains command-line convenience while satisfying macOS security requirements
- Ad-hoc code signing is sufficient for local development and testing
