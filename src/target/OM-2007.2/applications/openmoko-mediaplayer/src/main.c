/*
 *  OpenMoko Media Player
 *   http://openmoko.org/
 *
 *  Copyright (C) 2007 by the OpenMoko team
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
 * @file main.c
 * Main file
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <gdk/gdk.h>

#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include <spiff/spiff_c.h>

#include "main.h"
#include "guitools.h"
#include "playlist.h"

/// Determines how the segfault handler terminates the program
//define HANDLE_SIGSEGV

/// Enables GLib memory profiling when defined
//define DEBUG_MEM_PROFILE


/// The default configuration
struct _omp_config omp_default_config =
{
	FALSE,
	TRUE,
	OMP_REPEAT_OFF,
//	FALSE,
	FALSE,
	"%f",
	0.0,
	{0,0,0,0,0,0,0,0,0,0,0},
	TRUE,
	"",
	"../../test.xspf",
	0,
	0
};

struct _omp_config *omp_config = NULL;			///< Global and persistent configuration data



/*
void
init_dbus()
{
    //added by lijiang
    DBusConnection *bus;
    DBusError error;

    dbus_error_init(&error);
    bus = dbus_bus_get(DBUS_BUS_SESSION, &error);
    if(!bus)
    {
        g_print("Failed to connect to the D-Bus daemon: %s", error.message);
	dbus_error_free(&error);
	return;
    }
    dbus_connection_setup_with_g_main(bus, NULL);
    dbus_bus_add_match(bus, "type='signal',interface='com.burtonini.dbus.Signal'", &error);
    dbus_connection_add_filter(bus, signal_filter, mainwindow, NULL);
    //added end

}
*/

/**
 * SIGSEGV signal handler
 */
static void
handler_sigsegfault(int value)
{
	g_printerr(_("Received SIGSEGV\n"
		"This could be a bug in the OpenMoko Media Player.\n\n"));

#ifdef HANDLE_SIGSEGV
	exit(EXIT_FAILURE);
#else
	abort();
#endif
}

/**
 * SIGUSR1 signal handler
 * @todo Should bring currently active window to front - which isn't necessarily the main window
 */
static void
handler_sigusr1(int value)
{
	// Bring main window to front
	omp_main_window_show();

	// Re-install handler
	signal(SIGUSR1, handler_sigusr1);
}

/**
 * Load configuration and restore player state from last session
 */
void
omp_config_restore_state()
{
	// This mustn't be called more than once
	g_assert(omp_config == NULL);

	// Set default config
	omp_config = g_new(struct _omp_config, 1);
	g_memmove(omp_config, &omp_default_config, sizeof(struct _omp_config));

	// Load config and last used playlist if set
	omp_config_load();

	if (omp_config->playlist_file[0])
	{
		omp_playlist_load(omp_config->playlist_file);
	}

	// Check whether playlist_position is valid
	if (!omp_playlist_set_current_track(omp_config->playlist_position))
	{
		// Reset playlist state as it must have been modified since it was last loaded
		omp_config->playlist_position = 0;
		omp_config->track_position = 0;
	} 

	// Feed the track entity to the playback engine to obtain track information
	omp_playlist_load_current_track();
}

/**
 * Releases resources allocated for configuration data
 */
void
omp_config_free()
{
	g_free(omp_config);
}

/**
 * Saves the current configuration data to persistent storate
 */
void
omp_config_save()
{
}

/**
 * Reads the configuration data from persistent storage
 */
void
omp_config_load()
{
}

/**
 * Updates config with current player data and saves it to disk 
 */
void
omp_config_update()
{
	omp_config->playlist_position = omp_playlist_current_track_id;

	omp_config_save();
}


/**
 * Tests for a lock file and returns the pid of any process already running
 */
pid_t
test_lock(gchar *file_name)
{
	gint fd;
	struct flock fl;

	fd = open(file_name, O_WRONLY, S_IWUSR);

	if (fd < 0)
	{
		if (errno == ENOENT)
		{
			return 0;
		} else {
			g_printerr("Lock file test failed!\n");
			return -1;
		}
	}

	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;

	if (fcntl(fd, F_GETLK, &fl) < 0)
	{
		close(fd);
		return -1;
	}

	close(fd);

	return (fl.l_type == F_UNLCK) ? 0 : fl.l_pid;
}

