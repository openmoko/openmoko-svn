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
 * This class is responsible for all data related stuff including downloading
 * and updating feeds, proving a GtkTreeModel to the outside world. Holding
 * two other models for sorting and filtering.
 */

#ifndef RSS_READER_FEED_DATA_H
#define RSS_READER_FEED_DATA_H

#include "feed-configuration.h"

G_BEGIN_DECLS

#define RSS_TYPE_FEED_DATA              feed_data_get_type()
#define RSS_FEED_DATA(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), RSS_TYPE_FEED_DATA, FeedData))
#define RSS_FEED_DATA_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass),  RSS_TYPE_FEED_DATA, FeedDataClass))
#define RSS_IS_FEED_DATA(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), RSS_TYPE_FEED_DATA))
#define RSS_IS_FEED_DATA_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE((klass),  RSS_TYPE_FEED_DATA))
#define RSS_FEED_DATA_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj),  RSS_TYPE_FEED_DATA, FeedDataClass))

#define RSS_TYPE_FEED_FILTER            feed_filter_get_type()
#define RSS_TYPE_FEED_SORTER            feed_sorter_get_type()

typedef struct _FeedData FeedData;
typedef struct _FeedDataClass FeedDataClass;

struct _FeedData {
    GtkListStore parent;
};

struct _FeedDataClass {
    GtkListStoreClass parent;
};


GType       feed_data_get_type          (void);
GObject*    feed_data_get_instance      (void);

void        feed_data_update_all        (FeedData*);
void        feed_data_update            (FeedData*, Feed*);


GType       feed_filter_get_type        (void);
GObject*    feed_filter_new             (const FeedFilter*);
void        feed_filter_reset           (const FeedFilter*);
void        feed_filter_filter          (const FeedFilter*, const Feed*);

GType       feed_sorter_get_type        (void);
GObject*    feed_sorter_new             (const FeedFilter*);

G_END_DECLS

#endif
