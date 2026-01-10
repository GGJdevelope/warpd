/*
 * warpd - A modal keyboard-driven pointing system.
 *
 * © 2019 Raheman Vaiya (see: LICENSE).
 */

#include "X.h"

Display *dpy = NULL;

struct monitored_file monitored_files[32];
size_t nr_monitored_files = 0;

int hex_to_rgba(const char *str, uint8_t *r, uint8_t *g, uint8_t *b, uint8_t *a)
{
#define X2B(c) ((c >= '0' && c <= '9') ? (c & 0xF) : (((c | 0x20) - 'a') + 10))

	if (str == NULL)
		return 0;
	str = (*str == '#') ? str + 1 : str;

	ssize_t len = strlen(str);

	if (len != 6 && len != 8)
		return -1;

	*r = X2B(str[0]);
	*r <<= 4;
	*r |= X2B(str[1]);

	*g = X2B(str[2]);
	*g <<= 4;
	*g |= X2B(str[3]);

	*b = X2B(str[4]);
	*b <<= 4;
	*b |= X2B(str[5]);

	*a = 255;
	if (len == 8) {
		*a = X2B(str[6]);
		*a <<= 4;
		*a |= X2B(str[7]);
	}

	return 0;
}

uint32_t parse_xcolor(const char *s, uint8_t *opacity)
{
	XColor col;

	uint8_t r, g, b, a;

	hex_to_rgba(s, &r, &g, &b, &a);

	if (opacity)
		*opacity = a;

	col.red = (int)r << 8;
	col.green = (int)g << 8;
	col.blue = (int)b << 8;
	col.flags = DoRed | DoGreen | DoBlue;

	assert(
	    XAllocColor(dpy, XDefaultColormap(dpy, DefaultScreen(dpy)), &col));

	return col.pixel;
}

/*
 * Disable shadows for compton based compositors.
 *
 * Ref:
 * https://github.com/yshui/picom/blob/aa316aa3601a4f3ce9c1ca79932218ab574e61a7/src/win.c#L850
 */
static void disable_compton_shadow(Display *dpy, Window w)
{
	Atom _COMPTON_SHADOW = XInternAtom(dpy, "_COMPTON_SHADOW", False);
	unsigned int v = 0;

	XChangeProperty(dpy, w, _COMPTON_SHADOW, XA_CARDINAL, 32,
			PropModeReplace, (unsigned char *)&v, 1L);
}

static void set_opacity(Display *dpy, Window w, uint8_t _opacity)
{
	Atom OPACITY_ATOM = XInternAtom(dpy, "_NET_WM_WINDOW_OPACITY", False);

	unsigned int opacity =
	    (unsigned int)(((double)_opacity / 255) * (double)0xffffffff);

	XChangeProperty(dpy, w, OPACITY_ATOM, XA_CARDINAL, 32, PropModeReplace,
			(unsigned char *)&opacity, 1L);
}

void x_copy_selection()
{
	XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Control_L), True,
			  CurrentTime);
	XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Insert), True,
			  CurrentTime);

	XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Control_L), False,
			  CurrentTime);
	XTestFakeKeyEvent(dpy, XKeysymToKeycode(dpy, XK_Insert), False,
			  CurrentTime);
	XSync(dpy, False);

	system("xclip -o|xclip -selection CLIPBOARD");
}

void x_scroll(int direction)
{
	int btn = 0;

	switch (direction) {
	case SCROLL_UP:
		btn = 4;
		break;
	case SCROLL_DOWN:
		btn = 5;
		break;
	case SCROLL_RIGHT:
		btn = 7;
		break;
	case SCROLL_LEFT:
		btn = 6;
		break;
	}

	XTestFakeButtonEvent(dpy, btn, True, CurrentTime);
	XTestFakeButtonEvent(dpy, btn, False, CurrentTime);
}

Window create_window(const char *color)
{
	uint32_t col = 0;
	XClassHint *hint;
	uint8_t opacity;

	col = parse_xcolor(color, &opacity);

	Window win = XCreateWindow(
	    dpy, DefaultRootWindow(dpy), 0, 0, 1, 1, 0,
	    DefaultDepth(dpy, DefaultScreen(dpy)), InputOutput,
	    DefaultVisual(dpy, DefaultScreen(dpy)),
	    CWOverrideRedirect | CWBackPixel | CWBackingStore | CWBackingPixel,
	    &(XSetWindowAttributes){
		.backing_pixel = col,
		.background_pixel = col,
		.backing_store = Always,
		.override_redirect = 1,
	    });

	/* Requires a compositor. */
	set_opacity(dpy, win, opacity);

	disable_compton_shadow(dpy, win);

	hint = XAllocClassHint();
	hint->res_name = "warpd";
	hint->res_class = "warpd";
	XSetClassHint(dpy, win, hint);

	XFree(hint);

	return win;
}

void x_commit()
{
	XSync(dpy, False);
}

long x_get_mtime(const char *path)
{
	struct stat st;
	if (stat(path, &st))
		return 0;

	return st.st_mtime;
}

