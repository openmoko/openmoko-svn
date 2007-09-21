/*
 *  libmokoui -- OpenMoko Application Framework UI Library
 *
 *  Authored by OpenedHand Ltd <info@openedhand.com>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */


#include <gtk/gtk.h>
#include <moko-stock.h>
#include <glib/gi18n.h>
#include <config.h>


static const GtkStockItem moko_items [] =
{
  { MOKO_STOCK_CALL_ANSWER, N_("Answer"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CALL_DIAL, N_("Dial"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CALL_DIALED, N_("Dialed Calls"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CALL_HANGUP, N_("Hang Up"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CALL_HISTORY, N_("History"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CALL_HOLD, N_("Hold"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CALL_IN, N_("Received Calls"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CALL_MISSED, N_("Missed Calls"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CALL_REDIAL, N_("Redial"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CALL_REJECT, N_("Reject"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CONTACT_ADDRESS, N_("Address"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CONTACT_DELETE, N_("Delete Contact"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CONTACT_EMAIL, N_("E-Mail"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CONTACT_GROUPS, N_("Groups"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CONTACT_NEW, N_("New Contact"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CONTACT_PHONE, N_("Phone"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_FOLDER_DELETE, N_("Delete Folder"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_FOLDER_NEW, N_("New Folder"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_HISTORY, N_("History"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_MAIL_DELETE, N_("Delete"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_MAIL_FORWARD, N_("Forward"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_MAIL_MARK_READ, N_("Mark as Read"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_MAIL_NEW, N_("New Mail"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_MAIL_READ, N_("Read Mail"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_MAIL_REPLY_ALL, N_("Reply to All"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_MAIL_REPLY_SENDER, N_("Reply to Sender"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_MAIL_SEND, N_("Send"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_MAIL_SEND_RECEIVE, N_("Send/Receive"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_PHONE_BOOK, N_("Phone Book"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_SEARCH, N_("Search"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_SMS_NEW, N_("New SMS"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_SPEAKER, N_("Speaker"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_HANDSET, N_("Handset"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_VIEW, N_("View"), 0, 0, GETTEXT_PACKAGE },
};


static gboolean        registered        = FALSE;

static void
_moko_stock_add_icon (GtkIconFactory *factory, const GtkStockItem *item)
{
  static GtkIconTheme *theme = NULL;
  GtkIconSource       *source;
  GtkIconSet          *set = NULL;
  GdkPixbuf           *pixbuf = NULL;
  int i;

  if (theme == NULL)
    theme = gtk_icon_theme_get_default ();

  set = gtk_icon_set_new ();

  for (i = GTK_ICON_SIZE_MENU; i <= GTK_ICON_SIZE_DIALOG; i++)
  {
    gint width, height;

    gtk_icon_size_lookup (i, &width, &height);
    pixbuf = gtk_icon_theme_load_icon (theme, item->stock_id, width, 0, NULL);

    source = gtk_icon_source_new ();
    gtk_icon_source_set_size (source, i);
    gtk_icon_source_set_size_wildcarded (source, FALSE);
    gtk_icon_source_set_pixbuf (source, pixbuf);
    gtk_icon_set_add_source (set, source);
    gtk_icon_source_free (source);
  }




  gtk_icon_factory_add (factory, item->stock_id, set);
  gtk_icon_set_unref (set);



  g_object_unref (G_OBJECT (pixbuf));
}

void
moko_stock_register ()
{
  gint i = 0;
  static GtkIconFactory *moko_icon_factory = NULL;

  /* make sure we never register the icons twice */
  if (registered)
    return;
  
  moko_icon_factory = gtk_icon_factory_new ();
  
  for (i = 0; i < G_N_ELEMENTS (moko_items); i++)
  {
    _moko_stock_add_icon (moko_icon_factory, &moko_items[i]);
  }
  
  gtk_icon_factory_add_default (moko_icon_factory);
  
  gtk_stock_add_static (moko_items, G_N_ELEMENTS (moko_items));

  registered = TRUE;
}
