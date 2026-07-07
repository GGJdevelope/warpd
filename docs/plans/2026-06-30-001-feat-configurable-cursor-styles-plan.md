---
title: "feat: Add Configurable Cursor Styles"
type: feat
status: completed
date: 2026-06-30
origin: docs/brainstorms/2026-06-30-configurable-cursor-styles-requirements.md
---

# feat: Add Configurable Cursor Styles

## Summary

Add configurable styles for warpd's drawn overlay cursor with the modern crosshair style as the default. The plan keeps the change focused on cursor appearance, using the existing rectangle-based overlay drawing model rather than expanding platform drawing APIs.

## Problem Frame

warpd's internal overlay cursor is currently drawn as a small colored square in normal and grid modes. The confirmed requirements call for a more modern-looking target-style cursor option without changing movement, keybindings, click behavior, or the system cursor fallback. Because the documented cursor offset can become more noticeable with larger visuals, the implementation needs to keep the target point clear and avoid bulky filled markers.

## Requirements

**Cursor Style Choice**

- R1. Users can choose from a small set of overlay cursor styles through configuration.
- R2. Users can still choose the current square-like cursor explicitly.
- R3. At least one new style provides a clear center or target point instead of only a filled square.

**Visual Behavior**

- R4. Normal mode and grid mode use the same selected overlay cursor style.
- R5. Existing `cursor_color` and `cursor_size` settings remain meaningful for all supported styles, with `cursor_border_color` controlling the outline.
- R6. New styles remain visible without relying on large filled blocks that obscure nearby UI.

**Compatibility**

- R7. Existing configs that only set `cursor_color` or `cursor_size` require no changes.
- R8. `normal_system_cursor` remains available and is not made the default.

## Key Technical Decisions

- **Rectangle-composed styles:** Implement styles by emitting one or more existing overlay rectangles. This keeps X, macOS, and Windows drawing behavior inside the current `screen_draw_box` abstraction, avoiding a cross-platform shape API expansion for a small visual feature.
- **Shared cursor drawing helper:** Route normal-mode and grid-mode overlay cursor drawing through one helper so style behavior stays consistent and future cursor styles do not duplicate geometry logic.
- **Modern default:** Add a cursor style option whose default is the modern crosshair marker. Keep the square style available for users who want the traditional filled cursor.
- **Focused test harness:** Add small tests around style parsing and emitted draw rectangles rather than trying to automate full desktop overlay screenshots. Visual runtime checks can remain manual because platform overlay rendering is not currently covered by automated tests.

## Implementation Units

### U1. Cursor Style Configuration

- **Goal:** Add a configurable cursor style option that defaults to the modern marker and rejects or handles unknown values consistently with existing configuration expectations.
- **Requirements:** R1, R2, R5, R7
- **Dependencies:** None
- **Files:** `src/config.c`, `src/warpd.h`, `warpd.1.md`, `tests/cursor_style_test.c`
- **Approach:** Extend the configuration option list with a cursor style string. Keep the default mapped to the modern crosshair cursor and keep the traditional square-like cursor selectable. Expose the selected style through a small internal representation or lookup path that cursor rendering can consume without changing movement code.
- **Patterns to follow:** Existing string options in `src/config.c`; `config_print_options()` as the source for `--list-options` output.
- **Test scenarios:**
  - Given no cursor style setting, parsing config leaves the style at the modern crosshair default.
  - Given each supported style name, parsing config makes that style available to the cursor renderer.
  - Given existing configs with only `cursor_color` and `cursor_size`, parsing succeeds and preserves those values.
- **Verification:** `--list-options` exposes the new option with a clear default, existing cursor options still appear, and focused tests cover default and explicit style selection.

### U2. Shared Overlay Cursor Renderer

