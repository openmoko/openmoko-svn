/*
 * moko-style.h
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


#ifndef MOKO_STYLE_H
#define MOKO_STYLE_H

#include <gtk/gtk.h>
#include <gmodule.h>

#include "moko-draw.h"

G_BEGIN_DECLS

/*** Gtk Style Class **********************************************************/

extern GType moko_type_style;

#define MOKO_TYPE_STYLE              moko_type_style
#define MOKO_STYLE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOKO_TYPE_STYLE, MokoStyle))
#define MOKO_STYLE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_STYLE, MokoStyleClass))
#define MOKO_IS_STYLE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOKO_TYPE_STYLE))
#define MOKO_IS_STYLE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_STYLE))
#define MOKO_STYLE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOKO_TYPE_STYLE, MokoStyleClass))

typedef struct _MokoStyle MokoStyle;
typedef struct _MokoStyleClass MokoStyleClass;

struct _MokoStyle
{
  GtkStyle parent_instance;
};

struct _MokoStyleClass
{
  GtkStyleClass parent_class;
};

void moko_style_register_type (GTypeModule *module);

/******************************************************************************/

/*** Gtk Style RC Class *******************************************************/

extern GType moko_type_rc_style;

#define MOKO_TYPE_RC_STYLE              moko_type_rc_style
#define MOKO_RC_STYLE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), MOKO_TYPE_RC_STYLE, MokoRcStyle))
#define MOKO_RC_STYLE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_RC_STYLE, MokoRcStyleClass))
#define MOKO_IS_RC_STYLE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), MOKO_TYPE_RC_STYLE))
#define MOKO_IS_RC_STYLE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_RC_STYLE))
#define MOKO_RC_STYLE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), MOKO_TYPE_RC_STYLE, MokoRcStyleClass))

typedef struct _MokoRcStyle MokoRcStyle;
typedef struct _MokoRcStyleClass MokoRcStyleClass;

/* which properties have been set? */
enum {
  BORDER_SET = 1,
  GRADIENT_SET = 2
};

struct _MokoRcStyle
{
  GtkRcStyle parent_instance;

  guint flags;
  gboolean has_border;
  gboolean has_gradient;
};

struct _MokoRcStyleClass
{
  GtkRcStyleClass parent_class;

};

void moko_rc_style_register_type (GTypeModule *engine);

enum
{
  TOKEN_HAS_BORDER = G_TOKEN_LAST + 1,
  TOKEN_HAS_GRADIENT,

  TOKEN_TRUE,
  TOKEN_FALSE
};

/******************************************************************************/

G_END_DECLS

#endif
