#include <check.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>
#include <string.h>
#include "rfcdate.h"

#include "moko_cache.h"

START_TEST (test_create_cache)
{
    GObject *cache = moko_cache_new ("moko-cache-test");
    g_object_unref (cache);
}
END_TEST

/*
 * test error handling
 */
START_TEST (test_read_empty_cache)
{
    GObject *cache = moko_cache_new ("moko-cache-test");
    fail_unless (cache != NULL, "Creating failed?");

    gsize size;
    gchar *cache_data = moko_cache_read_object (MOKO_CACHE(cache), "http://www.openembedded.org/~zecke/does-not-exist", &size);
    fail_unless (cache_data == NULL, "Failed to read it");
    fail_unless (size == -1, "Size is wrong");
    g_object_unref (cache);
}
END_TEST

/*
 * test writing and reading a file
 */
START_TEST (test_read_write_cache)
{
    GObject *cache = moko_cache_new ("moko-cache-test");

    gchar *content = "Hey this is a cache test";
    gint result = moko_cache_write_object (MOKO_CACHE(cache), "http://openembedded.org/~zecke/foo.withtext", content, strlen(content), NULL);
    g_print ("Result was %d\n", result);
    fail_unless (result == MOKO_CACHE_WRITE_SUCCESS, "Writing the cache failed");

    /* now try to read it */
    gsize size;
    gchar *content_result = moko_cache_read_object (MOKO_CACHE(cache), "http://openembedded.org/~zecke/foo.withtext", &size);
    g_print ("String: %p size: %d\n", content_result, (int)size);
    fail_unless (content_result != NULL, "A valid string");
    fail_unless (size == 24, "Right size");
    fail_unless (strcmp(content_result,content) == 0, "Right text?");
    g_free(content_result);
    g_object_unref (cache);
}
END_TEST

START_TEST (test_read_write_empty_cache)
{
    GObject *cache = moko_cache_new ("moko-cache-test");

    gchar *content = "";
    gint result = moko_cache_write_object (MOKO_CACHE(cache), "http://openembedded.org/~zecke/foo.empty", content, strlen(content), NULL);
    g_print ("Result was %d\n", result);
    fail_unless (result == MOKO_CACHE_WRITE_SUCCESS, "Writing the cache failed");

    /* now try to read it */
    gsize size;
    content = moko_cache_read_object (MOKO_CACHE(cache), "http://openembedded.org/~zecke/foo.empty", &size);
    g_print ("String: %p size: %d\n", content, (int)size);
    fail_unless (content != NULL, "A valid string");
    fail_unless (size == 0, "Right size");
    g_free(content);
    g_object_unref (cache);
}
END_TEST

Suite*
cache_suite (void)
{
    Suite *s = suite_create( "Cache" );
    TCase *tc_core = tcase_create ("Core");
    tcase_add_test (tc_core, test_create_cache);
    tcase_add_test (tc_core, test_read_empty_cache);
    tcase_add_test (tc_core, test_read_write_cache);
    tcase_add_test (tc_core, test_read_write_empty_cache);
    suite_add_tcase (s, tc_core);

    return s;
}

int
main (void)
{
    g_type_init ();
    Suite *s = cache_suite ();
    SRunner *sr = srunner_create (s);
    srunner_run_all (sr, CK_NORMAL);
    int number_failed = srunner_ntests_failed (sr);
    srunner_free (sr);

    return  (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
