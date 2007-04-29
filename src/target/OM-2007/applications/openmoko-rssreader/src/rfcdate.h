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
#ifndef OPENMOKO_RSS_RFC_DATE_H
#define OPENMOKO_RSS_RFC_DATE_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define RSS_TYPE_RFC_DATE            (rss_rfc_date_get_type())
#define RSS_RFC_DATE(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), RSS_TYPE_RFC_DATE, RSSRFCDate))
#define RSS_RFC_DATE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((obj),    RSS_TYPE_RFC_DATE, RSSRFCDateClass))
#define RSS_IS_RFC_DATE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), RSS_TYPE_RFC_DATE))
#define RSS_IS_RFC_DATE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  RSS_TYPE_RFC_DATE))
#define RSS_RFC_DATE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  RSS_TYPE_RFC_DATE, RSSRFCDateClass))

typedef struct _RSSRFCDate RSSRFCDate;
typedef struct _RSSRFCDateClass RSSRFCDateClass;

struct _RSSRFCDate {
    GObject parent;

    GDate  *date;
    GTimeVal timeval;
};

struct _RSSRFCDateClass {
    GObjectClass parent;
};

GType     rss_rfc_date_get_type (void);
GObject*  rss_rfc_date_new      (void);
void      rss_rfc_date_set      (RSSRFCDate* self, const gchar* rfc822_date);
gint      rss_rfc_date_compare  (RSSRFCDate* self, RSSRFCDate *other);
gchar*    rss_rfc_date_as_string(RSSRFCDate* self);
void      rss_rfc_date_clear_cache (RSSRFCDate* self);


G_END_DECLS

#endif
