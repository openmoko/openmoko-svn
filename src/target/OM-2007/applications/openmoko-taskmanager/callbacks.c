#include "callbacks.h"
#include "footer.h"
#include "main.h"

/* footer */
void footer_leftbutton_clicked(GtkWidget *widget, gpointer my_data)
{
    g_debug( "left button clicked" );

    GtkMessageDialog* dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                     GTK_DIALOG_DESTROY_WITH_PARENT,
                                     GTK_MESSAGE_ERROR,
                                     GTK_BUTTONS_CLOSE,
                                     "Display Task List Now..." );
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

void footer_rightbutton_clicked(GtkWidget *widget, gpointer my_data)
{
    g_debug( "right button clicked" );

    GtkMessageDialog* dialog = gtk_message_dialog_new(GTK_WINDOW(gtk_widget_get_toplevel(widget)),
            GTK_DIALOG_DESTROY_WITH_PARENT,
            GTK_MESSAGE_ERROR,
            GTK_BUTTONS_CLOSE,
            "Flip current/list application now..." );
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

/* dbus */
DBusHandlerResult signal_filter(DBusConnection *connection, DBusMessage *message, void *user_data)
{
    g_debug( "signal_filter called" );
    g_debug( "type of message was %d", dbus_message_get_type(message) );
    g_debug( "path of message was %s", dbus_message_get_path(message) );
    g_debug( "interface of message was %s", dbus_message_get_interface(message) );

    /* Application object is the user data */
    Application* app = user_data;

    /* A signal from the bus saying we are about to be disconnected */
    if (dbus_message_is_signal
        (message, DBUS_INTERFACE_LOCAL, "Disconnected")) {
        /* Tell the main loop to quit */
        g_main_loop_quit(app->loop);
        /* We have handled this message, don't pass it on */
        return DBUS_HANDLER_RESULT_HANDLED;
        }
        /* A message on our interface */
        else if (dbus_message_is_signal(message, "org.openmoko.dbus.TaskManager", "push_statusbar_message")) {
            DBusError error;
            char *s;
            dbus_error_init (&error);
            if (dbus_message_get_args
                (message, &error, DBUS_TYPE_STRING, &s, DBUS_TYPE_INVALID)) {
                g_debug("Setting status bar text to '%s", s);
                footer_set_status( app->footer, s );
                //FIXME: SIGSEGV, when uncommented. It now leaks! :M:
                //dbus_free(s);
                } else {
                    g_print("Ping received, but error getting message: %s", error.message);
                    dbus_error_free (&error);
                }
                return DBUS_HANDLER_RESULT_HANDLED;
        }
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
