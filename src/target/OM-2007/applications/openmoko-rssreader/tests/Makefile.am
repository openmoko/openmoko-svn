AM_CFLAGS = -Wall -Werror -std=c99 @OPENMOKO_CFLAGS@ @MRSS_CFLAGS@ @GCOV_CPPFLAGS@
INCLUDES  = -I$(top_srcdir)/src

if ENABLE_TESTING
TESTS = datetest cachetest
check_PROGRAMS = datetest cachetest
endif

datatest_INCLUDES = @CHECK_CFLAGS@
datetest_SOURCES  = date_test.c ../src/rfcdate.c
datetest_LIBS     = @CHECK_LIBS@
datetest_LDFLAGS  = -lcheck @GCOV_LDFLAGS@ @OPENMOKO_LIBS@

cachetest_INCLUDES = @CHECK_CFLAGS@
cachetest_SOURCES  = cache_test.c ../src/moko_cache.c
cachetest_LIBS     = @CHECK_LIBS@
cachetest_LDFLAGS  = -lcheck @GCOV_LDFLAGS@ @OPENMOKO_LIBS@
