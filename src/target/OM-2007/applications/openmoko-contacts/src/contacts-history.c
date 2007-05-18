/*
 * Copyright (C) 2006-2007 by OpenMoko, Inc.
 * Written by OpenedHand Ltd <info@openedhand.com>
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "contacts-history.h"

#include <string.h>
#include <time.h>

G_DEFINE_TYPE (ContactsHistory, contacts_history, GTK_TYPE_VBOX);

#define CONTACTS_HISTORY_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj),\
	CONTACTS_TYPE_HISTORY, \
	ContactsHistoryPrivate))

struct _ContactsHistoryPrivate 
{
  MokoJournal       *journal;
  GtkWidget         *menu;
  GtkWidget         *open_item;
  
  gchar             *uid;
  
};

enum
{
  PROP_0,
  PROP_UID
};

enum
{
  ACTIVATED,
  LAST_SIGNAL
};

static guint _history_signals[LAST_SIGNAL] = { 0 };

static GtkVBoxClass *parent_class = NULL;

static GQuark entry_quark = 0;
static GQuark label_quark = 0;
static GQuark shown_quark = 0;
static GQuark press_quark = 0;
static GQuark preview_quark = 0;

#define HISTORY_EMAIL_ICON        "moko-history-mail"
#define HISTORY_SMS_ICON          "moko-history-im"
#define HISTORY_CALL_IN_ICON      "moko-history-call-in"
#define HISTORY_CALL_OUT_ICON     "moko-history-call-out"
#define HISTORY_CALL_MISSED_ICON  "moko-history-call-missed"

#define GBOOLEAN_TO_POINTER(i) ((gpointer) ((i) ? 2 : 1))
#define GPOINTER_TO_BOOLEAN(i) ((gboolean) ((((gint)(i)) == 2) ? TRUE : FALSE))

static void
contacts_history_preview_clicked (ContactsHistory *history, GtkWidget *preview)
{
  MokoJournalEntry *entry = NULL;
  GtkWidget *label;
  gboolean shown = 0;
  
  entry = g_object_get_qdata (G_OBJECT (preview), entry_quark);
  if (!entry)
    return;
  
  label = g_object_get_qdata (G_OBJECT (preview), label_quark);
  shown = GPOINTER_TO_BOOLEAN (g_object_get_qdata (G_OBJECT (preview),
                               shown_quark));

  switch (moko_journal_entry_get_type (entry)) {
    case EMAIL_JOURNAL_ENTRY:
    case SMS_JOURNAL_ENTRY:
      if (!label)
        break;
      if (shown) {
          gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
          gtk_label_set_line_wrap (GTK_LABEL (label), FALSE);
      } else {
          gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_NONE);
          gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
      }      
      break;
    
    case VOICE_JOURNAL_ENTRY:
    default:
      break;
  }
  g_object_set_qdata (G_OBJECT (preview), shown_quark, 
                      GBOOLEAN_TO_POINTER (!shown));
 
}

static void
contacts_history_preview_show_menu (ContactsHistory *history, 
                                    GtkWidget *preview)
{
  ContactsHistoryPrivate *priv;
  
  g_return_if_fail(CONTACTS_IS_HISTORY(history));
  priv = CONTACTS_HISTORY_GET_PRIVATE (history);  
  
  g_object_set_qdata (G_OBJECT (priv->menu), preview_quark, (gpointer)preview);
  
  gtk_menu_popup (GTK_MENU (priv->menu),
                  NULL, NULL,
                  NULL, NULL,
                  0, gtk_get_current_event_time ());  
}

static void
contacts_history_preview_select (GtkMenuItem *item, ContactsHistory *history)
{
  ContactsHistoryPrivate *priv;
  GtkWidget *preview = NULL;
  
  g_return_if_fail(CONTACTS_IS_HISTORY(history));
  priv = CONTACTS_HISTORY_GET_PRIVATE (history);  
  
  preview = g_object_get_qdata (G_OBJECT (priv->menu), preview_quark);
  if (!GTK_IS_WIDGET (preview))
    return;
  
  contacts_history_preview_clicked (history, preview);
}

static void
contacts_history_preview_open (GtkMenuItem *item, ContactsHistory *history)
{
  ContactsHistoryPrivate *priv;
  GtkWidget *preview = NULL;
  MokoJournalEntry *entry = NULL;
  
  g_return_if_fail(CONTACTS_IS_HISTORY(history));
  priv = CONTACTS_HISTORY_GET_PRIVATE (history);  
  
  preview = g_object_get_qdata (G_OBJECT (priv->menu), preview_quark);
  if (!GTK_IS_WIDGET (preview))
    return;
  
  entry = g_object_get_qdata (G_OBJECT (preview), entry_quark);
  
  g_signal_emit (history, _history_signals[ACTIVATED], 0, (gpointer)entry);
}

/* 
 * Handles all events for the preview widgets, deciding if there was a normal
 * clicked event, or a press-wait-release event
 */
