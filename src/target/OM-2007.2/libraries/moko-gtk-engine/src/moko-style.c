/*
 * moko-style.c
 * This file is part of moko-engine
 *
 * Copyright (C) 2006,2007 - OpenedHand Ltd
 *
 * Originally from OpenedHand's Sato GTK+ Engine
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#include <gtk/gtk.h>
#include <stdio.h>

#include "moko-style.h"

/*** Gtk Style Class **********************************************************/

GType moko_type_style = 0;

static void moko_style_class_init (MokoStyleClass *klass);

void
moko_style_register_type (GTypeModule *module)
{
  static const GTypeInfo object_info =
  {
    sizeof (MokoStyleClass),
    (GBaseInitFunc) NULL,
    (GBaseFinalizeFunc) NULL,
    (GClassInitFunc) moko_style_class_init,
    NULL,           /* class_finalize */
    NULL,           /* class_data */
    sizeof (MokoStyle),
    0,              /* n_preallocs */
    (GInstanceInitFunc) NULL,
  };

  moko_type_style = g_type_module_register_type (module,
						 GTK_TYPE_STYLE,
						 "MokoStyle",
						 &object_info, 0);
}

static void
moko_style_class_init (MokoStyleClass *klass)
{
  GtkStyleClass *style_class = GTK_STYLE_CLASS (klass);
  moko_draw_style_class_init (style_class);
}

/******************************************************************************/

/*** Gtk Style RC Class *******************************************************/

GType moko_type_rc_style = 0;

static GtkStyle *moko_rc_style_create_style (GtkRcStyle *rc_style);

static void
moko_rc_style_class_init (MokoRcStyleClass *klass)
{
	GtkRcStyleClass *rc_style_class = GTK_RC_STYLE_CLASS (klass);
	rc_style_class->create_style = moko_rc_style_create_style;
}

void
moko_rc_style_register_type (GTypeModule *module)
{
  static const GTypeInfo object_info =
  {
    sizeof (MokoRcStyleClass),
    (GBaseInitFunc) NULL,
    (GBaseFinalizeFunc) NULL,
    (GClassInitFunc) moko_rc_style_class_init,
    NULL,           /* class_finalize */
    NULL,           /* class_data */
    sizeof (MokoRcStyle),
    0,              /* n_preallocs */
    (GInstanceInitFunc) NULL,
  };

  moko_type_rc_style = g_type_module_register_type (module,
                                                      GTK_TYPE_RC_STYLE,
                                                      "MokoRcStyle",
                                                      &object_info, 0);
}

static GtkStyle *
moko_rc_style_create_style (GtkRcStyle *rc_style)
{
  return GTK_STYLE (g_object_new (MOKO_TYPE_STYLE, NULL));
}

/******************************************************************************/
