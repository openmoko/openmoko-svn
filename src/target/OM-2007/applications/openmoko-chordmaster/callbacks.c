#include "callbacks.h"

#include <gtk/gtkwidget.h>
#include <gtk/gtkbutton.h>

gboolean cb_filter_changed(GtkWidget* widget, gchar* text, gpointer user_data)
{
    g_debug( "openmoko-chordmaster: filter changed" );
    return FALSE;
}

void cb_button1_clicked(GtkButton *button, gpointer user_data)
{
    g_debug( "openmoko-chordmaster: button1 clicked" );
}

void cb_button2_clicked(GtkButton *button, gpointer user_data)
{
    g_debug( "openmoko-chordmaster: button2 clicked" );
}

void cb_button3_clicked(GtkButton *button, gpointer user_data)
{
    g_debug( "openmoko-chordmaster: button3 clicked" );
}

void cb_button4_clicked(GtkButton *button, gpointer user_data)
{
    g_debug( "openmoko-chordmaster: button4 clicked" );
}