void x_monitor_file(const char *path)
{
	/*
	 * OPT: We could use inotify, but that would reduce portability and involve
	 * additional logic to handle renames. Polling is good enough.
	 */
	struct monitored_file *mf = &monitored_files[nr_monitored_files];
	assert(nr_monitored_files < sizeof (monitored_files) / sizeof(monitored_files[0]));

	strncpy(mf->path, path, sizeof mf->path);
	mf->mtime = x_get_mtime(path);

	nr_monitored_files++;
}

void x_init(struct platform *platform)
{
	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		/* Since we can't show a modal without X11, just use fprintf */
		fprintf(stderr, "ERROR: Could not connect to X server\n");
		fprintf(stderr, "Please make sure X11 is running and DISPLAY is set correctly.\n");
		exit(-1);
	}

	/* TODO: account for screen hotplugging */
	init_xscreens();

	platform->monitor_file = x_monitor_file;
	platform->commit = x_commit;
	platform->copy_selection = x_copy_selection;
	platform->hint_draw = x_hint_draw;
	platform->init_hint = x_init_hint;
	platform->input_grab_keyboard = x_input_grab_keyboard;
	platform->input_lookup_code = x_input_lookup_code;
	platform->input_lookup_name = x_input_lookup_name;
	platform->input_next_event = x_input_next_event;
	platform->input_ungrab_keyboard = x_input_ungrab_keyboard;
	platform->input_wait = x_input_wait;
	platform->mouse_click = x_mouse_click;
	platform->mouse_down = x_mouse_down;
	platform->mouse_get_position = x_mouse_get_position;
	platform->mouse_hide = x_mouse_hide;
	platform->mouse_move = x_mouse_move;
	platform->mouse_show = x_mouse_show;
	platform->mouse_up = x_mouse_up;
	platform->screen_clear = x_screen_clear;
	platform->screen_draw_box = x_screen_draw_box;
	platform->screen_get_dimensions = x_screen_get_dimensions;
	platform->screen_list = x_screen_list;
	platform->scroll = x_scroll;
	platform->show_error_modal = x_show_error_modal;
}

void x_show_error_modal(const char *title, const char *message)
{
	if (!dpy)
		return;

	Window root = DefaultRootWindow(dpy);
	int screen = DefaultScreen(dpy);
	
	/* Create a simple modal window */
	int win_w = 400;
	int win_h = 150;
	int scr_w = DisplayWidth(dpy, screen);
	int scr_h = DisplayHeight(dpy, screen);
	int x = (scr_w - win_w) / 2;
	int y = (scr_h - win_h) / 2;
	
	Window win = XCreateSimpleWindow(dpy, root, x, y, win_w, win_h, 2,
					 BlackPixel(dpy, screen),
					 WhitePixel(dpy, screen));
	
	/* Set window properties */
	XStoreName(dpy, win, title);
	XSelectInput(dpy, win, ExposureMask | KeyPressMask | ButtonPressMask);
	
	/* Set window type to dialog */
	Atom wm_type = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE", False);
	Atom wm_type_dialog = XInternAtom(dpy, "_NET_WM_WINDOW_TYPE_DIALOG", False);
	XChangeProperty(dpy, win, wm_type, XA_ATOM, 32, PropModeReplace,
			(unsigned char *)&wm_type_dialog, 1);
	
	/* Set it to be on top */
	Atom wm_state = XInternAtom(dpy, "_NET_WM_STATE", False);
	Atom wm_state_above = XInternAtom(dpy, "_NET_WM_STATE_ABOVE", False);
	XChangeProperty(dpy, win, wm_state, XA_ATOM, 32, PropModeReplace,
			(unsigned char *)&wm_state_above, 1);
	
	XMapWindow(dpy, win);
	XRaiseWindow(dpy, win);
	XFlush(dpy);
	
	/* Draw the message */
	GC gc = XCreateGC(dpy, win, 0, NULL);
	XSetForeground(dpy, gc, BlackPixel(dpy, screen));
	
	/* Simple text rendering - split message into lines */
	int text_y = 30;
	const char *line_start = message;
	char line_buf[256];
	
	while (*line_start) {
		const char *line_end = strchr(line_start, '\n');
		int line_len;
		
		if (line_end) {
			line_len = line_end - line_start;
			if (line_len > 255) line_len = 255;
			strncpy(line_buf, line_start, line_len);
			line_buf[line_len] = '\0';
			line_start = line_end + 1;
		} else {
			strncpy(line_buf, line_start, 255);
			line_buf[255] = '\0';
			line_start += strlen(line_start);
		}
		
		XDrawString(dpy, win, gc, 10, text_y, line_buf, strlen(line_buf));
		text_y += 20;
		
		if (!*line_start)
			break;
	}
	
	XDrawString(dpy, win, gc, 10, win_h - 20, "Press any key or click to dismiss", 34);
	XFlush(dpy);
	
	/* Wait for user input with 10 second timeout */
	struct timeval tv;
	fd_set fds;
	int x11_fd = ConnectionNumber(dpy);
	
	tv.tv_sec = 10;
	tv.tv_usec = 0;
	
	FD_ZERO(&fds);
	FD_SET(x11_fd, &fds);
	
	int ret = select(x11_fd + 1, &fds, NULL, NULL, &tv);
	
	if (ret > 0) {
		/* Wait for any event */
		XEvent ev;
		XNextEvent(dpy, &ev);
	}
	
	XFreeGC(dpy, gc);
	XDestroyWindow(dpy, win);
	XFlush(dpy);
}
