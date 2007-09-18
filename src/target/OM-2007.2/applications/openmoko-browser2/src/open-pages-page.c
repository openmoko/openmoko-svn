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

static gint find_browser_page (struct BrowserPage* page, WebKitGtkPage* webKitPage)
{
    return !(page->webKitPage == webKitPage);
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

    WebKitGtkPage* page = 0;
    gtk_tree_model_get (model, &iter, 1, &page, -1);
    g_assert (page);

    /*
     * now find a page a BrowserPage
     */
    GList* element = g_list_find_custom(data->browserPages, page, (GCompareFunc)find_browser_page);
    if (element) {
        set_current_page ((struct BrowserPage*)element->data, data);
        gtk_notebook_set_current_page (GTK_NOTEBOOK (data->mainNotebook), 0);
    }
    
    g_object_unref (page);
}


static void pages_add_to_list_store(struct BrowserPage* page, GtkListStore* store)
{
    GtkTreeIter iter;
    gtk_list_store_append (store, &iter);
    gtk_list_store_set (store, &iter,
                        0, g_strdup (webkit_gtk_frame_get_title (webkit_gtk_page_get_main_frame (page->webKitPage))),
                        1, page->webKitPage,
                        -1);
}

/*
 * For now rebuild the GtkListStore
 *
 * XXX FIXME TODO Make Current, Go and Pages use the same GtkListStore
 * and share the code with the bookmarks.
 */
static void switched_notebook_tab(GtkNotebook* notebook, GtkNotebookPage* page, guint page_num, struct BrowserData* data)
{
    if (gtk_notebook_get_nth_page (notebook, page_num) == data->pagesBox) {
        gtk_list_store_clear (data->pagesStore);
        g_list_foreach (data->browserPages, (GFunc)pages_add_to_list_store, data->pagesStore);
    }
}

void setup_open_pages_page(GtkBox* box, struct BrowserData* data)
{
    data->pagesBox = GTK_WIDGET (box);
    g_signal_connect (data->mainNotebook, "switch-page", G_CALLBACK(switched_notebook_tab), data);

    /* URL/title and WebKitGtkPage, as it is a GObject */
    data->pagesStore = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_OBJECT);

    GtkContainer* scrolled = GTK_CONTAINER (moko_finger_scroll_new ());
    gtk_box_pack_start (box, GTK_WIDGET(scrolled), TRUE, TRUE, 0);
    data->pagesView = GTK_TREE_VIEW (gtk_tree_view_new_with_model (GTK_TREE_MODEL (data->pagesStore)));
    gtk_container_add (scrolled, GTK_WIDGET (data->pagesView));
    g_signal_connect (gtk_tree_view_get_selection (data->pagesView), "changed", G_CALLBACK(selection_changed), data);

    GtkCellRenderer* ren = GTK_CELL_RENDERER (gtk_cell_renderer_text_new ());
    GtkTreeViewColumn* column = gtk_tree_view_column_new_with_attributes (_("Title"), ren, "text", 0, NULL);
    gtk_tree_view_column_set_expand (column, TRUE);
    gtk_tree_view_append_column (data->pagesView, column);
}
