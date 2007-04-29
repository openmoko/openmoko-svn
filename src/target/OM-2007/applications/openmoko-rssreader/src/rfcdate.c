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
#include <glib/gi18n.h>

#include <stdlib.h>
#include <stdio.h>

G_DEFINE_TYPE(RSSRFCDate, rss_rfc_date, G_TYPE_OBJECT)

#define RSS_RFC_DATE_GET_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE((o), RSS_TYPE_RFC_DATE, RSSRFCDatePrivate))

typedef struct _RSSRFCDatePrivate RSSRFCDatePrivate;
struct _RSSRFCDatePrivate {
    gchar *string_cache;
};

static void
rss_rfc_date_finalize (GObject *object)
{
    RSSRFCDatePrivate *private = RSS_RFC_DATE_GET_PRIVATE (object);
    RSSRFCDate *self = RSS_RFC_DATE (object);

    if ( private->string_cache ) {
        free (private->string_cache);
        private->string_cache = NULL;
    }

    if ( self->date ) {
        g_date_free (self->date);
        self->date = NULL;
    }

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

/*
 * options
 *  a) use strcmp
 *  b) swicth/case
 */
static GDateMonth
rss_month_number( gchar *month_str )
{
    switch (month_str[0]) {
    case 'J':
        switch (month_str[1]) {
        case 'a':
            return G_DATE_JANUARY;
            break;
        case 'u':
            switch (month_str[2] ) {
            case 'n':
                return G_DATE_JUNE;
                break;
            case 'l':
                return G_DATE_JULY;
                break;
            }
        }
        break;
    case 'F':
        return G_DATE_FEBRUARY;
        break;
    case 'M':
        switch ( month_str[2] ) {
        case 'r':
            return G_DATE_MARCH;
            break;
        case 'y':
            return G_DATE_MAY;
            break;
        }
        break;
    case 'A':
        switch (month_str[1]){
        case 'p':
            return G_DATE_APRIL;
            break;
        case 'u':
            return G_DATE_AUGUST;
            break;
        }
        break;
    case 'O':
        return G_DATE_OCTOBER;
        break;
    case 'S':
        return G_DATE_SEPTEMBER;
        break;
    case 'N':
        return G_DATE_NOVEMBER;
        break;
    case 'D':
        return G_DATE_DECEMBER;
        break;
    }

    return G_DATE_BAD_MONTH;
}

static void
rss_rfc_date_init(RSSRFCDate *self)
{
    /* I don't know if memset gets called */
    RSS_RFC_DATE_GET_PRIVATE(self)->string_cache = NULL;
    self->date = g_date_new ();
}

GObject*
rss_rfc_date_new (void)
{
    return G_OBJECT(g_object_new(RSS_TYPE_RFC_DATE, NULL));
}

/**
 * Clear the internal string representation. This can be used
 * on the day switch
 */
void
rss_rfc_date_clear_cache (RSSRFCDate* self)
{
    RSSRFCDatePrivate *private = RSS_RFC_DATE_GET_PRIVATE (self);

    if ( private->string_cache ) {
        free ( private->string_cache );
        private->string_cache = NULL;
    }
}

void
rss_rfc_date_set (RSSRFCDate *self, const gchar* rfc822date)
{
    rss_rfc_date_clear_cache (self);


    /*
     * %a, %d %b %Y %H:%M:%S %z
     *
     * We try to parse this date representation. We can ignore
     * %a but for the %b we need to look it up properly
     */
    int day, year, hour, minute, second;
    day = year = hour = minute = second = 0;
    gchar month_name[4];
    sscanf (rfc822date, "%*3s, %d %3s %d %d:%d:%d", &day, month_name, &year, &hour, &minute, &second );

    if ( year < 100 )
        year += 1900;

    self->timeval.tv_sec  = hour*60*60 + minute*60 + second;
    self->timeval.tv_usec = 0;
    g_date_set_dmy ( self->date, day, rss_month_number (month_name), year);


    /*
     * broken software? The 29th February and 31st April simply doesn't exist
     * What should we do with these dates? round to the nearest legal date?
     */
    if ( !g_date_valid (self->date) ) {
        g_print ("Setting RFC Date failed: '%s' %d %d %s %d  %ld\n",
                        rfc822date,
                        day,
                        rss_month_number(month_name),
                        month_name,
                        year,
                        self->timeval.tv_sec);
    }
}

/*
 * Start by comparing the dates and only if they are equal compare
 * the times.
 */
gint
rss_rfc_date_compare (RSSRFCDate *left, RSSRFCDate *right)
{
    int date_result = g_date_compare( left->date, right->date );
    if (  date_result != 0 )
        return date_result;

    return left->timeval.tv_sec - right->timeval.tv_sec;
}

gchar*
rss_rfc_date_as_string (RSSRFCDate *self)
{
    RSSRFCDatePrivate *private = RSS_RFC_DATE_GET_PRIVATE(self);
    if ( private->string_cache )
        return private->string_cache;

    /*
     * format the date now
     */
    GString *date_string;
    GTimeVal now;
    g_get_current_time (&now);
    GDate *date = g_date_new ();
    g_date_set_time_val (date, &now);

    if ( g_date_compare( date, self->date ) == 0 ) {
        date_string = g_string_new (_("Today,"));
        goto exit;
    }

    g_date_subtract_days( date, 1 );
    if ( g_date_compare( date, self->date ) == 0 ) {
        date_string = g_string_new (_("Yesterday,"));
        goto exit;
    }

    /*
     * copy the date using the current locale. And retry
     * until the buffer is big enough
     */
    date_string = g_string_sized_new( 40 );
    gsize result;
    while ( (result = g_date_strftime( date_string->str, date_string->allocated_len-1, "%a, %e %b %Y", self->date )) == 0 ) {
        g_string_set_size( date_string, date_string->allocated_len + 10 );
    }

    g_string_set_size (date_string, result);

exit:
    /*
     * append the time
     */
    g_string_append_printf ( date_string, " %ld:%ld:%ld",
                             self->timeval.tv_sec/60/60,
                             self->timeval.tv_sec/60%60,
                             self->timeval.tv_sec%60);
    g_date_free (date);
    private->string_cache = g_string_free (date_string, FALSE);
    return private->string_cache;
}
