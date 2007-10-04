/*
 * A simple WebBrowser - Implementations for the "Current Page"
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
#include "current-page.h"

#include <moko-finger-scroll.h>
#include <webkitgtkframe.h>
#include <webkitgtkpage.h>

/*
 * From a list of BrowserPage's in BrowserData::currentPage show
 * one as the current one. This means we will GtkContainer::{add,remove}
 * the WebKitPage. To make that work, e.g. not destroying the WebKitPage
 * when we remove it from the container, we will keep a self added reference
 * on all WebKitPages we have created.
 */

static void current_back_clicked_closure(GtkWidget* button, struct BrowserData* data)
{
    g_return_if_fail (data->currentPage);
    webkit_page_go_backward(data->currentPage->webKitPage);
}

static void current_forward_clicked_closure(GtkWidget* button, struct BrowserData* data)
{
    g_return_if_fail (data->currentPage);
    webkit_page_go_forward(data->currentPage->webKitPage);
}

static void current_stop_reload_clicked_closure(GtkWidget* button, struct BrowserData* data)
{
}

static void current_add_bookmark_clicked_closure(GtkWidget* button, struct BrowserData* data)
{
    g_return_if_fail (data->currentPage);
    g_print ("Location to bookmark: %s\n", webkit_frame_get_title (webkit_page_get_main_frame (data->currentPage->webKitPage)));
}

static void current_progress_changed(WebKitPage* page, int prog, struct BrowserData* data)
{
    g_assert (page == data->currentPage->webKitPage);

    if (prog == 100) {
        gtk_widget_set_sensitive (GTK_WIDGET (data->currentBack), webkit_page_can_go_backward (page));
        gtk_widget_set_sensitive (GTK_WIDGET (data->currentForward), webkit_page_can_go_forward (page));
        gtk_widget_set_sensitive (GTK_WIDGET (data->currentAdd), webkit_frame_get_title (webkit_page_get_main_frame (page)) != NULL);
    }
}

static void current_close_page(GtkWidget* button, struct BrowserData* data)
{
    g_return_if_fail (data->currentPage);

    /* XXX FIXME TODO Select a better page, currently the first one is used */
    /*
     * Remove the current page and switch to the Go page or to select another tab
     */
    gtk_list_store_remove (data->browserPages, &data->currentPageIter);
    clear_current_page (data);

    GtkTreeIter iter;
    gtk_notebook_set_current_page (GTK_NOTEBOOK (data->mainNotebook),
                                   gtk_tree_model_get_iter_first (GTK_TREE_MODEL (data->browserPages), &iter) ? 2 : 1);
}

