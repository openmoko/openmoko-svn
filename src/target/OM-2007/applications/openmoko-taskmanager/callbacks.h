#include <gtk/gtk.h>
#include <dbus/dbus.h>
#include "main.h"

/* footer */
void footer_leftbutton_clicked(GtkWidget *widget, gpointer my_data);
void footer_rightbutton_clicked(GtkWidget *widget, gpointer my_data);

/* dbus */
DBusHandlerResult signal_filter (DBusConnection *connection, DBusMessage *message, void *user_data);
