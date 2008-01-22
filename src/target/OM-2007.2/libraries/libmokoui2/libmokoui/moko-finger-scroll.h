/*
 *  libmokoui -- OpenMoko Application Framework UI Library
 *
 *  Authored by Chris Lord <chris@openedhand.com>
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

#ifndef _MOKO_FINGER_SCROLL
#define _MOKO_FINGER_SCROLL

#include <glib-object.h>
#include <gtk/gtk.h>
#include "moko-type.h"

G_BEGIN_DECLS

#define MOKO_TYPE_FINGER_SCROLL moko_finger_scroll_get_type()
#define MOKO_FINGER_SCROLL(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
	MOKO_TYPE_FINGER_SCROLL, MokoFingerScroll))
#define MOKO_FINGER_SCROLL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), \
	MOKO_TYPE_FINGER_SCROLL, MokoFingerScrollClass))
#define MOKO_IS_FINGER_SCROLL(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
	MOKO_TYPE_FINGER_SCROLL))
#define MOKO_IS_FINGER_SCROLL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), \
	MOKO_TYPE_FINGER_SCROLL))
#define MOKO_FINGER_SCROLL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), \
	MOKO_TYPE_FINGER_SCROLL, MokoFingerScrollClass))

/**
 * MokoFingerScroll:
 *
 * MokoFingerScroll has no publicly accessible fields
 */
typedef struct _MokoFingerScroll MokoFingerScroll;
typedef struct _MokoFingerScrollClass MokoFingerScrollClass;

struct _MokoFingerScroll {
  GtkEventBox parent;
};

struct _MokoFingerScrollClass {
  GtkEventBoxClass parent_class;
};

/**
 * MokoFingerScrollMode:
 * @MOKO_FINGER_SCROLL_MODE_PUSH: Scrolling follows pointer
 * @MOKO_FINGER_SCROLL_MODE_ACCEL: Scrolling uses physics to "spin" the widget
 * @MOKO_FINGER_SCROLL_MODE_AUTO: Automatically chooses between push and accel 
 * modes, depending on input.
 *
 * Used to change the behaviour of the finger scrolling
 */
typedef enum {
	MOKO_FINGER_SCROLL_MODE_PUSH,
	MOKO_FINGER_SCROLL_MODE_ACCEL,
	MOKO_FINGER_SCROLL_MODE_AUTO
} MokoFingerScrollMode;

typedef enum {
	MOKO_FINGER_SCROLL_INDICATOR_MODE_AUTO,
	MOKO_FINGER_SCROLL_INDICATOR_MODE_SHOW,
	MOKO_FINGER_SCROLL_INDICATOR_MODE_HIDE
} MokoFingerScrollIndicatorMode;	

GType moko_finger_scroll_get_type (void);

GtkWidget* moko_finger_scroll_new (void);
GtkWidget* moko_finger_scroll_new_full (gint mode, gboolean enabled,
					gdouble vel_min, gdouble vel_max,
					gdouble decel, guint sps);
void moko_finger_scroll_add_with_viewport (MokoFingerScroll *scroll,
					   GtkWidget *child);

G_END_DECLS

#endif /* _MOKO_FINGER_SCROLL */

