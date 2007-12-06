/*
 *  OpenMoko Media Player
 *   http://openmoko.org/
 *
 *  Copyright (C) 2007 by Soeren Apel (abraxa@dar-clan.de)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/**
 * @file mplayer_playback.c
 * Playback engine interface for utilizing mplayer
 */

#include <fcntl.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <linux/vt.h>
#include <linux/kd.h>
#include <linux/input.h>
#include <tslib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "framebuffer.h"
#include "main.h"
#include "main_page.h"
#include "mplayer_playback.h"
#include "mplayer_startup_img.h"
#include "persistent.h"
#include "playback.h"
#include "utils.h"

// Originally defined in linux/stat.h
#define S_IFIFO  0010000

// If set to 1 the media player will make sure we're using a sane fbset
#define OMP_MPLAYBACK_FBSET_CHECK 0

// ttyX that X11 uses
#define OMP_MPLAYBACK_X_CONSOLE 3

// ttyX that we will let mplayer use for framebuffer access
#define OMP_MPLAYBACK_FB_CONSOLE 4

// LCM driver state file
#define OMP_MPLAYBACK_LCM_STATE_FILE "/sys/devices/platform/s3c24xx-spi-gpio.1/spi0.0/state"

// Named pipe to use for mplayer interface
#define OMP_MPLAYBACK_PIPE_NAME "/tmp/mplayer_pipe"

// Command lines used to run mplayer in X11 and framebuffer modes
#define OMP_MPLAYBACK_MPLAYER_BINARY "/usr/bin/mplayer"
#define OMP_MPLAYBACK_X_CMDLINE "-slave -input file=%s -really-quiet -framedrop -noslices -lavdopts fast -pp 0 -sws 0 -wid %d -zoom %s"
#define OMP_MPLAYBACK_FB_CMDLINE "-slave -input file=%s -really-quiet -framedrop -noslices -lavdopts fast -pp 0 -sws 0 -vo fbdev -vf rotate=2 %s"
// -zoom -vf scale=320:-3,rotate=2
// -idle

// This is the amount the cursor must have moved in either direction to be considered moving
// If we don't do this then it will constantly trigger gesture recognition due to jitter on the touchscreen
#define OMP_MP_MIN_CURSOR_DELTA 3


/// Is set if fbset is usable for our purposes
gboolean omp_mplayback_fbset_usable = FALSE;

/// URI of currently loaded resource
gchar *omp_mplayback_uri = NULL;

/// Status of mplayer playback engine
gboolean omp_mplayback_playing = FALSE;

/// Pipe handle used to communicate with the mplayer slave instance
gint omp_mplayback_pipe = -1;

/// Flag stating whether mplayer was already sent a "quit" command
gboolean omp_mplayback_terminating;

/// Flag stating whether we're windowed (X11) or in fullscreen (FB)
gboolean omp_mplayback_windowed;

/// File handle of the touchscreen input device
GPollFD omp_mplayback_ts_fd;

/// Touchscreen device handle for use with tslib
struct tsdev *omp_mplayback_ts_dev = NULL;

/// This source notifies us when the touchscreen received input
GSource *omp_mplayback_ts_source = NULL;

/// The overlay window's handle
GdkWindow *omp_mplayback_overlay_window = NULL;


/// Possible gestures
typedef enum
{
	OMP_MP_GESTURE_NONE,
	OMP_MP_GESTURE_LEFT,
	OMP_MP_GESTURE_RIGHT,
	OMP_MP_GESTURE_UP,
	OMP_MP_GESTURE_DOWN,
	OMP_MP_GESTURE_TAP,
} omp_mp_gesture;

/// Holds the necessary infos to record and identify the gestures
struct
{
	gboolean pressed;
	GTimeVal start_time;
	guint x_origin, y_origin;
	guint last_x, last_y;
	gboolean cursor_idle;        // TRUE if cursor isn't moving

