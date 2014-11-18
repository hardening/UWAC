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
#include "uwac-priv.h"
#include "uwac-utils.h"
#include "uwac.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>


#if 0
static void keyboard_repeat_func(struct task *task, uint32_t events)
{
	struct input *input =
		container_of(task, struct input, repeat_task);
	struct window *window = input->keyboard_focus;
	uint64_t exp;

	if (read(input->repeat_timer_fd, &exp, sizeof exp) != sizeof exp)
		/* If we change the timer between the fd becoming
		 * readable and getting here, there'll be nothing to
		 * read and we get EAGAIN. */
		return;

	if (window && window->key_handler) {
		(*window->key_handler)(window, input, input->repeat_time,
				       input->repeat_key, input->repeat_sym,
				       WL_KEYBOARD_KEY_STATE_PRESSED,
				       window->user_data);
	}
}
#endif

static void keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard,
		       uint32_t format, int fd, uint32_t size)
{
	UwacSeat *input = data;
	struct xkb_keymap *keymap;
	struct xkb_state *state;
	char *map_str;

	if (!data) {
		close(fd);
		return;
	}

	if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
		close(fd);
		return;
	}

	map_str = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	if (map_str == MAP_FAILED) {
		close(fd);
		return;
	}

	keymap = xkb_keymap_new_from_string(input->xkb_context, map_str, XKB_KEYMAP_FORMAT_TEXT_V1, 0);
	munmap(map_str, size);
	close(fd);

	if (!keymap) {
		fprintf(stderr, "failed to compile keymap\n");
		return;
	}

	state = xkb_state_new(keymap);
	if (!state) {
		fprintf(stderr, "failed to create XKB state\n");
		xkb_keymap_unref(keymap);
		return;
	}

	xkb_keymap_unref(input->xkb.keymap);
	xkb_state_unref(input->xkb.state);
	input->xkb.keymap = keymap;
	input->xkb.state = state;

	input->xkb.control_mask = 1 << xkb_keymap_mod_get_index(input->xkb.keymap, "Control");
	input->xkb.alt_mask = 1 << xkb_keymap_mod_get_index(input->xkb.keymap, "Mod1");
	input->xkb.shift_mask =	1 << xkb_keymap_mod_get_index(input->xkb.keymap, "Shift");
}

static void keyboard_handle_enter(void *data, struct wl_keyboard *keyboard, uint32_t serial,
		struct wl_surface *surface, struct wl_array *keys)
{
	UwacSeat *input = data;
	struct window *window;

#if 0
	input->display->serial = serial;
	input->keyboard_focus = wl_surface_get_user_data(surface);

	window = input->keyboard_focus;
	if (window->keyboard_focus_handler)
		(*window->keyboard_focus_handler)(window, input, window->user_data);
#endif
}

static void keyboard_handle_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial,
		struct wl_surface *surface)
{
	UwacSeat *input = data;

#if 0
	input->display->serial = serial;
	input_remove_keyboard_focus(input);
#endif
}

static void keyboard_handle_key(void *data, struct wl_keyboard *keyboard,
		    uint32_t serial, uint32_t time, uint32_t key,
		    uint32_t state_w)
{
	UwacSeat *input = data;

#if 0
	struct window *window = input->keyboard_focus;
	uint32_t code, num_syms;
	enum wl_keyboard_key_state state = state_w;
	const xkb_keysym_t *syms;
	xkb_keysym_t sym;
	struct itimerspec its;

	input->display->serial = serial;
	code = key + 8;
	if (!window || !input->xkb.state)
		return;

	/* We only use input grabs for pointer events for now, so just
	 * ignore key presses if a grab is active.  We expand the key
	 * event delivery mechanism to route events to widgets to
	 * properly handle key grabs.  In the meantime, this prevents
	 * key event devlivery while a grab is active. */
	if (input->grab && input->grab_button == 0)
		return;

	num_syms = xkb_state_key_get_syms(input->xkb.state, code, &syms);

	sym = XKB_KEY_NoSymbol;
	if (num_syms == 1)
		sym = syms[0];


	if (sym == XKB_KEY_F5 && input->modifiers == MOD_ALT_MASK) {
		if (state == WL_KEYBOARD_KEY_STATE_PRESSED)
			window_set_maximized(window, !window->maximized);
	} else if (sym == XKB_KEY_F11 &&
		   window->fullscreen_handler &&
		   state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		window->fullscreen_handler(window, window->user_data);
	} else if (sym == XKB_KEY_F4 &&
		   input->modifiers == MOD_ALT_MASK &&
		   state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		window_close(window);
	} else if (window->key_handler) {
		(*window->key_handler)(window, input, time, key,
				       sym, state, window->user_data);
	}

	if (state == WL_KEYBOARD_KEY_STATE_RELEASED && key == input->repeat_key) {
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0;
		its.it_value.tv_sec = 0;
		its.it_value.tv_nsec = 0;
		timerfd_settime(input->repeat_timer_fd, 0, &its, NULL);
	} else if (state == WL_KEYBOARD_KEY_STATE_PRESSED && xkb_keymap_key_repeats(input->xkb.keymap, code)) {
		input->repeat_sym = sym;
		input->repeat_key = key;
		input->repeat_time = time;
		its.it_interval.tv_sec = input->repeat_rate_sec;
		its.it_interval.tv_nsec = input->repeat_rate_nsec;
		its.it_value.tv_sec = input->repeat_delay_sec;
		its.it_value.tv_nsec = input->repeat_delay_nsec;
		timerfd_settime(input->repeat_timer_fd, 0, &its, NULL);
	}
#endif
}

