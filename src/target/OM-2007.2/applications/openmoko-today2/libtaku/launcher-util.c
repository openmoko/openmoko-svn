/* 
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <config.h>

#include <string.h>
#include <unistd.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include "launcher-util.h"
#include "xutil.h"

#ifdef USE_LIBSN
#define SN_API_NOT_YET_FROZEN 1
#include <libsn/sn.h>
#endif

/* Convert command line to argv array, stripping % conversions on the way */
#define MAX_ARGS 255

#define DESKTOP "Desktop Entry"

char **
exec_to_argv (const char *exec)
{
  const char *p;
  char *buf, *bufp, **argv;
  int nargs;
  gboolean escape, single_quote, double_quote;
  
  argv = g_new (char *, MAX_ARGS + 1);
  buf = g_alloca (strlen (exec) + 1);
  bufp = buf;
  nargs = 0;
  escape = single_quote = double_quote = FALSE;
  
  for (p = exec; *p; p++) {
    if (escape) {
      *bufp++ = *p;
      
      escape = FALSE;
    } else {
      switch (*p) {
      case '\\':
        escape = TRUE;

        break;
      case '%':
        /* Strip '%' conversions */
        if (p[1] && p[1] == '%')
          *bufp++ = *p;
        
        p++;

        break;
      case '\'':
        if (double_quote)
          *bufp++ = *p;
        else
          single_quote = !single_quote;
        
        break;
      case '\"':
        if (single_quote)
          *bufp++ = *p;
        else
          double_quote = !double_quote;
        
        break;
      case ' ':
        if (single_quote || double_quote)
          *bufp++ = *p;
        else {
          *bufp = 0;
          
          if (nargs < MAX_ARGS)
            argv[nargs++] = g_strdup (buf);
          
          bufp = buf;
        }
        
        break;
      default:
        *bufp++ = *p;
        break;
      }
    }
  }
  
  if (bufp != buf) {
    *bufp = 0;
    
    if (nargs < MAX_ARGS)
      argv[nargs++] = g_strdup (buf);
  }
  
  argv[nargs] = NULL;
  
  return argv;
}

/*
 * Get the boolean for the key @key from @key_file, and if it cannot be parsed
 * or does not exist return @def.
 */
static gboolean
get_desktop_boolean (GKeyFile *key_file, const char *key, gboolean def)
{
  GError *error = NULL;
  gboolean b;

  g_assert (key_file);
  g_assert (key);

  b = g_key_file_get_boolean (key_file, DESKTOP, key, &error);
  if (error) {
    g_error_free (error);
    b = def;
  }

  return b;
}

static char *
get_desktop_string (GKeyFile *key_file, const char *key)
{
  char *s;

  g_assert (key_file);
  g_assert (key);

  /* Get the key */
  s = g_key_file_get_locale_string (key_file, DESKTOP, key, NULL, NULL);
  /* Strip any whitespace */
  s = s ? g_strstrip (s) : NULL;
  if (s && s[0] != '\0') {
    return s;
  } else {
    if (s) g_free (s);
    return NULL;
  }
}

LauncherData *
launcher_parse_desktop_file (const char *filename, GError **error)
{
  GError *err = NULL;
  LauncherData *data;
  GKeyFile *key_file;
  char *exec, *categories;

  key_file = g_key_file_new ();

  if (!g_key_file_load_from_file (key_file, filename, G_KEY_FILE_NONE, &err)) {
    g_key_file_free (key_file);
    g_propagate_error (error, err);
    return NULL;
  }

  if (get_desktop_boolean (key_file, "NoDisplay", FALSE)) {
    g_key_file_free (key_file);
    return NULL;
  }

  /* This is the important one, so read it first to simplify cleanup */
  exec = get_desktop_string (key_file, "Exec");
  if (exec == NULL) {
    g_free (exec);
    g_key_file_free (key_file);
    return NULL;
  }

  data = g_slice_new0 (LauncherData);
  data->argv = exec_to_argv (exec);
  g_free (exec);

  data->name = get_desktop_string (key_file, "Name");

  data->description = get_desktop_string (key_file, "Comment");

  data->icon = get_desktop_string (key_file, "Icon");

  categories = get_desktop_string (key_file, "Categories");
  if (categories == NULL)
    categories = g_strdup ("");
  data->categories = g_strsplit (categories, ";", -1);
  g_free (categories);

  data->use_sn = get_desktop_boolean (key_file, "StartupNotify", FALSE);

  data->single_instance = get_desktop_boolean (key_file, "SingleInstance", FALSE);

  g_key_file_free (key_file);

  return data;
}

