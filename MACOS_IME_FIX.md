# macOS IME (Input Method Editor) Freezing Fix

## Problem

When using non-ASCII input methods (particularly Korean, Japanese, or Chinese IME) on macOS, entering warpd modes (hint, grid, or normal mode) would cause the entire keyboard to freeze and become unresponsive.

### Symptoms
- Korean IME (or other IME) is active
- User presses the activation key (e.g., `Alt+Cmd+x` for hint mode)
- Keyboard becomes completely unresponsive
- No keyboard input is registered by any application
- User must force quit applications or restart to recover

### Root Cause

This is a classic macOS accessibility + IME conflict:

1. **IME Composition State**: When an IME is active, it can be in an intermediate composition state (e.g., while typing Korean characters that require multiple keypresses to form a complete syllable).

2. **Event Tap Interception**: warpd uses macOS Accessibility API's event tap to capture keyboard events globally. When warpd grabs the keyboard (by returning `nil` from the event tap callback), it blocks all keyboard events from reaching other applications.

3. **IME State Corruption**: If warpd grabs the keyboard while the IME is in the middle of a composition:
   - The IME never receives the events needed to complete or cancel the composition
   - The IME enters a stuck state where it's waiting for input that will never arrive
   - Since the IME has focus for text input processing, no other application can receive keyboard input
   - The entire keyboard input system becomes frozen

## Solution

The fix temporarily switches to an ASCII-capable input source before grabbing the keyboard, and restores the original input source when releasing the grab.

### How It Works

1. **Before Grabbing** (`osx_input_grab_keyboard()`):
   - Check if the current input source is ASCII-capable
   - If not (e.g., Korean IME is active):
     - Save the current input source
     - Switch to an ASCII-capable input source
     - This cleanly terminates any active IME composition

2. **After Releasing** (`osx_input_ungrab_keyboard()`):
   - Restore the previously saved input source
   - User can continue typing in their preferred language

### Implementation Details

The fix is implemented in `src/platform/macos/input.m`:

- `save_and_switch_to_ascii_input()`: Saves current input source and switches to ASCII
- `restore_input_source()`: Restores the previously saved input source
- Modified `osx_input_grab_keyboard()` to call `save_and_switch_to_ascii_input()`
- Modified `osx_input_ungrab_keyboard()` to call `restore_input_source()`

## Testing

To verify the fix works:

1. **Enable Korean IME** (or another non-ASCII input method):
   - Go to System Settings → Keyboard → Input Sources
   - Add Korean (2-Set Korean) or your preferred IME

2. **Switch to the IME**:
   - Use the Input Source selector in the menu bar
   - Or press the keyboard shortcut (usually Ctrl+Space or Cmd+Space)

3. **Test with warpd**:
   - Press `Alt+Cmd+x` (or your configured activation key) to enter hint mode
   - Keyboard should remain responsive
   - Press Esc to exit hint mode
   - IME should be restored to Korean

4. **Test other modes**:
   - Try grid mode (`Alt+Cmd+g`)
   - Try normal mode (`Alt+Cmd+c`)
   - All should work without freezing

5. **Test IME composition**:
   - Open a text editor
   - Switch to Korean IME
   - Start typing a Korean character (don't complete it)
   - Press `Alt+Cmd+x` to enter hint mode
   - The partial character should be cancelled cleanly
   - Keyboard should remain responsive

## Compatibility

This fix:
- ✅ Works with all IMEs (Korean, Japanese, Chinese, etc.)
- ✅ Works with ASCII-capable input sources (no change in behavior)
- ✅ Maintains the original input source preference
- ✅ Does not interfere with normal warpd operation
- ✅ Gracefully handles cases where ASCII input source is unavailable

## Technical Notes

- Uses macOS Text Input Services (TIS) API
- `TISCopyCurrentKeyboardInputSource()` - Gets current input source
- `TISCopyCurrentASCIICapableKeyboardInputSource()` - Gets ASCII fallback
- `TISSelectInputSource()` - Switches input source
- `kTISPropertyInputSourceIsASCIICapable` - Checks if source is ASCII-capable

## Related Issues

This fix addresses the keyboard freezing issue described in issue #7 and is similar to the keyboard freeze fix for Linux X11 in v1.3.5 (see CHANGELOG.md).