void setup_current_page(GtkBox* box, struct BrowserData* data)
{
    GtkWidget* toolbar = gtk_toolbar_new ();
    gtk_box_pack_start (box, toolbar, FALSE, FALSE, 0);


    data->currentBack = gtk_tool_button_new_from_stock (GTK_STOCK_GO_BACK);
    gtk_tool_item_set_expand (GTK_TOOL_ITEM (data->currentBack), TRUE);
    g_signal_connect (data->currentBack, "clicked", G_CALLBACK(current_back_clicked_closure), data);
    gtk_widget_set_sensitive (GTK_WIDGET(data->currentBack), FALSE);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), data->currentBack, 0);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new (), 1);

    data->currentForward = gtk_tool_button_new_from_stock (GTK_STOCK_GO_FORWARD);
    gtk_tool_item_set_expand (GTK_TOOL_ITEM (data->currentForward), TRUE);
    g_signal_connect (data->currentForward, "clicked", G_CALLBACK(current_forward_clicked_closure), data);
    gtk_widget_set_sensitive (GTK_WIDGET(data->currentForward), FALSE);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), data->currentForward, 2);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new (), 3);

    data->currentStop = gtk_tool_button_new_from_stock (GTK_STOCK_STOP);
    gtk_tool_item_set_expand (GTK_TOOL_ITEM (data->currentStop), TRUE);
    g_signal_connect (data->currentStop, "clicked", G_CALLBACK(current_stop_reload_clicked_closure), data);
    gtk_widget_set_sensitive (GTK_WIDGET(data->currentStop), FALSE);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), data->currentStop, 4);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new (), 5);

    data->currentAdd = gtk_tool_button_new_from_stock (GTK_STOCK_ADD);
    gtk_tool_item_set_expand (GTK_TOOL_ITEM (data->currentAdd), TRUE);
    g_signal_connect (data->currentAdd, "clicked", G_CALLBACK(current_add_bookmark_clicked_closure), data);
    gtk_widget_set_sensitive (GTK_WIDGET(data->currentAdd), FALSE);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), data->currentAdd, 6);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new (), 7);

    data->currentClose = gtk_tool_button_new_from_stock (GTK_STOCK_CLOSE);
    gtk_tool_item_set_expand (GTK_TOOL_ITEM (data->currentClose), TRUE);
    g_signal_connect (data->currentClose, "clicked", G_CALLBACK(current_close_page), data);
    gtk_widget_set_sensitive (GTK_WIDGET(data->currentClose), FALSE);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), data->currentClose, 8);


    data->currentFingerScroll = gtk_scrolled_window_new (NULL, NULL); //moko_finger_scroll_new ();
    gtk_box_pack_start (box, data->currentFingerScroll, TRUE, TRUE, 0);
}

void clear_current_page (struct BrowserData* data)
{
    if (data->currentPage) {
        gtk_container_remove (GTK_CONTAINER (data->currentFingerScroll), GTK_WIDGET (data->currentPage->webKitPage));
        g_signal_handlers_disconnect_by_func(data->currentPage->webKitPage, (gpointer)current_progress_changed, data);
    }

    data->currentPage = NULL;
    gtk_widget_set_sensitive (GTK_WIDGET (data->currentBack), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (data->currentForward), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (data->currentStop), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (data->currentAdd), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET (data->currentClose), FALSE);
}

/*
 * The current page changed
 */
void update_current_page_from_iter(struct BrowserData* data)
{
    g_assert (gtk_list_store_iter_is_valid (data->browserPages, &data->currentPageIter));

    BrowserPage* page;
    gtk_tree_model_get (GTK_TREE_MODEL (data->browserPages), &data->currentPageIter, 0, &page, -1);
    g_object_unref (page);

    if (page == data->currentPage)
        return;

    if (data->currentPage) {
        gtk_container_remove (GTK_CONTAINER (data->currentFingerScroll), GTK_WIDGET (data->currentPage->webKitPage));
        g_signal_handlers_disconnect_by_func(data->currentPage->webKitPage, (gpointer)current_progress_changed, data);
    }

    data->currentPage = page;
    g_signal_connect(data->currentPage->webKitPage, "load-progress-changed", G_CALLBACK(current_progress_changed), data);
    gtk_container_add (GTK_CONTAINER (data->currentFingerScroll), GTK_WIDGET (data->currentPage->webKitPage));
    gtk_widget_show (GTK_WIDGET (data->currentPage->webKitPage));

    /*
     * Update the GtkToolItems
     */
    /* XXX ### FIXME TODO check if we should show stop/reload */
    gtk_widget_set_sensitive (GTK_WIDGET (data->currentBack), webkit_page_can_go_backward (data->currentPage->webKitPage));
    gtk_widget_set_sensitive (GTK_WIDGET (data->currentForward), webkit_page_can_go_forward (data->currentPage->webKitPage));
    gtk_widget_set_sensitive (GTK_WIDGET (data->currentAdd), webkit_frame_get_title (webkit_page_get_main_frame (data->currentPage->webKitPage)) != NULL);
    gtk_widget_set_sensitive (GTK_WIDGET (data->currentClose), TRUE);
}