	gint radius, angle;
	omp_mp_gesture gesture;
	gboolean repeating;
	guint repeat_timeout;
} mp_gesture_data;


// Some forward declarations for internal use
void omp_mplayback_mplayer_terminate();
void omp_mplayback_pause();


/*************************************************************************************************/
/*************************************** Gesture recognition *************************************/
/*************************************************************************************************/

/**
 * Tries to determine which gesture the user has performed and fills the gesture data struct accordingly
 * @param x X coordinate of current pressure point
 * @param y Y coordinate of current pressure point
 */
void
omp_mplayback_gesture_identify(guint x, guint y)
{
	gint delta_x, delta_y, gamma;

	// Perform rect->polar conversion of the differential cursor movement
	delta_x = x - mp_gesture_data.x_origin;
	delta_y = y - mp_gesture_data.y_origin;

	mp_gesture_data.radius = approx_radius(delta_x, delta_y);

	// angle = arccos(gamma) but arccos() is too slow to compute -> range comparison
	// We shift the comma by 3 digits so we can use integer math
	gamma = delta_x*1000 / mp_gesture_data.radius;

	if (mp_gesture_data.radius > omp_config_get_min_gesture_radius())
	{

		// Determine direction of movement
		if (gamma < -707)
		{
			mp_gesture_data.gesture = OMP_MP_GESTURE_LEFT;

		} else {

			if (gamma > 707)
			{
				mp_gesture_data.gesture = OMP_MP_GESTURE_RIGHT;
			} else {
				mp_gesture_data.gesture = (delta_y < 0) ? OMP_MP_GESTURE_UP : OMP_MP_GESTURE_DOWN;
			}

		}

	} else {

		// Radius too small
		mp_gesture_data.gesture = OMP_MP_GESTURE_TAP;
	}

g_printf("G:%d\t", mp_gesture_data.gesture);
}

/**
 * Performs the action the current gesture commands
 */
void
omp_mplayback_gesture_trigger()
{
	if (mp_gesture_data.gesture == OMP_MP_GESTURE_NONE) return;

	switch (mp_gesture_data.gesture)
	{
		// The gestures are rotated by 90 degrees!
		case OMP_MP_GESTURE_DOWN:
			write(omp_mplayback_pipe, "seek -10\n", 9);
			break;

		case OMP_MP_GESTURE_UP:
			write(omp_mplayback_pipe, "seek 10\n", 8);
			break;

		case OMP_MP_GESTURE_LEFT:
			omp_playback_set_volume(min(100, omp_playback_get_volume()+10));
			break;

		case OMP_MP_GESTURE_RIGHT:
			omp_playback_set_volume(max(0, omp_playback_get_volume()-10));
			break;

		case OMP_MP_GESTURE_TAP:
			omp_mplayback_mplayer_terminate();

/*
			if (omp_mplayback_playing)
				omp_mplayback_pause();
			else
				omp_mplayback_play();
*/
			break;

		default: break;
	}
}

/**
 * This callback repeatedly performs the action the current gesture commands
 */
static gboolean
omp_mplayback_gesture_repeat_callback(gpointer data)
{
	if (!mp_gesture_data.repeating) return FALSE;

	omp_mplayback_gesture_trigger();

	return TRUE;
}

/**
 * Sets up the repeat timeout if the touchscreen is being pressed for a certain amount of time
 */
void
omp_mplayback_gesture_check_repeat()
{
	GTimeVal current_time, delta_t;

	if (!mp_gesture_data.repeating)
	{
		// Calculate duration of touchscreen press
		g_get_current_time(&current_time);
		delta_t.tv_sec  = current_time.tv_sec  - mp_gesture_data.start_time.tv_sec;
		delta_t.tv_usec = current_time.tv_usec - mp_gesture_data.start_time.tv_usec;

		if (delta_t.tv_usec >= omp_config_get_gesture_repeat_tresh()*1000)
		{
			mp_gesture_data.repeating = TRUE;
			g_timeout_add(omp_config_get_gesture_repeat_intv(), omp_mplayback_gesture_repeat_callback, NULL);
		}

	}
}

