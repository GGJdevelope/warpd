VERSION=1.3.5
PREFIX?=/usr/local
COMMITSTR=$(shell commit=$$(git rev-parse --short HEAD 2> /dev/null) && echo " (built from: $$commit)")
.DEFAULT_GOAL := all

ifeq ($(shell uname -s), Darwin)
	PLATFORM?=macos
endif

ifeq ($(PLATFORM), macos)
	VERSION:=$(VERSION)-osx
endif

%.o: %.c Makefile mk/*.mk
	$(CC) -c $< -o $@ $(CFLAGS)

CFLAGS:=-g\
       -Wall\
       -Wextra\
       -pedantic\
       -Wno-deprecated-declarations\
       -Wno-unused-parameter\
       -std=c99\
       -DVERSION='"v$(VERSION)$(COMMITSTR)"'\
       -D_DEFAULT_SOURCE \
       -D_FORTIFY_SOURCE=2  $(CFLAGS)

ifeq ($(PLATFORM), macos)
	include mk/macos.mk
else ifeq ($(PLATFORM), windows)
	include mk/windows.mk
else
	include mk/linux.mk
endif

.PHONY: test

test: tests/cursor_style_test tests/input_interrupt_test tests/mode_interrupt_test
	./tests/cursor_style_test
	./tests/input_interrupt_test
	./tests/mode_interrupt_test

tests/cursor_style_test: tests/cursor_style_test.c src/cursor.c src/config.c src/warpd.h src/platform.h
	$(CC) -o tests/cursor_style_test tests/cursor_style_test.c src/cursor.c src/config.c $(CFLAGS)

tests/input_interrupt_test: tests/input_interrupt_test.c src/input.c src/warpd.h src/platform.h
	$(CC) -o tests/input_interrupt_test tests/input_interrupt_test.c src/input.c $(CFLAGS)

tests/mode_interrupt_test: tests/mode_interrupt_test.c src/normal.c src/warpd.h src/platform.h
	$(CC) -o tests/mode_interrupt_test tests/mode_interrupt_test.c src/normal.c $(CFLAGS)

man:
	scdoc < warpd.1.md | gzip > files/warpd.1.gz
