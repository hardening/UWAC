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
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <sys/mman.h>


static void UwacSubmitBufferPtr(UwacWindow *window, UwacBuffer *buffer);

static int bppFromShmFormat(enum wl_shm_format format) {
	switch (format) {
	case WL_SHM_FORMAT_ARGB8888:
	case WL_SHM_FORMAT_XRGB8888:
	default:
		return 4;
	}
}


static void buffer_release(void *data, struct wl_buffer *buffer) {
	UwacWindow *window = (UwacWindow *)data;
	UwacBuffer *submittedBuffer = &window->buffers[window->submittedBuffer];
	UwacBuffer *pendingBuffer = &window->buffers[window->pendingBuffer];

	window->submittedBuffer = (window->submittedBuffer + 1) % UWAC_N_BUFFERING;
	if (window->pendingBuffer != window->drawingBuffer) {
		UwacSubmitBufferPtr(window, pendingBuffer);
		window->submittedBuffer = window->pendingBuffer;
		window->pendingBuffer = (window->pendingBuffer + 1) % UWAC_N_BUFFERING;
	}
}

static const struct wl_buffer_listener buffer_listener = {
	buffer_release
};

static void handle_configure(void *data, struct xdg_surface *surface,
		 int32_t width, int32_t height,
		 struct wl_array *states, uint32_t serial)
{
	xdg_surface_ack_configure(surface, serial);
}

static void handle_delete(void *data, struct xdg_surface *xdg_surface)
{
	//running = 0;
}

static const struct xdg_surface_listener xdg_surface_listener = {
	handle_configure,
	handle_delete,
};


