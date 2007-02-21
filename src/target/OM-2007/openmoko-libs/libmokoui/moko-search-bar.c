
/*  moko_search_bar.c
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
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
 *  Current Version: $Rev$ ($Date) [$Author: mickey $]
 */

#include "moko-search-bar.h"
#include <gtk/gtkalignment.h>
#include <gtk/gtkentry.h>

G_DEFINE_TYPE (MokoSearchBar, moko_search_bar, GTK_TYPE_TOOLBAR)

#define MOKO_SEARCH_BAR_GET_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_SEARCH_BAR, MokoSearchBarPrivate))

typedef struct _MokoSearchBarPrivate MokoSearchBarPrivate;

struct _MokoSearchBarPrivate
{
    GtkWidget* alignment; /* GtkAlignment */
    GtkWindow* entry;     /* GtkEntry     */
};

static void
moko_search_bar_class_init (MokoSearchBarClass *klass);
static void
moko_search_bar_init (MokoSearchBar *self);

static void
moko_search_bar_class_init (MokoSearchBarClass *klass)
{
  /* GObjectClass *object_class = G_OBJECT_CLASS (klass); */

  g_type_class_add_private (klass, sizeof (MokoSearchBarPrivate));
}

static void
moko_search_bar_init (MokoSearchBar *self)
{
    GtkToolItem* item = gtk_tool_item_new();
    gtk_widget_set_size_request( GTK_WIDGET(item), 320, 10 ); //FIXME get from style
    GtkWidget* entry = gtk_entry_new();
    gtk_widget_set_name( entry, "moko_search_entry" );
    gtk_entry_set_has_frame( GTK_ENTRY (entry), FALSE );
    gtk_entry_set_text( GTK_ENTRY(entry), "foo" );
    gtk_container_add( GTK_CONTAINER(item), entry );
    gtk_toolbar_insert( GTK_TOOLBAR (self), GTK_TOOL_ITEM(item), 0 );
}

GtkWidget*
moko_search_bar_new (void)
{
    return GTK_WIDGET(g_object_new(moko_search_bar_get_type(), NULL));
}
