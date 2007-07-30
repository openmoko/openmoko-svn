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

#ifndef _KOTO_DATE_COMBO
#define _KOTO_DATE_COMBO

#include <gtk/gtktogglebutton.h>

G_BEGIN_DECLS

#define KOTO_TYPE_DATE_COMBO koto_date_combo_get_type()

#define KOTO_DATE_COMBO(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  KOTO_TYPE_DATE_COMBO, KotoDateCombo))

#define KOTO_DATE_COMBO_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  KOTO_TYPE_DATE_COMBO, KotoDateComboClass))

#define KOTO_IS_DATE_COMBO(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  KOTO_TYPE_DATE_COMBO))

#define KOTO_IS_DATE_COMBO_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  KOTO_TYPE_DATE_COMBO))

#define KOTO_DATE_COMBO_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  KOTO_TYPE_DATE_COMBO, KotoDateComboClass))

typedef struct {
  GtkToggleButton parent;
} KotoDateCombo;

typedef struct {
  GtkToggleButtonClass parent_class;
} KotoDateComboClass;

GType koto_date_combo_get_type (void);

GtkWidget* koto_date_combo_new (void);

void koto_date_combo_set_date (KotoDateCombo *combo, GDate *date);

GDate * koto_date_combo_get_date (KotoDateCombo *combo);

G_END_DECLS

#endif /* _KOTO_DATE_COMBO */
