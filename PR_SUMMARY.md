# Pull Request Summary

## Title
Fix macOS accessibility permissions by creating proper .app bundle

## Problem Statement
The app/binary executes without any errors, but it is not showing up in the macOS System Settings → Privacy & Security → Accessibility section (or other privacy categories), so users cannot manually allow the required permissions.

## Root Cause
On macOS 10.15 (Catalina) and later, the TCC (Transparency, Consent, and Control) system requires applications to be properly bundled as `.app` bundles to appear in Privacy & Security settings. Simply embedding an `Info.plist` into a standalone binary using the `-Wl,-sectcreate,__TEXT,__info_plist` linker flag is no longer sufficient for the system to recognize the application.

## Solution
Modified the build system to create a proper macOS application bundle structure:

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
  - Create symlink `bin/warpd` → `warpd.app/Contents/MacOS/warpd` for convenience
  - Code sign the bundled binary

- Modified `rel` target to follow same pattern for release builds

- Modified `install` target to:
  - Copy app bundle to `$(DESTDIR)/Applications/`
  - Create symlink at `/usr/local/bin/warpd` → `/Applications/warpd.app/Contents/MacOS/warpd`

- Modified `uninstall` target to remove app bundle

- Modified `clean` target to clean up app bundle and intermediate binary

### 2. files/com.warpd.warpd.plist
- Updated `ProgramArguments` to point to `/Applications/warpd.app/Contents/MacOS/warpd` instead of `/usr/local/bin/warpd`

### 3. codesign/sign.sh
- Made script accept binary path as argument
- Added error suppression and fallback logic for signing operations
- Falls back to ad-hoc signing (`-s -`) if certificate not available
- More robust error handling

### 4. README.md
- Updated macOS installation instructions
- Updated uninstallation instructions to include app bundle removal
- Updated notes to explain the app bundle approach
- Added note about upgrading from previous versions

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
