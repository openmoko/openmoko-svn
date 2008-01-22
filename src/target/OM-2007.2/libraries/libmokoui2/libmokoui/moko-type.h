/*
 *  libmokoui -- OpenMoko Application Framework UI Library
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

#ifndef MOKO_TYPE_H
#define MOKO_TYPE_H

#include <glib-object.h>

G_BEGIN_DECLS

GType moko_finger_scroll_mode_get_type (void) G_GNUC_CONST;
#define MOKO_TYPE_FINGER_SCROLL_MODE (moko_finger_scroll_mode_get_type())

GType moko_finger_scroll_indicator_mode_get_type (void)  G_GNUC_CONST;
#define MOKO_TYPE_FINGER_SCROLL_INDICATOR_MODE \
	(moko_finger_scroll_indicator_mode_get_type())

G_END_DECLS

#endif

