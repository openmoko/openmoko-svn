#ifndef MYAPP_MAIN_H
#define MYAPP_MAIN_H

#include "footer.h"
#include <glib/gmain.h>
#include <dbus/dbus.h>
#include <gtk/gtkwidget.h>

/* Types */
typedef struct _Application {
    DBusConnection* bus;
    GMainLoop* loop;
    GtkWidget* window;
    Footer* footer;
} Application;

#endif /* MAIN_H */
