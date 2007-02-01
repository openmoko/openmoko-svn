/* moko-dialer-panel.h
 *
 *  Authored by Tony Guan<tonyguan@fic-sh.com.cn>
 *
 *  Copyright (C) 2006 FIC Shanghai Lab
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date) [$Author: Tony Guan $]
 */
#ifndef _MOKO_DIALER_PANEL_H_
#define _MOKO_DIALER_PANEL_H_




#include <gdk/gdk.h>
#include <gtk/gtkvbox.h>
#include "moko-digit-button.h"
#include <glib-object.h>
#include <gtk/gtktable.h>
#include <gtk/gtkobject.h>
#include <gtk/gtksignal.h>

G_BEGIN_DECLS
#define MOKO_TYPE_DIALER_PANEL                (moko_dialer_panel_get_type())
#define MOKO_DIALER_PANEL (obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_DIALER_PANEL, MokoDialerPanel))
#define MOKO_DIALER_PANEL_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass),MOKO_TYPE_DIALER_PANEL,MokoDialerPanelClass))
#define MOKO_IS_DIALER_PANEL(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_DIALER_PANEL))
#define MOKO_IS_DIALER_PANEL_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_DIALER_PANEL))
#define MOKO_DIALER_PANEL_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), MOKO_TYPE_DIALER_PANEL, MokoDialerPanelClass))
typedef struct _MokoDialerPanelClass MokoDialerPanelClass;

typedef struct _MokoDialerPanel MokoDialerPanel;
struct _MokoDialerPanel
{
  GtkVBox vbox;
  GtkWidget *buttons[4][3];
};

struct _MokoDialerPanelClass
{
  GtkVBoxClass parent_class;
  void (*moko_dialer_panel_input) (MokoDialerPanel * moko_dialer_panel,
                                   gchar parac);
  void (*moko_dialer_panel_hold) (MokoDialerPanel * moko_dialer_panel,
                                  gchar parac);
};


void moko_dialer_panel_clear (MokoDialerPanel * moko_dialer_panel);

GType moko_dialer_panel_get_type (void);

GtkWidget *moko_dialer_panel_new ();





G_END_DECLS
#endif //