static void keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard, uint32_t serial,
		uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
	UwacSeat *input = data;
	xkb_mod_mask_t mask;

	/* If we're not using a keymap, then we don't handle PC-style modifiers */
	if (!input->xkb.keymap)
		return;

	xkb_state_update_mask(input->xkb.state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
	mask = xkb_state_serialize_mods(input->xkb.state, XKB_STATE_MODS_DEPRESSED | XKB_STATE_MODS_LATCHED);
	input->modifiers = 0;
	if (mask & input->xkb.control_mask)
		input->modifiers |= UWAC_MOD_CONTROL_MASK;
	if (mask & input->xkb.alt_mask)
		input->modifiers |= UWAC_MOD_ALT_MASK;
	if (mask & input->xkb.shift_mask)
		input->modifiers |= UWAC_MOD_SHIFT_MASK;
}

#if 0
static void set_repeat_info(struct input *input, int32_t rate, int32_t delay)
{
	input->repeat_rate_sec = input->repeat_rate_nsec = 0;
	input->repeat_delay_sec = input->repeat_delay_nsec = 0;

	/* a rate of zero disables any repeating, regardless of the delay's
	 * value */
	if (rate == 0)
		return;

	if (rate == 1)
		input->repeat_rate_sec = 1;
	else
		input->repeat_rate_nsec = 1000000000 / rate;

	input->repeat_delay_sec = delay / 1000;
	delay -= (input->repeat_delay_sec * 1000);
	input->repeat_delay_nsec = delay * 1000 * 1000;
}
#endif

static void keyboard_handle_repeat_info(void *data, struct wl_keyboard *keyboard,
			    int32_t rate, int32_t delay)
{
	UwacSeat *input = data;

	//set_repeat_info(input, rate, delay);
}

static const struct wl_keyboard_listener keyboard_listener = {
	keyboard_handle_keymap,
	keyboard_handle_enter,
	keyboard_handle_leave,
	keyboard_handle_key,
	keyboard_handle_modifiers,
	keyboard_handle_repeat_info
};

static void touch_handle_down(void *data, struct wl_touch *wl_touch,
		  uint32_t serial, uint32_t time, struct wl_surface *surface,
		  int32_t id, wl_fixed_t x_w, wl_fixed_t y_w)
{
	UwacSeat *input = data;
#if 0
	struct widget *widget;
	float sx = wl_fixed_to_double(x_w);
	float sy = wl_fixed_to_double(y_w);

	input->display->serial = serial;
	input->touch_focus = wl_surface_get_user_data(surface);
	if (!input->touch_focus) {
		DBG("Failed to find to touch focus for surface %p\n", surface);
		return;
	}

	if (surface != input->touch_focus->main_surface->surface) {
		DBG("Ignoring input event from subsurface %p\n", surface);
		input->touch_focus = NULL;
		return;
	}

	if (input->grab)
		widget = input->grab;
	else
		widget = window_find_widget(input->touch_focus,
					    wl_fixed_to_double(x_w),
					    wl_fixed_to_double(y_w));
	if (widget) {
		struct touch_point *tp = xmalloc(sizeof *tp);
		if (tp) {
			tp->id = id;
			tp->widget = widget;
			tp->x = sx;
			tp->y = sy;
			wl_list_insert(&input->touch_point_list, &tp->link);

			if (widget->touch_down_handler)
				(*widget->touch_down_handler)(widget, input,
							      serial, time, id,
							      sx, sy,
							      widget->user_data);
		}
	}
#endif
}

