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

#include "feed-data.h"
#include "feed-configuration.h"
#include "rfcdate.h"
#include "config.h"

#include <mrss.h>
#include <string.h>
#include <stdlib.h>

#include <glib/gi18n.h>
    
static gboolean
rss_filter_entries (GtkTreeModel *model, GtkTreeIter *iter, FeedFilter *data)
{
    /*
     * filter the category
     */
    if (!data->is_all_filter) {
        gchar *category;
        gtk_tree_model_get (model, iter,  RSS_READER_COLUMN_CATEGORY, &category,  -1);

        /*
         * how does this happen?
         */
        if (!category)
            return FALSE;

        if (strcmp(category, data->filter_string) != 0)
            return FALSE;

        g_free (category);
    }


    /*
     * filter the text according to the search now
     */
    if (data->filter_string) {
        gchar *text;

        #define FILTER_SEARCH(column)                                      \
        gtk_tree_model_get (model, iter, column, &text, -1);               \
        if (text && strcasestr (text, data->filter_string) != NULL) {      \
            g_free (text);                                                 \
            return TRUE;                                                   \
        }

        FILTER_SEARCH(RSS_READER_COLUMN_AUTHOR)
        FILTER_SEARCH(RSS_READER_COLUMN_SUBJECT)
        FILTER_SEARCH(RSS_READER_COLUMN_SOURCE)
        FILTER_SEARCH(RSS_READER_COLUMN_LINK)
        FILTER_SEARCH(RSS_READER_COLUMN_TEXT)

        #undef FILTER_SEARCH
        return FALSE;
    }

    return TRUE;
}

/*
 * sort the dates according to zsort. Ideally they should sort ascending
 */
static gint
rss_sort_dates (GtkTreeModel *model, GtkTreeIter *_left, GtkTreeIter *_right, gpointer that)
{
    RSSRFCDate *left, *right;
    gtk_tree_model_get (model, _left,  RSS_READER_COLUMN_DATE, &left,  -1);
    gtk_tree_model_get (model, _right, RSS_READER_COLUMN_DATE, &right, -1);

    int result;
    if (left == NULL)
        result = -1;
    else if (right == NULL)
        result = 1;
    else
        result = rss_rfc_date_compare (left, right);

    if (left)
        g_object_unref (left);
    if (right)
        g_object_unref (right);

    return result;
}

/*
 * Add items from rss_data to the GtkListStore/FeedData
 */
