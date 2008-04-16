/*  
 *  Copyright (C) 2007 Openmoko, Inc.
 *
 *  Authored by OpenedHand Ltd <info@openedhand.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-bindings.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <glib-object.h>
#include <libnotify/notify.h>
#include <libebook/e-book.h>

#include "moko-network.h"
#include "moko-dialer.h"
#include "moko-sms.h"
#include "moko-pb.h"
#include "moko-headset.h"
#include "moko-sound.h"

#include "moko-contacts.h"

#define PHONEKIT_NAMESPACE "org.openmoko.PhoneKit"
#define NETWORK_PATH "/org/openmoko/PhoneKit/Network"
#define DIALER_PATH "/org/openmoko/PhoneKit/Dialer"
#define SMS_PATH "/org/openmoko/PhoneKit/Sms"
#define DIALER_INTERFACE "org.openmoko.PhoneKit.Dialer"

#define PK_X_PBENTRY "X-PHONEKIT-PBENTRY"

static gchar *number = NULL;

static void
_dial_number (DBusGConnection *conn)
{
  DBusGProxy *proxy = NULL;
  GError *error = NULL;

  proxy = dbus_g_proxy_new_for_name (conn,
                                      PHONEKIT_NAMESPACE,
                                      DIALER_PATH,
                                      DIALER_INTERFACE);

  if (!proxy)
    return;
  
  dbus_g_proxy_call (proxy, "Dial", &error,
                     G_TYPE_STRING, number,
                     G_TYPE_INVALID, G_TYPE_INVALID);
  if (error)
    g_warning (error->message);

}

static EContact *
ebook_find (EBook *book, EContactField field, const gchar *val)
{
  EBookQuery *query;
  GList *list;
  EContact *contact = NULL;

  query = e_book_query_field_test(field, E_BOOK_QUERY_IS, val);

  if (!e_book_get_contacts (book, query, &list, NULL)) {
    e_book_query_unref(query);

    return NULL;
  }

  if (list) {
    contact = list->data;
    g_list_free(list);
  }

  e_book_query_unref(query);

  return contact;
}

/*
 * XXX EBook signals contacts-changed for imported phonebook entries.
 *
 * Keep a record of imported phonebook entries and do not remove PK_X_PBENTRY
 * from them.
 */
static void
ebook_pb_session_begin (EBook *book)
{
}

static void
ebook_pb_session_end (EBook *book)
{
}

static void
ebook_pb_session_add (EBook *book, const gchar *id)
{
  GHashTable *table;

  table = g_object_get_data (G_OBJECT (book), "pk-newly-added");
  if (!table) {
    table = g_hash_table_new_full (g_str_hash, g_str_equal,
	g_free, NULL);

    g_object_set_data_full (G_OBJECT (book),
	"pk-newly-added", table, (GDestroyNotify) g_hash_table_destroy);
  }

  g_hash_table_insert (table, g_strdup (id), GINT_TO_POINTER(1));
}

static gboolean
ebook_pb_session_remove (EBook *book, const gchar *id)
{
  GHashTable *table;

  table = g_object_get_data (G_OBJECT (book), "pk-newly-added");
  if (!table)
    return FALSE;

  return g_hash_table_remove (table, id);
}

static void
ebook_commit_contact(EBook *book, EBookStatus status, const char *id, gpointer data)
{
  EContact *contact;

  e_book_get_contact (book, id, &contact, NULL);

  if (contact) {
    ebook_pb_session_add (book, id);
    e_book_async_commit_contact (book, contact, NULL, NULL);
  }
}

static void
on_ebook_changed (EBookView  *view,
                  GList      *contacts,
		  EBook      *book)
{
  EVCard *evc;
  EVCardAttribute *attr;
  const gchar *uid;

  for (; contacts; contacts = contacts->next) {
    evc = E_VCARD (contacts->data);
    attr = e_vcard_get_attribute (evc, PK_X_PBENTRY);

    uid = e_contact_get_const (E_CONTACT (evc), E_CONTACT_UID);
    if (ebook_pb_session_remove (book, uid))
      continue;

    if (attr) {
      e_vcard_remove_attribute (evc, attr);
      e_book_async_commit_contact (book, E_CONTACT (evc),
	  NULL, NULL);
    }
  }
}

static void
ebook_clear_pb (EBook *book)
{
  EBookQuery *query;
  GList *contacts, *tmp_list;

  query = e_book_query_vcard_field_exists (PK_X_PBENTRY);

  if (!e_book_get_contacts (book, query, &contacts, NULL)) {
    e_book_query_unref (query);

    return;
  }

  if (!contacts) {
    e_book_query_unref (query);

    return;
  }

  for (tmp_list = contacts; tmp_list; tmp_list = tmp_list->next) {
    EContact *contact = E_CONTACT (tmp_list->data);

    tmp_list->data = (gpointer) e_contact_get_const (contact, E_CONTACT_UID);
  }

  e_book_async_remove_contacts (book, contacts, NULL, NULL);

  g_list_free (contacts);
  e_book_query_unref (query);
}

