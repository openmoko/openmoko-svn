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
#include "feed-selection-view.h"

#include <moko-finger-scroll.h>
#include <glib/gi18n.h>
#include <gdk/gdkkeysyms.h>

enum {
    ITEM_CHANGED,
    LAST_SIGNAL
};

static guint feed_selection_view_signals[LAST_SIGNAL] = { 0, };

static void
treeview_selection_changed (GtkTreeSelection *selection, FeedSelectionView *view)
{
    GtkTreeModel* model;
    GtkTreeIter iter;
    gboolean has_selection = gtk_tree_selection_get_selected (selection, &model, &iter);
    gchar* message = 0;

    if (has_selection) {
        gtk_tree_model_get (model, &iter, RSS_READER_COLUMN_TEXT, &message, -1 );
        if (!message)
            message = g_strdup (_("Failed to read the text."));
    } else
        message = g_strdup (_("Please select a feed."));

    g_signal_emit (view, feed_selection_view_signals[ITEM_CHANGED], 0, message);
    g_free (message);
}

gboolean treeview_keypress_event( GtkWidget *tree_view, GdkEventKey *key, FeedSelectionView *view) {
    if (key->keyval == GDK_Left || key->keyval == GDK_Right || key->keyval == GDK_Up || key->keyval == GDK_Down)
        return FALSE;
    
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (view->search_toggle), TRUE);
    gtk_entry_set_text (GTK_ENTRY(view->search_entry), "");
    GTK_WIDGET_CLASS(GTK_ENTRY_GET_CLASS(view->search_entry))->key_press_event (GTK_WIDGET(view->search_entry), key);

    return TRUE;
}

static void
search_toggled (GtkWidget* button, FeedSelectionView* view)
{
    gboolean search_active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (button));

    if (search_active) {
        gtk_widget_show (GTK_WIDGET (view->search_entry));
        gtk_widget_hide (GTK_WIDGET (view->category_combo));
        gtk_widget_grab_focus (GTK_WIDGET (view->search_entry));
    } else {
        gtk_widget_hide (GTK_WIDGET (view->search_entry));
        gtk_widget_show (GTK_WIDGET (view->category_combo));
    }
}

static void
search_entry_changed (GtkEntry* entry, FeedSelectionView* view)
{
    feed_filter_filter_text (view->filter, gtk_entry_get_text (entry)); 
}

static void
refresh_feeds_closure (GtkWidget* button, FeedSelectionView* view)
{
    feed_data_update_all (RSS_FEED_DATA (feed_data_get_instance ()));
}

