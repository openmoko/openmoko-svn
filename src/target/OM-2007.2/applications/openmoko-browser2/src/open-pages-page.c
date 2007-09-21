/*
 * A simple WebBrowser
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
#include "open-pages-page.h"
#include "current-page.h"

#include <webkitgtkpage.h>
#include <webkitgtkframe.h>
#include <moko-finger-scroll.h>

#include <glib/gi18n.h>

static void open_pages_page_cell_data_func(GtkTreeViewColumn* tree_column, GtkCellRenderer* ren, GtkTreeModel* tree_model, GtkTreeIter* iter, gpointer data)
{
    BrowserPage* page;
    gtk_tree_model_get (tree_model, iter, 0, &page, -1);
    g_assert (page);

    /* XXX, FIXME, TODO check that we don't have any race conditions here. We might get a new title inside WebKit while using that string? */
    g_object_set (G_OBJECT (ren), "text", webkit_gtk_frame_get_title (webkit_gtk_page_get_main_frame (page->webKitPage)), NULL);
    g_object_unref (page);
}

/*
 * Switch the current page and the GtkNotebook page
 */
static void selection_changed(GtkTreeSelection* selection, struct BrowserData* data)
{
    GtkTreeModel* model;
    GtkTreeIter iter;
    gboolean has_selection = gtk_tree_selection_get_selected (selection, &model, &iter);

    if (!has_selection)
        return;

    data->currentPageIter = iter;
    update_current_page_from_iter (data);
}


void setup_open_pages_page(GtkBox* box, struct BrowserData* data)
{
    data->pagesBox = GTK_WIDGET (box);

    GtkContainer* scrolled = GTK_CONTAINER (moko_finger_scroll_new ());
    gtk_box_pack_start (box, GTK_WIDGET(scrolled), TRUE, TRUE, 0);
    data->pagesView = GTK_TREE_VIEW (gtk_tree_view_new_with_model (GTK_TREE_MODEL (data->browserPages)));
    gtk_container_add (scrolled, GTK_WIDGET (data->pagesView));
    g_signal_connect (gtk_tree_view_get_selection (data->pagesView), "changed", G_CALLBACK(selection_changed), data);

    GtkCellRenderer* ren = GTK_CELL_RENDERER (gtk_cell_renderer_text_new ());
    GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes (_("Title"), ren, NULL);
    gtk_tree_view_column_set_cell_data_func (column, ren, open_pages_page_cell_data_func, NULL, NULL);
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_append_column (data->pagesView, column);
}
