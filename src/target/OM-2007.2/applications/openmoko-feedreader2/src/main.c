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

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include <webkitgtkglobal.h>

#include <moko-finger-scroll.h>
#include <moko-stock.h>

#include "application-data.h"
#include "feed-data.h"
#include "feed-configuration.h"
#include "feed-selection-view.h"
#include "config.h"

static void
window_delete_event (GtkWidget* widget, GdkEvent* event, struct ApplicationData* data)
{
    gtk_main_quit ();
}

static void
feed_selection_changed (FeedSelectionView* view, const gchar* text, const gboolean backward, const gboolean forward, struct ApplicationData* data)
{
    feed_item_view_display (data->view, text ?  text : _("Failed to read the text."));
    feed_item_view_set_can_go_back (data->view, backward);
    feed_item_view_set_can_go_forward (data->view, forward);

    if (feed_selection_view_get_search_string (view))
        feed_item_view_highlight (data->view, feed_selection_view_get_search_string (view));

    gtk_notebook_set_current_page (GTK_NOTEBOOK (data->notebook), 1);
}

/*
 * Feed View
 */
static void
create_feed_view (struct ApplicationData* data)
{
    GtkWidget *box = feed_selection_view_new ();
    gtk_notebook_prepend_page (data->notebook, box, gtk_image_new_from_stock (GTK_STOCK_MISSING_IMAGE, GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_container_child_set (GTK_CONTAINER(data->notebook), box, "tab-expand", TRUE, "tab-fill", TRUE, NULL);

    feed_selection_view_add_column (RSS_FEED_SELECTION_VIEW (box), RSS_READER_COLUMN_SUBJECT, _("Subject"));
    feed_selection_view_add_column (RSS_FEED_SELECTION_VIEW (box), RSS_READER_COLUMN_DATE, _("Date"));

    g_signal_connect (G_OBJECT(box), "item-changed", G_CALLBACK(feed_selection_changed), data);

    data->selection_view = RSS_FEED_SELECTION_VIEW (box);
}

/*
 * Text View
 */
static void
create_text_view (struct ApplicationData* data)
{
    data->view = RSS_FEED_ITEM_VIEW(feed_item_view_new ());
    gtk_notebook_append_page (data->notebook, GTK_WIDGET(data->view), gtk_image_new_from_stock (GTK_STOCK_MISSING_IMAGE, GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_container_child_set (GTK_CONTAINER(data->notebook), GTK_WIDGET(data->view), "tab-expand", TRUE, "tab-fill", TRUE, NULL);

    g_signal_connect_swapped (G_OBJECT (data->view), "next", G_CALLBACK(feed_selection_view_next_item), data->selection_view);
    g_signal_connect_swapped (G_OBJECT (data->view), "previous", G_CALLBACK(feed_selection_view_prev_item), data->selection_view);
}


/*
 * Config related functions
 */
static void
config_new_clicked_closure(GtkWidget* button, struct ApplicationData* data)
{
}

static void
config_delete_clicked_closure(GtkWidget* button, struct ApplicationData* data)
{
}

static void
create_configuration_ui (struct ApplicationData* data, GtkCellRenderer* text_renderer)
{
    /*
     * toolbar
     */
    GtkWidget* box = gtk_vbox_new (FALSE, 0);
    gtk_notebook_append_page (data->notebook, box, gtk_image_new_from_stock (GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_container_child_set (GTK_CONTAINER(data->notebook), box, "tab-expand", TRUE, "tab-fill", TRUE, NULL);

    GtkWidget* toolbar = gtk_toolbar_new ();
    gtk_box_pack_start (GTK_BOX(box), toolbar, FALSE, FALSE, 0);

    GtkToolItem* toolitem = gtk_tool_button_new_from_stock (GTK_STOCK_NEW);
    gtk_tool_item_set_expand (GTK_TOOL_ITEM(toolitem), TRUE);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), toolitem, 0);
    g_signal_connect (toolitem, "clicked", G_CALLBACK(config_new_clicked_closure), data);

    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new (), 1);

    toolitem = gtk_tool_button_new_from_stock (GTK_STOCK_DELETE);
    gtk_tool_item_set_expand (GTK_TOOL_ITEM(toolitem), TRUE);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), toolitem, 2);
    g_signal_connect (toolitem, "clicked", G_CALLBACK(config_delete_clicked_closure), data);
    gtk_widget_set_sensitive (GTK_WIDGET(toolitem), FALSE);
    
    /* main view */
    GtkWidget* scrolled = moko_finger_scroll_new ();
    gtk_box_pack_start (GTK_BOX(box), scrolled, TRUE, TRUE, 0);
    GtkWidget* treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL(feed_configuration_get_configuration ()));
    GtkTreeViewColumn* column = GTK_TREE_VIEW_COLUMN(gtk_tree_view_column_new_with_attributes( _("Name"),
                                                                                              text_renderer,
                                                                                              "text", FEED_NAME, NULL));
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_sort_column_id (column, FEED_NAME);
    gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

    column = GTK_TREE_VIEW_COLUMN(gtk_tree_view_column_new_with_attributes( _("Url"),
                                                                            text_renderer,
                                                                            "text", FEED_URL, NULL));
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_sort_column_id (column, FEED_URL);
    gtk_tree_view_append_column (GTK_TREE_VIEW(treeview), column);

    
    gtk_container_add (GTK_CONTAINER (scrolled), treeview);
}

