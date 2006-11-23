#include <gtk/gtk.h>


void
on_showstatus_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_showsource_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_Install_single_application_activate (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_showhelp_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_quit_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_appmgr_destroy                      (GtkObject       *object,
                                        gpointer         user_data);

void
on_bmainmenu_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_bfilter_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_unmark_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_markinstall_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_markupgrade_activate                (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_markremove_activate                 (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

gboolean
on_treepkglist_button_press_event      (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void
on_treepkglist_cursor_changed          (GtkTreeView     *treeview,
                                        gpointer         user_data);

void
on_bapply_clicked                      (GtkButton       *button,
                                        gpointer         user_data);

void
on_entrysearch_changed                 (GtkEditable     *editable,
                                        gpointer         user_data);

gboolean
on_treepkglist_focus_out_event         (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

void
on_bupgrade_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_bdown_clicked                       (GtkButton       *button,
                                        gpointer         user_data);

void
on_bup_clicked                         (GtkButton       *button,
                                        gpointer         user_data);

void
on_bsearchon_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_bsearch_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_entrysearch_focus_in_event          (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

gboolean
on_entrysearch_focus_out_event         (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

gboolean
on_bbarsep_button_release_event        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void
on_bfullscreen_clicked                 (GtkButton       *button,
                                        gpointer         user_data);
