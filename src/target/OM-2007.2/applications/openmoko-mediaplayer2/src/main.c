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

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include "libmokoui2/moko-stock.h"

#include "main.h"
#include "editor_page.h"
#include "files_page.h"
#include "main_page.h"
#include "persistent.h"
#include "playlist_page.h"
#include "playlist.h"
#include "playback.h"
#include "utils.h"

// Determines how the segfault handler terminates the program
//define HANDLE_SIGSEGV

// Enables GLib memory profiling when defined
//define DEBUG_MEM_PROFILE

// Forces the window to the native size of the Neo1973's screen area if enabled
//define EMULATE_SIZE

/// Application's main window
GtkWidget *omp_window = NULL;

/// GtkNotebook containing the pages making up the UI
GtkWidget *omp_notebook = NULL;

/// Holds the tab widgets of the GtkNotebook
GtkWidget *omp_notebook_tabs[OMP_TABS];



/**
 * Terminate the entire program normally
 */
void
omp_application_terminate()
{
	gtk_main_quit();
}

/**
 * SIGSEGV signal handler
 */
static void
handler_sigsegfault(int value)
{
	g_printerr(_("Received SIGSEGV\n"
		"This might be a bug in the OpenMoko Media Player.\n\n"));

#ifdef HANDLE_SIGSEGV
	exit(EXIT_FAILURE);
#else
	abort();
#endif
}

/**
 * SIGUSR1 signal handler, restores already running application
 */
static void
handler_sigusr1(int value)
{
	// Bring main window to front
	gtk_window_present(GTK_WINDOW(omp_window));

	// Re-install handler
	signal(SIGUSR1, handler_sigusr1);
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

	fd = open(file_name, O_WRONLY|O_CREAT, S_IWUSR);
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
 * @return TRUE if lock is in place, FALSE if it wasn't
 * @todo Single-instance mechanism needs to be improved
 */
gboolean
check_lock()
{
	gchar *lock_file = "/tmp/mediaplayer.lock";
	pid_t pid;

	pid = test_lock(lock_file);
	if (pid > 0)
	{
		g_printf(_("Already running an instance of the Media Player, bringing that to the front instead.\n"));

		// Was signaling the other process successful?
		if (kill(pid, SIGUSR1) == 0)
		{
			return TRUE;
		} else {
			g_printf(_("Previous instance invalid, proceeding with regular startup.\n"));
		}
	}

	set_lock(lock_file);
	return FALSE;
}

/**
 * Determines theme to be used, uses this app's theme file and sets up related UI properties
 */
void
omp_init_theme()
{
	gchar *theme_dir, *theme_name, *theme_rc;

	moko_stock_register();
	omp_ui_image_path = g_build_path("/", DATA_DIR, OMP_RELATIVE_UI_IMAGE_PATH, NULL);

	g_set_application_name(_("Media Player"));
	gtk_window_set_default_icon_name("openmoko-soundandvideo");

	theme_dir = gtk_rc_get_theme_dir();
	g_object_get(G_OBJECT(gtk_settings_get_default()), "gtk-theme-name", &theme_name, NULL);
	theme_rc = g_build_filename(theme_dir, "/", theme_name, "/gtk-2.0/openmoko-mediaplayer", NULL);

	gtk_rc_parse(theme_rc);

	g_free(theme_rc);
	g_free(theme_name);
	g_free(theme_dir);
}

/**
 * Program termination event triggered by main window
 */
void
omp_close(GtkWidget *widget, gpointer data)
{
	omp_application_terminate();
}

/**
 * Creates the application's main window
 */
void
omp_window_create()
{
	g_return_if_fail(omp_window == NULL);

	// Create window
	omp_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(omp_window), _("Media Player"));
	g_signal_connect(G_OBJECT(omp_window), "destroy", G_CALLBACK(omp_close), NULL);

	#ifdef EMULATE_SIZE
		gtk_widget_set_size_request(GTK_WIDGET(omp_window), 480, 600);
	#endif
}

/**
 * Creates the individual pages that make up the UI
 * @note Must be called after the backends have been initialized so the signals exist that the UIs hook to
 */
