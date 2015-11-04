/*
 * Copyright © 2014-2015 David FORT <contact@hardening-consulting.com>
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

#ifndef __UWAC_H_
#define __UWAC_H_

#include <wayland-client.h>
#include <xdg-shell-client-protocol.h>
#include <stdbool.h>

typedef struct uwac_size UwacSize;
typedef struct uwac_display UwacDisplay;
typedef struct uwac_output UwacOutput;
typedef struct uwac_window UwacWindow;
typedef struct uwac_seat UwacSeat;


/** @brief error codes */
enum {
	UWAC_SUCCESS = 0,
	UWAC_ERROR_NOMEMORY,
	UWAC_ERROR_UNABLE_TO_CONNECT,
	UWAC_ERROR_INVALID_DISPLAY,
	UWAC_NOT_ENOUGH_RESOURCES,
	UWAC_TIMEDOUT,
	UWAC_NOT_FOUND,
	UWAC_ERROR_CLOSED,
	UWAC_ERROR_INTERNAL,

	UWAC_ERROR_LAST,
};

/** @brief input modifiers */
enum {
	UWAC_MOD_SHIFT_MASK	= 0x01,
	UWAC_MOD_ALT_MASK = 0x02,
	UWAC_MOD_CONTROL_MASK = 0x04,
};

/** @brief a rectangle size measure */
struct uwac_size {
	int width;
	int height;
};


/** @brief event types */
enum {
	UWAC_EVENT_NEW_SEAT = 0,
	UWAC_EVENT_REMOVED_SEAT,
	UWAC_EVENT_NEW_OUTPUT,
	UWAC_EVENT_CONFIGURE,
	UWAC_EVENT_POINTER_ENTER,
	UWAC_EVENT_POINTER_LEAVE,
	UWAC_EVENT_POINTER_MOTION,
	UWAC_EVENT_POINTER_BUTTONS,
	UWAC_EVENT_POINTER_AXIS,
	UWAC_EVENT_KEY,
	UWAC_EVENT_TOUCH_FRAME_BEGIN,
	UWAC_EVENT_TOUCH_UP,
	UWAC_EVENT_TOUCH_DOWN,
	UWAC_EVENT_TOUCH_MOTION,
	UWAC_EVENT_TOUCH_CANCEL,
	UWAC_EVENT_TOUCH_FRAME_END,
	UWAC_EVENT_FRAME_DONE
};

struct uwac_new_output_event {
	int type;
	UwacOutput *output;
};
typedef struct uwac_new_output_event UwacOutputNewEvent;

struct uwac_new_seat_event {
	int type;
	UwacSeat *seat;
};
typedef struct uwac_new_seat_event UwacSeatNewEvent;

typedef struct uwac_new_seat_event UwacSeatRemovedEvent;

struct uwac_pointer_enter_event {
	int type;
	UwacWindow *window;
	UwacSeat *seat;
};
typedef struct uwac_pointer_enter_event UwacPointerEnterLeaveEvent;

struct uwac_pointer_motion_event {
	int type;
	UwacWindow *window;
	UwacSeat *seat;
	uint32_t x, y;
};
typedef struct uwac_pointer_motion_event UwacPointerMotionEvent;

struct uwac_pointer_button_event {
	int type;
	UwacWindow *window;
	UwacSeat *seat;
	wl_fixed_t x, y;
	uint32_t button;
	enum wl_pointer_button_state state;
};
typedef struct uwac_pointer_button_event UwacPointerButtonEvent;

struct uwac_pointer_axis_event {
	int type;
	UwacWindow *window;
	UwacSeat *seat;
	uint32_t x, y;
	uint32_t axis;
	wl_fixed_t value;
};
typedef struct uwac_pointer_axis_event UwacPointerAxisEvent;

struct uwac_touch_frame_event {
	int type;
	UwacWindow *window;
	UwacSeat *seat;
};
typedef struct uwac_touch_frame_event UwacTouchFrameBegin;
typedef struct uwac_touch_frame_event UwacTouchFrameEnd;
typedef struct uwac_touch_frame_event UwacTouchCancel;

struct uwac_touch_data {
	int type;
	UwacWindow *window;
	UwacSeat *seat;
	int32_t id;
	wl_fixed_t x;
	wl_fixed_t y;
};
typedef struct uwac_touch_data UwacTouchUp;
typedef struct uwac_touch_data UwacTouchDown;
typedef struct uwac_touch_data UwacTouchMotion;

struct uwac_frame_done_event {
	int type;
	UwacWindow *window;
};
typedef struct uwac_frame_done_event UwacFrameDoneEvent;

struct uwac_key_event {
	int type;
	UwacWindow *window;
	uint32_t sym;
	bool pressed;
};
typedef struct uwac_key_event UwacKeyEvent;


