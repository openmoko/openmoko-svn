#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>
#include "rfcdate.h"

START_TEST (test_create_date)
{
    RSSRFCDate *date = RSS_RFC_DATE(rss_rfc_date_new ());
    g_object_unref( G_OBJECT(date) );
}
END_TEST

START_TEST (test_parsing_date)
{
    RSSRFCDate *date = RSS_RFC_DATE(rss_rfc_date_new ());

    rss_rfc_date_set (date, "Wed, 26 Jan 83 07:57");
    fail_unless (g_date_valid (date->date), "Date is valid");
    fail_unless (g_date_get_month (date->date) == G_DATE_JANUARY, "Seldom year number, seldom time");
    fail_unless (g_date_get_day (date->date) == 26, "Parsed my birthday");
    fail_unless (g_date_get_year (date->date) == 1983, "Parsed my birthyear");
    fail_unless (date->timeval.tv_sec == (7*60*60+57*60), "Parsed my birthtime properly");

    g_object_unref (date);
}
END_TEST

START_TEST(test_month_names)
{
    gchar date_buffer[40];
    gchar *month_names[] = {
        "Jan",
        "Feb",
        "Mar",
        "Apr",
        "May",
        "Jun",
        "Jul",
        "Aug",
        "Sep",
        "Oct",
        "Nov",
        "Dec" };

    RSSRFCDate *date = RSS_RFC_DATE(rss_rfc_date_new ());
    fail_unless( date != NULL, "Allocated date" );

    for ( int i = 0; i < 12; ++i ) {
        snprintf (date_buffer, 40, "Mon, 9 %s 2041 23:23:23", month_names[i] );
        rss_rfc_date_set ( date,  date_buffer);
        fail_unless (g_date_valid(date->date), "Parsing the date failed" );
        fail_unless (g_date_get_month(date->date) == i+1, "Parsing the month named failed" );
        fail_unless (date->timeval.tv_sec == 23*60*60+23*60+23 );
    }

    g_object_unref (date);
}
END_TEST

/*
 * make sure to run this with LC_ALL=C
 */
START_TEST(test_string_representation)
{

    static gchar * month_names[] = { "", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    RSSRFCDate *date = RSS_RFC_DATE(rss_rfc_date_new ());
    rss_rfc_date_set( date, "Mon, 6 Mar 1848 13:24:43" );
    g_print ("Date: %s\n", rss_rfc_date_as_string (date));
    fail_unless( strcmp(rss_rfc_date_as_string (date), "Mon,  6 Mar 1848 13:24:43") == 0, "formatted string 1" );
    fail_unless( strcmp(rss_rfc_date_as_string (date), "Mon,  6 Mar 1848 13:24:43") == 0, "formatted stringc cached?" );


    GTimeVal now;
    g_get_current_time (&now);
    GDate *gdate = g_date_new ();
    g_date_set_time_val (gdate, &now);
    g_print( "Day: %d Month: %d Year: %d\n", g_date_day (gdate), g_date_month (gdate), g_date_year (gdate) );

    gchar date_string[40];
    snprintf( date_string, 40, "Mon, %d %s %d 23:10:53", g_date_day (gdate), month_names[g_date_month (gdate)], g_date_year (gdate) );
    rss_rfc_date_set (date, date_string);
    g_print ("Date string '%s' '%s'\n", date_string, rss_rfc_date_as_string (date));
    fail_unless ( strcmp( "Today, 23:10:53", rss_rfc_date_as_string (date) ) == 0, "Correct today's formatting");
    fail_unless ( strcmp( "Today, 23:10:53", rss_rfc_date_as_string (date) ) == 0, "Correct today's formatting (cached)");

    g_date_subtract_days (gdate, 1);
    snprintf( date_string, 40, "Mon, %d %s %d 23:10:53", g_date_day (gdate), month_names[g_date_month (gdate)], g_date_year (gdate) );
    rss_rfc_date_set (date, date_string);
    g_print ("Date string '%s' '%s'\n", date_string, rss_rfc_date_as_string (date));
    fail_unless ( strcmp( "Yesterday, 23:10:53", rss_rfc_date_as_string (date) ) == 0, "Correct yesterday's formatting");
    fail_unless ( strcmp( "Yesterday, 23:10:53", rss_rfc_date_as_string (date) ) == 0, "Correct yesterday's formatting (cached)");

    g_date_free (gdate);
    g_object_unref (date);
}
END_TEST

Suite*
date_suite (void)
{
    Suite *s = suite_create( "Date" );
    TCase *tc_core = tcase_create ("Core");
    tcase_add_test (tc_core, test_create_date);
    tcase_add_test (tc_core, test_parsing_date);
    tcase_add_test (tc_core, test_month_names);
    tcase_add_test (tc_core, test_string_representation);
    suite_add_tcase (s, tc_core);


    return s;
}

int
main (void)
{
    g_type_init ();
    Suite *s = date_suite ();
    SRunner *sr = srunner_create (s);
    srunner_run_all (sr, CK_NORMAL);
    int number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);

    return  (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
