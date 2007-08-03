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
 * Configuration related classes including information of a Feed
 * and the corresponding model.
 */

#ifndef RSS_READER_FEED_CONFIGURATION
#define RSS_READER_FEED_CONFIGURATION

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define RSS_TYPE_FEED_CONFIGURATION             feed_configuration_get_type()
#define RSS_FEED_CONFIGURATION(obj)             (G_TYPE_CHECK_INSTANCE_CAST((obj), RSS_TYPE_FEED_CONFIGURATION, FeedConfiguration))
#define RSS_FEED_CONFIGURATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST((klass),  RSS_TYPE_FEED_CONFIGURATION, FeedConfigurationClass))
#define RSS_IS_FEED_CONFIGURATION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE((obj), RSS_TYPE_FEED_CONFIGURATION))
#define RSS_IS_FEED_CONFIGURATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE((klass),  RSS_TYPE_FEED_CONFIGURATION))
#define RSS_FEED_CONFIGURATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS((obj),  RSS_TYPE_FEED_CONFIGURATION, FeedConfigurationClass))

typedef struct _FeedConfiguration FeedConfiguration;
typedef struct _FeedConfigurationClass FeedConfigurationClass;

enum {
    FEED_NAME,
    FEED_URL,
    FEED_USER_NAME,
    FEED_PASSWORD,
    FEED_NUMBER_OF_ITEMS_TO_CACHE,
    FEED_NUMBER_OF_COLUMNS
};

struct _FeedConfiguration {
    GtkListStore parent;
};

struct _FeedConfigurationClass {
    GtkListStoreClass parent;
};


GType       feed_configuration_get_type             (void);

/*
 * singleton
 */
GObject*    feed_configuration_get_configuration    (void);

G_END_DECLS

#endif
