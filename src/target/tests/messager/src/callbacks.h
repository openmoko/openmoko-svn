#include <gtk/gtk.h>


void
on_message_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_help_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_quit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_sms_activate                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_email_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_im_activate                         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_btnSend_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_btnSave_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_window1_destroy                     (GtkObject       *object,
                                        gpointer         user_data);

void 
resetScrolledWindow			(GtkWidget     *menuitem);

void
display                                 (GtkWidget     *widget);

gboolean
on_window1_expose_event                (GtkWidget       *widget,
		                        GdkEventExpose  *event,
					gpointer         user_data);

gboolean
on_txtView_key_release_event         (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);

void
on_txtView_backspace                 (GtkTextView     *textview,
                                        gpointer         user_data);
