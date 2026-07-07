#include "../src/warpd.h"

#include <stdlib.h>

static struct platform test_platform;
struct platform *platform = &test_platform;

static struct input_event drag_ev = {
	.code = 1,
	.pressed = 1,
};
static struct input_event interrupt_ev = {
	.pressed = 1,
};

static int next_event_idx;
static int mouse_down_count;
static int mouse_up_count;
static int mouse_show_count;
static int screen_clear_count;
static int ungrab_count;

const char *config_get(const char *key)
{
	if (!strcmp(key, "normal_blink_interval"))
		return "0";
	if (!strcmp(key, "indicator"))
		return "none";
	if (!strcmp(key, "cursor_color"))
		return "#000000";
	if (!strcmp(key, "cursor_border_color"))
		return "#ffffff";
	if (!strcmp(key, "cursor_style"))
		return "square";

	return "";
}

int config_get_int(const char *key)
{
	if (!strcmp(key, "cursor_size"))
		return 7;
	if (!strcmp(key, "normal_system_cursor"))
		return 0;
	if (!strcmp(key, "drag_button"))
		return 1;

	return 0;
}

void config_input_whitelist(const char **keys, size_t sz)
{
	(void)keys;
	(void)sz;
}

int config_input_match(struct input_event *ev, const char *config_key)
{
	if (ev == &drag_ev && !strcmp(config_key, "drag"))
		return 1;

	return 0;
}

int input_event_is_interrupt(struct input_event *ev)
{
	return ev && ev->code == 0 && ev->mods == 0;
}

const char *input_event_tostr(struct input_event *ev)
{
	(void)ev;
	return "";
}

enum cursor_style cursor_style_from_name(const char *name)
{
	(void)name;
	return CURSOR_STYLE_SQUARE;
}

void cursor_draw(screen_t scr, int x, int y, int sz, const char *color,
		 const char *border_color, enum cursor_style style,
		 int centered)
{
	(void)scr;
	(void)x;
	(void)y;
	(void)sz;
	(void)color;
	(void)border_color;
	(void)style;
	(void)centered;
}

int mouse_process_key(struct input_event *ev, const char *up_key,
		      const char *down_key, const char *left_key,
		      const char *right_key)
{
	(void)ev;
	(void)up_key;
	(void)down_key;
	(void)left_key;
	(void)right_key;
	return 0;
}

void mouse_reset(void)
{
}

void mouse_fast(void)
{
}

void mouse_normal(void)
{
}

void mouse_slow(void)
{
}

void scroll_tick(void)
{
}

void scroll_stop(void)
{
}

void scroll_accelerate(int direction)
{
	(void)direction;
}

void scroll_decelerate(void)
{
}

void hist_add(int x, int y)
{
	(void)x;
	(void)y;
}

int hist_get(int *x, int *y)
{
	*x = 0;
	*y = 0;
	return 0;
}

void hist_prev(void)
{
}

void hist_next(void)
{
}

void histfile_add(int x, int y)
{
	(void)x;
	(void)y;
}

static void input_grab_keyboard(void)
{
}

static void input_ungrab_keyboard(void)
{
	ungrab_count++;
}

static struct input_event *input_next_event(int timeout)
{
	(void)timeout;

	if (next_event_idx++ == 0)
		return &interrupt_ev;

	return NULL;
}

static void mouse_get_position(screen_t *scr, int *x, int *y)
{
	static int fake_screen;

	if (scr)
		*scr = (screen_t)&fake_screen;
	if (x)
		*x = 10;
	if (y)
		*y = 20;
}

static void screen_get_dimensions(screen_t scr, int *w, int *h)
{
	(void)scr;
	*w = 100;
	*h = 100;
}

static void mouse_hide(void)
{
}

static void mouse_down(int btn)
{
	assert(btn == 1);
	mouse_down_count++;
}

static void mouse_show(void)
{
	mouse_show_count++;
}

static void mouse_up(int btn)
{
	assert(btn == 1);
	mouse_up_count++;
}

static void screen_clear(screen_t scr)
{
	(void)scr;
	screen_clear_count++;
}

static void screen_draw_box(screen_t scr, int x, int y, int w, int h,
			    const char *color)
{
	(void)scr;
	(void)x;
	(void)y;
	(void)w;
	(void)h;
	(void)color;
}

static void commit(void)
{
}

static void test_normal_mode_interrupt_releases_drag(void)
{
	struct input_event *ev;

	next_event_idx = 0;
	mouse_down_count = 0;
	mouse_up_count = 0;
	mouse_show_count = 0;
	screen_clear_count = 0;
	ungrab_count = 0;

	test_platform.input_grab_keyboard = input_grab_keyboard;
	test_platform.input_ungrab_keyboard = input_ungrab_keyboard;
	test_platform.input_next_event = input_next_event;
	test_platform.mouse_get_position = mouse_get_position;
	test_platform.screen_get_dimensions = screen_get_dimensions;
	test_platform.mouse_hide = mouse_hide;
	test_platform.mouse_show = mouse_show;
	test_platform.mouse_down = mouse_down;
	test_platform.mouse_up = mouse_up;
	test_platform.screen_clear = screen_clear;
	test_platform.screen_draw_box = screen_draw_box;
	test_platform.commit = commit;

	ev = normal_mode(&drag_ev, 0);

	assert(ev == NULL);
	assert(mouse_down_count == 1);
	assert(mouse_up_count == 1);
	assert(mouse_show_count == 1);
	assert(screen_clear_count > 0);
	assert(ungrab_count == 1);
}

int main(void)
{
	test_normal_mode_interrupt_releases_drag();

	return 0;
}
