#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_

#include <gtk/gtkbutton.h>
#include <gtk/gtkwidget.h>

#include <glib.h>

gboolean cb_filter_changed(GtkWidget* widget, gchar* text, gpointer user_data);
void cb_button1_clicked(GtkButton *button, gpointer user_data);
void cb_button2_clicked(GtkButton *button, gpointer user_data);
void cb_button3_clicked(GtkButton *button, gpointer user_data);
void cb_button4_clicked(GtkButton *button, gpointer user_data);

#endif
