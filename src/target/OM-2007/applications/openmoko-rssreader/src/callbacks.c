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

#include <mrss.h>
#include <string.h>

struct FeedEntry {
    gchar *category;
    gchar *url;
};

static struct FeedEntry s_feeds[] = {
    { "OpenMoko",   "http://planet.openmoko.org/atom.xml"  },
    { "GNOME"   ,   "http://planet.gnome.org/atom.xml"     },
    { "KDE",        "http://planet.kde.org/rss20.xml"      },
    { "Linux Togo", "http://planet.linuxtogo.org/atom.xml" },
    { "zecke"   , "http://zecke.blogspot.com/atom.xml"     },
};
static const int NUMBER_OF_FEEDS = sizeof(s_feeds)/sizeof(s_feeds[0]);

static void remove_container_item( GtkWidget *item, GtkWidget *container ) {
    gtk_container_remove(GTK_CONTAINER(container), item);
}


/*
 * TODO: Use the ModelFilter to filter for All or Category
 */
void filter_feeds( struct RSSReaderData *data ) {
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

    filter_feeds( data );
    return TRUE;
}

void cb_subscribe_button_clicked( GtkButton *btn, struct RSSReaderData *data ) {}


/*
 * asynchronous update thread!
 * This breaks with ATK+
 */
static void feed_update_thread( struct RSSReaderData *data ) {
    GtkTreeIter iter;

    for ( int i = 0; i < NUMBER_OF_FEEDS; ++i ) {
        mrss_t *rss_data;
        int ret = mrss_parse_url( s_feeds[i].url, &rss_data );
        if ( ret ) {
            /* TODO use the footer to report error? */
            g_debug( "parse_url failed.." );
            continue;
        }

        mrss_item_t *item = rss_data->item;
        while ( item ) {
            gint content_type = RSS_READER_TEXT_TYPE_NONE;
            gchar *description = item->description;

            /*
             * let us try to find the 'content' tag
             * and then extract the type
             */
            if ( !description && rss_data->version == MRSS_VERSION_ATOM_1_0 && item->other_tags ) {
                for ( mrss_tag_t *tag = item->other_tags; tag; tag = tag->next ) {
                    if ( strcmp( tag->name, "content" ) == 0 ) {
                        description = tag->value;

                        for ( mrss_attribute_t *attribute = tag->attributes; attribute; attribute = attribute->next ) {
                            if ( strcmp( attribute->name, "type" ) == 0 ) {
                                if ( strcmp( attribute->value, "plain" ) == 0 ) {
                                    content_type = RSS_READER_TEXT_TYPE_PLAIN;
                                } else if ( strcmp( attribute->name, "html" ) == 0 ) {
                                    content_type = RSS_READER_TEXT_TYPE_HTML;
                                } else {
                                    content_type = RSS_READER_TEXT_TYPE_UNKNOWN;
                                }
                            }
                        }

                        /* we are done */
                        break;
                    }
                }
            }

            /*
             * update the model here
             */
            gdk_threads_enter();
            gtk_list_store_append( data->feed_data, &iter );
            gtk_list_store_set   ( data->feed_data, &iter,
                                   RSS_READER_COLUMN_AUTHOR, g_strdup( item->author  ),
                                   RSS_READER_COLUMN_SUBJECT,g_strdup( item->title   ),
                                   RSS_READER_COLUMN_DATE,   g_strdup( item->pubDate ),
                                   RSS_READER_COLUMN_LINK,   g_strdup( item->link    ),
                                   RSS_READER_COLUMN_TEXT,   g_strdup( description   ),
                                   RSS_READER_COLUMN_TEXT_TYPE, content_type          ,
                                   RSS_READER_COLUMN_CATEGORY, g_strdup( s_feeds[i].category ),
                                   RSS_READER_COLUMN_SOURCE,  g_strdup( s_feeds[i].url ),
                                   -1 );
            gdk_threads_leave();
            item = item->next;
        }

        mrss_free( data );
    }

    gdk_threads_enter();
    filter_feeds( data );
    gdk_threads_leave();
}

/*
 * TODO use gconf and GThread as this will block
 * DISCUSSION:
 *    - Should everything be updated?
 *    - What happens if we update but some feeds can not be connected? Should the old
 *      data be kept?
 *      ( Just keeping/building a new ListStore does not help, we need a model that consists
 *        out of models... )
 *
 *
 * Clear the current model
 *
 * For each feed:
 *    - Get the Data
 *    - Fill the GtkListStore
 *
 * Refilter the model...
 */
void cb_refresh_all_button_clicked( GtkButton *btn, struct RSSReaderData *data ) {
    /*
     * once we have a SCM make it not clear the list but remove all items
     * with the same URL
     */
    gtk_list_store_clear( data->feed_data );

    /*
     * do the complete work in a new thread
     */
    GError *error = NULL;
    (void)g_thread_create( (GThreadFunc)feed_update_thread, data, FALSE, &error );

    /* XXX FOOTER do error reporting */
    if ( error != NULL ) {
        fprintf( stderr, "Can not create thread %s:%d '%s'\n", __FILE__, __LINE__, error->message );
    }

}
void cb_searchbox_visible( MokoToolBox *box, struct RSSReaderData *data ) {}
void cb_searchbox_invisible( MokoToolBox *box, struct RSSReaderData *data ) {}

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
            gtk_text_buffer_set_text( data->textBuffer, message, -1 );
        else
            gtk_text_buffer_set_text( data->textBuffer, g_strdup( "Failed to read the text" ), -1 );
    }
}

gboolean cb_treeview_keypress_event( GtkWidget *entry, GdkEventKey *key, struct RSSReaderData *data ) { return TRUE; }
void cb_search_entry_changed      ( GtkWidget *entry, struct RSSReaderData *data ) {}

