/*
 *  RSS Reader, a simple RSS reader
 *  RFC822 date parser implementation
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

#include "rfcdate.h"

#include <malloc.h>

G_DEFINE_TYPE(RSSRFCDate, rss_rfc_date, G_TYPE_OBJECT)

#define RSS_RFC_DATE_GET_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE((o), RSS_TYPE_RFC_DATE, RSSRFCDatePrivate))

typedef struct _RSSRFCDatePrivate RSSRFCDatePrivate;
struct _RSSRFCDatePrivate {
    gchar *string_cache;
};

static void
rss_rfc_date_finalize (GObject *object)
{
    if ( RSS_RFC_DATE_GET_PRIVATE(object)->string_cache )
        free (RSS_RFC_DATE_GET_PRIVATE(object)->string_cache);

    G_OBJECT_CLASS(rss_rfc_date_parent_class)->finalize (object);
}

static void
rss_rfc_date_class_init (RSSRFCDateClass *klass)
{
    g_type_class_add_private (klass, sizeof (RSSRFCDatePrivate));

    /*
     * virtual functions
     */
    G_OBJECT_CLASS(klass)->finalize = rss_rfc_date_finalize;
}

static void
rss_rfc_date_init(RSSRFCDate *self)
{
    /* I don't know if memset gets called */
    RSS_RFC_DATE_GET_PRIVATE(self)->string_cache = NULL;
}

GObject*
rss_rfc_date_new ()
{
    return G_OBJECT(g_object_new(RSS_TYPE_RFC_DATE, NULL));
}

void
rss_rfc_date_set (RSSRFCDate *self, const gchar* rfc822date)
{
    if ( RSS_RFC_DATE_GET_PRIVATE(self)->string_cache ) {
        free ( RSS_RFC_DATE_GET_PRIVATE(self)->string_cache );
        RSS_RFC_DATE_GET_PRIVATE(self)->string_cache = NULL;
    }


    /*
     * XXX parse the date
     */
}

gint
rss_rfc_date_compare (RSSRFCDate *left, RSSRFCDate *right)
{
    /* XXX, FIXME do the comparsion */
    return 0;
}

gchar*
rss_rfc_date_as_string (RSSRFCDate *self)
{
    /* XXX, FIXME */
    return "";
}
