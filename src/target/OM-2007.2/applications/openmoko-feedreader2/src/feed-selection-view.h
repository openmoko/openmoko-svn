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
 * A simple GtkTreeView representing the FeedData
 */

#ifndef FEED_SELECTION_VIEW_H
#define FEED_SELECTION_VIEW_H

#include <gtk/gtk.h>
#include "feed-data.h"

G_BEGIN_DECLS

#define RSS_TYPE_FEED_SELECTION_VIEW            feed_selection_view_get_type()
#define RSS_FEED_SELECTION_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), RSS_TYPE_FEED_SELECTION_VIEW, FeedSelectionView))
#define RSS_FEED_SELECTION_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  RSS_TYPE_FEED_SELECTION_VIEW, FeedSelectionViewClass))
#define RSS_IS_FEED_SELECTION_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), RSS_TYPE_FEED_SELECTION_VIEW))
#define RSS_IS_FEED_SELECTION_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  RSS_TYPE_FEED_SELECTION_VIEW))
#define RSS_FEED_SELECTION_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  RSS_TYPE_FEED_SELECTION_VIEW, FeedSelectionViewClass))

typedef struct _FeedSelectionView FeedSelectionView;
typedef struct _FeedSelectionViewClass FeedSelectionViewClass;

struct _FeedSelectionView {
    GtkVBox parent;

    GtkEntry        *search_entry;
    GtkWidget       *search_toggle;
    GtkWidget       *category_combo;

    FeedFilter      *filter;
    FeedSort        *sort;   
    GtkTreeView     *view;
};

struct _FeedSelectionViewClass {
    GtkVBoxClass parent;
};

GType       feed_selection_view_get_type                (void);
GtkWidget*  feed_selection_view_new                     (void);
void        feed_selection_view_add_column              (const FeedSelectionView*, int column_type, const gchar* txt);
gchar*      feed_selection_view_get_search_string       (const FeedSelectionView*);

G_END_DECLS

#endif
