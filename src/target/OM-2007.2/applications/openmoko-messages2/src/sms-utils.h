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

#ifndef SMS_UTILS_H
#define SMS_UTILS_H

#include "sms.h"

GList *hito_vcard_get_named_attributes (EVCard *contact, const char *name);
gchar *hito_vcard_attribute_get_value_string (EVCardAttribute *attr);

void sms_clear_combo_box_text (GtkComboBox *combo);
EContact *sms_get_selected_contact (SmsData *data);
GdkPixbuf *sms_contact_load_photo (EContact *contact);
gboolean sms_contacts_note_count_update (SmsData *data);
gboolean sms_delete_selected_contact_messages (SmsData *data);
gboolean sms_select_contact (SmsData *data, const gchar *uid);
gboolean sms_contact_picker_dialog (SmsData *data, const gchar *message);

#endif /* SMS_UTILS_H */
