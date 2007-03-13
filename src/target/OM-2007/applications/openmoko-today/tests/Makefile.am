noinst_PROGRAMS    = ecaltest eventsareatest

ecaltest_SOURCES	   = ecal-test.c
ecaltest_LDADD		   = @TODAY_LIBS@

eventsareatest_SOURCES     = events-area-test.c \
$(top_srcdir)/src/today-events-area.c

eventsareatest_LDADD       = @TODAY_LIBS@

INCLUDES           = -I$(top_srcdir) @TODAY_CFLAGS@