static gboolean
contacts_history_button_released (GtkWidget *eb, 
                                  GdkEventButton *event, 
                                  ContactsHistory *history)
{
#define RELEASE_TIMEOUT 500
  gint pressed =GPOINTER_TO_INT(g_object_get_qdata(G_OBJECT (eb), press_quark));
  gint released = event->time;
  gint diff = released - pressed;
  
  
  if (diff < 500) {
    contacts_history_preview_clicked (history, eb);
  } else {
    contacts_history_preview_show_menu (history, eb);
  }
  
  return FALSE;
}

/* 
 * Sets the press_quark on the event box the time of the button press event
 */
static gboolean
contacts_history_button_pressed (GtkWidget *eb, 
                                 GdkEventButton *event, 
                                 ContactsHistory *history)
{
  g_object_set_qdata (G_OBJECT (eb), press_quark,GINT_TO_POINTER (event->time));
  return FALSE;
}
  

/* 
 * Creates a nice email preview widget
 */
static GtkWidget*
contacts_history_create_email_preview (MokoJournalEntry *entry,
                                       enum MessageDirection direction)
{
#define BUF 256
  GtkWidget *eb, *hbox, *icon, *label;
  const MokoTime *mtime;
  gchar *blurb = NULL;
  gchar t[BUF];
  GDate *date = NULL;
  time_t time;
  
  /* Make MokoTime into something human-readable */
  mtime = moko_journal_entry_get_dtstart (entry);
  if (mtime) {
    time = moko_time_as_timet (mtime);
    date = g_date_new ();
    g_date_set_time_t (date, time);
    g_date_strftime (t, BUF, "%a %d %B %Y", date);
  
  } else {
    t[0] = '\0';
  }
    
  /* Create the event box */
  eb = gtk_event_box_new ();
  gtk_event_box_set_visible_window (GTK_EVENT_BOX (eb), TRUE);
  gtk_event_box_set_above_child (GTK_EVENT_BOX (eb), FALSE);
  
  hbox = gtk_hbox_new (FALSE, 4);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  gtk_container_add (GTK_CONTAINER (eb), hbox);
  
  /* Depending on the direction, we load the correct icon, and set the correct
     name for theming */
  gtk_widget_set_name (eb, "mokohistoryemail");
  icon = gtk_image_new_from_icon_name (HISTORY_EMAIL_ICON, 
                                       GTK_ICON_SIZE_MENU);
  blurb = g_strdup_printf ("<b>%s</b>\n<span size='x-small'>%s</span>",
                           moko_journal_entry_get_summary (entry),
                           t);
  
  gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 2);
  
  label = gtk_label_new ("");
  gtk_widget_set_name (label, "mokohistorylabel");
  gtk_label_set_markup (GTK_LABEL (label), blurb);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
  gtk_label_set_line_wrap (GTK_LABEL (label), FALSE);  
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 2);
  
  /* Force theming */
  gtk_widget_set_state (eb, GTK_STATE_PRELIGHT);
  
  /* Set quark(s) */
  g_object_set_qdata (G_OBJECT (eb), entry_quark, (gpointer)entry);  
  g_object_set_qdata (G_OBJECT (eb), label_quark, (gpointer)label);  
  
  if (date)
    g_date_free (date);
  g_free (blurb);
  
  return eb;
}

/* 
 * Creates a nice sms preview widget
 */
static GtkWidget*
contacts_history_create_sms_preview (MokoJournalEntry *entry,
                                     enum MessageDirection direction)
{
  GtkWidget *eb, *hbox, *icon, *label;
  gchar *blurb = NULL;
  
  /* Create the event box */
  eb = gtk_event_box_new ();
  gtk_event_box_set_visible_window (GTK_EVENT_BOX (eb), TRUE);
  gtk_event_box_set_above_child (GTK_EVENT_BOX (eb), FALSE);
  
  hbox = gtk_hbox_new (FALSE, 4);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  gtk_container_add (GTK_CONTAINER (eb), hbox);
  
