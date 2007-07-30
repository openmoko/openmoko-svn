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

#ifndef _KOTO_HINT_ENTRY
#define _KOTO_HINT_ENTRY

#include <gtk/gtkentry.h>

G_BEGIN_DECLS

#define KOTO_TYPE_HINT_ENTRY koto_hint_entry_get_type()

#define KOTO_HINT_ENTRY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  KOTO_TYPE_HINT_ENTRY, KotoHintEntry))

#define KOTO_HINT_ENTRY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  KOTO_TYPE_HINT_ENTRY, KotoHintEntryClass))

#define KOTO_IS_HINT_ENTRY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  KOTO_TYPE_HINT_ENTRY))

#define KOTO_IS_HINT_ENTRY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  KOTO_TYPE_HINT_ENTRY))

#define KOTO_HINT_ENTRY_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  KOTO_TYPE_HINT_ENTRY, KotoGroupComboClass))

typedef struct {
  GtkEntry parent;
} KotoHintEntry;

typedef struct {
  GtkEntryClass parent_class;
} KotoHintEntryClass;

GType koto_hint_entry_get_type (void);

GtkWidget* koto_hint_entry_new (const char *hint);

void koto_hint_entry_clear (KotoHintEntry *entry);

gboolean koto_hint_entry_is_empty (KotoHintEntry *entry);

G_END_DECLS

#endif /* _KOTO_HINT_ENTRY */
