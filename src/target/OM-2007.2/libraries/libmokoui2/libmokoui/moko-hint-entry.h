/*
 *
 * moko-hint-history.c - taken from koto-hint-history.h
 *   <http://svn.o-hand.com/repos/tasks/trunk/libkoto/koto-hint-entry.h>
 *
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

#ifndef _MOKO_HINT_ENTRY
#define _MOKO_HINT_ENTRY

#include <gtk/gtkentry.h>

G_BEGIN_DECLS

#define MOKO_TYPE_HINT_ENTRY moko_hint_entry_get_type()

#define MOKO_HINT_ENTRY(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  MOKO_TYPE_HINT_ENTRY, MokoHintEntry))

#define MOKO_HINT_ENTRY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  MOKO_TYPE_HINT_ENTRY, MokoHintEntryClass))

#define MOKO_IS_HINT_ENTRY(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  MOKO_TYPE_HINT_ENTRY))

#define MOKO_IS_HINT_ENTRY_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  MOKO_TYPE_HINT_ENTRY))

#define MOKO_HINT_ENTRY_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  MOKO_TYPE_HINT_ENTRY, MokoGroupComboClass))

typedef struct {
  GtkEntry parent;
} MokoHintEntry;

typedef struct {
  GtkEntryClass parent_class;
} MokoHintEntryClass;

GType moko_hint_entry_get_type (void);

GtkWidget* moko_hint_entry_new (const char *hint);

void moko_hint_entry_clear (MokoHintEntry *entry);

gboolean moko_hint_entry_is_empty (MokoHintEntry *entry);

G_END_DECLS

#endif /* _MOKO_HINT_ENTRY */