  /* Depending on the direction, we load the correct icon, and set the correct
     name for theming */
  gtk_widget_set_name (eb, "mokohistorysms");
  icon = gtk_image_new_from_icon_name (HISTORY_SMS_ICON, GTK_ICON_SIZE_MENU);
  blurb = g_strdup_printf ("<b>%s</b>", moko_journal_entry_get_summary (entry));
  
  gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 2);
  
  label = gtk_label_new ("");
  gtk_widget_set_name (label, "mokohistorylabel");
  gtk_label_set_markup (GTK_LABEL (label), blurb);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
  gtk_label_set_line_wrap (GTK_LABEL (label), FALSE);
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 2);
  
  /* Force theming */
  gtk_widget_set_state (eb, GTK_STATE_PRELIGHT);
  
  /* Set quark(s) */
  g_object_set_qdata (G_OBJECT (eb), entry_quark, (gpointer)entry);  
  g_object_set_qdata (G_OBJECT (eb), label_quark, (gpointer)label);
    
  g_free (blurb);
  return eb;
}

/* 
 * Creates a nice voice call preview widget
 */
static GtkWidget*
contacts_history_create_voice_preview (MokoJournalEntry *entry,
                                       enum MessageDirection direction)
{
#define BUF 256
  GtkWidget *eb, *hbox, *icon, *label;
  MokoJournalVoiceInfo *info;
  const MokoTime *mtime;
  gchar *blurb = NULL;
  gchar t[BUF];
  GDate *date = NULL;
  time_t time;
  gboolean missed = FALSE;

  /* If we have call info, lets load it */
  if (moko_journal_entry_get_voice_info (entry, &info)) {
    missed = moko_journal_voice_info_get_was_missed (info);
  }
  
  /* Make MokoTime into something human-readable */
  mtime = moko_journal_entry_get_dtstart (entry);
  if (mtime) {
    time = moko_time_as_timet (mtime);
    date = g_date_new ();
    g_date_set_time_t (date, time);
    g_date_strftime (t, BUF, "%a %d %B %Y", date);
  
  } else {
    t[0] = '\0';
  }
    
  /* Create the event box */
  eb = gtk_event_box_new ();
  gtk_event_box_set_visible_window (GTK_EVENT_BOX (eb), TRUE);
  gtk_event_box_set_above_child (GTK_EVENT_BOX (eb), FALSE);
  
  hbox = gtk_hbox_new (FALSE, 4);
  gtk_container_set_border_width (GTK_CONTAINER (hbox), 6);
  gtk_container_add (GTK_CONTAINER (eb), hbox);
  
  /* Depending on the direction, we load the correct icon, and set the correct
     name for theming */
  if (missed) {
    gtk_widget_set_name (eb, "mokohistorycall-missed");
    icon = gtk_image_new_from_icon_name (HISTORY_CALL_MISSED_ICON, 
                                         GTK_ICON_SIZE_MENU);
    blurb = g_strdup_printf ("<b>Missed Call :</b> "
                             "<span weight='heavy'>%s</span>", t);
    
  } else if (direction == DIRECTION_IN) {
    gtk_widget_set_name (eb, "mokohistorycall-in");
    icon = gtk_image_new_from_icon_name (HISTORY_CALL_IN_ICON, 
                                         GTK_ICON_SIZE_MENU);
    blurb = g_strdup_printf ("Recieved Call : <b>%s</b>", t);
    
  } else {
    gtk_widget_set_name (eb, "mokohistorycall-out");
    icon = gtk_image_new_from_icon_name (HISTORY_CALL_OUT_ICON, 
                                         GTK_ICON_SIZE_MENU);
    blurb = g_strdup_printf ("Dialed Call : <b>%s</b>", t);
    
  }
  gtk_box_pack_start (GTK_BOX (hbox), icon, FALSE, FALSE, 2);
  
  label = gtk_label_new ("");
  gtk_label_set_markup (GTK_LABEL (label), blurb);
  gtk_widget_set_name (label, "mokohistorylabel");
  gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 2);
  
  /* Force theming */
  gtk_widget_set_state (eb, GTK_STATE_PRELIGHT);
  
  /* Set quark(s) */
  g_object_set_qdata (G_OBJECT (eb), entry_quark, (gpointer)entry);
  
  if (date)
    g_date_free (date);
  g_free (blurb);
  
  return eb;
}

/*  
 * Filters the entry depending on type 
 */
