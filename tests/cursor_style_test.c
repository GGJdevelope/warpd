#include "../src/warpd.h"

#include <stdlib.h>

struct draw_call {
	int x;
	int y;
	int w;
	int h;
	char color[64];
};

static struct draw_call calls[16];
static size_t nr_calls;
static struct platform test_platform;

struct platform *platform = &test_platform;

int input_parse_string(struct input_event *ev, const char *s)
{
	(void)ev;
	(void)s;
	return 0;
}

int input_eq(struct input_event *ev, const char *str)
{
	(void)ev;
	(void)str;
	return 0;
}

static void record_box(screen_t scr, int x, int y, int w, int h,
		       const char *color)
{
	(void)scr;
	assert(nr_calls < sizeof calls / sizeof calls[0]);
	calls[nr_calls].x = x;
	calls[nr_calls].y = y;
	calls[nr_calls].w = w;
	calls[nr_calls].h = h;
	snprintf(calls[nr_calls].color, sizeof calls[nr_calls].color,
			"%s", color);
	nr_calls++;
}

static void reset_calls(void)
{
	nr_calls = 0;
	memset(calls, 0, sizeof calls);
}

static void write_config(const char *path, const char *body)
{
	FILE *fh = fopen(path, "w");
	assert(fh);
	fputs(body, fh);
	fclose(fh);
}

static void test_default_config(void)
{
	const char *path = "tests/.cursor-style-empty.conf";

	write_config(path, "");
	parse_config(path);

	assert(!strcmp(config_get("cursor_style"), "crosshair"));
	assert(!strcmp(config_get("cursor_color"), "#000000"));
	assert(!strcmp(config_get("cursor_border_color"), "#ffffff"));
	assert(!strcmp(config_get("cursor_size"), "7"));

	remove(path);
}

static void test_explicit_config(void)
{
	const char *path = "tests/.cursor-style-explicit.conf";

	write_config(path, "cursor_style: crosshair\ncursor_color: #abcdef\ncursor_border_color: #fedcba\ncursor_size: 11\n");
	parse_config(path);

	assert(!strcmp(config_get("cursor_style"), "crosshair"));
	assert(cursor_style_from_name(config_get("cursor_style")) ==
			CURSOR_STYLE_CROSSHAIR);
	assert(!strcmp(config_get("cursor_color"), "#abcdef"));
	assert(!strcmp(config_get("cursor_border_color"), "#fedcba"));
	assert(!strcmp(config_get("cursor_size"), "11"));

	remove(path);
}

static void test_square_draw(void)
{
	reset_calls();

	cursor_draw(NULL, 20, 30, 7, "#123456", "#ffffff",
			CURSOR_STYLE_SQUARE, 0);

	assert(nr_calls == 2);
	assert(calls[0].x == 21);
	assert(calls[0].y == 26);
	assert(calls[0].w == 9);
	assert(calls[0].h == 9);
	assert(!strcmp(calls[0].color, "#ffffff"));
	assert(calls[1].x == 22);
	assert(calls[1].y == 27);
	assert(calls[1].w == 7);
	assert(calls[1].h == 7);
	assert(!strcmp(calls[1].color, "#123456"));
}

static void test_centered_square_draw(void)
{
	reset_calls();

	cursor_draw(NULL, 20, 30, 8, "#123456", "#ffffff",
			CURSOR_STYLE_SQUARE, 1);

	assert(nr_calls == 2);
	assert(calls[0].x == 15);
	assert(calls[0].y == 25);
	assert(calls[0].w == 10);
	assert(calls[0].h == 10);
	assert(!strcmp(calls[0].color, "#ffffff"));
	assert(calls[1].x == 16);
	assert(calls[1].y == 26);
	assert(calls[1].w == 8);
	assert(calls[1].h == 8);
	assert(!strcmp(calls[1].color, "#123456"));
}

static void test_crosshair_draw(void)
{
	size_t i;

	reset_calls();

	cursor_draw(NULL, 20, 30, 9, "#123456", "#ffffff",
			CURSOR_STYLE_CROSSHAIR, 0);

	assert(nr_calls == 8);
	for (i = 0; i < nr_calls; i++) {
		assert(calls[i].w > 0);
		assert(calls[i].h > 0);
		assert(!strcmp(calls[i].color, i % 2 ? "#123456" : "#ffffff"));
	}
}

static void test_normal_crosshair_leaves_click_point_clear(void)
{
	size_t i;

	reset_calls();

	cursor_draw(NULL, 20, 30, 9, "#123456", "#ffffff",
			CURSOR_STYLE_CROSSHAIR, 0);

	for (i = 0; i < nr_calls; i++) {
		assert(!(20 >= calls[i].x && 20 < calls[i].x + calls[i].w &&
			 30 >= calls[i].y && 30 < calls[i].y + calls[i].h));
	}
}

static void test_centered_crosshair_draw(void)
{
	size_t i;

	reset_calls();

	cursor_draw(NULL, 20, 30, 9, "#123456", "#ffffff",
			CURSOR_STYLE_CROSSHAIR, 1);

	assert(nr_calls == 10);
	for (i = 0; i < nr_calls; i++) {
		assert(calls[i].w > 0);
		assert(calls[i].h > 0);
		assert(!strcmp(calls[i].color, i % 2 ? "#123456" : "#ffffff"));
	}
}

static void test_small_crosshair_draw(void)
{
	size_t i;

	reset_calls();

	cursor_draw(NULL, 20, 30, 1, "#123456", "#ffffff",
			CURSOR_STYLE_CROSSHAIR, 0);

	assert(nr_calls == 8);
	for (i = 0; i < nr_calls; i++) {
		assert(calls[i].w > 0);
		assert(calls[i].h > 0);
	}
}

int main(void)
{
	test_platform.screen_draw_box = record_box;

	test_default_config();
	test_explicit_config();
	test_square_draw();
	test_centered_square_draw();
	test_crosshair_draw();
	test_normal_crosshair_leaves_click_point_clear();
	test_centered_crosshair_draw();
	test_small_crosshair_draw();

	return 0;
}
