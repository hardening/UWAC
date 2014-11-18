/*
 * Copyright © 2014 David FORT <contact@hardening-consulting.com>
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

#ifndef __UWAC_PRIV_H_
#define __UWAC_PRIV_H_

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include "uwac.h"

/** @brief a global registry object */
struct uwac_global {
	uint32_t name;
	char *interface;
	uint32_t version;
	struct wl_list link;
};
typedef struct uwac_global UwacGlobal;

/** @brief main connection object to a wayland display */
struct uwac_display {
	struct wl_list globals;

	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct wl_subcompositor *subcompositor;

	struct wl_shm *shm;
	enum wl_shm_format *shm_formats;
	uint32_t shm_formats_nb;
	bool has_rgb565;

	struct wl_data_device_manager *data_device_manager;
	struct text_cursor_position *text_cursor_position;
	struct workspace_manager *workspace_manager;
	struct xdg_shell *xdg_shell;

	struct wl_list seats;

	int display_fd;
	int last_error;
	uint32_t display_fd_events;
	int epoll_fd;
	bool running;
	UwacTask dispatch_fd_task;

	struct wl_cursor_theme *cursor_theme;
	struct wl_cursor **cursors;

	struct wl_list windows;

	struct wl_list outputs;
};

/** @brief an output on a wayland display */
struct uwac_output {
	UwacDisplay *display;

	bool doneNeeded;
	bool doneReceived;

	UwacSize resolution;
	int transform;
	int scale;
	char *make;
	char *model;
	uint32_t server_output_id;
	struct wl_output *output;

	struct wl_list link;
};

/** @brief a seat attached to a wayland display */
struct uwac_seat {
	UwacDisplay *display;
	struct wl_seat *seat;
	struct wl_pointer *pointer;
	struct wl_keyboard *keyboard;
	struct wl_touch *touch;
	struct xkb_context *xkb_context;

	struct {
		struct xkb_keymap *keymap;
		struct xkb_state *state;
		xkb_mod_mask_t control_mask;
		xkb_mod_mask_t alt_mask;
		xkb_mod_mask_t shift_mask;
	} xkb;
	uint32_t modifiers;

	UwacWindow *mouse_focus;
	UwacWindow *keyboard_focus;
	UwacWindow *touch_focus;
	struct wl_list link;
};

/** @brief a window */
struct uwac_window {
	UwacDisplay *diplay;
	struct wl_list link;
};

/* in uwac-input.c */
UwacSeat *UwacSeatNew(UwacDisplay *d, uint32_t id, uint32_t version);
void UwacSeatDestroy(UwacSeat *s);

#endif /* __UWAC_PRIV_H_ */