static GtkWidget*
contacts_history_make_preview (ContactsHistory *history, 
                               MokoJournalEntry *entry)
{
  ContactsHistoryPrivate *priv;
  GtkWidget *preview = NULL;
  enum MessageDirection	direction;
  
  g_return_val_if_fail(CONTACTS_IS_HISTORY(history), NULL);
  priv = CONTACTS_HISTORY_GET_PRIVATE (history);
  
  if (!moko_journal_entry_get_direction (entry, &direction))
    direction = DIRECTION_IN;
    
  switch (moko_journal_entry_get_type (entry)) {
    case EMAIL_JOURNAL_ENTRY:
      preview = contacts_history_create_email_preview (entry, direction);
      break;
    
    case SMS_JOURNAL_ENTRY:
      preview = contacts_history_create_sms_preview (entry, direction);
      break;
    
    case VOICE_JOURNAL_ENTRY:
      preview = contacts_history_create_voice_preview (entry, direction);
      break;
    
    default:
      break;
  }
  
  /* Hook up the signals */
  if (!GTK_WIDGET_NO_WINDOW (preview)) {
    g_signal_connect (G_OBJECT (preview), "button-press-event",
                      G_CALLBACK (contacts_history_button_pressed), 
                      (gpointer)history);
    g_signal_connect (G_OBJECT (preview), "button-release-event",
                      G_CALLBACK (contacts_history_button_released), 
                      (gpointer)history);
  }
  return preview;
}

/* Sorts a list of MokoJournalEntrys by their date */
static gint
_sort_by_date (MokoJournalEntry *a, MokoJournalEntry *b)
{
  const MokoTime *time;
  time_t t1;
  time_t t2;
  
  time = moko_journal_entry_get_dtstart (a);
  t1 = moko_time_as_timet (time);
  
  time = moko_journal_entry_get_dtstart (b);
  t2 = moko_time_as_timet (time);
  
  return -1 * (t1 - t2);
}

/*
 * Will remove the current history, and create all the widgets again.
*/
static void
contacts_history_refresh_ui (ContactsHistory *history)
{
  ContactsHistoryPrivate *priv;
  GList *children, *c;
	
  g_return_if_fail(CONTACTS_IS_HISTORY(history));
  priv = CONTACTS_HISTORY_GET_PRIVATE (history);
  
  g_return_if_fail (priv->journal);
  
  if (!priv->uid)
    return;
  
  /* First we remove all of the existing history-widgets */
  children = gtk_container_get_children (GTK_CONTAINER (history));
  for (c = children; c != NULL; c = c->next) {
    gtk_container_remove (GTK_CONTAINER (history), GTK_WIDGET (c->data));
  }
  
  g_list_free (children);
  children = NULL;
  
  /* Traverse though the Journal looking for all entries that 
     correspond to the the priv->uid
     FIXME : MokoJournal should have a special function for this. 
  */
  gint len = moko_journal_get_nb_entries (priv->journal);
  gint i;
  for (i = 0; i < len; i++) {
    MokoJournalEntry *entry = NULL;
    
    if (moko_journal_get_entry_at (priv->journal, i, &entry)) {
      if (strcmp (priv->uid, moko_journal_entry_get_contact_uid (entry)) == 0) {
        children = g_list_append (children, entry);
      }
    }
  }
  
  /* Sort the list in order of time, most recent first */
  children = g_list_sort (children, (GCompareFunc)_sort_by_date);
  
  if (!children) {
    GtkWidget *label = gtk_label_new ("No communication history");
    gtk_box_pack_start (GTK_BOX (history), label, TRUE, TRUE, 0);
    gtk_widget_show_all (GTK_WIDGET (history));
    return;
  }
    
  /* Finally, go through the sorted list creating the widgets and adding them */
  for (c = children; c != NULL; c = c->next) {
    GtkWidget *preview = NULL;
    if ((preview = contacts_history_make_preview (history, c->data)))
      gtk_box_pack_start (GTK_BOX (history), preview, FALSE, FALSE, 0);
  }
  
  gtk_widget_show_all (GTK_WIDGET (history));
}

void
contacts_history_update_uid (ContactsHistory *history, const gchar *uid)
{
  g_return_if_fail (CONTACTS_IS_HISTORY (history));
  
  g_object_set (G_OBJECT (history), "uid", uid, NULL);
}

/* GObject functions */