/**
 * Gets called whenever the cursor has been moved, handling gesture recognition and triggering
 */
void
omp_mplayback_pointer_moved(guint x, guint y)
{
	gint delta_last_x, delta_last_y;

	if (mp_gesture_data.pressed)
	{
		delta_last_x = abs(x - mp_gesture_data.last_x);
		delta_last_y = abs(y - mp_gesture_data.last_y);

		// Did the cursor move a substantial amount?
		if ( (delta_last_x > OMP_MP_MIN_CURSOR_DELTA) && (delta_last_y > OMP_MP_MIN_CURSOR_DELTA) )
		{
			// Yes it did, so it's most likely being moved
			mp_gesture_data.cursor_idle = FALSE;
			mp_gesture_data.last_x = x;
			mp_gesture_data.last_y = y;

			// Make sure we won't trigger anymore (if we were before, that is)
			mp_gesture_data.repeating = FALSE;

		} else {

			// Cursor is idle, so lets update the gesture data if it wasn't idle before
			if (!mp_gesture_data.cursor_idle)
				omp_mplayback_gesture_identify(x, y);

			omp_mplayback_gesture_check_repeat();

			mp_gesture_data.cursor_idle = TRUE;
		}
	}
}

/**
 * Gets called whenever the touchscreen has been pressed, handling gesture recognition and triggering
 */
void
omp_mplayback_pointer_pressed(guint x, guint y)
{
	mp_gesture_data.pressed = TRUE;
	g_get_current_time(&mp_gesture_data.start_time);
	mp_gesture_data.x_origin = x;
	mp_gesture_data.y_origin = y;
	mp_gesture_data.last_x = x;
	mp_gesture_data.last_y = y;
	mp_gesture_data.cursor_idle = FALSE;
	mp_gesture_data.gesture = OMP_MP_GESTURE_NONE;
	mp_gesture_data.repeating = FALSE;
}

/**
 * Gets called whenever the touchscreen has been released, handling gesture recognition and triggering
 */
void
omp_mplayback_pointer_released()
{
	// Stop repeat trigger if necessary - or trigger action
	if (mp_gesture_data.repeating)
	{
		mp_gesture_data.repeating = FALSE;
	} else {
		omp_mplayback_gesture_trigger();
	}

	mp_gesture_data.pressed = FALSE;
}

/*************************************************************************************************/
/****************************************** QVGA handling ****************************************/
/*************************************************************************************************/

/**
 * Determines the current display mode
 * @return TRUE if display is in QVGA mode, FALSE otherwise
 */
gboolean
omp_mplayback_is_qvga()
{
	gchar *state;
	gboolean is_qvga;

	// Fetch and determine display state
	if (g_file_get_contents(OMP_MPLAYBACK_LCM_STATE_FILE, &state, NULL, NULL))
	{
		is_qvga = strstr(state, "qvga") ? TRUE : FALSE;
		g_free(state);

	} else {

		g_printerr("Could not read LCM state file: %s\n", OMP_MPLAYBACK_LCM_STATE_FILE);
		is_qvga = FALSE;  // If kernel doesn't support QVGA it obviously can't be enabled
	}

	return is_qvga;
}

/**
 * Switches between VGA/QVGA display modes
 * @param qvga_state TRUE if display should switch to QVGA mode, FALSE for VGA
 * @return TRUE for success, FALSE otherwise
 */