/**
 * Sets a lock file
 */
void
set_lock(gchar *file_name)
{
	gint fd;
	struct flock fl;

	fd = open(file_name, O_WRONLY|O_CREAT,  S_IWUSR);
	if (fd < 0)
	{
		g_printerr("Failed opening lock file\n");
		return;
	}

	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	fl.l_start = 0;
	fl.l_len = 0;

	if (fcntl(fd, F_SETLK, &fl) < 0)
	{
		g_printerr("Failed writing lock file\n");
		close(fd);
	}
}

/**
 * Makes sure only one instance of the media player is running at a time
 * @todo Single-instance mechanism needs to be improved
 */
void
handle_locking()
{
	gchar *lock_file = "/tmp/mediaplayer.lock";
	pid_t pid;

	pid = test_lock(lock_file);
	if (pid > 0)
	{
		g_printf(_("Already running an instance of the Media Player, bringing that to the front instead.\n"));
		kill(pid, SIGUSR1);
		return;
	}

	set_lock(lock_file);
}

/**
 * If only I knew what this is
 */
gint
main(int argc, char *argv[])
{
#ifdef DEBUG_MEM_PROFILE
	g_mem_set_vtable(glib_mem_profiler_table);
#endif

	// Set l10n up early so we can print localized error messages
	gtk_set_locale();
	bindtextdomain(PACKAGE, LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
	textdomain(PACKAGE);

	// Check for threads support and initialize them if available
	g_thread_init(NULL);
	if (!g_thread_supported())
	{
		g_printerr(_("Sorry, threads aren't supported on your platform.\n\n"
			"If you're on a libc5 based linux system and installed Glib & GTK+ before you\n"
			"installed LinuxThreads then you need to recompile Glib & GTK+.\n"));
		exit(EXIT_FAILURE);
	}

	gdk_threads_init();
	gdk_threads_enter();

	// Initialize miscellaneous things needed for the remote control interface
	g_random_set_seed(time(NULL));

	// See if user wants a GUI or just remote control an already running instance
	if (!gtk_init_check(&argc, &argv))
	{
		if (argc < 2)
		{
			/* GTK check failed, and no arguments passed to indicate
				 that user is intending to only remote control a running
				 session */
			g_printerr(_("Unable to initialize GTK+, exiting. Maybe set a proper display?\n"));
			exit(EXIT_FAILURE);
		}

		/// @todo Handle remote controlling here
		exit(EXIT_SUCCESS);
	}

	// Make sure we have only one instance running
	handle_locking();

	// Initialize gstreamer, must be last in the chain of command line parameter processors

	if (!gst_init_check(&argc, &argv))
	{
		g_printerr(_("Unable to initialize gstreamer, exiting.\n"
			"As gstreamer also fails to initialize when encountering unknown "
			"command line parameters you should check those as well.\n"));
		exit(EXIT_FAILURE);
	}

	// Initialize various things necessary for the full player UI
	ui_image_path = g_build_filename(DATA_DIR, RELATIVE_UI_IMAGE_PATH, NULL);

	g_set_application_name(_("Media Player"));
	gtk_window_set_default_icon_name("openmoko-soundandvideo");

	// Set up signal handlers
	signal(SIGSEGV, handler_sigsegfault);
	signal(SIGUSR1, handler_sigusr1);

	// Load config and restore playback state
	omp_config_restore_state();

	// Initialize playback, playlist and UI handling
	omp_main_window_create();
	omp_playback_init();
	omp_playlist_init();
	omp_main_connect_signals();
	omp_main_update_track_info();
	omp_main_window_show();

	gtk_main();
	gdk_threads_leave();

	// Store and free configuration resources
	omp_config_save();
	omp_config_free();

	// Free remaining resources
	omp_playback_free();
	omp_playlist_free();
	gst_deinit();

#ifdef DEBUG_MEM_PROFILE
	g_mem_profile();
#endif

	return EXIT_SUCCESS;
}
