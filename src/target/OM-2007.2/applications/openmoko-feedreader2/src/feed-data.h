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

#include <gtk/gtk.h>
#include "moko_cache.h"

G_BEGIN_DECLS

#define RSS_TYPE_FEED_DATA                  feed_data_get_type()
#define RSS_FEED_DATA(obj)                  (G_TYPE_CHECK_INSTANCE_CAST((obj), RSS_TYPE_FEED_DATA, FeedData))
#define RSS_FEED_DATA_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST((klass),  RSS_TYPE_FEED_DATA, FeedDataClass))
#define RSS_IS_FEED_DATA(obj)               (G_TYPE_CHECK_INSTANCE_TYPE((obj), RSS_TYPE_FEED_DATA))
#define RSS_IS_FEED_DATA_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE((klass),  RSS_TYPE_FEED_DATA))
#define RSS_FEED_DATA_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS((obj),  RSS_TYPE_FEED_DATA, FeedDataClass))

#define RSS_TYPE_FEED_FILTER                feed_filter_get_type()
#define RSS_FEED_FILTER(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), RSS_TYPE_FEED_FILTER, FeedFilter))
#define RSS_FEED_FILTER_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST((klass),  RSS_TYPE_FEED_FILTER, FeedFilterClass))
#define RSS_IS_FEED_FILTER(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), RSS_TYPE_FEED_FILTER))
#define RSS_IS_FEED_FILTER_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE((klass),  RSS_TYPE_FEED_FILTER))
#define RSS_FEED_FILTER_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj),  RSS_TYPE_FEED_FILTER, FeedFilterClass))



#define RSS_TYPE_FEED_SORT                feed_sort_get_type()
#define RSS_FEED_SORT(obj)                (G_TYPE_CHECK_INSTANCE_CAST((obj), RSS_TYPE_FEED_SORT, FeedSort))
#define RSS_FEED_SORT_CLASS(klass)        (G_TYPE_CHECK_CLASS_CAST((klass),  RSS_TYPE_FEED_SORT, FeedSortClass))
#define RSS_IS_FEED_SORT(obj)             (G_TYPE_CHECK_INSTANCE_TYPE((obj), RSS_TYPE_FEED_SORT))
#define RSS_IS_FEED_SORT_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE((klass),  RSS_TYPE_FEED_SORT))
#define RSS_FEED_SORT_GET_CLASS(obj)      (G_TYPE_INSTANCE_GET_CLASS((obj),  RSS_TYPE_FEED_SORT, FeedSortClass))

typedef struct _FeedData FeedData;
typedef struct _FeedDataClass FeedDataClass;
typedef struct _FeedFilter FeedFilter;
typedef struct _FeedFilterClass FeedFilterClass;
typedef struct _FeedSort FeedSort;
typedef struct _FeedSortClass FeedSortClass;

struct _FeedData {
    GtkListStore parent;
    MokoCache    *cache;
};

struct _FeedDataClass {
    GtkListStoreClass parent;
};

struct _FeedFilter {
    GtkTreeModelFilter parent;

    /*
     * Do we have a filter set at all?
     */
    gboolean all_filter;

    /*
     * The category to filter. Coming from the feed-configuration
     */
    gchar   *category;

    /*
     * The filter string.
     */
    gchar  *filter_string;
};

struct _FeedFilterClass {
    GtkTreeModelFilterClass parent;
};

struct _FeedSort {
    GtkTreeModelSort parent;
};

struct  _FeedSortClass {
    GtkTreeModelSortClass parent;
};


GType       feed_data_get_type          (void);
GObject*    feed_data_get_instance      (void);

void        feed_data_update_all        (FeedData*);
void        feed_data_set_cache         (FeedData*, MokoCache*);
void        feed_data_load_from_cache   (FeedData*);


GType       feed_filter_get_type        (void);
GObject*    feed_filter_new             (const FeedData*);
void        feed_filter_reset           (FeedFilter*);
void        feed_filter_filter_category (FeedFilter*, GtkTreeIter*);
void        feed_filter_filter_text     (FeedFilter*, const gchar*);

GType       feed_sort_get_type        (void);
GObject*    feed_sort_new             (const FeedFilter*);

G_END_DECLS

#endif
