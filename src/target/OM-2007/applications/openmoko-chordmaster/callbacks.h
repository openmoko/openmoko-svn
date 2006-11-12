#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_

#include "main.h"

#include <libmokoui/moko-tool-box.h>

#include <gtk/gtkbutton.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtktreeview.h>

#include <glib.h>

gboolean cb_entry_completion_completed(GtkEntryCompletion *widget, GtkTreeModel *model, GtkTreeIter *iter, ChordMasterData* d);
gboolean cb_filter_changed(GtkWidget* widget, gchar* text, ChordMasterData* d);
void cb_button1_clicked(GtkButton *button, ChordMasterData* d);
void cb_button2_clicked(GtkButton *button, ChordMasterData* d);
void cb_button3_clicked(GtkButton *button, ChordMasterData* d);
void cb_button4_clicked(GtkButton *button, ChordMasterData* d);
void cb_cursor_changed(GtkTreeSelection* selection, ChordMasterData* d);
void cb_search_visible(MokoToolBox* toolbox, ChordMasterData* d);
void cb_search_invisible(MokoToolBox* toolbox, ChordMasterData* d);
#endif
