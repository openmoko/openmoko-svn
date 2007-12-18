/*
 *  openmoko-messages -- OpenMoko SMS Application
 *
 *  Authored by Chris Lord <chris@openedhand.com>
 *
 *  Copyright (C) 2007 OpenMoko Inc.
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

#ifndef _SMS_CONTACTS_H
#define _SMS_CONTACTS_H

#include "sms.h"

enum {
	COL_UID,
	COL_NAME,
	COL_DETAIL,
	COL_ICON,
	COL_PRIORITY,
	COL_UNKNOWN,
	COL_LAST
};

GtkWidget *sms_contacts_page_new (SmsData *data);
void sms_contacts_update_delete_all (SmsData *data);

#endif