gboolean
omp_mplayback_set_qvga(gboolean qvga_state)
{
	gint fd, tty;
	gchar *vt;
	FILE *fh;
	gboolean result;
	GError *error;

	if (!omp_mplayback_fbset_usable)
	{
		g_printerr("No usable fbset was detected earlier, not switching display mode.\n");
		return FALSE;
	}

#if 0
	// Mute the kernel so it doesn't spill messages onto our neat console
	fh = fopen("/proc/sys/kernel/printk", "w");
	fputs("0", fh);
	fclose(fh);

	tty = qvga_state ? OMP_MPLAYBACK_FB_CONSOLE : OMP_MPLAYBACK_X_CONSOLE;
	vt = g_strdup_printf("/dev/tty%d", tty);
	fd = open(vt, O_RDWR);

	This code does not return to X as it should

	// Restore text mode on our FB terminal before switching back to X
	if (!qvga_state)
	{
		if (ioctl(fd, KDSETMODE, KD_TEXT))
		{
			g_printerr("Could not change console mode on tty%d\n", tty);
			goto error;
		}
	}

	// Switch to a different VT
	if (ioctl(fd, VT_ACTIVATE, tty))
	{
		g_printerr("Could not change terminal to %s (VT_ACTIVATE)\n", vt);
		return FALSE;
	}

	if (ioctl(fd, VT_WAITACTIVE, tty))
	{
		g_printerr("Could not change terminal to %s (VT_WAITACTIVE)\n", vt);
		goto error;
	}

	// Enable graphics mode on the FB terminal but leave the X terminal alone
	if (qvga_state)
	{
		if (ioctl(fd, KDSETMODE, KD_GRAPHICS))
		{
			g_printerr("Could not change console mode on tty%d\n", tty);
			goto error;
		}
	}

	// Disable console blanking
	if (qvga_state)
	{
		fh = fopen(vt, "w");
		fputs("\27[9;0]", fh);
		fclose(fh);
	}

	close(fd);
	g_free(vt);
#endif

	// Run fbset
	error = NULL;
	g_spawn_command_line_sync(qvga_state ? "fbset qvga" : "fbset vga", NULL, NULL, NULL, &error);

	if (error)
	{
		g_printerr("Could not invoke fbset: %s\n", error->message);
		g_error_free(error);
		goto error;
	}

	// Set LCM mode
	// Can't use g_file_set_contents here because it doesn't write to the file directly
	fh = fopen(OMP_MPLAYBACK_LCM_STATE_FILE, "w");

	result = FALSE;
	if (fh)
	{
		fputs(qvga_state ? "qvga-normal\n" : "normal\n", fh);
		result = (fclose(fh) == 0);
	}

	if (!result)
	{
		g_printerr("Could not write to LCM state file: %s\n", OMP_MPLAYBACK_LCM_STATE_FILE);
		goto error;
	}

	return TRUE;

error:

	omp_mplayback_set_qvga(FALSE);  // Try to get back to X
	return FALSE;
}

/*************************************************************************************************/
/***************************************** Overlay Window ****************************************/
/*************************************************************************************************/

/**
 * Creates a black overlay window that covers the entire screen so that X doesn't interfere with mplayer
 */
void
omp_mplayback_create_overlay()
{
	GdkWindowAttr attr;

	if (omp_mplayback_overlay_window) return;

	attr.width = 10;
	attr.height = 10;
	attr.wclass = GDK_INPUT_OUTPUT;
	attr.window_type = GDK_WINDOW_TOPLEVEL;

	omp_mplayback_overlay_window = gdk_window_new(omp_window->window, &attr, 0);

	if (!omp_mplayback_overlay_window)
	{
		g_printerr("Failed creating the overlay window to intercept X I/O\n");
		return;
	}
}

/**
 * Closes the overlay window
 */
void
omp_mplayback_destroy_overlay()
{
	if (!omp_mplayback_overlay_window) return;

	gdk_window_destroy(omp_mplayback_overlay_window);
	omp_mplayback_overlay_window = NULL;
}

/**
 * Shows the overlay window
 */
void
omp_mplayback_show_overlay()
{
	gdk_window_show(omp_mplayback_overlay_window);
	gdk_window_fullscreen(omp_mplayback_overlay_window);
	gdk_window_set_keep_above(omp_mplayback_overlay_window, TRUE);

	// Give X time to update before switching to QVGA and messing up the screen
//	usleep(50*1000);  // 50ms
}