static void
add_mrss_item (FeedData *data, const mrss_t *rss_data, const gchar *url, const gchar *category)
{
    GtkTreeIter iter;
    mrss_item_t *item = rss_data->item;

    while (item) {
        gint content_type = RSS_READER_TEXT_TYPE_NONE;
        gchar *description = item->description;

        /*
         * let us try to find the 'content' tag
         * and then extract the type
         */
        if (!description && rss_data->version == MRSS_VERSION_ATOM_1_0 && item->other_tags) {
            for (mrss_tag_t *tag = item->other_tags; tag; tag = tag->next) {
                if (strcmp( tag->name, "content") == 0) {
                    description = tag->value;

                    for (mrss_attribute_t *attribute = tag->attributes; attribute; attribute = attribute->next) {
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
        gtk_list_store_append ( GTK_LIST_STORE (data), &iter );
        gtk_list_store_set    ( GTK_LIST_STORE (data), &iter,
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
static void
feed_update_thread (FeedData *data) {
    GtkTreeModel *store = GTK_TREE_MODEL (feed_configuration_get_configuration ());
    GtkTreeIter iter;

    gboolean valid = gtk_tree_model_get_iter_first (store, &iter);
    while (valid) {
        mrss_t *rss_data;
        gchar *url;
        gchar *category;
        gchar *buffer = NULL;
        int  size;

        gtk_tree_model_get (store, &iter, FEED_URL, &url, FEED_NAME, &category, -1);
        int ret = mrss_parse_url_and_transfer_buffer( url, &rss_data, NULL, &buffer, &size );

        if (ret) {
            /* TODO use the footer to report error? */
            g_debug ("parse_url failed.. '%s'", url);
            goto next;
        }

        /*
         * create the new item(s)
         */
        add_mrss_item (data, rss_data, url, category);

        /*
         * now cache the feed, a bit inefficient as we do not write to a file directly
         */
        if (buffer) {
            moko_cache_write_object (data->cache, url, buffer, size, NULL);
            free (buffer);
        }

        mrss_free( rss_data );

next:
        g_free (url);
        g_free (category);
        valid = gtk_tree_model_iter_next (store, &iter);
    }

#ifdef UNSURE
    gdk_threads_enter();
    filter_feeds( data );
    gdk_threads_leave();
#endif
}

G_DEFINE_TYPE(FeedData,   feed_data,   GTK_TYPE_LIST_STORE)
G_DEFINE_TYPE(FeedFilter, feed_filter, GTK_TYPE_TREE_MODEL_FILTER)
G_DEFINE_TYPE(FeedSort,   feed_sort,   GTK_TYPE_TREE_MODEL_SORT)

static void
feed_data_finalize (GObject* obj)
{
    if (RSS_FEED_DATA (obj)->cache) {
        g_object_unref (RSS_FEED_DATA (obj)->cache);
        RSS_FEED_DATA (obj)->cache = NULL;
    }

    G_OBJECT_CLASS (feed_data_parent_class)->finalize (obj);
}

static void
feed_data_init (FeedData *data)
{
    GType types[RSS_READER_NUM_COLS];
    types[0] = G_TYPE_STRING;     /* Author    */
    types[1] = G_TYPE_STRING;     /* Subject   */
    types[2] = RSS_TYPE_RFC_DATE; /* Date      */
    types[3] = G_TYPE_STRING;     /* Link      */
    types[4] = G_TYPE_STRING;     /* Text      */
    types[5] = G_TYPE_INT;        /* Text_Type */
    types[6] = G_TYPE_STRING;     /* Category/Feed name  */
    types[7] = G_TYPE_STRING;     /* Source    */
    gtk_list_store_set_column_types (GTK_LIST_STORE (data), RSS_READER_NUM_COLS, types);
}

static void
feed_data_class_init (FeedDataClass *feed_class)
{
    G_OBJECT_CLASS(feed_class)->finalize = feed_data_finalize;
}

static void
feed_filter_init (FeedFilter *filter)
{
    filter->is_all_filter = TRUE;
    filter->category = NULL;
    filter->filter_string = NULL,

    gtk_tree_model_filter_set_visible_func (GTK_TREE_MODEL_FILTER (filter), (GtkTreeModelFilterVisibleFunc)rss_filter_entries, filter, NULL);
}

static void
feed_filter_class_init (FeedFilterClass *filter_class)
{
}

static void
feed_sort_init (FeedSort* sort)
{
}

static void
feed_sort_class_init (FeedSortClass* sort_class)
{
}

GObject*
feed_data_get_instance (void)
{
    static GObject* instance = 0;
    if (!instance)
        instance = g_object_new(RSS_TYPE_FEED_DATA, NULL);

    return instance;
}

void
feed_data_update_all (FeedData* data)
{
    gtk_list_store_clear (GTK_LIST_STORE (data));

    GError *error = NULL;
    (void)g_thread_create( (GThreadFunc)feed_update_thread, data, FALSE, &error );

    /* XXX FOOTER do error reporting */
    if (error != NULL)
        fprintf( stderr, "Can not create thread %s:%d '%s'\n", __FILE__, __LINE__, error->message );
}

void
feed_data_set_cache (FeedData* data, MokoCache* cache)
{
    if (data->cache)
        g_object_unref (data->cache);

    g_object_ref (cache);
    data->cache = cache; 
}

/*
 * read the feeds from disk
 * Similiar to the Thread, but
 *  -We do not run from a thread, so no gdk locking is necessary
 *  -We do not load from a url but from the cache
 *  -We do not need to cache ;)
 */
void
feed_data_load_from_cache (FeedData* data)
{
    gsize size;
    GtkTreeIter iter;

    GtkTreeModel *store = GTK_TREE_MODEL (feed_configuration_get_configuration ());
    gboolean valid = gtk_tree_model_get_iter_first ( GTK_TREE_MODEL (store), &iter);

    while (valid) {
        mrss_t *rss_data;
        gchar *url;
        gchar *category;

        gtk_tree_model_get ( GTK_TREE_MODEL (store), &iter, FEED_URL, &url, FEED_NAME, &category, -1);
        g_debug ("Reading cached object '%s'\n", url);
        gchar *content = moko_cache_read_object (data->cache, url, &size);
        if ( !content || size == -1 ) {
            g_debug ("Noting in the cache for '%s'\n", url);
            goto next;
        }

        int ret = mrss_parse_buffer( content, size, &rss_data );
        if ( ret ) {
            /* TODO use the footer to report error? */
            g_debug( "parse_buffer of '%s' failed with '%d'", url, ret );
            goto next;
        }

        /*
         * create the new item(s)
         */
        gdk_threads_leave();
        add_mrss_item (data, rss_data, url, category);
        gdk_threads_enter();

        g_free (content);
        mrss_free (rss_data);
    next:
        g_free (url);
        g_free (category);
        valid = gtk_tree_model_iter_next ( GTK_TREE_MODEL (store), &iter);
    }

    g_debug ("Done loading from cache\n");
}


GObject*
feed_filter_new (const FeedData* data)
{
    GObject* obj = g_object_new(RSS_TYPE_FEED_FILTER,
                                "child-model", data,
                                "virtual-root", NULL,
                                NULL);

    return obj;
}

void
feed_filter_reset (FeedFilter* filter)
{
}

void
feed_filter_filter_category (FeedFilter* filter, const gchar* text)
{
    if (filter->category)
        g_free (filter->category);

    filter->category = g_strdup (text);
    filter->is_all_filter = strcmp (_("All"), text) == 0;
    gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (filter));
}

void
feed_filter_filter_text (FeedFilter* filter, const gchar* text)
{
    if (filter->filter_string)
        g_free (filter->filter_string);

    filter->filter_string = g_strdup (text);
    gtk_tree_model_filter_refilter (GTK_TREE_MODEL_FILTER (filter));
}

GObject*
feed_sort_new (const FeedFilter* filter)
{
    GObject* obj = g_object_new (RSS_TYPE_FEED_SORT,
                                 "model", filter,
                                 NULL);

    gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (obj), RSS_READER_COLUMN_DATE, GTK_SORT_DESCENDING);
    gtk_tree_sortable_set_sort_func (GTK_TREE_SORTABLE (obj), RSS_READER_COLUMN_DATE, rss_sort_dates, NULL, NULL);

    return obj;
}



void
feed_date_cell_data_func (GtkTreeViewColumn *tree_column, GtkCellRenderer *renderer, GtkTreeModel *tree_model, GtkTreeIter *iter, gpointer data)
{
    RSSRFCDate *date;
    gtk_tree_model_get (tree_model, iter, RSS_READER_COLUMN_DATE, &date, -1);

    g_assert (date);
    g_object_set ( G_OBJECT(renderer), "text", rss_rfc_date_as_string(date), NULL);
    g_object_unref (G_OBJECT(date));
}