static void
category_combo_update (FeedSelectionView* view, ...)
{
    gtk_list_store_clear (GTK_LIST_STORE (gtk_combo_box_get_model (GTK_COMBO_BOX (view->category_combo))));
    gtk_combo_box_append_text (GTK_COMBO_BOX (view->category_combo), _("All"));

    GtkTreeModel *store = GTK_TREE_MODEL (feed_configuration_get_configuration ());
    GtkTreeIter iter;

    gboolean valid = gtk_tree_model_get_iter_first (store, &iter);
    while (valid) {
        gchar *category;
        gtk_tree_model_get (store, &iter, FEED_NAME, &category, -1);

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
}

G_DEFINE_TYPE(FeedSelectionView, feed_selection_view, GTK_TYPE_VBOX)

static void
feed_selection_view_init (FeedSelectionView* view)
{
    GTK_BOX(view)->spacing = 0;
    GTK_BOX(view)->homogeneous = FALSE;

    view->filter = RSS_FEED_FILTER (feed_filter_new (RSS_FEED_DATA (feed_data_get_instance ())));
    view->sort = RSS_FEED_SORT (feed_sort_new (view->filter));

    /*
     * toolbar
     */
    GtkWidget* toolbar = gtk_toolbar_new ();
    gtk_box_pack_start (GTK_BOX(view), toolbar, FALSE, FALSE, 0);

    GtkToolItem* toolitem = gtk_tool_button_new_from_stock (GTK_STOCK_MISSING_IMAGE);
    gtk_tool_item_set_expand (GTK_TOOL_ITEM(toolitem), TRUE);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), toolitem, 0);
    g_signal_connect (toolitem, "clicked", G_CALLBACK(refresh_feeds_closure), view);

    /*
     * search/filter
     */
    GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (view), hbox, FALSE, FALSE, 0);

    view->search_toggle = gtk_toggle_button_new ();
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (view->search_toggle), FALSE, FALSE, 0);
    gtk_widget_set_name (GTK_WIDGET (view->search_toggle), "mokosearchbutton");
    gtk_button_set_image (GTK_BUTTON (view->search_toggle), gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_SMALL_TOOLBAR));
    g_signal_connect (G_OBJECT (view->search_toggle), "toggled", G_CALLBACK (search_toggled), view);

    view->search_entry = GTK_ENTRY (gtk_entry_new ());
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (view->search_entry), TRUE, TRUE, 0);
    gtk_widget_set_name (GTK_WIDGET (view->search_entry), "mokosearchentry");
    g_signal_connect (G_OBJECT (view->search_entry), "changed", G_CALLBACK (search_entry_changed), view);
    g_object_set (G_OBJECT (view->search_entry), "no-show-all", TRUE, NULL);

    view->category_combo = GTK_WIDGET (gtk_combo_box_new ());
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (view->category_combo), TRUE, TRUE, 0);
    category_combo_update (view);
    g_signal_connect (G_OBJECT (view->category_combo), "changed", G_CALLBACK(category_selection_changed), view);
    g_signal_connect_swapped (feed_configuration_get_configuration (), "row-changed",  G_CALLBACK(category_combo_update), view);
    g_signal_connect_swapped (feed_configuration_get_configuration (), "row-inserted", G_CALLBACK(category_combo_update), view);
    g_signal_connect_swapped (feed_configuration_get_configuration (), "row-deleted",  G_CALLBACK(category_combo_update), view);

    /*
     * selection view
     */
    GtkContainer* scrolled = GTK_CONTAINER (moko_finger_scroll_new ());
    gtk_box_pack_start (GTK_BOX (view), GTK_WIDGET (scrolled), TRUE, TRUE, 0);
    
    view->view = GTK_TREE_VIEW (gtk_tree_view_new_with_model (GTK_TREE_MODEL (view->sort)));
    gtk_container_add (scrolled, GTK_WIDGET(view->view));

    GtkTreeSelection *selection = GTK_TREE_SELECTION (gtk_tree_view_get_selection(GTK_TREE_VIEW (view->view)));
    g_signal_connect (G_OBJECT (selection), "changed", G_CALLBACK (treeview_selection_changed), view);
    g_signal_connect (G_OBJECT (view->view), "key_press_event", G_CALLBACK (treeview_keypress_event), view);
}

static void
feed_selection_view_class_init (FeedSelectionViewClass* class)
{
    feed_selection_view_signals[ITEM_CHANGED] = g_signal_new ("item_changed",
            G_TYPE_FROM_CLASS(class),
            (GSignalFlags) (G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__STRING,
            G_TYPE_NONE, 1, G_TYPE_STRING);
}


GtkWidget*
feed_selection_view_new (void)
{
    return GTK_WIDGET (g_object_new (RSS_TYPE_FEED_SELECTION_VIEW, NULL));
}

void
feed_selection_view_add_column (const FeedSelectionView* view, int column_type, const gchar* name)
{
    GtkCellRenderer *ren = GTK_CELL_RENDERER (gtk_cell_renderer_text_new ());
    GtkTreeViewColumn *column;

    if (column_type == RSS_READER_COLUMN_DATE) {
        column = GTK_TREE_VIEW_COLUMN (gtk_tree_view_column_new_with_attributes (name, ren, NULL));
        gtk_tree_view_column_set_cell_data_func (column, ren, feed_date_cell_data_func, NULL, NULL);
    } else
        column = GTK_TREE_VIEW_COLUMN (gtk_tree_view_column_new_with_attributes (name, ren, "text", column_type, NULL));

    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_sort_column_id (column, RSS_READER_COLUMN_SUBJECT);
    gtk_tree_view_append_column (view->view, column);
}

gchar*
feed_selection_view_get_search_string (const FeedSelectionView* view)
{
    return view->filter->filter_string;
}


