/*
 * warpd - A modal keyboard-driven pointing system.
 *
 * © 2019 Raheman Vaiya (see: LICENSE).
 */

#include "warpd.h"

enum cursor_style cursor_style_from_name(const char *name)
{
	if (!strcmp(name, "crosshair"))
		return CURSOR_STYLE_CROSSHAIR;

	return CURSOR_STYLE_SQUARE;
}

static int clamp_one(int n)
{
	return n < 1 ? 1 : n;
}

static void draw_box(screen_t scr, int x, int y, int w, int h,
		     const char *color, const char *border_color)
{
	platform->screen_draw_box(scr, x - 1, y - 1, w + 2, h + 2,
			border_color);
	platform->screen_draw_box(scr, x, y, w, h, color);
}

static void draw_crosshair(screen_t scr, int x, int y, int sz,
			   const char *color, const char *border_color,
			   int centered)
{
	const int arm = clamp_one(sz);
	const int thickness = clamp_one(sz / 3);
	const int half = thickness / 2;
	const int gap = centered ? clamp_one(sz / 2) : clamp_one(sz / 2) + 1;

	draw_box(scr, x - gap - arm, y - half, arm, thickness,
			color, border_color);
	draw_box(scr, x + gap, y - half, arm, thickness,
			color, border_color);
	draw_box(scr, x - half, y - gap - arm, thickness, arm,
			color, border_color);
	draw_box(scr, x - half, y + gap, thickness, arm,
			color, border_color);
	if (centered)
		draw_box(scr, x - half, y - half, thickness, thickness,
				color, border_color);
}

void cursor_draw(screen_t scr, int x, int y, int sz, const char *color,
		 const char *border_color, enum cursor_style style,
		 int centered)
{
	if (style == CURSOR_STYLE_CROSSHAIR) {
		draw_crosshair(scr, x, y, sz, color, border_color, centered);
		return;
	}

	if (centered)
		draw_box(scr, x - sz / 2, y - sz / 2, sz, sz,
				color, border_color);
	else
		draw_box(scr, x + 2, y - sz / 2, sz, sz,
				color, border_color);
}
