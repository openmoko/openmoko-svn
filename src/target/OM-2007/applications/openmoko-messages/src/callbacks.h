#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_

#include <gtk/gtk.h>
#include "sms-dialog-window.h"
#include "sms-membership-window.h"
#include "main.h"

const static gint states[] = { UNREAD, READ, UNREAD, UNREAD, FORWARD,
																	UNREAD, UNREAD, REPLIED, READ, UNREAD };
const static gchar *names[] = { "John B.", "Jane Z.", "Carl O.", "Owen P.", "Jeremy F.",
	                         "Michael M.", "Ute D.", "Akira T.", "Thomas F.", "Matthew J."};
const static gchar *subjects[] = { "Hello Alex", "We need sms support", "I need u", "Help harald", "The gui is really cool",
	                         "Can't u see", "2:00 pm", "Bugzillia page", "Hi there", "Target support"};
const static gchar *folders[] = { "Inbox", "Outbox", "Sent", "Inbox", "Inbox",
	                         "Inbox", "Inbox", "Inbox", "Inbox", "Inbox"};
const static gchar *contents[] = {"Hello Alex", "We need sms support", "I need u", "Help harald", "The gui is really cool",
	                         "Can't u see", "2:00 pm", "Bugzillia page", "Hi there", "Target support"};

gboolean cb_filter_changed(GtkWidget* widget, gchar* text, MessengerData* d);

void cb_new_sms (GtkMenuItem* item, MessengerData* d);
void cb_new_mail (GtkMenuItem* item, MessengerData* d);
void cb_new_folder (GtkMenuItem* item, MessengerData* d);
void cb_mode_read (GtkMenuItem* item, MessengerData* d);
void cb_mode_reply (GtkMenuItem* item, MessengerData* d);
void cb_mode_forward (GtkMenuItem* item, MessengerData* d);
void cb_delete_folder (GtkMenuItem* item, MessengerData* d);
void cb_delete_message (GtkMenuItem* item, MessengerData* d);
void cb_mmitem_activate (GtkMenuItem* item, MessengerData* d);
void cb_fnitem_activate (GtkMenuItem* item, MessengerData* d);
void cb_cursor_changed(GtkTreeSelection* selection, MessengerData* d);

void cb_nfBtn_clicked (GtkButton* button, MessengerData* d);
void cb_nfResetBtn_clicked (GtkButton* button, MessengerData* d);
void cb_frBtn_clicked (GtkButton* button, MessengerData* d);
void cb_frResetBtn_clicked (GtkButton* button, GtkWidget* entry);
void cb_dfBtn_clicked (GtkButton* button, MessengerData* d);
void on_mmode_rdo_btn_clicked (gchar* folder);
void on_btnsend_clicked                 (GtkButton       *button,
                                        gpointer         user_data);
void on_btn_address_clicked							(GtkButton       *button,
                                        gpointer         user_data);
                                        
void cb_search_entry_changed (GtkEditable* editable, MessengerData* d);
void cb_search_on (MessengerData* d);
void cb_search_off (MessengerData* d);
#endif