/* Strips extension off filename */
static char *
strip_extension (const char *file)
{
        char *stripped, *p;

        stripped = g_strdup (file);

        p = strrchr (stripped, '.');
        if (p &&
            (!strcmp (p, ".png") ||
             !strcmp (p, ".svg") ||
             !strcmp (p, ".xpm")))
	        *p = 0;

        return stripped;
}

char *
launcher_get_icon (GtkIconTheme *icon_theme, LauncherData *data, int size)
{
  GtkIconInfo *info;
  char *new_icon, *stripped;

  if (data->icon == NULL) {
    return NULL;
  }

  /* TODO: remove? sync with screen? */  
  if (icon_theme == NULL) {
    icon_theme = gtk_icon_theme_get_default();
  }

  if (g_path_is_absolute (data->icon)) {
    if (g_file_test (data->icon, G_FILE_TEST_EXISTS))
      return g_strdup (data->icon);
    else
      new_icon = g_path_get_basename (data->icon);
  } else
    new_icon = (char *) data->icon;
  
  stripped = strip_extension (new_icon);
  
  if (new_icon != data->icon)
    g_free (new_icon);
  
  info = gtk_icon_theme_lookup_icon (icon_theme, 
                                     stripped,
                                     size,
                                     0);
  
  g_free (stripped);
  
  if (info) {
    char *file;
    
    file = g_strdup (gtk_icon_info_get_filename (info));
    
    gtk_icon_info_free (info);
    
    return file;
  } else
    return NULL;
}

static void
child_setup (gpointer user_data)
{
#ifdef USE_LIBSN
  if (user_data) {
    sn_launcher_context_setup_child_process (user_data);
  }
#endif
}


/* TODO: optionally link to GtkUnique and directly handle that? */
void
launcher_start (GtkWidget *widget, LauncherData *data)
{
  GError *error = NULL;
#ifdef USE_LIBSN
  SnLauncherContext *context;
#endif

  /* Check for an existing instance if Matchbox single instance */
  if (data->single_instance) {
    Window win_found;

    if (mb_single_instance_is_starting (data->argv[0]))
      return;

    win_found = mb_single_instance_get_window (data->argv[0]);
    if (win_found != None) {
      x_window_activate (win_found);

      return;
    }
  }
  
#ifdef USE_LIBSN
  context = NULL;
  
  if (data->use_sn) {
    SnDisplay *sn_dpy;
    Display *display;
    int screen;
    
    display = gdk_x11_display_get_xdisplay (gtk_widget_get_display (widget));
    sn_dpy = sn_display_new (display, NULL, NULL);
    
    screen = gdk_screen_get_number (gtk_widget_get_screen (widget));
    context = sn_launcher_context_new (sn_dpy, screen);
    sn_display_unref (sn_dpy);
    
    sn_launcher_context_set_name (context, data->name);
    sn_launcher_context_set_binary_name (context, data->argv[0]);
    /* TODO: set workspace, steal gedit_utils_get_current_workspace */
    
    sn_launcher_context_initiate (context,
                                  g_get_prgname () ?: "unknown",
                                  data->argv[0],
                                  gtk_get_current_event_time ());
  }
#endif
  
  /* GTK+ 2.11.3 has a gdk_spawn_on_screen which doesn't trash envp */
#if GTK_CHECK_VERSION(2,11,3)
  if (!gdk_spawn_on_screen (gtk_widget_get_screen (widget),
#else
  if (!g_spawn_async (
#endif
                            NULL, data->argv, NULL,
                            G_SPAWN_SEARCH_PATH,
                            child_setup,
#ifdef USE_LIBSN
                            data->use_sn ? context : NULL,
#else
                            NULL,
#endif
                            NULL,
                            &error)) {
    g_warning ("Cannot launch %s: %s", data->argv[0], error->message);
    g_error_free (error);
#ifdef USE_LIBSN
    if (context)
      sn_launcher_context_complete (context);
#endif
  }
  
#ifdef USE_LIBSN
  if (data->use_sn)
    sn_launcher_context_unref (context);
#endif
}

void
launcher_destroy (LauncherData *data)
{
  g_return_if_fail (data);
  
  g_free (data->name);
  g_free (data->description);
  g_free (data->icon);
  g_strfreev (data->categories);
  g_strfreev (data->argv);
  g_slice_free (LauncherData, data);
}