void
omp_window_create_pages()
{
	GtkWidget *page;

	// Create and set up the notebook that contains the individual UI pages
	omp_notebook = gtk_notebook_new();
	g_object_set(G_OBJECT(omp_notebook), "can-focus", FALSE, "homogeneous", TRUE, NULL);
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(omp_notebook), GTK_POS_BOTTOM);
	gtk_container_add(GTK_CONTAINER(omp_window), GTK_WIDGET(omp_notebook));
	gtk_widget_show(omp_notebook);

	// Add main page
	page = omp_main_page_create();
	notebook_add_page_with_stock(omp_notebook, page, MOKO_STOCK_SPEAKER, 0);
	omp_notebook_tabs[OMP_TAB_MAIN] = page;

	// Add playlist page
	page = omp_playlist_page_create();
	notebook_add_page_with_image(omp_notebook, page, "ico-playlists.png", 0);
	omp_notebook_tabs[OMP_TAB_PLAYLISTS] = page;

	// Add playlist editor page
	page = omp_editor_page_create();
	notebook_add_page_with_image(omp_notebook, page, "ico-playlist-editor.png", 0);
	omp_notebook_tabs[OMP_TAB_PLAYLIST_EDITOR] = page;
	gtk_widget_hide(page);	// We show the page once a playlist was loaded

	// Add file chooser page
	page = omp_files_page_create();
	notebook_add_page_with_image(omp_notebook, page, "ico-file-chooser.png", 0);
	omp_notebook_tabs[OMP_TAB_FILE_CHOOSER] = page;
	gtk_widget_hide(page);	// We show the page once user wants to add files
}

/**
 * Lets all UI pages clean up
 */
void
omp_window_free_pages()
{
	omp_files_page_free();
}

/**
 * Displays the main window and all widgets it contains
 */
void
omp_window_show()
{
	gtk_widget_show(omp_window);
}

/**
 * Makes one of the UI tabs visible on the notebook
 * @param tab_id An ID taken from the omp_notebook_tabs enum
 * @see omp_notebook_tabs
 */
void
omp_tab_show(omp_notebook_tab tab)
{
	g_return_if_fail(tab < OMP_TABS);

	gtk_widget_show(GTK_WIDGET(omp_notebook_tabs[tab]));
}

/**
 * Hides one of the UI tabs from the notebook
 * @param tab_id An ID taken from the omp_notebook_tabs enum
 * @see omp_notebook_tabs
 */
void
omp_tab_hide(omp_notebook_tab tab)
{
	g_return_if_fail(tab < OMP_TABS);

	gtk_widget_hide(GTK_WIDGET(omp_notebook_tabs[tab]));
}

/**
 *
 */
void
omp_tab_focus(omp_notebook_tab tab)
{
	g_return_if_fail(tab < OMP_TABS);

	gtk_notebook_set_current_page(GTK_NOTEBOOK(omp_notebook), tab);
}

/**
 * If only I knew what this is
 */
gint
main(int argc, char *argv[])
{
	GError *error;

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
	if (check_lock()) return EXIT_SUCCESS;

	// Initialize gstreamer, must be last in the chain of command line parameter processors
	if (!gst_init_check(&argc, &argv, &error))
	{
		g_printerr(_("Exiting due to gstreamer error: %s\n"), error->message);
		g_error_free(error);
		exit(EXIT_FAILURE);
	}

	// Initialize UI theme
	omp_init_theme();
	
	// Set up signal handlers
	signal(SIGSEGV, handler_sigsegfault);
	signal(SIGUSR1, handler_sigusr1);

	// Initialize backends and user interfaces
	omp_session_init();
	omp_config_init();
	omp_window_create();
	if (!omp_playback_init()) return EXIT_FAILURE;
	omp_playlist_init();
	omp_window_create_pages();

	// Let the UI catch up
	while (gtk_events_pending()) gtk_main_iteration();

	omp_session_restore_state();
	omp_window_show();

	gtk_main();

	// Clean up
	omp_window_free_pages();
	omp_playback_save_state();
	omp_playback_free();
	omp_playlist_free();
	gst_deinit();
	g_free(omp_ui_image_path);

	omp_session_free();
	omp_config_free();

#ifdef DEBUG_MEM_PROFILE
	g_mem_profile();
#endif

	return EXIT_SUCCESS;
}
