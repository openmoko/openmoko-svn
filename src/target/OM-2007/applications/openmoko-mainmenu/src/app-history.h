#ifndef _MOKO_APP_HISTORY_H
#define _MOKO_APP_HISTORY_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <libmokoui/moko-pixmap-button.h>

void
moko_update_history_app_list ();

void
moko_add_history_app_image (MokoPixmapButton* btn, GdkPixbuf *pixbuf);

#endif "app-history.h"
