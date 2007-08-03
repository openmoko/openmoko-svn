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

#include "feed-data.h"

G_DEFINE_TYPE(FeedData,   feed_data,   GTK_TYPE_LIST_STORE)
G_DEFINE_TYPE(FeedFilter, feed_filter, GTK_TYPE_TREE_MODEL_FILTER)
G_DEFINE_TYPE(FeedSort,   feed_sort,   GTK_TYPE_TREE_MODEL_SORT)

static void
feed_data_init (FeedData *data)
{
}

static void
feed_data_class_init (FeedDataClass *feed_class)
{
}

static void
feed_filter_init (FeedFilter *filter)
{
}

static void
feed_filter_class_init (FeedFilterClass *filter_class)
{
}

static void
feed_sort_init (FeedSort* init)
{
}

static void
feed_sort_class_init (FeedSortClass* sort_class)
{
}

GObject*
feed_data_get_instance (void)
{
    static GObject* instance = 0;
    if (!instance)
        instance = g_object_new(RSS_TYPE_FEED_DATA, NULL);

    return instance;
}

void
feed_data_update_all (FeedData* data)
{
}

void
feed_data_set_cache (FeedData* data, MokoCache* cache)
{
}

void
feed_data_load_from_cache (FeedData* data)
{
}


GObject*
feed_filter_new (const FeedData* data)
{
    GObject* obj = g_object_new(RSS_TYPE_FEED_FILTER,
                                "child-model", data,
                                "root", NULL,
                                NULL);

    return obj;
}

void
feed_filter_reset (FeedFilter* filter)
{
}

void
feed_filter_filter_category (FeedFilter* filter, GtkTreeIter* iter)
{
}

void
feed_filter_filter_text (FeedFilter* filter, const gchar* text)
{
}

GObject*
feed_sort_new (const FeedFilter* filter)
{
    GObject* obj = g_object_new (RSS_TYPE_FEED_SORT,
                                 "model", filter,
                                 NULL);

    return obj;
}