static void
contacts_history_set_property (GObject      *object, 
			       guint         prop_id,
			       const GValue *value, 
			       GParamSpec   *pspec)
{
  ContactsHistoryPrivate *priv;
	
  g_return_if_fail(CONTACTS_IS_HISTORY(object));
  priv = CONTACTS_HISTORY_GET_PRIVATE (object);

  switch (prop_id) {
    case PROP_UID:
      if (priv->uid) 
        g_free (priv->uid);
      priv->uid = g_strdup (g_value_get_string (value)); 
      contacts_history_refresh_ui (CONTACTS_HISTORY (object));
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
contacts_history_get_property (GObject    *object, 
			       guint       prop_id,
			       GValue     *value, 
			       GParamSpec *pspec)
{
  ContactsHistoryPrivate *priv;
	
  g_return_if_fail(CONTACTS_IS_HISTORY(object));
  priv = CONTACTS_HISTORY_GET_PRIVATE (object);

  switch (prop_id) {
    case PROP_UID:
      g_value_set_string (value, priv->uid);
      break;
    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  } 
}


static void
contacts_history_finalize(GObject *obj)
{
  ContactsHistoryPrivate *priv;
	
  g_return_if_fail(CONTACTS_IS_HISTORY(obj));

  priv = CONTACTS_HISTORY_GET_PRIVATE (obj);
  
  if (priv->uid)
   g_free (priv->uid);
  
  if (priv->journal)
    moko_journal_close (priv->journal);
  
  if (G_OBJECT_CLASS(parent_class)->finalize)
    G_OBJECT_CLASS(parent_class)->finalize(obj);
}

static void
contacts_history_class_init (ContactsHistoryClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  parent_class = GTK_VBOX_CLASS (klass);
  
  gobject_class->finalize = contacts_history_finalize;
  gobject_class->get_property = contacts_history_get_property;
  gobject_class->set_property = contacts_history_set_property;  
  
  g_type_class_add_private (gobject_class, sizeof (ContactsHistoryPrivate));
  
  /* Install class properties */
  g_object_class_install_property (gobject_class,
		                   PROP_UID,
		                   g_param_spec_string ("uid",
		                     "UID",
		                     "The Contact uid",
		                     NULL,
		                     G_PARAM_CONSTRUCT|G_PARAM_READWRITE));

  /* Class signals */
  _history_signals[ACTIVATED] =
    g_signal_new ("entry-activated",
		  G_OBJECT_CLASS_TYPE (gobject_class),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (ContactsHistoryClass, entry_activated),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__POINTER,
		  G_TYPE_NONE, 1, G_TYPE_POINTER);
}

static void
contacts_history_init (ContactsHistory *self)
{
  ContactsHistoryPrivate *priv;
  GtkWidget *menu;
  GtkWidget *item;
	
  priv = CONTACTS_HISTORY_GET_PRIVATE (self);
  
  /* Create the quarks */
  entry_quark = g_quark_from_static_string("contacts-history-entry");
  label_quark = g_quark_from_static_string("contacts-history-label");
  shown_quark = g_quark_from_static_string("contacts-history-shown");
  press_quark = g_quark_from_static_string("contacts-history-button-pressed");
  preview_quark = g_quark_from_static_string ("contacts-histroy-preview");
  
  /* Create the MokoJournal object */
  priv->journal = moko_journal_open_default ();
  if (!priv->journal)
    return;
  
  /* load all journal entries from the journal on storage*/
  if (!moko_journal_load_from_storage (priv->journal)) {
    g_warning ("Unable to load journal from storage\n") ;
    return;
  }  
  
  /* Create the popmenu for tap-and-hold */
  menu = gtk_menu_new ();
  priv->menu = menu;
  
  item = gtk_menu_item_new_with_label ("Select");
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  g_signal_connect (G_OBJECT (item), "activate",
                    G_CALLBACK (contacts_history_preview_select),
                    (gpointer)self);

  item = gtk_menu_item_new_with_label ("Open...");
  priv->open_item = item;
  gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);
  g_signal_connect (G_OBJECT (item), "activate",
                    G_CALLBACK (contacts_history_preview_open),
                    (gpointer)self);

  gtk_widget_show_all (menu);
}

GtkWidget*
contacts_history_new (void)
{
  ContactsHistory *history;
  history = g_object_new (CONTACTS_TYPE_HISTORY, 
                          "homogeneous", FALSE,
			  "spacing", 1,
			  "border-width", 10,
			  NULL);
        
  return GTK_WIDGET(history);
}