UwacWindow *UwacCreateWindowShm(UwacDisplay *display, uint32_t width, uint32_t height, enum wl_shm_format format) {
	UwacWindow *w;
	char *tmpname;
	int allocSize, fd, i;

	if (!display) {
		display->last_error = UWAC_ERROR_INVALID_DISPLAY;
		return NULL;
	}

	w = zalloc(sizeof(*w));
	if (!w) {
		display->last_error = UWAC_ERROR_NOMEMORY;
		goto out_error_close;
	}

	w->diplay = display;
	w->width = width;
	w->height = height;
	w->stride = width * bppFromShmFormat(format);
	allocSize = w->stride * height;

	w->shm.fd = fd = uwac_create_anonymous_file(allocSize * UWAC_N_BUFFERING);
	if (fd < 0) {
		display->last_error = UWAC_ERROR_INTERNAL;
		goto out_error_free;
	}

	w->shm.shm_data = mmap(NULL, allocSize * UWAC_N_BUFFERING, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (w->shm.shm_data == MAP_FAILED)	{
		display->last_error = UWAC_ERROR_NOMEMORY;
		goto out_error_close;
	}

	w->shm.shm_pool = wl_shm_create_pool(display->shm, w->shm.fd, allocSize * UWAC_N_BUFFERING);

	for (i = 0; i < UWAC_N_BUFFERING; i++) {
		UwacBuffer *buffer = &w->buffers[i];

		pixman_region32_init(&buffer->damage);
		buffer->data = w->shm.shm_data + (allocSize * i);

		buffer->wayland_buffer = wl_shm_pool_create_buffer(w->shm.shm_pool, allocSize * i, width, height, w->stride, format);
		wl_buffer_add_listener(w->buffers[i].wayland_buffer, &buffer_listener, w);
	}

	w->surface = wl_compositor_create_surface(display->compositor);
	wl_surface_set_user_data(w->surface, w);
	if (display->xdg_shell) {
		w->xdg_surface = xdg_shell_get_xdg_surface(display->xdg_shell, w->surface);

		assert(w->xdg_surface);

		xdg_surface_add_listener(w->xdg_surface, &xdg_surface_listener, w);
		xdg_surface_set_title(w->xdg_surface, "simple-shm");
	}

	wl_shm_pool_destroy(w->shm.shm_pool);
	close(fd);

	wl_list_insert(display->windows.prev, &w->link);


	display->last_error = UWAC_SUCCESS;
	return w;

out_error_close:
	close(fd);
	shm_unlink(tmpname);
out_error_free:
	free(w);
	return NULL;
}


int UwacDestroyWindow(UwacWindow *w) {
	int i;

	for (i = 0; i < UWAC_N_BUFFERING; i++) {
		UwacBuffer *buffer = &w->buffers[i];

		pixman_region32_fini(&buffer->damage);
		wl_buffer_destroy(buffer->wayland_buffer);
	}

	if (w->frame_callback)
		wl_callback_destroy(w->frame_callback);

	if (w->xdg_surface)
		xdg_surface_destroy(w->xdg_surface);

	wl_surface_destroy(w->surface);
	wl_list_remove(&w->link);
	free(w);

	return UWAC_SUCCESS;
}

void *UwacWindowGetDrawingBuffer(UwacWindow *window) {
	return window->buffers[window->drawingBuffer].data;
}

static void frame_done_cb(void *data, struct wl_callback *callback, uint32_t time);

static const struct wl_callback_listener frame_listener = {
		frame_done_cb
};


static void UwacSubmitBufferPtr(UwacWindow *window, UwacBuffer *buffer) {
	const pixman_box32_t *box;
	int nrects, i;

	wl_surface_attach(window->surface, buffer->wayland_buffer, 0, 0);

	box = pixman_region32_rectangles(&buffer->damage, &nrects);
	for (i = 0; i < nrects; i++, box++)
		wl_surface_damage(window->surface, box->x1, box->y1, (box->x2 - box->x1), (box->y2 - box->y1));

	if (window->frame_callback)
		wl_callback_destroy(window->frame_callback);

	window->frame_callback = wl_surface_frame(window->surface);
	wl_callback_add_listener(window->frame_callback, &frame_listener, window);

	wl_surface_commit(window->surface);
	pixman_region32_clear(&buffer->damage);
}


static void frame_done_cb(void *data, struct wl_callback *callback, uint32_t time) {
	UwacWindow *window = (UwacWindow *)data;

	UwacFrameDoneEvent *event = (UwacFrameDoneEvent *)UwacDisplayNewEvent(window->diplay);
	event->type = UWAC_EVENT_FRAME_DONE;
	event->window = window;
}


int UwacWindowAddDamage(UwacWindow *window, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
	UwacBuffer *buffer;

	buffer = &window->buffers[window->drawingBuffer];

	if(!pixman_region32_union_rect(&buffer->damage, &buffer->damage, x, y, width, height))
		return UWAC_ERROR_INTERNAL;

	return UWAC_SUCCESS;
}


int UwacWindowSubmitBuffer(UwacWindow *window, bool copyContentForNextFrame) {
	UwacBuffer *drawingBuffer = &window->buffers[window->drawingBuffer];
	pixman_box32_t box;

	if (window->pendingBuffer != window->drawingBuffer) {
		/* we already have a pending frame, don't do anything*/
		return UWAC_SUCCESS;
	}

	if (window->submittedBuffer != window->pendingBuffer) {
		/* create a pending frame */
		window->drawingBuffer = (window->drawingBuffer + 1) % UWAC_N_BUFFERING;
		return UWAC_SUCCESS;
	}

	window->submittedBuffer = window->drawingBuffer;
	window->pendingBuffer = window->drawingBuffer = (window->drawingBuffer + 1) % UWAC_N_BUFFERING;
	if (copyContentForNextFrame) {
		UwacBuffer *drawingBuffer = &window->buffers[window->drawingBuffer];
		memcpy(drawingBuffer->data, drawingBuffer->data, window->stride * window->height);
	}

	UwacSubmitBufferPtr(window, drawingBuffer);

	return UWAC_SUCCESS;
}

