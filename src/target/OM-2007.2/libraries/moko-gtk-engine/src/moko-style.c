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

static struct
{
  gchar *name;
  guint token;
}
moko_rc_symbols[] = 
{
  { "border", TOKEN_HAS_BORDER },
  { "gradient", TOKEN_HAS_GRADIENT },

  { "TRUE", TOKEN_TRUE },
  { "FALSE", TOKEN_FALSE }
};

GType moko_type_rc_style = 0;
static GtkRcStyleClass *moko_parent_rc_style_class;

static GtkStyle *moko_rc_style_create_style (GtkRcStyle *rc_style);
static void moko_rc_style_merge (GtkRcStyle *dest, GtkRcStyle *src);
static guint moko_rc_style_parse (GtkRcStyle *rc_style, GtkSettings *settings, GScanner *scanner);

static void
moko_rc_style_class_init (MokoRcStyleClass *klass)
{
	GtkRcStyleClass *rc_style_class = GTK_RC_STYLE_CLASS (klass);

  moko_parent_rc_style_class = g_type_class_peek_parent (klass);

	rc_style_class->create_style = moko_rc_style_create_style;
  rc_style_class->parse = moko_rc_style_parse;
  rc_style_class->merge = moko_rc_style_merge;
}


static void
moko_rc_style_merge (GtkRcStyle *dest, GtkRcStyle *src)
{
  if (MOKO_IS_RC_STYLE (src))
  {
    MokoRcStyle *src_data = MOKO_RC_STYLE (src);
    MokoRcStyle *dest_data = MOKO_RC_STYLE (dest);

    dest_data->has_border = src_data->has_border;
    dest_data->has_gradient = src_data->has_gradient;
  }

  moko_parent_rc_style_class->merge (dest, src);
}

static guint
moko_rc_parse_boolean(GScanner *scanner, GTokenType wanted_token, guint *retval)
{
  guint token;
  
  token = g_scanner_get_next_token(scanner);
  if (token != wanted_token)
    return wanted_token;
  
  token = g_scanner_get_next_token(scanner);
  if (token != G_TOKEN_EQUAL_SIGN)
    return G_TOKEN_EQUAL_SIGN;
  
  token = g_scanner_get_next_token(scanner);
  if (token == TOKEN_TRUE)
    *retval = TRUE;
  else if (token == TOKEN_FALSE)
    *retval = FALSE;
  else
    return TOKEN_TRUE;

  return G_TOKEN_NONE;
}

static guint 
moko_rc_style_parse (GtkRcStyle *rc_style, GtkSettings *settings, GScanner *scanner)
{
  static GQuark scope_id = 0;
  MokoRcStyle *theme_data = MOKO_RC_STYLE (rc_style);
  guint old_scope;
  guint token;

  /* Set up a our own scope for this scanner */

  if (!scope_id)
    scope_id = g_quark_from_string ("moko_gtk_engine");

  old_scope = g_scanner_set_scope (scanner, scope_id);

  /* check we haven't already added the moko symbols to this scanner */

  if (!g_scanner_lookup_symbol (scanner, moko_rc_symbols[0].name))
  {
    gint i;
    for (i = 0; i < G_N_ELEMENTS (moko_rc_symbols); i++)
    {
      g_scanner_scope_add_symbol (scanner, scope_id, moko_rc_symbols[i].name,
          GINT_TO_POINTER (moko_rc_symbols[i].token));
    }
  }


  token = g_scanner_peek_next_token (scanner);

  while (token != G_TOKEN_RIGHT_CURLY)
  {
    guint i;

    switch (token)
    {
      case TOKEN_HAS_BORDER:
        token = moko_rc_parse_boolean (scanner, TOKEN_HAS_BORDER, &i);
        if (token != G_TOKEN_NONE)
          break;
        theme_data->has_border = i;
        break;
      case TOKEN_HAS_GRADIENT:
        token = moko_rc_parse_boolean (scanner, TOKEN_HAS_GRADIENT, &i);
        if (token != G_TOKEN_NONE)
          break;
        theme_data->has_gradient = i;
        break;

      default:
        g_scanner_get_next_token (scanner);
        token = G_TOKEN_RIGHT_CURLY;
        break;

    }

    if (token != G_TOKEN_NONE)
    {
      return token;
    }
    token = g_scanner_peek_next_token (scanner);
  }

  g_scanner_get_next_token (scanner);
  g_scanner_set_scope (scanner, old_scope);

  return G_TOKEN_NONE;
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
