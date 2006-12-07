/**
 *  @file select-menu.c
 *  @brief The select menu that popuo in the treeview
 *
 *  Copyright (C) 2006 First International Computer Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 *
 *  @author Chaowei Song (songcw@fic-sh.com.cn)
 */

#include "select-menu.h"
#include "appmanager-window.h"

static void moko_select_menu_class_init (MokoSelectMenuClass *klass);
static void moko_select_menu_init (MokoSelectMenu *data);

G_DEFINE_TYPE (MokoSelectMenu, moko_select_menu, GTK_TYPE_MENU)

#define MOKO_SELECT_MENU_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MOKO_TYPE_SELECT_MENU, MokoSelectMenuPriv))

typedef struct _MokoSelectMenuPriv {
  GtkWidget   *unmark;
  GtkWidget   *markinstall;
  GtkWidget   *markupgrade;
  GtkWidget   *markremove;
} MokoSelectMenuPriv;

static void 
moko_select_menu_class_init (MokoSelectMenuClass *klass)
{
  g_type_class_add_private (klass, sizeof (MokoSelectMenuPriv));
}

static void 
moko_select_menu_init (MokoSelectMenu *data)
{
  g_debug ("New a moko select menu");
}

/**
 * @brief Create a new select menu
 * @param appdata The application manager data
 * @return The MokoSelectMenu
 */
MokoSelectMenu *
moko_select_menu_new (ApplicationManagerData *appdata)
{
  MokoSelectMenu *self = MOKO_SELECT_MENU (g_object_new (MOKO_TYPE_SELECT_MENU, NULL));
  MokoSelectMenuPriv *priv = MOKO_SELECT_MENU_GET_PRIVATE (self);

  priv->unmark = gtk_menu_item_new_with_mnemonic (_("unmark"));
  gtk_widget_show (priv->unmark);
  gtk_container_add (GTK_CONTAINER (self), priv->unmark);


  priv->markinstall = gtk_menu_item_new_with_mnemonic (_("mark install"));
  gtk_widget_show (priv->markinstall);
  gtk_container_add (GTK_CONTAINER (self), priv->markinstall);

  priv->markupgrade = gtk_menu_item_new_with_mnemonic (_("mark upgrade"));
  gtk_widget_show (priv->markupgrade);
  gtk_container_add (GTK_CONTAINER (self), priv->markupgrade);

  priv->markremove = gtk_menu_item_new_with_mnemonic (_("mark remove"));
  gtk_widget_show (priv->markremove);
  gtk_container_add (GTK_CONTAINER (self), priv->markremove);

  return self;
}

