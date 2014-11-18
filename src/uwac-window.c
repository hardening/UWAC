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

UwacWindow *UwacCreateWindowShm(UwacDisplay *display, uint32_t width, uint32_t height, enum wl_shm_format format) {
	UwacWindow *w;

	if (!display) {
		display->last_error = UWAC_ERROR_INVALID_DISPLAY;
		return NULL;
	}

	w = zalloc(sizeof(*w));
	if (!w) {
		display->last_error = UWAC_ERROR_NOMEMORY;
		return NULL;
	}

	wl_list_insert(display->windows.prev, &w->link);
	display->last_error = UWAC_SUCCESS;
	return w;
}


int UwacDestroyWindow(UwacWindow *w) {
	wl_list_remove(&w->link);
	free(w);

	return UWAC_SUCCESS;
}
