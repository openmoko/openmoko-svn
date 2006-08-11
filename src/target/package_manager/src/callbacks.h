#include <gtk/gtk.h>


void
on_winappmgr_size_allocate             (GtkWidget       *widget,
                                        GdkRectangle    *allocation,
                                        gpointer         user_data);

void
on_scrolledwindow1_check_resize        (GtkContainer    *container,
                                        gpointer         user_data);

void
on_scrolledwindow1_size_allocate       (GtkWidget       *widget,
                                        GdkRectangle    *allocation,
                                        gpointer         user_data);
