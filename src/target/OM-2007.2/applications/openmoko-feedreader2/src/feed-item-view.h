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

/*
 * Display one Feed Item/Entry.
 *
 *  WebKitGtk will be used to display the entry. This is the whole view including
 *  a button for the next and previous item to allow fast switching of items.
 *  
 *  The views representing the filtered and sorted FeedData are expected to follow
 *  the next and previous request.
 */


#ifndef RSS_FEED_ITEM_VIEW_H
#define RSS_FEED_ITEM_VIEW_H

#include <gtk/gtk.h>


GType           feed_item_view_get_type             (void);
GtkWidget*      feed_item_view_new                  (void);

void            feed_item_view_set_can_go_back      (FeedItemView*, gboolean);
void            feed_item_view_set_can_go_forward   (FeedItemView*, gboolean);

/*
 * display the text
 */
void            feed_item_view_display              (FeedItemView*, gchar*);

/*
 * highlight words from a search or such
 */
void            feed_item_view_highlight            (FeedItemView*, gchar*);

/*
 *  The following signals are emitted:
 *      next
 *      prev
 *      visit_url   G_TYPE_STRING
 */

#endif