static void
on_pb_status_changed (MokoPb *pb, GParamSpec *pspec, EBook *book)
{
  MokoPbStatus status;

  g_object_get (G_OBJECT (pb), pspec->name, &status, NULL);

  if (status == MOKO_PB_STATUS_READY)
    ebook_pb_session_end (book);
  else if (status == MOKO_PB_STATUS_BUSY)
    ebook_pb_session_begin (book);
}

static void
ebook_monitor (EBook *book, MokoPb *pb)
{
  EBookView *view;
  EBookQuery *query;

  query = e_book_query_vcard_field_exists (PK_X_PBENTRY);

  if (e_book_get_book_view (book, query, NULL, 0, &view, NULL)) {
    g_signal_connect (G_OBJECT (view), "contacts-changed",
                    G_CALLBACK (on_ebook_changed), book);

    g_signal_connect (G_OBJECT (pb), "notify::status",
                    G_CALLBACK (on_pb_status_changed), book);

    e_book_view_start (view);
  }
  e_book_query_unref(query);
}

static EBook *
ebook_get (void)
{
  MokoContacts *m_contacts;
  gpointer backend;

  m_contacts = moko_contacts_get_default ();
  if (!m_contacts)
    return NULL;

  backend = moko_contacts_get_backend (m_contacts);

  return (E_IS_BOOK (backend)) ? backend : NULL;
}

static void
on_pb_entry (MokoPb *pb, gint index, const gchar *number, const gchar *text, EBook *book)
{
  EContact *contact;
  EVCard *evc;
  EVCardAttribute *attr;
  gchar *vcard;
  gchar buf[8];

  contact = ebook_find (book, E_CONTACT_FULL_NAME, text);
  g_debug ("%s new entry: %s", (contact) ? "ignore" : "import", text);

  if (contact)
    return;

  evc = e_vcard_new ();

  attr = e_vcard_attribute_new (NULL, EVC_FN);
  e_vcard_attribute_add_value (attr, text);
  e_vcard_add_attribute (evc, attr);

  attr = e_vcard_attribute_new (NULL, EVC_TEL);
  e_vcard_attribute_add_value (attr, number);
  e_vcard_add_attribute (evc, attr);

  attr = e_vcard_attribute_new (NULL, PK_X_PBENTRY);
  g_snprintf (buf, sizeof (buf), "%d", index);
  e_vcard_attribute_add_value (attr, buf);
  e_vcard_add_attribute (evc, attr);

  vcard = e_vcard_to_string (evc, EVC_FORMAT_VCARD_30);
  contact = e_contact_new_from_vcard (vcard);
  g_free (vcard);
  g_object_unref (evc);

  e_book_async_add_contact (book, contact,
      (EBookIdCallback) ebook_commit_contact, NULL);
}

static gboolean
pb_import (MokoPb *pb)
{
  EBook *book;

  book = ebook_get();
  if (!book)
    return FALSE;

  ebook_monitor (book, pb);

  g_signal_connect (pb, "entry", G_CALLBACK (on_pb_entry), book);
  moko_pb_get_entries (pb, NULL);

  return FALSE;
}

static void
on_network_status_changed (MokoNetwork *network, PhoneKitNetworkStatus status, MokoPb *pb)
{
  /* we can't read phonebook right after modem power-on;
   * wait some more time */
  g_timeout_add_seconds (15, (GSourceFunc) pb_import, pb);
  g_signal_handlers_disconnect_by_func (network,
      on_network_status_changed, pb);
}

static void
pb_sync (MokoPb *pb, MokoNetwork *network)
{
  EBook *book;
  PhoneKitNetworkStatus status;

  book = ebook_get ();
  if (!book)
    return;

  ebook_clear_pb (book);

  /* wait for modem power-on */
  g_signal_connect (network, "status-changed",
                    G_CALLBACK (on_network_status_changed), pb);
  status = moko_network_get_status (network);
  if (status != PK_NETWORK_POWERDOWN)
    on_network_status_changed (network, status, pb);
}

