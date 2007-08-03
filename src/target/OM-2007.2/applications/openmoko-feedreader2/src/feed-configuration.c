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

#include "feed-configuration.h"

struct FeedEntry {
    gchar *category;
    gchar *url;
};

static struct FeedEntry s_feeds[] = {
    { "OpenMoko",   "http://planet.openmoko.org/atom.xml"  },
    { "GNOME"   ,   "http://planet.gnome.org/atom.xml"     },
    { "KDE",        "http://planet.kde.org/rss20.xml"      },
    { "Linux Togo", "http://planet.linuxtogo.org/atom.xml" },
    { "zecke"   , "http://zecke.blogspot.com/atom.xml"     },
};

static const int NUMBER_OF_FEEDS = sizeof(s_feeds)/sizeof(s_feeds[0]);

G_DEFINE_TYPE(FeedConfiguration, feed_configuration, GTK_TYPE_LIST_STORE)

static void feed_configuration_class_init (FeedConfigurationClass* klass)
{
}

static void feed_configuration_init (FeedConfiguration* config)
{
    GType types[FEED_NUMBER_OF_COLUMNS];
    types[0] = G_TYPE_STRING;
    types[1] = G_TYPE_STRING;
    types[2] = G_TYPE_STRING;
    types[3] = G_TYPE_STRING;
    types[4] = G_TYPE_INT;
    gtk_list_store_set_column_types (GTK_LIST_STORE(config), 5, types);

    /*
     * Add some dummy entries until we have something GConf binding
     */
    GtkTreeIter iter;
    for (int i = 0; i < NUMBER_OF_FEEDS; ++i) {
        gtk_list_store_append (GTK_LIST_STORE(config), &iter );
        gtk_list_store_set (GTK_LIST_STORE(config), &iter,
                            FEED_NAME, g_strdup(s_feeds[i].category),
                            FEED_URL,  g_strdup(s_feeds[i].url),
                            -1);
    }
}


GObject* feed_configuration_get_configuration (void)
{
    static FeedConfiguration* config = 0;
    if (!config)
        config = RSS_FEED_CONFIGURATION(g_object_new(RSS_TYPE_FEED_CONFIGURATION, NULL));

    return G_OBJECT(config);
}
