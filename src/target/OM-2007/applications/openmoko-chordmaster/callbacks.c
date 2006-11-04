#include "callbacks.h"

#include "main.h"
#include "chordsdb.h"
#include "fretboard-widget.h"

#include <gtk/gtkwidget.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtktreeview.h>
#include <gtk/gtktreemodel.h>

gboolean cb_filter_changed(GtkWidget* widget, gchar* text, ChordMasterData* d)
{
    g_debug( "openmoko-chordmaster: filter changed" );
    g_assert( d->chordsdb );
    if ( !d->liststore )
        d->liststore = gtk_list_store_new( NUM_COLS, G_TYPE_STRING, G_TYPE_STRING );
    gtk_list_store_clear( d->liststore );
    GtkTreeIter iter;
    GSList* chords = chordsdb_get_chords( d->chordsdb );
    for ( ; chords; chords = g_slist_next( chords ) )
    {
        Chord* chord = chords->data;
        if ( strcmp( text, "All") == 0 || text[0] == chord->name[0] )
        {
            g_debug( "chordmaster: adding chord '%s' = '%s'", chord->name, chord->frets );
            gtk_list_store_append( d->liststore, &iter );
            gtk_list_store_set( d->liststore, &iter, COLUMN_NAME, chord->name, COLUMN_FRETS, chord->frets, -1 );
        }
    }

    //g_assert( FALSE );
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
    gboolean has_selection = gtk_tree_selection_get_selected( selection, &model, &iter );

    gchar* frets = NULL;
    if ( has_selection )
        gtk_tree_model_get( model, &iter, COLUMN_FRETS, &frets, -1 );

    fretboard_widget_set_frets( d->fretboard, frets );
    gtk_widget_queue_draw( GTK_WIDGET(d->fretboard) );
}

