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
  { MOKO_STOCK_CALL_HANGUP, N_("Hang Up"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CALL_HOLD, N_("Hold"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CALL_REDIAL, N_("Redial"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CONTACT_ADDRESS, N_("Address"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CONTACT_DELETE, N_("Delete Contact"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CONTACT_EMAIL, N_("E-Mail"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CONTACT_GROUPS, N_("Groups"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CONTACT_NEW, N_("New Contact"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_CONTACT_PHONE, N_("Phone"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_FOLDER_DELETE, N_("Delete Folder"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_HISTORY, N_("History"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_MAIL_FORWARD, N_("Forward"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_MAIL_MARK_READ, N_("Mark as Read"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_MAIL_NEW, N_("New Mail"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_MAIL_READ, N_("Read Mail"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_MAIL_REPLY_ALL, N_("Reply to All"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_MAIL_REPLY_SENDER, N_("Reply to Sender"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_MAIL_SEND, N_("Send"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_PHONE_BOOK, N_("Phone Book"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_SMS_NEW, N_("New SMS"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_SPEAKER, N_("Speaker"), 0, 0, GETTEXT_PACKAGE },
  { MOKO_STOCK_VIEW, N_("View"), 0, 0, GETTEXT_PACKAGE },
};


static gboolean registered = FALSE;

void
moko_stock_register ()
{
  /* make sure we never register the icons twice */
  if (registered)
    return;

  gtk_stock_add_static (moko_items, G_N_ELEMENTS (moko_items));


  registered = TRUE;
}
