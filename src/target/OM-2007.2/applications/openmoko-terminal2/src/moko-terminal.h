/*  moko-terminal.h
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2007 OpenMoko, Inc.
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
 */

#ifndef _MOKO_TERMINAL_H_
#define _MOKO_TERMINAL_H_

#include <gtk/gtkhbox.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define MOKO_TYPE_TERMINAL moko_terminal_get_type()
#define MOKO_TERMINAL(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_TERMINAL, MokoTerminal))
#define MOKO_TERMINAL_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_TERMINAL, MokoTerminalClass))
#define MOKO_IS_TERMINAL(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_TERMINAL))
#define MOKO_IS_TERMINAL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_TERMINAL))
#define MOKO_TERMINAL_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), MOKO_TYPE_TERMINAL, MokoTerminalClass))

typedef struct {
    GtkHBox parent;
} MokoTerminal;

typedef struct {
    GtkHBoxClass parent_class;
} MokoTerminalClass;

GType moko_terminal_get_type();
MokoTerminal* moko_terminal_new();

G_END_DECLS

#endif // _MOKO_TERMINAL_H_

