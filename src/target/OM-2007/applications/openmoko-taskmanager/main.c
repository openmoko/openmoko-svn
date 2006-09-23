#include "footer.h"

#include <gtk/gtk.h>

int main( int argc, char **argv )
{
    gtk_init (&argc, &argv);

    GtkWidget* window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    gtk_widget_show( window );

    Footer* footer = footer_new();
    gtk_container_add( GTK_CONTAINER(window), GTK_WIDGET(footer) );

    gtk_widget_show_all( footer );
    gtk_main();

    return 0;
}
