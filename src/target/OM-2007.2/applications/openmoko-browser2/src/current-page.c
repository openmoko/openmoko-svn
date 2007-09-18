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
 * the WebKitGtkPage. To make that work, e.g. not destroying the WebKitGtkPage
 * when we remove it from the container, we will keep a self added reference
 * on all WebKitGtkPages we have created.
 */

static void current_back_clicked_closure(GtkWidget* button, struct BrowserData* data)
{
    g_return_if_fail (data->currentPage);
    webkit_gtk_page_go_backward(data->currentPage->webKitPage);
}

static void current_forward_clicked_closure(GtkWidget* button, struct BrowserData* data)
{
    g_return_if_fail (data->currentPage);
    webkit_gtk_page_go_forward(data->currentPage->webKitPage);
}

static void current_stop_reload_clicked_closure(GtkWidget* button, struct BrowserData* data)
{
}

static void current_add_bookmark_clicked_closure(GtkWidget* button, struct BrowserData* data)
{
    g_return_if_fail (data->currentPage);
    g_print ("Location to bookmark: %s\n", webkit_gtk_frame_get_title (webkit_gtk_page_get_main_frame (data->currentPage->webKitPage)));
}

static void current_progress_changed(WebKitGtkPage* page, int prog, struct BrowserData* data)
{
}

static void current_close_page(GtkWidget* button, struct BrowserData* data)
{
    g_return_if_fail (data->currentPage);

    /* XXX FIXME TODO Select a better page, currently the first one is used */
    /* To destroy the WebKitGtkPage we will g_object_unref it */
    struct BrowserPage* oldCurrent = data->currentPage;
    data->browserPages = g_list_remove (data->browserPages, oldCurrent);
    struct BrowserPage* newPage = g_list_first(data->browserPages) ? g_list_first(data->browserPages)->data : NULL;
    set_current_page (newPage, data);


    g_object_unref (oldCurrent->webKitPage);
    g_free (oldCurrent);

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

/*
 * The current page changed
 */
void set_current_page(struct BrowserPage* page, struct BrowserData* data)
{
    if (page == data->currentPage)
        return;

    if (data->currentPage) {
        gtk_container_remove (GTK_CONTAINER (data->currentFingerScroll), GTK_WIDGET (data->currentPage->webKitPage));
        g_signal_handlers_disconnect_by_func(data->currentPage->webKitPage, (gpointer)current_progress_changed, data);
    }

    if (!page) {
        data->currentPage = NULL;
        gtk_widget_set_sensitive (GTK_WIDGET (data->currentBack), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (data->currentForward), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (data->currentStop), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (data->currentAdd), FALSE);
        gtk_widget_set_sensitive (GTK_WIDGET (data->currentClose), FALSE);
    } else {
        data->currentPage = page;
        g_signal_connect(data->currentPage->webKitPage, "load-progress-changed", G_CALLBACK(current_progress_changed), data);
        gtk_container_add (GTK_CONTAINER (data->currentFingerScroll), GTK_WIDGET (data->currentPage->webKitPage));

        /*
         * Update the GtkToolItems
         */
        /* XXX ### FIXME TODO check if we should show stop/reload */
        gtk_widget_set_sensitive (GTK_WIDGET (data->currentBack), webkit_gtk_page_can_go_backward (data->currentPage->webKitPage));
        gtk_widget_set_sensitive (GTK_WIDGET (data->currentForward), webkit_gtk_page_can_go_forward (data->currentPage->webKitPage));
        gtk_widget_set_sensitive (GTK_WIDGET (data->currentAdd), webkit_gtk_frame_get_title (webkit_gtk_page_get_main_frame (data->currentPage->webKitPage)) != NULL);
        gtk_widget_set_sensitive (GTK_WIDGET (data->currentClose), TRUE);
    }
}