static void touch_handle_up(void *data, struct wl_touch *wl_touch,
		uint32_t serial, uint32_t time, int32_t id)
{
	UwacSeat *input = data;
#if 0
	struct touch_point *tp, *tmp;

	if (!input->touch_focus) {
		DBG("No touch focus found for touch up event!\n");
		return;
	}

	wl_list_for_each_safe(tp, tmp, &input->touch_point_list, link) {
		if (tp->id != id)
			continue;

		if (tp->widget->touch_up_handler)
			(*tp->widget->touch_up_handler)(tp->widget, input, serial,
							time, id,
							tp->widget->user_data);

		wl_list_remove(&tp->link);
		free(tp);

		return;
	}
#endif
}

static void touch_handle_motion(void *data, struct wl_touch *wl_touch,
		    uint32_t time, int32_t id, wl_fixed_t x_w, wl_fixed_t y_w)
{
	UwacSeat *input = data;
#if 0
	struct touch_point *tp;
	float sx = wl_fixed_to_double(x_w);
	float sy = wl_fixed_to_double(y_w);

	DBG("touch_handle_motion: %i %i\n", id, wl_list_length(&input->touch_point_list));

	if (!input->touch_focus) {
		DBG("No touch focus found for touch motion event!\n");
		return;
	}

	wl_list_for_each(tp, &input->touch_point_list, link) {
		if (tp->id != id)
			continue;

		tp->x = sx;
		tp->y = sy;
		if (tp->widget->touch_motion_handler)
			(*tp->widget->touch_motion_handler)(tp->widget, input, time,
							    id, sx, sy,
							    tp->widget->user_data);
		return;
	}
#endif
}

static void touch_handle_frame(void *data, struct wl_touch *wl_touch)
{
	UwacSeat *input = data;
#if 0
	struct touch_point *tp, *tmp;

	DBG("touch_handle_frame\n");

	if (!input->touch_focus) {
		DBG("No touch focus found for touch frame event!\n");
		return;
	}

	wl_list_for_each_safe(tp, tmp, &input->touch_point_list, link) {
		if (tp->widget->touch_frame_handler)
			(*tp->widget->touch_frame_handler)(tp->widget, input,
							   tp->widget->user_data);
	}
#endif
}

static void touch_handle_cancel(void *data, struct wl_touch *wl_touch)
{
	UwacSeat *input = data;
#if 0
	struct touch_point *tp, *tmp;

	DBG("touch_handle_cancel\n");

	if (!input->touch_focus) {
		DBG("No touch focus found for touch cancel event!\n");
		return;
	}

	wl_list_for_each_safe(tp, tmp, &input->touch_point_list, link) {
		if (tp->widget->touch_cancel_handler)
			(*tp->widget->touch_cancel_handler)(tp->widget, input,
							    tp->widget->user_data);

		wl_list_remove(&tp->link);
		free(tp);
	}
#endif
}

static const struct wl_touch_listener touch_listener = {
	touch_handle_down,
	touch_handle_up,
	touch_handle_motion,
	touch_handle_frame,
	touch_handle_cancel,
};


static void pointer_handle_enter(void *data, struct wl_pointer *pointer, uint32_t serial,
		struct wl_surface *surface, wl_fixed_t sx_w, wl_fixed_t sy_w)
{
	UwacSeat *input = data;
	//struct window *window;
	//struct widget *widget;
	float sx = wl_fixed_to_double(sx_w);
	float sy = wl_fixed_to_double(sy_w);

	if (!surface) {
		/* enter event for a window we've just destroyed */
		return;
	}

#if 0
	window = wl_surface_get_user_data(surface);
	if (surface != window->main_surface->surface) {
		DBG("Ignoring input event from subsurface %p\n", surface);
		return;
	}

	input->display->serial = serial;
	input->pointer_enter_serial = serial;
	input->pointer_focus = window;

	input->sx = sx;
	input->sy = sy;

	widget = window_find_widget(window, sx, sy);
	input_set_focus_widget(input, widget, sx, sy);
#endif
}

static void pointer_handle_leave(void *data, struct wl_pointer *pointer, uint32_t serial,
		struct wl_surface *surface)
{
	UwacSeat *input = data;

#if 0
	input->display->serial = serial;
	input_remove_pointer_focus(input);
#endif
}

static void pointer_handle_motion(void *data, struct wl_pointer *pointer, uint32_t time,
		wl_fixed_t sx_w, wl_fixed_t sy_w)
{
	UwacSeat *input = data;
#if 0
	struct window *window = input->pointer_focus;
	struct widget *widget;
	int cursor;
	float sx = wl_fixed_to_double(sx_w);
	float sy = wl_fixed_to_double(sy_w);

	if (!window)
		return;

	input->sx = sx;
	input->sy = sy;

	/* when making the window smaller - e.g. after a unmaximise we might
	 * still have a pending motion event that the compositor has picked
	 * based on the old surface dimensions
	 */
	if (sx > window->main_surface->allocation.width ||
	    sy > window->main_surface->allocation.height)
		return;

	if (!(input->grab && input->grab_button)) {
		widget = window_find_widget(window, sx, sy);
		input_set_focus_widget(input, widget, sx, sy);
	}

	if (input->grab)
		widget = input->grab;
	else
		widget = input->focus_widget;
	if (widget) {
		if (widget->motion_handler)
			cursor = widget->motion_handler(input->focus_widget,
							input, time, sx, sy,
							widget->user_data);
		else
			cursor = widget->default_cursor;
	} else
		cursor = CURSOR_LEFT_PTR;

	input_set_pointer_image(input, cursor);
#endif
}