/**
 * Hides the overlay window
 */
void
omp_mplayback_hide_overlay()
{
	gdk_window_hide(omp_mplayback_overlay_window);
}

/*************************************************************************************************/
/****************************************** Splash screen ****************************************/
/*************************************************************************************************/

/**
 * Shows a "video player loading" splash screen using DirectFB
 */
void
omp_mplayback_show_splash()
{
	framebuffer *fb;

	fb = fb_new(0);

	fb_draw_image(fb,
		(fb->width - MPLAYER_STARTUP_IMG_WIDTH) >> 1,
		(fb->height - MPLAYER_STARTUP_IMG_HEIGHT) >> 1,
		MPLAYER_STARTUP_IMG_WIDTH,
		MPLAYER_STARTUP_IMG_HEIGHT,
		MPLAYER_STARTUP_IMG_BYTES_PER_PIXEL,
		MPLAYER_STARTUP_IMG_RLE_PIXEL_DATA);

	if (fb) fb_free(fb);
}

/*************************************************************************************************/
/************************************* Touchscreen handling **************************************/
/*************************************************************************************************/

/**
 * Perform any necessary preparations on incoming source events
 */
gboolean
omp_mplayback_input_handler_prepare(GSource *source, gint *timeout)
{
	return FALSE;
}

/**
 * Verify if we actually have data to handle
 * @return TRUE if we have data to handle, FALSE otherwise
 */
gboolean
omp_mplayback_input_handler_check(GSource *source)
{
	return omp_mplayback_ts_fd.revents & G_IO_IN;
}

/**
 * Process the touchscreen event that we were notified of
 */
gboolean
omp_mplayback_input_handler_dispatch(GSource *source, GSourceFunc callback, gpointer data)
{
	struct ts_sample sample;
	static gint8 prev_pressure;

	// We *could* directly read touchscreen information from the input device,
	// however it would not be filtered at all (jitter/calibration/etc.) and thus
	// pretty much pointless.
	// When we get here we however know that a new event is waiting to be processed
	// and so we simply query tslib as it does all the filtering.

	if (ts_read(omp_mplayback_ts_dev, &sample, 1))
	{
		// Determine if we have a trigger/release situation or a mere position movement
		if (sample.pressure && prev_pressure)
		{
			omp_mplayback_pointer_moved(sample.x, sample.y);
g_printf("MOVE ");
		} else {

			prev_pressure = sample.pressure;

			if (sample.pressure)
g_printf("DOWN ");
else
g_printf("UP ");

			if (sample.pressure)
				omp_mplayback_pointer_pressed(sample.x, sample.y);
			else
				omp_mplayback_pointer_released();
		}

//		g_printf("%ld.%06ld: %6d %6d %6d\n", sample.tv.tv_sec, sample.tv.tv_usec, sample.x, sample.y, sample.pressure);
	}

	return TRUE;
}

/**
 * Attaches input event handlers to the touchscreen so we can process user input
 * @return TRUE for success, FALSE for failure
 * @note Inspired by neod's buttonactions.c, written by Michael Lauer
 */
