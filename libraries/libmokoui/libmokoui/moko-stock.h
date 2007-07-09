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

/*
 * OpenMoko Stock Items
 *
 * Some of these correspond to icon names from the Freedesktop.org Icon Naming
 * Specification:
 * http://standards.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html
 *
 * Names that start with "moko-" are currently specific to libmokoui
 *
 * The stock items are named by context, then action. Please keep the list in
 * alphabetical order.
 *
 */


#ifndef _MOKO_STOCK_H_
#define _MOKO_STOCK_H_

#define MOKO_STOCK_CALL_ANSWER "moko-call-answer"
#define MOKO_STOCK_CALL_DIAL "moko-call-dial"
#define MOKO_STOCK_CALL_DIALED "moko-history-call-out"
#define MOKO_STOCK_CALL_HANGUP "moko-call-hangup"
#define MOKO_STOCK_CALL_HISTORY "moko-call-history"
#define MOKO_STOCK_CALL_HOLD "moko-call-hold"
#define MOKO_STOCK_CALL_IN "moko-history-call-in"
#define MOKO_STOCK_CALL_MISSED "moko-history-call-missed"
#define MOKO_STOCK_CALL_REDIAL "moko-call-redial"
#define MOKO_STOCK_CALL_REJECT "moko-call-reject"
#define MOKO_STOCK_CONTACT_ADDRESS "moko-contact-address"
#define MOKO_STOCK_CONTACT_DELETE "moko-contact-delete"
#define MOKO_STOCK_CONTACT_EMAIL "moko-contact-email"
#define MOKO_STOCK_CONTACT_GROUPS "moko-contact-groups"
#define MOKO_STOCK_CONTACT_MODE "contact-mode"
#define MOKO_STOCK_CONTACT_NEW "contact-new"
#define MOKO_STOCK_CONTACT_PHONE "moko-contact-phone"
#define MOKO_STOCK_FOLDER_DELETE "moko-folder-delete"
#define MOKO_STOCK_FOLDER_NEW "moko-folder-new"
#define MOKO_STOCK_HISTORY "moko-history"
#define MOKO_STOCK_MAIL_DELETE "mail-delete"
#define MOKO_STOCK_MAIL_FORWARD "mail-forward"
#define MOKO_STOCK_MAIL_MARK_READ "mail-mark-read"
#define MOKO_STOCK_MAIL_NEW "mail-message-new"
#define MOKO_STOCK_MAIL_READ "mail-read"
#define MOKO_STOCK_MAIL_REPLY_ALL "mail-reply-all"
#define MOKO_STOCK_MAIL_REPLY_SENDER "mail-reply-sender"
#define MOKO_STOCK_MAIL_SEND "mail-send"
#define MOKO_STOCK_MAIL_SEND_RECEIVE "mail-send-receive"
#define MOKO_STOCK_PHONE_BOOK "moko-phone-book"
#define MOKO_STOCK_SEARCH "system-search"
#define MOKO_STOCK_SMS_NEW "moko-sms-new"
#define MOKO_STOCK_SPEAKER "moko-speaker"
#define MOKO_STOCK_HANDSET "moko-handset"
#define MOKO_STOCK_VIEW "moko-view"


void moko_stock_register ();


#endif /* _MOKO_STOCK_H_ */