static void pointer_handle_button(void *data, struct wl_pointer *pointer, uint32_t serial,
		      uint32_t time, uint32_t button, uint32_t state_w)
{
	UwacSeat *input = data;
	//struct widget *widget;
	enum wl_pointer_button_state state = state_w;

#if 0
	input->display->serial = serial;
	if (input->focus_widget && input->grab == NULL &&
	    state == WL_POINTER_BUTTON_STATE_PRESSED)
		input_grab(input, input->focus_widget, button);

	widget = input->grab;
	if (widget && widget->button_handler)
		(*widget->button_handler)(widget,
					  input, time,
					  button, state,
					  input->grab->user_data);

	if (input->grab && input->grab_button == button &&
	    state == WL_POINTER_BUTTON_STATE_RELEASED)
		input_ungrab(input);
#endif
}

static void pointer_handle_axis(void *data, struct wl_pointer *pointer, uint32_t time,
		uint32_t axis, wl_fixed_t value)
{
	UwacSeat *input = data;
#if 0
	struct widget *widget;

	widget = input->focus_widget;
	if (input->grab)
		widget = input->grab;
	if (widget && widget->axis_handler)
		(*widget->axis_handler)(widget,
					input, time,
					axis, value,
					widget->user_data);
#endif
}

static const struct wl_pointer_listener pointer_listener = {
	pointer_handle_enter,
	pointer_handle_leave,
	pointer_handle_motion,
	pointer_handle_button,
	pointer_handle_axis,
};



static void seat_handle_capabilities(void *data, struct wl_seat *seat, enum wl_seat_capability caps)
{
	UwacSeat *input = data;

	if ((caps & WL_SEAT_CAPABILITY_POINTER) && !input->pointer) {
		input->pointer = wl_seat_get_pointer(seat);
		wl_pointer_set_user_data(input->pointer, input);
		wl_pointer_add_listener(input->pointer, &pointer_listener, input);
	} else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && input->pointer) {
		wl_pointer_destroy(input->pointer);
		input->pointer = NULL;
	}

	if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !input->keyboard) {
		input->keyboard = wl_seat_get_keyboard(seat);
		wl_keyboard_set_user_data(input->keyboard, input);
		wl_keyboard_add_listener(input->keyboard, &keyboard_listener, input);
	} else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && input->keyboard) {
		wl_keyboard_destroy(input->keyboard);
		input->keyboard = NULL;
	}

	if ((caps & WL_SEAT_CAPABILITY_TOUCH) && !input->touch) {
		input->touch = wl_seat_get_touch(seat);
		wl_touch_set_user_data(input->touch, input);
		wl_touch_add_listener(input->touch, &touch_listener, input);
	} else if (!(caps & WL_SEAT_CAPABILITY_TOUCH) && input->touch) {
		wl_touch_destroy(input->touch);
		input->touch = NULL;
	}
}

static void
seat_handle_name(void *data, struct wl_seat *seat,
		 const char *name)
{

}

static const struct wl_seat_listener seat_listener = {
	seat_handle_capabilities,
	seat_handle_name
};


UwacSeat *UwacSeatNew(UwacDisplay *d, uint32_t id, uint32_t version) {
	UwacSeat *ret;

	ret = zalloc(sizeof(UwacSeat));
	ret->display = d;
	ret->xkb_context = xkb_context_new(0);
	if (!ret->xkb_context) {
		fprintf(stderr, "%s: unable to allocate a xkb_context\n", __FUNCTION__);
		goto out_free;
	}

	ret->seat = wl_registry_bind(d->registry, id, &wl_seat_interface, version);
	wl_seat_add_listener(ret->seat, &seat_listener, ret);
	wl_seat_set_user_data(ret->seat, ret);


	wl_list_insert(d->seats.prev, &ret->link);
	return ret;

out_free:
	free(ret);
	return NULL;
}

void UwacSeatDestroy(UwacSeat *s) {
	wl_seat_destroy(s->seat);
	xkb_context_unref(s->xkb_context);
	wl_list_remove(&s->link);
	free(s);
}
