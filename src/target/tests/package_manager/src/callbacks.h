#include <gtk/gtk.h>


void
on_winappmgr_size_allocate             (GtkWidget       *widget,
                                        GdkRectangle    *allocation,
                                        gpointer         user_data);

void
on_tvpackage_cursor_changed            (GtkTreeView     *treeview,
                                        gpointer         user_data);

void
on_winappmgr_destroy                   (GtkObject       *object,
                                        gpointer         user_data);


void
on_buttonall_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_entrysearch_changed                 (GtkEditable     *editable,
                                        gpointer         user_data);

gboolean
on_entrysearch_focus                   (GtkWidget       *widget,
                                        GtkDirectionType  direction,
                                        gpointer         user_data);

gboolean
on_entrysearch_focus_out_event         (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

gboolean
on_entrysearch_focus_in_event          (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

void
on_buttonsearch_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonpackage_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonaction_clicked                (GtkButton       *button,
                                        gpointer         user_data);
