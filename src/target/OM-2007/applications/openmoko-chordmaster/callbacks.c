#include "callbacks.h"
#include "fretboard-widget.h"

#include <gtk/gtkwidget.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtktreemodel.h>

gboolean cb_filter_changed(GtkWidget* widget, gchar* text, gpointer user_data)
{
    g_debug( "openmoko-chordmaster: filter changed" );
    return FALSE;
}

void cb_button1_clicked(GtkButton *button, gpointer user_data)
{
    g_debug( "openmoko-chordmaster: button1 clicked" );
}

void cb_button2_clicked(GtkButton *button, gpointer user_data)
{
    g_debug( "openmoko-chordmaster: button2 clicked" );
}

void cb_button3_clicked(GtkButton *button, gpointer user_data)
{
    g_debug( "openmoko-chordmaster: button3 clicked" );
}

void cb_button4_clicked(GtkButton *button, gpointer user_data)
{
    g_debug( "openmoko-chordmaster: button4 clicked" );
}

void cb_cursor_changed(GtkTreeSelection* selection, ChordMasterData* d)
{
    g_debug( "openmoko-chordmaster: selection changed" );
    GtkTreeView* view = gtk_tree_selection_get_tree_view( selection );
    GtkTreeModel* model;
    GtkTreeIter iter;
    gtk_tree_selection_get_selected( selection, &model, &iter );

    gchar* frets;
    gtk_tree_model_get( model, &iter, COLUMN_FRETS, &frets, -1 );

    fretboard_widget_set_frets( d->fretboard, frets );
    gtk_widget_queue_draw( GTK_WIDGET(d->fretboard) );
}