gboolean
omp_mplayback_input_handler_attach()
{
	gchar *ts_device = NULL;

	static GSourceFuncs funcs =
	{
		omp_mplayback_input_handler_prepare,
		omp_mplayback_input_handler_check,
		omp_mplayback_input_handler_dispatch,
		NULL,
	};

	// Find touchscreen input device...
	ts_device = getenv("TSLIB_TSDEVICE");
	if (ts_device)
	{
		// ...and try to open it
		omp_mplayback_ts_fd.fd = open(ts_device, O_RDONLY);

	} else {

		g_printerr("TSLIB_TSDEVICE environment variable not set, trying /dev/input/touchscreen0.\n");
		omp_mplayback_ts_fd.fd = open("/dev/input/touchscreen0", O_RDONLY);
	}

	if (omp_mplayback_ts_fd.fd < 0)
	{
		g_printerr("Failed opening %s\n", ts_device);
		return FALSE;
	}

	// Set up input event handlers and attach them to the main loop
	omp_mplayback_ts_source = g_source_new(&funcs, sizeof(GSource));

	omp_mplayback_ts_fd.events = G_IO_IN | G_IO_HUP | G_IO_ERR;
	omp_mplayback_ts_fd.revents = 0;

	g_source_add_poll(omp_mplayback_ts_source, &omp_mplayback_ts_fd);
	g_source_attach(omp_mplayback_ts_source, NULL);

	// Open tslib for the touchscreen device as well
	omp_mplayback_ts_dev = ts_open(ts_device, 0);

	if (omp_mplayback_ts_dev)
	{
		if (ts_config(omp_mplayback_ts_dev)) goto ts_error;

	} else {

		goto ts_error;
	}

	return TRUE;

ts_error:

	g_printerr("Failed opening tslib for %s!\n", ts_device);

	// Clean up and return
	if (omp_mplayback_ts_dev) ts_close(omp_mplayback_ts_dev);
	omp_mplayback_ts_dev = NULL;

	g_source_destroy(omp_mplayback_ts_source);
	close(omp_mplayback_ts_fd.fd);

	return FALSE;
}

/*************************************************************************************************/
/**************************************** mplayer interface **************************************/
/*************************************************************************************************/

/**
 * Gets called after mplayer was terminated
 */
void
omp_mplayback_termination_callback(GPid pid, gint status, gpointer data)
{
	g_print("%s called\n", __FUNCTION__);

	g_spawn_close_pid(pid);

	// Stop listening for touchscreen input events
	close(omp_mplayback_ts_fd.fd);
	g_source_destroy(omp_mplayback_ts_source);

	// Clean up the pipe
	close(omp_mplayback_pipe);
	omp_mplayback_pipe = -1;

	// mplayer terminated so we want to get back into VGA mode
	omp_mplayback_set_qvga(FALSE);

	// Hide overlay window
	omp_mplayback_hide_overlay();

	g_free(omp_mplayback_uri);
	omp_mplayback_uri = NULL;
	omp_mplayback_playing = FALSE;

	g_signal_emit_by_name(G_OBJECT(omp_window), OMP_EVENT_PLAYBACK_EOS);
}

/**
 * Terminates mplayer
 */
void
omp_mplayback_mplayer_terminate()
{
	if (omp_mplayback_pipe && !omp_mplayback_terminating)
	{
		omp_mplayback_terminating = TRUE;
		write(omp_mplayback_pipe, "quit\n", 5);

		g_printf("Terminating mplayer\n");
	}
}

/**
 * Initializes the mplayer playback interface
 * @return TRUE on success, FALSE on failure
 */
gboolean
omp_mplayback_init()
{
#if OMP_MPLAYBACK_FBSET_CHECK
	gchar *fbset, *fbset_binary, *text;

	// Check whether we'd be using busybox' fbset as that one simply does not work
	omp_mplayback_fbset_usable = FALSE;
	fbset = g_find_program_in_path("fbset");

	if (fbset)
	{
		// We found fbset which is good...
		omp_mplayback_fbset_usable = TRUE;

		// ...but it might be a symlink to busybox
		if (g_file_test(fbset, G_FILE_TEST_IS_SYMLINK))
		{
			fbset_binary = g_file_read_link(fbset, NULL);

			// fbset is invalid if an error occured or it's the one busybox provides
			if (!fbset_binary || strstr(fbset_binary, "busybox"))
				omp_mplayback_fbset_usable = FALSE;

			g_free(fbset_binary);
		}

		g_free(fbset);
	}

	if (!omp_mplayback_fbset_usable)
	{
		text = g_strdup(_("Could not find suitable fbset, please install the fbset package."));
		g_printerr("%s\n", text);
		error_dialog_modal(text);

		g_free(text);
		return FALSE;
	}
#else
	omp_mplayback_fbset_usable = TRUE;
#endif

	// Prepare overlay window so we can be quicker when actually displaying it
	omp_mplayback_create_overlay();

	return TRUE;
}

