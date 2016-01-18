/*
 * Copyright © 2015 David FORT <contact@hardening-consulting.com>
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */
#include <stdio.h>
#include <assert.h>
#include <uwac/uwac.h>

#include <xkbcommon/xkbcommon.h>

static void draw(UwacWindow *window, uint32_t time) {
	int x, y;
	uint32_t *winData;
	UwacSize geometry;

	UwacWindowGetGeometry(window, &geometry);
	winData = (uint32_t *)UwacWindowGetDrawingBuffer(window);
	assert(winData);

	for (y = 0; y < geometry.height; y++) {
		for(x = 0; x < geometry.width; x++, winData++) {
			*winData = 0xff000000 + ((x & 0xff) << 8) + y +
					(time & 0xff) << 16;
		}
	}

	UwacWindowAddDamage(window, 0, 0, geometry.width, geometry.height);
	UwacWindowSubmitBuffer(window, false);
}

int main(int argc, char *argv[]) {
	UwacDisplay *display;
	UwacWindow *window;
	UwacEvent event;
	UwacReturnCode err;
	int doRun;
	uint32_t time;
	bool fullscreen = false;

	display = UwacOpenDisplay(NULL, &err);
	if (!display) {
		fprintf(stderr, "unable to open wayland display: %s\n", UwacErrorString(err));
		return 1;
	}


	window = UwacCreateWindowShm(display, 640, 480, WL_SHM_FORMAT_ARGB8888);
	if (!window) {
		fprintf(stderr, "unable to create a window: %s\n", UwacErrorString(UwacDisplayGetLastError(display)) );
		return 1;
	}

	time = 0;
	draw(window, time);
	time ++;

	doRun = 1;
	while (doRun) {
		err = UwacNextEvent(display, &event);
		if (err != UWAC_SUCCESS) {
			doRun = 0;
			break;
		}

		switch (event.type) {
		case UWAC_EVENT_FRAME_DONE:
			draw(window, time);
			time++;
			break;
		case UWAC_EVENT_KEY:
			printf("key sym=0x%x pressed=%d\n", event.key.sym, event.key.pressed);

			/* we act only when the key is released */
			if (event.key.pressed)
				continue;

			switch (event.key.sym) {
			case XKB_KEY_Escape:
				doRun = false;
				break;
			case XKB_KEY_f:
				fullscreen ^= true;
				UwacWindowSetFullscreenState(window, NULL, fullscreen);
				break;
			}
			break;
		case UWAC_EVENT_NEW_SEAT:
			printf("new seat %p\n", event.seat_new.seat);
			break;
		case UWAC_EVENT_REMOVED_SEAT:
			printf("removed seat %p\n", event.seat_new.seat);
			break;
		case UWAC_EVENT_CONFIGURE: {
			UwacConfigureEvent *ev = &event.configure;
			printf("configure event %dx%d", ev->width, ev->height);
			if (ev->states & UWAC_WINDOW_ACTIVATED)
				printf(" activated");
			if (ev->states & UWAC_WINDOW_FULLSCREEN)
				printf(" fullscreen");
			if (ev->states & UWAC_WINDOW_MAXIMIZED)
				printf(" maximized");
			if (ev->states & UWAC_WINDOW_RESIZING)
				printf(" resizing");

			printf("\n");
			break;
		}
		default:
			break;
		}
	}

	UwacDestroyWindow(&window);
	UwacCloseDisplay(&display);
}
