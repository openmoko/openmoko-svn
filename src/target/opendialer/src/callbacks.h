#include <gtk/gtk.h>


void
on_buttonUpper_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonStylusMode_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_button1_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_button2ABC_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_button3DEF_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_button4GHI_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_button5JKL_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_button6MNO_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_button7PQRS_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_button8TUV_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_button9WXYZ_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonAsterisk_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_button0_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonSharp_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonPlus_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonBack_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonForward_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonDel_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_window1_destroy                     (GtkObject       *object,
                                        gpointer         user_data);

void
on_textviewCodes_backspace             (GtkTextView     *textview,
                                        gpointer         user_data);

void
on_textviewCodes_delete_from_cursor    (GtkTextView     *textview,
                                        GtkDeleteType    type,
                                        gint             count,
                                        gpointer         user_data);

void
on_textviewCodes_insert_at_cursor      (GtkTextView     *textview,
                                        gchar           *string,
                                        gpointer         user_data);

void
on_textviewCodes_move_cursor           (GtkTextView     *textview,
                                        GtkMovementStep  step,
                                        gint             count,
                                        gboolean         extend_selection,
                                        gpointer         user_data);

gboolean
on_textviewCodes_focus_out_event       (GtkWidget       *widget,
                                        GdkEventFocus   *event,
                                        gpointer         user_data);

void
on_buttonActions_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonMagic_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_treeviewContacts_cursor_changed     (GtkTreeView     *treeview,
                                        gpointer         user_data);

void
on_buttonPrevious_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonDown_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_treeviewContacts_row_collapsed      (GtkTreeView     *treeview,
                                        GtkTreeIter     *iter,
                                        GtkTreePath     *path,
                                        gpointer         user_data);

void
on_new1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_open1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_save_as1_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_quit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_cut1_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_copy1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_paste1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_delete1_activate                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_about1_activate                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_dial_activate                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_send_message_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_add_number_to_contact_activate      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_new_contact_with_this_number_activate
                                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_entrySearch_changed                 (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_entrySearch_activate                (GtkEntry        *entry,
                                        gpointer         user_data);

void
on_menuhelp_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_menuquit_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_menuhelp_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_menuquit_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_buttonDialer_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_menudial_activate                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_menusend_message_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_menuadd_number_to_contact1_activate (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_menunew_contact_with_this_number1_activate
                                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_buttonActions_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_buttonIcon_clicked                  (GtkButton       *button,
                                        gpointer         user_data);
