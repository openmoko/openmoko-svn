/*
 * Copyright (C) 2007 OpenedHand Ltd
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _KOTO_FIELD_EDITOR_FACTORY
#define _KOTO_FIELD_EDITOR_FACTORY

#include <libical/icalproperty.h>
#include <gtk/gtkwidget.h>
#include "koto-task.h"

G_BEGIN_DECLS

GtkWidget *koto_field_editor_create (icalproperty_kind kind, gboolean *dirty);

void koto_field_editor_set (GtkWidget *widget, KotoTask *task);

G_END_DECLS

#endif /* _KOTO_FIELD_EDITOR_FACTORY */
