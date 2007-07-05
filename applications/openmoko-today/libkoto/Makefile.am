localedir = $(datadir)/locale
AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\" -DPKGDATADIR=\"$(pkgdatadir)\" $(GTK_CFLAGS) $(ECAL_CFLAGS) $(SEXY_CFLAGS) -Wall
AM_LDFLAGS = $(GTK_LIBS) $(ECAL_LIBS) $(SEXY_LIBS)

# Really nasty, but I don't want to check for this every time I want to use
# g_date_set_time_t.
if ! HAVE_GDATE_TIME_T
AM_CPPFLAGS += -D'g_date_set_time_t(d,t)=g_date_set_time(d,t)'
endif

noinst_LIBRARIES = libkoto.a
libkoto_a_SOURCES = \
	ical-util.h ical-util.c \
	koto-task.h koto-task.c \
	koto-hint-entry.h koto-hint-entry.c \
	koto-field-editor-factory.h koto-field-editor-factory.c \
	koto-task-editor.h koto-task-editor.c \
	koto-task-editor-dialog.h koto-task-editor-dialog.c \
	koto-task-store.h koto-task-store.c \
	koto-task-summary.h koto-task-summary.c \
	koto-task-view.h koto-task-view.c \
	koto-group.h koto-group.c \
	koto-all-group.h koto-all-group.c \
	koto-category-group.h koto-category-group.c \
	koto-no-category-group.h koto-no-category-group.c \
	koto-meta-group.h koto-meta-group.c \
	koto-group-store.h koto-group-store.c \
	koto-group-filter-model.h koto-group-filter-model.c \
	koto-group-combo.h koto-group-combo.c \
	koto-date-combo.h koto-date-combo.c \
	koto-priority-combo.h koto-priority-combo.c \
	koto-cell-renderer-pixbuf.h koto-cell-renderer-pixbuf.c \
	koto-utils.h koto-utils.c \
	koto-platform.h


TESTS = test-group-store test-ical-util
check_PROGRAMS = test-group-store test-ical-util

test_group_store_SOURCES = koto-group-store.c
test_group_store_CPPFLAGS = $(AM_CPPFLAGS) -DWITH_TESTS

test_ical_util_SOURCES = ical-util.c
test_ical_util_CPPFLAGS = $(AM_CPPFLAGS) -DWITH_TESTS
