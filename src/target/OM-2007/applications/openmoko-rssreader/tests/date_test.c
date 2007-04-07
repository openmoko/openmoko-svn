#include <check.h>
#include <stdlib.h>

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
    fail_unless (g_date_get_month (date->date) == G_DATE_JANUARY, "Seldom year number, seldom time");
    fail_unless (g_date_get_day (date->date) == 26, "Parsed my birthday");
    fail_unless (g_date_get_year (date->date) == 1983, "Parsed my birthyear");
    fail_unless (date->timeval.tv_sec == (8*60*60+57*60), "Parsed my birthtime properly");
}
END_TEST

Suite*
date_suite (void)
{
    Suite *s = suite_create( "Date" );
    TCase *tc_core = tcase_create ("Core");
    tcase_add_test (tc_core, test_create_date);
    tcase_add_test (tc_core, test_parsing_date);
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
