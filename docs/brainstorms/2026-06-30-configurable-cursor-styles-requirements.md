---
date: 2026-06-30
topic: configurable-cursor-styles
---

# Configurable Cursor Styles Requirements

## Summary

Add a small configurable cursor-style choice for warpd's drawn overlay cursor. The change should make the cursor feel more modern than the current red square-dot while preserving precise keyboard pointer control.

## Problem Frame

warpd currently represents its internal overlay cursor as a small colored square. That is functional, but it can read as dated or visually crude compared with modern pointer affordances. Since the cursor is the primary visual anchor during normal and grid modes, improving its appearance can make the tool feel more polished without changing how users move or click.

The existing documentation also notes that the drawn cursor is slightly offset from the actual pointer, and that larger cursor sizes can make this more noticeable. Any visual refresh needs to improve scanability without making target precision feel worse.

## Key Decisions

- **Configurable styles over one replacement default.** "Modern" is partly visual preference, and warpd already exposes cursor tuning through configuration.
- **Appearance-only scope.** This work should affect the overlay cursor's look, not pointer movement, acceleration, keybindings, or click behavior.
- **Precision-preserving styles.** Supported styles should keep the actual target point clear and avoid bulky shapes that obscure nearby UI.

## Requirements

**Cursor Style Choice**

- R1. Users can choose from a small set of overlay cursor styles through configuration.
- R2. The supported style set includes the existing square-like style for compatibility.
- R3. At least one new modern style provides a clear center or target point rather than only a filled square.

**Visual Behavior**

- R4. The selected cursor style is used consistently anywhere warpd draws the overlay cursor for normal pointer control.
- R5. Cursor color and size remain meaningful controls for the supported styles unless a style has a clear reason to treat them differently.
- R6. New styles should remain visible on varied desktop backgrounds without relying on large filled blocks.

**Compatibility**

- R7. Existing configurations that only set `cursor_color` or `cursor_size` continue to work.
- R8. The system cursor fallback remains available and is not made the default as part of this change.

## Acceptance Examples

- AE1. **Covers R1, R3.** Given a user selects a modern target-style cursor, when normal mode is active, then the overlay shows a cleaner target marker instead of only the old filled square.
- AE2. **Covers R2, R7.** Given a user prefers the old filled cursor, when they set the square cursor style, then warpd renders the traditional square-like overlay.
- AE3. **Covers R4, R5.** Given a user changes cursor color or size, when the selected style is drawn, then the style reflects those existing visual settings in a predictable way.

## Scope Boundaries

- Reworking pointer physics, acceleration, movement limits, or keybindings is out of scope.
- Making the OS system cursor the default is out of scope.
- A large theme system or user-defined custom cursor drawing language is out of scope.
- Fixing the documented cursor offset can be considered during planning only if a chosen style makes the issue materially worse; otherwise it is not part of this brainstorm's scope.

## Success Criteria

- The overlay cursor has at least one modern-looking option that is easy to see and precise enough for target selection.
- Existing users can restore the traditional square-like overlay with an explicit cursor style setting.
- The resulting scope is small enough to plan as a focused cursor rendering/configuration change.

## Sources / Research

- `src/normal.c` draws the normal-mode overlay cursor as a configured box.
- `src/grid.c` draws the grid-mode center cursor using the same cursor color setting.
- `src/config.c` defines existing cursor-related options, including `cursor_color`, `cursor_size`, `normal_system_cursor`, and blink behavior.
- `warpd.1.md` documents the cursor offset limitation and the precision concern around larger cursor sizes.
