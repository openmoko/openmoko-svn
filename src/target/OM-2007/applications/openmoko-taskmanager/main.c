#include "main.h"
#include "callbacks.h"

int main( int argc, char **argv )
{
    Application* app = g_malloc( sizeof(Application) );
    DBusError error;
    dbus_error_init(&error);

    app->loop = g_main_loop_new( NULL, FALSE );

    // using the low-level dbus interface here to get the point across
    // final version should rather create a task manager gobject and hook
    // into the dbus-glib API
    app->bus = dbus_bus_get(DBUS_BUS_SESSION, &error);
    if (!app->bus) {
        g_warning ("Failed to connect to the D-BUS daemon: %s", error.message);
        dbus_error_free(&error);
        return 1;
    }

    gtk_init (&argc, &argv);

    app->window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_widget_show( app->window );
    app->footer = FOOTER(footer_new());
    gtk_container_add( GTK_CONTAINER(app->window), GTK_WIDGET(app->footer) );
    // this violates the privacy concept, but it's a demo for now...
    g_signal_connect( GTK_WIDGET(app->footer->leftbutton), "clicked", G_CALLBACK(footer_leftbutton_clicked), NULL );
    g_signal_connect( GTK_WIDGET(app->footer->rightbutton), "clicked", G_CALLBACK(footer_rightbutton_clicked), NULL );

    gtk_widget_show_all( GTK_WIDGET(app->footer) );

    dbus_connection_setup_with_g_main(app->bus, NULL);
    dbus_bus_add_match(app->bus, "type='signal',interface='org.openmoko.dbus.TaskManager'", &error );
    dbus_connection_add_filter(app->bus, signal_filter, app, NULL );

    g_main_loop_run( app->loop );
    g_free( app );
    return 0;
}
