#include <gtk/gtk.h>
#include <string.h>

void
on_call_handlering_activate            (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_help_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_quit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_infoArea_show                       (GtkWidget       *widget,
                                        gpointer         user_data);

void
on_btnAnswer_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_btnIgnore_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_infoArea_expose_event               (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

void
on_window1_destroy                     (GtkObject       *object,
                                        gpointer         user_data);

gboolean
on_gwInfoArea_expose_event               (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

void 
display						  		  (GtkWidget       *widget);

void
on_btnCancel_clicked                   (GtkButton       *button,
                                        gpointer         user_data);


void
on_btnRecord_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_btnHangup_clicked                   (GtkButton       *button,
                                        gpointer         user_data);
