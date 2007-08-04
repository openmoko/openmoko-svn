/*
 *  RSS Reader, a simple RSS reader
 *
 *  Copyright (C) 2007 Holger Hans Peter Freyther
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 *  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 *  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *  OTHER DEALINGS IN THE SOFTWARE.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#include "config.h"
#include <glib/gi18n.h>

#include "callbacks.h"
#include "rfcdate.h"
#include "moko_cache.h"

#include <libmokoui/moko-tool-box.h>
#include <gdk/gdkkeysyms.h>

#include <mrss.h>
#include <string.h>
#include <stdlib.h>

static void remove_container_item( GtkWidget *item, GtkWidget *container ) {
    gtk_container_remove(GTK_CONTAINER(container), item);
}

void filter_feeds( struct RSSReaderData *data ) {
    gtk_tree_model_filter_refilter (data->filter_model);
}

/*
 * TODO: use GConf to load the feed data
 */
void refresh_categories( struct RSSReaderData *data ) {
    /*
     * clear the old menu and add the new one
     */
    gtk_container_foreach( GTK_CONTAINER(data->filter), (GtkCallback)remove_container_item, data->filter );

    for ( int i = 0; i < NUMBER_OF_FEEDS; ++i )
        gtk_menu_shell_append( GTK_MENU_SHELL(data->filter), gtk_menu_item_new_with_label( s_feeds[i].category ) );

    /*
     * add separator + All
     */
    gtk_menu_shell_append( GTK_MENU_SHELL(data->filter), gtk_separator_menu_item_new() );
    gtk_menu_shell_append( GTK_MENU_SHELL(data->filter), GTK_WIDGET(gtk_menu_item_new_with_label(_("All"))) );
}

gboolean cb_filter_changed( GtkWidget* widget, gchar *text, struct RSSReaderData *data ) {
    if ( data->current_filter )
        g_free( data->current_filter );
    data->current_filter = g_strdup( text );

    /*
     * shortcut for the later filter function
     */
    data->is_all_filter = strcmp(  _( "All" ), text ) == 0;
    filter_feeds( data );
    return TRUE;
}

/*
 * TODO: Update the text and make it rich text.
 *
 * TODO: Decide if a browser should be opened or not... e.g. spiegel.de only distributes
 *       the headlines and not the content. Either we fetch the code or leave it
 */
void cb_treeview_selection_changed( GtkTreeSelection *selection, struct RSSReaderData *data ) {
    GtkTreeModel* model;
    GtkTreeIter iter;
    gboolean has_selection = gtk_tree_selection_get_selected( selection, &model, &iter );

    /*
     * Update the text
     */
    if ( has_selection ) {
        gchar *message;
        gtk_tree_model_get( model, &iter, RSS_READER_COLUMN_TEXT, &message, -1 );
        if ( message )
            webkit_gtk_page_load_html_string (data->textPage, message, "");
        else
            webkit_gtk_page_load_html_string (data->textPage, _("Failed to read the text."), "");
    }
}

/*
 * search functionality
 */
void cb_searchbox_visible( MokoToolBox *box, struct RSSReaderData *data ) {
    cb_search_entry_changed (moko_tool_box_get_entry (box), data);
}

void cb_searchbox_invisible( MokoToolBox *box, struct RSSReaderData *data ) {
    if ( data->current_search_text ) {
        g_free (data->current_search_text);
        data->current_search_text = NULL;
    }

    filter_feeds (data);
    gtk_widget_grab_focus (GTK_WIDGET(data->treeView));
}

/*
 * route this to the search box unless this is a cursor movement
 */
gboolean cb_treeview_keypress_event( GtkWidget *tree_view, GdkEventKey *key, struct RSSReaderData *data ) {
    if ( key->keyval == GDK_Left || key->keyval == GDK_Right ||
         key->keyval == GDK_Up   || key->keyval == GDK_Down ) {
        return FALSE;
    }
    
    moko_tool_box_set_search_visible (data->box, TRUE);
    gtk_entry_set_text (GTK_ENTRY(moko_tool_box_get_entry(data->box)), "");

    /*
     * forward the key event
     */
    GtkEntry *entry = GTK_ENTRY(moko_tool_box_get_entry(data->box));
    GTK_WIDGET_CLASS(GTK_ENTRY_GET_CLASS(entry))->key_press_event (GTK_WIDGET(entry), key);

    return TRUE;
}

void cb_search_entry_changed      ( GtkWidget *entry, struct RSSReaderData *data ) {
    if ( data->current_search_text )
        g_free (data->current_search_text);

    data->current_search_text = g_strdup (gtk_entry_get_text (GTK_ENTRY(entry)));
    filter_feeds (data);
}