/*
 *  ToolBar with add and remove
 *  And a view with available feeds. No filtering for them
 *
 *  TODO:
 *      How to do the actual configuration?
 *        Name, URL are easy and fit on screen
 *        What about username, password, number of items to cache?
 */
static void
create_ui (struct ApplicationData* data)
{
    data->window = GTK_WINDOW(gtk_window_new (GTK_WINDOW_TOPLEVEL));
    g_signal_connect(data->window, "delete-event", G_CALLBACK(window_delete_event), data);

    data->notebook = GTK_NOTEBOOK(gtk_notebook_new ());
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (data->notebook), GTK_POS_BOTTOM);
    gtk_container_add (GTK_CONTAINER(data->window), GTK_WIDGET(data->notebook));

    /*
     * Create the pages of interest
     *
     * 1.) Feed Overview Subject/Date
     * 2.) Text View
     * 3.) Configuration
     */
    GtkCellRenderer *text_renderer = GTK_CELL_RENDERER(gtk_cell_renderer_text_new ());

    /*
     * 1. The default feed view
     */
    create_feed_view (data);
    
    /*
     * 2. Text View
     */
    create_text_view (data);

    /*
     * 3. Configuration
     */
    create_configuration_ui (data, text_renderer);


    gtk_widget_show_all (GTK_WIDGET(data->window));
}


int main (int argc, char** argv)
{
    g_debug( "openmoko-feedreader2 starting up" );

    /* i18n boiler plate */
    bindtextdomain (GETTEXT_PACKAGE, RSSREADER_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);


    /*
     * initialize threads for fetching the RSS in the background
     */
    g_thread_init (NULL);
    gdk_threads_init ();
    gdk_threads_enter ();
    gtk_init (&argc, &argv);
    webkit_init ();
    moko_stock_register ();
    g_set_application_name( _("FeedReader") );

    struct ApplicationData* data = g_new (struct ApplicationData, 1);
    data->cache = MOKO_CACHE(moko_cache_new ("feed-reader"));
    feed_data_set_cache (RSS_FEED_DATA(feed_data_get_instance ()), data->cache);

    create_ui (data);
    feed_data_load_from_cache (RSS_FEED_DATA(feed_data_get_instance ()));

    gtk_main();
    gdk_threads_leave();

    g_object_unref (data->cache);
    g_free (data);
    
    return 0;
}
