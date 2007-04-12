noinst_PROGRAMS=testcreate testdelete testdeleteuid

testcreate_SOURCES=test-create.c
testcreate_LDADD=@LIBS@ $(top_srcdir)/libmokojournal/src/libmokojournal.la

testdelete_SOURCES=test-delete.c
testdelete_LDADD=@LIBS@ $(top_srcdir)/libmokojournal/src/libmokojournal.la

testdeleteuid_SOURCES=test-delete-uid.c
testdeleteuid_LDADD=@LIBS@ $(top_srcdir)/libmokojournal/src/libmokojournal.la

INCLUDES= -I $(top_srcdir)/libmokojournal/src