- **Goal:** Centralize overlay cursor drawing so normal mode and grid mode render the same selected style.
- **Requirements:** R3, R4, R5, R6
- **Dependencies:** U1
- **Files:** `src/normal.c`, `src/grid.c`, `src/warpd.h`, `src/cursor.c`, `tests/cursor_style_test.c`
- **Approach:** Introduce a small cursor-rendering helper that accepts a screen, pointer coordinates, configured size, color, and selected style. The existing square behavior should remain one style. Add at least one modern target-style cursor composed from thin rectangles with an obvious center point.
- **Execution note:** Implement the helper with testable geometry first, then swap normal and grid modes to call it.
- **Patterns to follow:** Current calls to `platform->screen_draw_box()` in `src/normal.c` and `src/grid.c`; current coordinate conventions where normal mode draws near the pointer and grid mode draws centered in the grid.
- **Test scenarios:**
  - Covers AE1. Given the modern target-style cursor, the renderer emits multiple thin draw operations that create a target marker instead of a single filled square.
  - Given the explicit square-compatible style, the renderer emits geometry equivalent to the current square-like marker.
  - Covers AE3. Given custom cursor body, border, and size settings, emitted draw operations use those colors and scale dimensions from that size.
  - Given small cursor sizes, the modern style still emits nonzero dimensions and keeps the target point readable.
- **Verification:** Normal mode and grid mode no longer carry separate cursor box geometry for style behavior, and focused tests validate the emitted draw operations for default and modern styles.

### U3. Compatibility, Documentation, and Build Verification

- **Goal:** Document the new option and verify the focused change builds cleanly in the existing project structure.
- **Requirements:** R2, R7, R8
- **Dependencies:** U1, U2
- **Files:** `warpd.1.md`, `README.md`, `Makefile`, `tests/cursor_style_test.c`
- **Approach:** Update user-facing configuration documentation to name the supported cursor styles and explain that the system cursor fallback is unchanged. Wire any new test file into the existing build flow in the smallest practical way, keeping regular binary builds intact.
- **Patterns to follow:** `warpd.1.md` configuration option guidance; platform-specific build selection in `Makefile` and `mk/*.mk`.
- **Test scenarios:**
  - Covers AE2. Given a user chooses the square style, the documented option preserves access to the traditional cursor.
  - Given `normal_system_cursor` is enabled, the internal overlay cursor remains bypassed as before.
  - The focused cursor-style tests can run without needing a live desktop overlay.
- **Verification:** The project builds for the current platform, the focused cursor-style tests run, and the user documentation describes the new option without implying movement or system-cursor behavior changed.

## Scope Boundaries

- Pointer physics, acceleration, movement limits, click behavior, and keybindings stay out of scope.
- The OS system cursor fallback remains opt-in and is not made default.
- A large theme system or user-defined drawing language stays out of scope.
- Fixing the documented cursor offset is not planned unless implementation proves a chosen style materially worsens precision.
- Adding non-rectangular platform drawing primitives is deferred; the initial styles should use the existing overlay rectangle abstraction.

## Risks & Dependencies

- **Geometry precision:** A visually nicer style can still feel worse if it obscures the target point. Keep modern styles thin and centered around the same target assumptions as the current cursor.
- **Platform parity:** Because platform overlays all support rectangles today, rectangle-composed styles should behave consistently, but the implementer should still build on the current platform and avoid platform-specific drawing calls.
- **Test harness friction:** The repo does not currently show a broad automated test harness. The plan assumes a small focused test binary or equivalent harness is acceptable for validating cursor style geometry.

## Documentation / Operational Notes

- Update `warpd.1.md` so users can discover the new cursor style option through the manual page.
- Regenerate `files/warpd.1.gz` only if the project convention for committed manpage artifacts expects generated manpage updates in the same change.
- Keep the option description concise in `--list-options`, matching existing config option style.

## Sources / Research

- `docs/brainstorms/2026-06-30-configurable-cursor-styles-requirements.md` is the origin document for scope and success criteria.
- `src/normal.c` currently draws normal-mode overlay cursor geometry inline.
- `src/grid.c` currently draws the grid-mode cursor inline at the grid center.
- `src/config.c` defines cursor-related options and prints option descriptions.
- `src/platform.h` exposes rectangle-only overlay drawing through `screen_draw_box`.
- `src/platform/linux/X/screen.c`, `src/platform/macos/screen.m`, and `src/platform/windows/winscreen.c` confirm the current cross-platform overlay drawing pattern is rectangle-based.
- `warpd.1.md` documents the cursor offset limitation that should shape precision-preserving style design.
