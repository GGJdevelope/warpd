#include "../src/warpd.h"

#include <stdlib.h>

static struct platform test_platform;

struct platform *platform = &test_platform;

static void test_null_is_not_interrupt(void)
{
	assert(!input_event_is_interrupt(NULL));
}

static void test_zero_code_event_is_interrupt(void)
{
	struct input_event ev = { .pressed = 1 };

	assert(input_event_is_interrupt(&ev));
}

static void test_real_key_event_is_not_interrupt(void)
{
	struct input_event ev = {
		.code = 42,
		.mods = PLATFORM_MOD_CONTROL,
		.pressed = 1,
	};

	assert(!input_event_is_interrupt(&ev));
}

int main(void)
{
	test_null_is_not_interrupt();
	test_zero_code_event_is_interrupt();
	test_real_key_event_is_not_interrupt();

	return 0;
}