/** @brief */
struct uwac_event {
	union {
		int type;
		UwacOutputNewEvent output_new;
		UwacSeatNewEvent seat_new;
		UwacPointerEnterLeaveEvent mouse_enter_leave;
		UwacPointerMotionEvent mouse_motion;
		UwacKeyEvent key;
		UwacTouchFrameBegin touchFrameBegin;
		UwacTouchUp touchUp;
		UwacTouchDown touchDown;
		UwacTouchMotion touchMotion;
		UwacTouchFrameEnd touchFrameEnd;
		UwacTouchCancel touchCancel;
		UwacFrameDoneEvent frame_done;
	};

};
typedef struct uwac_event UwacEvent;

typedef bool (*UwacErrorHandler)(UwacDisplay *d, int code, const char *msg, ...);

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * @param handler
 */
void UwacInstallErrorHandler(UwacErrorHandler handler);


/**
 *
 * @param name
 * @return
 */
UwacDisplay *UwacOpenDisplay(const char *name, int *err);

/**
 *
 * @param display the display to close
 * @return 0 if the operation was successful, -1 otherwise
 */
int UwacCloseDisplay(UwacDisplay *display);


/**
 *
 * @param error
 * @return
 */
const char *UwacErrorString(int error);

/**
 * returns the last error that occurred on a display
 *
 * @param display the display
 * @return the last error that have been set for this display
 */
int UwacDisplayGetLastError(const UwacDisplay *display);

/**
 * retrieves the version of a given interface
 *
 * @param display the display connection
 * @param name the name of the interface
 * @param version the output variable for the version
 * @return UWAC_SUCCESS if the interface was found, UWAC_NOT_FOUND otherwise
 */
int UwacDisplayQueryInterfaceVersion(const UwacDisplay *display, const char *name, uint32_t *version);

/**
 *
 * @param display
 * @return the number of SHM formats supported
 */
uint32_t UwacDisplayQueryGetNbShmFormats(UwacDisplay *display);

/**
 *
 * @param display
 * @param formats
 * @param formats_size
 * @param filled
 * @return
 */
int UwacDisplayQueryShmFormats(const UwacDisplay *display, enum wl_shm_format *formats, int formats_size, int *filled);

/**
 *	returns the number of registered outputs
 *
 * @param display the display to query
 * @return the number of outputs
 */
uint32_t UwacDisplayGetNbOutputs(UwacDisplay *display);

/**
 *	retrieve a particular output
 *
 * @param display the display to query
 * @param index index of the output
 * @return the given output, NULL if something failed (so you should query UwacDisplayGetLastError() to have the reason)
 */
UwacOutput *UwacDisplayGetOutput(UwacDisplay *display, int index);

/**
 *
 * @param output
 * @param resolution
 * @return
 */
int UwacOutputGetResolution(UwacOutput *output, UwacSize *resolution);


/**
 *	creates a window using a SHM surface
 *
 * @param display the display to attach the window to
 * @param width the width of the window
 * @param height the heigh of the window
 * @param format format to use for the SHM surface
 * @return the created UwacWindow, NULL if something failed (use UwacDisplayGetLastError() to know more about this)
 */
UwacWindow *UwacCreateWindowShm(UwacDisplay *display, uint32_t width, uint32_t height, enum wl_shm_format format);

/**
 *	destroys a window
 *
 * @param window the window to destroy
 * @return if the operation completed successfully
 */
int UwacDestroyWindow(UwacWindow *window);

/**
 *	retrieves a pointer on the current window content to draw a frame
 * @param window the UwacWindow
 * @return a pointer on the current window content
 */
void *UwacWindowGetDrawingBuffer(UwacWindow *window);

/**
 *	sets a rectangle as dirty for the next frame of a window
 *
 * @param window the UwacWindow
 * @param x left coordinate
 * @param y top coordinate
 * @param width the width of the dirty rectangle
 * @param height the height of the dirty rectangle
 * @return UWAC_SUCCESS on sucess, an Uwac error otherwise
 */
int UwacWindowAddDamage(UwacWindow *window, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

/**
 *
 * @param window
 * @param copyContentForNextFrame
 * @return
 */
int UwacWindowSubmitBuffer(UwacWindow *window, bool copyContentForNextFrame);


/** Waits until an event occurs, and when it's there copy the event from the queue to
 * event.
 *
 * @param display the Uwac display
 * @param event the event to fill
 * @return if the operation completed successfully
 */
int UwacNextEvent(UwacDisplay *display, UwacEvent *event);


/**
 * @param seat
 * @return the name of the seat
 */
const char *UwacSeatGetName(const UwacSeat *seat);

#ifdef __cplusplus
}
#endif

#endif /* __UWAC_H_ */