/**
 * Frees resources allocated for the mplayer playback interface
 */
void
omp_mplayback_free()
{
	if (omp_mplayback_uri) g_free(omp_mplayback_uri);

	// Shutdown mplayer interface if necessary
	if (omp_mplayback_pipe != -1)
	{
		write(omp_mplayback_pipe, "quit\n", 5);
		close(omp_mplayback_pipe);
	}

	// Make sure we leave the device in VGA mode on program termination
	if (omp_mplayback_is_qvga()) omp_mplayback_set_qvga(FALSE);

	// Remove overlay window
	omp_mplayback_destroy_overlay();
}


/**
 * Check whether URI is a movie file and loads it if it is
 * @return TRUE if file was loaded, FALSE otherwise
 */
gboolean
omp_mplayback_load_video_from_uri(gchar *uri)
{
	guint i, j;
	gchar *extension, *cmd;
	gboolean is_video;
	g_print("%s: %s\n", __FUNCTION__, uri);

	// Get file extension by finding last dot in the uri
	for (i=0,j=0; uri[i]; i++)
		if (uri[i] == '.') j = i;

	extension = g_strdup(uri+j+1);
	extension = g_ascii_strdown(extension, -1);

	// Check extension to see if we have a playable file type
	is_video = FALSE;

	if ( (strcmp(extension, "mpg") == 0) ||
	     (strcmp(extension, "mpeg") == 0) ||
	     (strcmp(extension, "avi") == 0) ||
	     (strcmp(extension, "3gp") == 0) ||
	     (strcmp(extension, "wmv") == 0) )
	{
		is_video = TRUE;

		// Copy URI
		if (omp_mplayback_uri) g_free(omp_mplayback_uri);
		omp_mplayback_uri = g_strdup(uri);

		if (omp_mplayback_pipe != -1)
		{
			// mplayer was already running so we just set a new URI to play
			cmd = g_strdup_printf("loadfile %s\n", uri);
			write(omp_mplayback_pipe, cmd, strlen(cmd));
			g_free(cmd);
		}
	}

	// Scenario: video file loaded but not played, non-video file loaded afterwards
	// In this case we need to empty the playback URI to make sure the old video won't
	// be mistreated as the current file
	if (!is_video && omp_mplayback_uri)
	{
		g_free(omp_mplayback_uri);
		omp_mplayback_uri = NULL;
	}

	// Clean up
	g_free(extension);

	g_print("is_video = %d\n", is_video);

	return is_video;
}

/**
 * Tells whether the mplayer playback interface has a file loaded or not
 * @return TRUE if a file is loaded, FALSE otherwise
 */
gboolean
omp_mplayback_video_loaded()
{
	return omp_mplayback_uri != NULL;
}

/**
 * Starts playback
 */
