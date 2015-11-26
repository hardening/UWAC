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
#include <stdio.h>
#include "uwac.h"

int main(int argc, char *argv[]) {
	UwacDisplay *display;
	UwacOutput *output;
	UwacReturnCode err;
	int ret, i;
	uint32_t val;
	uint32_t version;
	const char *interface_names[] = {
			"wl_compositor",
			"wl_output",
			"wl_seat",
			NULL
	};

	ret = 1;
	display = UwacOpenDisplay(NULL, &err);
	if (!display) {
		fprintf(stderr, "error opening display: %s\n", UwacErrorString(err));
		goto out;
	}

	/* let's query some interfaces, we're expecting to be there */
	for (i = 0; interface_names[i]; i++) {
		err = UwacDisplayQueryInterfaceVersion(display, interface_names[i], &version);
		if (err != UWAC_SUCCESS) {
			fprintf(stderr, "error retrieving interface version for %s: %s\n", interface_names[i], UwacErrorString(err));
			goto out;
		}
	}

	/* checking we can retrieve an output */
	val = UwacDisplayGetNbOutputs(display);
	if (!val)
		fprintf(stderr, "WARNING: strange, there's no output\n");

	output = UwacDisplayGetOutput(display, 0);
	if (!output) {
		err = UwacDisplayGetLastError(display);
		fprintf(stderr, "error retrieving output 0, error=%s\n", UwacErrorString(err));
		goto out;
	}

	val = UwacDisplayQueryGetNbShmFormats(display);
	if (val < 2) {
		fprintf(stderr, "too few SHM formats (val=%d), error=%s\n", val,
				UwacErrorString( UwacDisplayGetLastError(display) )
		);
		goto out;
	}

	UwacCloseDisplay(&display);
	ret = 0;

out:
	return ret;
}
