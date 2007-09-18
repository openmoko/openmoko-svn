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

static void current_back_clicked_closure(GtkWidget* button, struct BrowserData* data)
{
}

static void current_forward_clicked_closure(GtkWidget* button, struct BrowserData* data)
{
}

static void current_stop_reload_clicked_closure(GtkWidget* button, struct BrowserData* data)
{
}

static void current_add_bookmark_clicked_closure(GtkWidget* button, struct BrowserData* data)
{
}

void setup_current_page(GtkBox* box, struct BrowserData* data)
{
    GtkWidget* toolbar = gtk_toolbar_new ();
    gtk_box_pack_start (box, toolbar, FALSE, FALSE, 0);


    GtkToolItem* toolitem;
    toolitem = gtk_tool_button_new_from_stock (GTK_STOCK_GO_BACK);
    gtk_tool_item_set_expand (GTK_TOOL_ITEM (toolitem), TRUE);
    g_signal_connect (toolitem, "clicked", G_CALLBACK(current_back_clicked_closure), data);
    gtk_widget_set_sensitive (GTK_WIDGET(toolitem), FALSE);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), toolitem, 0);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new (), 1);

    toolitem = gtk_tool_button_new_from_stock (GTK_STOCK_GO_FORWARD);
    gtk_tool_item_set_expand (GTK_TOOL_ITEM (toolitem), TRUE);
    g_signal_connect (toolitem, "clicked", G_CALLBACK(current_forward_clicked_closure), data);
    gtk_widget_set_sensitive (GTK_WIDGET(toolitem), FALSE);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), toolitem, 2);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new (), 3);

    toolitem = gtk_tool_button_new_from_stock (GTK_STOCK_STOP);
    gtk_tool_item_set_expand (GTK_TOOL_ITEM (toolitem), TRUE);
    g_signal_connect (toolitem, "clicked", G_CALLBACK(current_stop_reload_clicked_closure), data);
    gtk_widget_set_sensitive (GTK_WIDGET(toolitem), FALSE);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), toolitem, 4);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), gtk_separator_tool_item_new (), 5);

    toolitem = gtk_tool_button_new_from_stock (GTK_STOCK_ADD);
    gtk_tool_item_set_expand (GTK_TOOL_ITEM (toolitem), TRUE);
    g_signal_connect (toolitem, "clicked", G_CALLBACK(current_add_bookmark_clicked_closure), data);
    gtk_widget_set_sensitive (GTK_WIDGET(toolitem), FALSE);
    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), toolitem, 6);

    data->currentFingerScroll = moko_finger_scroll_new ();
    gtk_box_pack_start (box, data->currentFingerScroll, TRUE, TRUE, 0);
}
