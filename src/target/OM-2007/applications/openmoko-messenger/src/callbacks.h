#include <gtk/gtk.h>

enum
{
  COL_ICON = 0,
  COL_FROM,
  COL_CONTENT,
  COL_TIME,
  COL_STATUS,
  COL_FOLDER,
  COL_MSG_ID,
  NUM_COLS
};

enum
{
	MODE_SMS_NEW,
	MODE_MAIL_NEW,
	MODE_SMS_READ,
	MODE_MAIL_READ,
	MODE_MESSAGE_MEMBERSHIP,
	NUM_MODES
};

#define MODE_SMS_REPLY 0
#define MODE_MAIL_REPLY 1
#define MODE_SMS_FORWARD 0
#define MODE_MAIL_FORWARD 1
#define TOOLBAR_PAGE 0
#define SEARCH_PAGE 1

GtkTreeModel *filter;
GtkWidget *window1;
guint handler_id;
gint current_status;
GtkListStore *liststore;

void set_information_dialog (GtkWidget *infor_dialog, gchar *information);

void cb_new_folder(gpointer user_data);

void set_filter_menu (GtkWidget *widget);

void set_application_menu (GtkWidget *widget);

void load_messages (GtkWidget *widget);

void set_action_button_menu (GtkWidget *widget);

void cb_clean_mode();

void on_mitem_new_folder();

void on_mitem_mode_reply ();

void on_mitem_mode_forward ();

void cb_delete_message (gpointer user_data);

void on_mitem_delete_selected_message ();

void cb_new_folder(gpointer user_data);

void cb_delete_folder (gpointer user_data);

void on_mitem_delete_current_folder();

void
on_toolbar_search_btn_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_search_entry_changed                (GtkEditable     *editable,
                                        gpointer         user_data);

void
on_searchbar_quit_btn_clicked          (GtkButton       *button,
                                        gpointer         user_data);

void
on_sms_btnsend_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_btn_address_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_sms_txtview_key_release_event       (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);

void
on_bar_separate_btn_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_email_btnsend_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_email_btnattach_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
open_mail_account                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
help                                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_btn_rf_reset_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_btn_rf_clicked                      (GtkButton       *button,
                                        gpointer         user_data);

void
cb_open_mm_mode                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
cb_folder_rename                       (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_btn_nf_reset_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
cb_action_mitem_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