void
omp_mplayback_play()
{
	#define MAX_ARGS 25
	g_print("%s called\n", __FUNCTION__);

	gchar *cmdline, *tmp, *msg;
	gchar *argv[MAX_ARGS];
	gint i, argc, cmdline_len;
	GPid pid;
	GError *error;

	// play() should never be called without a valid URI set
	g_return_if_fail(omp_mplayback_uri);

	// Create mplayer instance if needed
	if (omp_mplayback_pipe == -1)
	{
		omp_mplayback_terminating = FALSE;

		// Reveal black overlay window to prevent X from receiving touchscreen events
		omp_mplayback_show_overlay();

		// Switch to QVGA mode
		omp_mplayback_set_qvga(TRUE);

		// Show splash screen
		omp_mplayback_show_splash();

		// Listen for input events, too
		if (!omp_mplayback_input_handler_attach())
		{
			omp_mplayback_set_qvga(FALSE);
			omp_mplayback_hide_overlay();
			error_dialog_modal(_("Failed setting up touchscreen interface"));
			return;
		}

		// Acquire pipe to talk to mplayer with
		mkfifo(OMP_MPLAYBACK_PIPE_NAME, S_IFIFO|0666);
		omp_mplayback_pipe = open(OMP_MPLAYBACK_PIPE_NAME, O_RDWR);

		// Assemble command line
		cmdline = g_strdup_printf(OMP_MPLAYBACK_FB_CMDLINE, OMP_MPLAYBACK_PIPE_NAME, omp_mplayback_uri);

		#ifdef DEBUG
			g_printf("Launching %s\n", cmdline);
		#endif

		// Split command line up
		// To do this we replace all spaces by #0 and fill the argv array with the individual string chunks
		argv[0] = OMP_MPLAYBACK_MPLAYER_BINARY;

		cmdline_len = strlen(cmdline);
		tmp = cmdline;
		for (i=0,argc=1; (i<cmdline_len) && (argc<MAX_ARGS); argc++)
		{
			argv[argc] = tmp;
			while ( (cmdline[i] != 32) && (cmdline[i] != 0) ) i++;
			cmdline[i] = 0;
			i++;
			tmp = cmdline+i;
		}
		argv[argc] = NULL;

		// Launch mplayer, making sure we can get notificed of its termination
		error = NULL;
		pid = 0;
		g_spawn_async(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, &pid, &error);

		if (error || !pid)
		{
			// ...back to VGA
			omp_mplayback_set_qvga(FALSE);
			omp_mplayback_hide_overlay();

			msg = g_strdup_printf(_("Failed to launch mplayer (%s), falling back to gstreamer"), error->message);
			g_printerr("%s\n", msg);
			error_dialog_modal(msg);
			g_free(msg);

			g_error_free(error);
			g_free(cmdline);
			g_free(omp_mplayback_uri);
			close(omp_mplayback_pipe);
			omp_mplayback_pipe = -1;

			return;
		}

		// Add child PID watcher
		g_child_watch_add(pid, omp_mplayback_termination_callback, NULL);

		// Clean up
		g_free(cmdline);

		omp_mplayback_windowed = FALSE;

	} else {

		// pipe is open, mplayer was already started
		if (!omp_mplayback_playing)
			write(omp_mplayback_pipe, "pause\n", 6);

	}

	omp_mplayback_playing = TRUE;
}

/**
 * Pauses playback
 */
void
omp_mplayback_pause()
{
	g_print("%s called\n", __FUNCTION__);

	if ( (omp_mplayback_pipe != -1) && omp_mplayback_is_playing() )
		write(omp_mplayback_pipe, "pause\n", 6);

	omp_mplayback_playing = FALSE;
}

/**
 * Tells the current status of the mplayer playback engine
 * @return TRUE if playing, FALSE otherwise
 */
gboolean
omp_mplayback_is_playing()
{
	g_print("%s: %d\n", __FUNCTION__, omp_mplayback_playing);

	return omp_mplayback_playing;
}

/**
 *
 */
gint
omp_mplayback_get_state()
{
	g_print("%s: %d\n", __FUNCTION__, omp_mplayback_playing);

	return omp_mplayback_playing ? OMP_PLAYBACK_STATE_PLAYING : OMP_PLAYBACK_STATE_PAUSED;
}

/**
 *
 */
gulong
omp_mplayback_get_video_position()
{
	return 0;
}

/**
 *
 */
void
omp_mplayback_set_video_position(gulong position)
{
}

/**
 * Sets mplayer volume
 */
void
omp_mplayback_set_volume(guint volume)
{
	gchar *cmd;

	if (!omp_mplayback_pipe) return;

	cmd = g_strdup_printf("set_property volume %d\n", volume);
g_printf(cmd);
	write(omp_mplayback_pipe, cmd, strlen(cmd));
	g_free(cmd);
}
