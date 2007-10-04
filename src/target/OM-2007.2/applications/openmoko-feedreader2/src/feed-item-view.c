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

#include "feed-item-view.h"

#include <moko-finger-scroll.h>
#include <moko-stock.h>

enum {
    NEXT,
    PREV,
    MAIL,
    LAST_SIGNAL
};

static guint feed_item_view_signals[LAST_SIGNAL] = { 0, };

static void
prev_clicked (GtkWidget* wid, FeedItemView* view)
{
    g_signal_emit (view, feed_item_view_signals[PREV], 0);
}

static void
next_clicked (GtkWidget* wid, FeedItemView* view)
{
    g_signal_emit (view, feed_item_view_signals[NEXT], 0);
}

static void
mail_clicked (GtkWidget* wid, FeedItemView* view)
{
    g_signal_emit (view, feed_item_view_signals[MAIL], 0);
}

static void
do_highlight (FeedItemView* view, const gchar* search_string)
{
    /* webkit_page_search (view->page, search_string); */
}

static void
search_entry_changed_closure (GtkEntry* entry, FeedItemView* view)
{
    do_highlight (view, gtk_entry_get_text (entry));
}


G_DEFINE_TYPE(FeedItemView, feed_item_view, GTK_TYPE_VBOX)

static void
feed_item_view_init (FeedItemView* view)
{
    GTK_BOX(view)->spacing = 0;
    GTK_BOX(view)->homogeneous = FALSE;

    GtkWidget* toolbar = gtk_toolbar_new ();
    gtk_box_pack_start (GTK_BOX(view), toolbar, FALSE, FALSE, 0);

    view->back = gtk_tool_button_new_from_stock (GTK_STOCK_GO_BACK);
    gtk_tool_item_set_expand (GTK_TOOL_ITEM(view->back), TRUE);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), view->back, 0);
    g_signal_connect (view->back, "clicked", G_CALLBACK(prev_clicked), view);

    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), gtk_separator_tool_item_new (), 1);
    
    view->mail = gtk_tool_button_new_from_stock (MOKO_STOCK_MAIL_SEND);
    gtk_tool_item_set_expand (GTK_TOOL_ITEM(view->mail), TRUE);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), view->mail, 2);
    g_signal_connect (view->mail, "clicked", G_CALLBACK(mail_clicked), view);

    gtk_toolbar_insert (GTK_TOOLBAR (toolbar), gtk_separator_tool_item_new (), 3);

    view->forward = gtk_tool_button_new_from_stock (GTK_STOCK_GO_FORWARD);
    gtk_tool_item_set_expand (GTK_TOOL_ITEM(view->forward), TRUE);
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), view->forward, 4);
    g_signal_connect (view->forward, "clicked", G_CALLBACK(next_clicked), view);

    /*
     * Search Entry
     */
    GtkWidget *hbox = gtk_hbox_new (FALSE, 0);
    gtk_box_pack_start (GTK_BOX (view), hbox, FALSE, FALSE, 0);

    view->search_button = gtk_button_new ();
    gtk_widget_set_name (GTK_WIDGET (view->search_button), "mokosearchbutton");
    gtk_button_set_image (GTK_BUTTON (view->search_button), gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_SMALL_TOOLBAR));
    gtk_box_pack_start (GTK_BOX (hbox), view->search_button, FALSE, FALSE, 0);

    view->search_entry = GTK_ENTRY (gtk_entry_new ());
    gtk_widget_set_name (GTK_WIDGET (view->search_entry), "mokosearchentry");
    g_signal_connect (G_OBJECT (view->search_entry), "changed", G_CALLBACK (search_entry_changed_closure), view);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (view->search_entry), TRUE, TRUE, 0);

    /*
     * Details 'pane'
     */
    GtkWidget* scrolled = moko_finger_scroll_new ();
    gtk_box_pack_start (GTK_BOX(view), scrolled, TRUE, TRUE, 0);

    view->page = WEBKIT_PAGE(webkit_page_new ());
    gtk_container_add (GTK_CONTAINER(scrolled), GTK_WIDGET(view->page));

    /*
     * disable this page
     */
    gtk_widget_set_sensitive (GTK_WIDGET(view->back), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET(view->forward), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET(view->mail), FALSE);
    gtk_widget_set_sensitive (GTK_WIDGET(view->page), FALSE);
}

static void
feed_item_view_class_init (FeedItemViewClass* view_class)
{
    feed_item_view_signals[NEXT] = g_signal_new ("next",
            G_TYPE_FROM_CLASS(view_class),
            (GSignalFlags) (G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);

    feed_item_view_signals[PREV] = g_signal_new ("previous",
            G_TYPE_FROM_CLASS(view_class),
            (GSignalFlags) (G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
    feed_item_view_signals[MAIL] = g_signal_new ("mail",
            G_TYPE_FROM_CLASS(view_class),
            (GSignalFlags) (G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION),
            0,
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
}

GtkWidget*
feed_item_view_new (void)
{
    return GTK_WIDGET(g_object_new (RSS_TYPE_FEED_ITEM_VIEW, NULL));
}

void
feed_item_view_set_can_go_back (FeedItemView* view, gboolean can_go_back)
{
    gtk_widget_set_sensitive (GTK_WIDGET(view->back), can_go_back);
}

void
feed_item_view_set_can_go_forward (FeedItemView* view, gboolean can_go_forward)
{
    gtk_widget_set_sensitive (GTK_WIDGET(view->forward), can_go_forward);
}

void
feed_item_view_display (FeedItemView* view, const gchar* text)
{
    gtk_widget_set_sensitive (GTK_WIDGET(view->mail), TRUE);
    gtk_widget_set_sensitive (GTK_WIDGET(view->page), TRUE);
    webkit_page_load_html_string (view->page, text, "");
}

void
feed_item_view_highlight (FeedItemView* view, const gchar* search_string)
{
    gtk_entry_set_text (GTK_ENTRY(view->search_entry), search_string);
    do_highlight (view, search_string);
}