DBusHandlerResult headset_signal_filter (DBusConnection *bus, DBusMessage *msg, void *user_data) 
{
	PhoneKitDialerStatus status;
	MokoNetwork *network;
	MokoDialer *dialer;

	g_debug( "headset signal filter" );

	network = moko_network_get_default ();
	dialer = moko_dialer_get_default (network);
	status = moko_dialer_get_status(dialer);  

	if ( dbus_message_is_signal( msg, "org.openmoko.PhoneKit.Headset", "HeadsetIn" ) )
	{
		moko_headset_status_set(HEADSET_STATUS_IN);
		g_debug( "Headset In" );

		if ( PK_DIALER_TALKING == status ) {
			moko_sound_profile_set(SOUND_PROFILE_GSM_HEADSET);
			g_debug("SOUND_PROFILE_GSM_HEADSET\n");
		}	
		else {
			moko_sound_profile_set(SOUND_PROFILE_HEADSET);
			g_debug("SOUND_PROFILE_HEADSET\n");
		}	
		return DBUS_HANDLER_RESULT_HANDLED;
	}
	else if ( dbus_message_is_signal( msg,"org.openmoko.PhoneKit.Headset", "HeadsetOut" ) )
	{
	        moko_headset_status_set(HEADSET_STATUS_OUT);
		g_debug( "Headset Out" );

		if ( PK_DIALER_TALKING == status ) {
			moko_sound_profile_set(SOUND_PROFILE_GSM_HANDSET);
			g_debug("SOUND_PROFILE_GSM_HANDSET\n");
		}	
		else {	
			moko_sound_profile_set(SOUND_PROFILE_STEREO_OUT);
			g_debug("SOUND_PROFILE_STEREO_OUT\n");
		}	
		return DBUS_HANDLER_RESULT_HANDLED;
	}

	g_debug( "(unknown dbus message, ignoring)" );
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}    

int
main (int argc, char **argv)
{
  MokoNetwork *network;
  MokoDialer *dialer;
  MokoSms *sms;
  MokoPb *pb;
  DBusGConnection *connection;
  DBusGProxy *proxy;
  GError *error = NULL;
  guint32 ret;

  DBusError err = {0};
  DBusConnection* bus = dbus_bus_get (DBUS_BUS_SESSION, &err);

  if (!bus)
  {   
     gchar buffer[100];
     sprintf (buffer, "Failed to connect to the D-BUS daemon: %s", err.message);
     g_critical (buffer);
     dbus_error_free (&err);
     return 1;
  }   


  /* initialise type system */
  g_type_init ();

  /* Try and setup our DBus service */
  connection = dbus_g_bus_get (DBUS_BUS_SESSION, &error);
  if (connection == NULL)
  {
    g_warning ("Failed to make a connection to the session bus: %s", 
               error->message);
    g_error_free (error);
    return 1;
  }
  proxy = dbus_g_proxy_new_for_name (connection, 
                                     DBUS_SERVICE_DBUS,
                                     DBUS_PATH_DBUS, 
                                     DBUS_INTERFACE_DBUS);

  if (!org_freedesktop_DBus_request_name (proxy,
                                          PHONEKIT_NAMESPACE,
                                          0, &ret, &error))
  {
    /* Error requesting the name */
    g_warning ("There was an error requesting the name: %s\n",error->message);
    g_error_free (error);

    gdk_init (&argc, &argv);
    gdk_notify_startup_complete ();

    return 1;
  }
  if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER)
  {
    /* Someone else has registered the object */
    if (number)
      _dial_number (connection);

    dbus_g_connection_unref (connection);

    gdk_init (&argc, &argv);
    gdk_notify_startup_complete ();
    return 0;
  }


  /* Initialize Threading & GTK+ */
  gtk_init (&argc, &argv);
  
  /* Initialise libnotify */
  notify_init (basename (argv[0]));

  /* Create the PhoneKit objects */
  network = moko_network_get_default ();
  dialer = moko_dialer_get_default (network);
  sms = moko_sms_get_default (network);
  pb = moko_pb_get_default (network);

  /* Add the objects onto the bus */
  dbus_g_connection_register_g_object (connection, 
                                       NETWORK_PATH,
                                       G_OBJECT (network));
  dbus_g_connection_register_g_object (connection, 
                                       DIALER_PATH,
                                       G_OBJECT (dialer));
  dbus_g_connection_register_g_object (connection, 
                                       SMS_PATH,
                                       G_OBJECT (sms));

  dbus_connection_setup_with_g_main (bus, NULL);

  dbus_bus_add_match (bus, "type='signal',interface='org.openmoko.PhoneKit.Headset'", &err);
  dbus_connection_add_filter (bus, headset_signal_filter, NULL, NULL);  

  /* Sync phonebook */
  /* XXX this is not the right place! */
  pb_sync (pb, network);

  /* application object */
  g_set_application_name ("OpenMoko Dialer");
 
  if (number)
    moko_dialer_dial (dialer, number, NULL);

  gtk_main ();
  
  return 0;
}
