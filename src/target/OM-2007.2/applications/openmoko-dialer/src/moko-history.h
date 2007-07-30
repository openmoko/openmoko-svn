/*
 *  moko-history; a GObject wrapper for the history which exports method and
 *  signals over dbus
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

#ifndef _HAVE_MOKO_HISTORY_H
#define _HAVE_MOKO_HISTORY_H

#include <gtk/gtk.h>
#include <moko-journal.h>

G_BEGIN_DECLS

#define MOKO_TYPE_HISTORY (moko_history_get_type ())

#define MOKO_HISTORY(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
        MOKO_TYPE_HISTORY, MokoHistory))

#define MOKO_HISTORY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        MOKO_TYPE_HISTORY, MokoHistoryClass))

#define MOKO_IS_HISTORY(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
        MOKO_TYPE_HISTORY))

#define MOKO_IS_HISTORY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
        MOKO_TYPE_HISTORY))

#define MOKO_HISTORY_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
        MOKO_TYPE_HISTORY, MokoHistoryClass))

typedef struct _MokoHistory MokoHistory;
typedef struct _MokoHistoryClass MokoHistoryClass;
typedef struct _MokoHistoryPrivate MokoHistoryPrivate;

struct _MokoHistory
{
  GtkVBox         parent;

  /*< private >*/
  MokoHistoryPrivate   *priv;
};

struct _MokoHistoryClass 
{
  /*< private >*/
  GtkVBoxClass    parent_class;

  /* signals */
  void (*dial_number) (MokoHistory *history, const gchar *number);

};

typedef enum {
  HISTORY_FILTER_ALL = 0,
  HISTORY_FILTER_MISSED,
  HISTORY_FILTER_DIALED,
  HISTORY_FILTER_RECEIVED

} MokoHistoryFilter;

GType moko_history_get_type (void) G_GNUC_CONST;

GtkWidget*        
moko_history_new (MokoJournal *journal);

void
moko_history_set_filter (MokoHistory *history,  gint filter);

G_END_DECLS

#endif /* _HAVE_MOKO_HISTORY_H */
