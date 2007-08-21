/*
 *  moko-tips; The autocomplete tips.
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

#ifndef _HAVE_MOKO_TIPS_H
#define _HAVE_MOKO_TIPS_H

#include <glib.h>
#include <gtk/gtk.h>

#include "moko-contacts.h"

G_BEGIN_DECLS

#define MOKO_TYPE_TIPS (moko_tips_get_type ())

#define MOKO_TIPS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
        MOKO_TYPE_TIPS, MokoTips))

#define MOKO_TIPS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
        MOKO_TYPE_TIPS, MokoTipsClass))

#define MOKO_IS_TIPS(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
        MOKO_TYPE_TIPS))

#define MOKO_IS_TIPS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
        MOKO_TYPE_TIPS))

#define MOKO_TIPS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
        MOKO_TYPE_TIPS, MokoTipsClass))

typedef struct _MokoTips MokoTips;
typedef struct _MokoTipsClass MokoTipsClass;
typedef struct _MokoTipsPrivate MokoTipsPrivate;

struct _MokoTips
{
  GtkHBox         parent;

  /*< private >*/
  MokoTipsPrivate   *priv;
};

struct _MokoTipsClass 
{
  /*< private >*/
  GtkHBoxClass    parent_class;
  
  /* signals */
  void (*selected) (MokoTips *tips, MokoContactEntry *entry);

  /* future padding */
  void (*_moko_tips_1) (void);
  void (*_moko_tips_2) (void);
  void (*_moko_tips_3) (void);
  void (*_moko_tips_4) (void);
}; 

GType moko_tips_get_type (void) G_GNUC_CONST;

GtkWidget*        
moko_tips_new (void);

void
moko_tips_set_matches (MokoTips *tips, GList *list);

G_END_DECLS

#endif /* _HAVE_MOKO_TIPS_H */
