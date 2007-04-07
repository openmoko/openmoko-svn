AM_CFLAGS = -Wall -Werror -std=c99 @OPENMOKO_CFLAGS@ @MRSS_CFLAGS@ @GCOV_CPPFLAGS@
INCLUDES  = -I$(top_srcdir)/src

if ENABLE_TESTING
TESTS = datetest
check_PROGRAMS = datetest
endif

datatest_INCLUDES = @CHECK_CFLAGS@
datetest_SOURCES  = date_test.c ../src/rfcdate.c
datetest_LIBS     = @CHECK_LIBS@
datetest_LDFLAGS  = -lcheck @GCOV_LDFLAGS@ @OPENMOKO_LIBS@
