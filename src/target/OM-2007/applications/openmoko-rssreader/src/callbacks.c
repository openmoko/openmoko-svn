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

void cb_subscribe_button_clicked( GtkButton *btn, struct RSSReaderData *data ) {}


static
void add_mrss_item ( struct RSSReaderData *data, const mrss_t *rss_data, const gchar *url, const gchar *category)
{
    GtkTreeIter iter;
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
                        /*
                         * Detect the type of the content. Currently we know about text/plain and html
                         */
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
         * update the model here. The order in gtk_list_store_set must match
         * with the order in application-data.h
         */
        RSSRFCDate *date = RSS_RFC_DATE(rss_rfc_date_new ());
        rss_rfc_date_set (date, item->pubDate ? item->pubDate : "");
        gdk_threads_enter();
        gtk_list_store_append( data->feed_data, &iter );
        gtk_list_store_set   ( data->feed_data, &iter,
                RSS_READER_COLUMN_AUTHOR, g_strdup( item->author  ),
                RSS_READER_COLUMN_SUBJECT,g_strdup( item->title   ),
                RSS_READER_COLUMN_DATE,   date,
                RSS_READER_COLUMN_LINK,   g_strdup( item->link    ),
                RSS_READER_COLUMN_TEXT,   g_strdup( description   ),
                RSS_READER_COLUMN_TEXT_TYPE, content_type          ,
                RSS_READER_COLUMN_CATEGORY, g_strdup( category ),
                RSS_READER_COLUMN_SOURCE,  g_strdup( url ),
                -1 );
        gdk_threads_leave();
        item = item->next;
    }

}

/*
 * asynchronous update thread!
 * This breaks with ATK+. See http://wiki.ekiga.org/index.php/Bug::ATK::Threads and bugs
 * #329454, #349047, #335838
 *
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
static void feed_update_thread( struct RSSReaderData *data ) {
    for ( int i = 0; i < NUMBER_OF_FEEDS; ++i ) {
        mrss_t *rss_data;
        gchar *url = s_feeds[i].url;
        gchar *buffer = NULL;
        int  size;
        int ret = mrss_parse_url_and_transfer_buffer( url, &rss_data, NULL, &buffer, &size );
        if ( ret ) {
            /* TODO use the footer to report error? */
            g_debug( "parse_url failed.." );
            continue;
        }

        /*
         * create the new item(s)
         */
        add_mrss_item (data, rss_data, url, s_feeds[i].category);

        /*
         * now cache the feed, a bit inefficient as we do not write to a file directly
         */
        if (buffer) {
            moko_cache_write_object (data->cache, url, buffer, size, NULL);
            free (buffer);
        }

        mrss_free( rss_data );
    }

    gdk_threads_enter();
    filter_feeds( data );
    gdk_threads_leave();
}

/**
 * read the feeds from disk
 * Similiar to the Thread, but
 *  -We do not run from a thread, so no gdk locking is necessary
 *  -We do not load from a url but from the cache
 *  -We do not need to cache ;)
 */
void load_data_from_cache (struct RSSReaderData *data)
{
    gsize size;

    for ( int i = 0; i < NUMBER_OF_FEEDS; ++i ) {
        mrss_t *rss_data;
        gchar *url = s_feeds[i].url;
        g_debug ("Reading cached object '%s'\n", url);
        gchar *content = moko_cache_read_object (data->cache, url, &size);
        if ( !content || size == -1 ) {
            g_debug ("Noting in the cache for '%s'\n", url);
            continue;
        }

        int ret = mrss_parse_buffer( content, size, &rss_data );
        if ( ret ) {
            /* TODO use the footer to report error? */
            g_debug( "parse_buffer of '%s' failed with '%d'", url, ret );
            continue;
        }

        /*
         * create the new item(s)
         */
        gdk_threads_leave();
        add_mrss_item (data, rss_data, url, s_feeds[i].category);
        gdk_threads_enter();

        g_free (content);
        mrss_free (rss_data);
    }

    g_debug ("Done loading from cache\n");
}

/*
 * Start the update-job in a separate thread
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

