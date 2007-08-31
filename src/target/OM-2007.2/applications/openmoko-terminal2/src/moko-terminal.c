/*  moko-terminal.c
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

#include "moko-terminal.h"

#include <vte/vte.h>

#undef DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##_VA_ARGS_)
#else
#define moko_debug(fmt,...)
#endif

G_DEFINE_TYPE (MokoTerminal, moko_terminal, GTK_TYPE_HBOX)

#define TERMINAL_GET_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_TERMINAL, MokoTerminalPrivate))

typedef struct _MokoTerminalPrivate
{
    GtkScrollbar* scrollbar;
    VteTerminal* terminal;
} MokoTerminalPrivate;

/* parent class pointer */
static GtkHBoxClass* parent_class = NULL;

/* signals */
/* ... */

/* forward declarations */
/* ... */

static void
moko_terminal_dispose(GObject* object)
{
    if (G_OBJECT_CLASS (moko_terminal_parent_class)->dispose)
        G_OBJECT_CLASS (moko_terminal_parent_class)->dispose (object);
}

static void
moko_terminal_finalize(GObject* object)
{
    G_OBJECT_CLASS (moko_terminal_parent_class)->finalize (object);
}

static void
moko_terminal_class_init(MokoTerminalClass* klass)
{
    /* hook parent */
    GObjectClass* object_class = G_OBJECT_CLASS (klass);
    parent_class = g_type_class_peek_parent(klass);

    /* add private */
    g_type_class_add_private (klass, sizeof(MokoTerminalPrivate));

    /* hook destruction */
    object_class->dispose = moko_terminal_dispose;
    object_class->finalize = moko_terminal_finalize;

    /* register signals */

    /* virtual methods */

    /* install properties */
}

static void on_window_title_changed(VteTerminal* terminal, GtkWidget* self)
{
    char* title = g_strdup_printf("%s", terminal->window_title);
    gtk_window_set_title( gtk_widget_get_toplevel( self ), title);
    g_free (title);
}

static void on_eof(VteTerminal* terminal, gpointer user_data)
{
    gtk_main_quit();
}

MokoTerminal*
moko_terminal_new(void)
{
    return g_object_new(MOKO_TYPE_TERMINAL, NULL);
}

static void
moko_terminal_init(MokoTerminal* self)
{
    MokoTerminalPrivate* priv = TERMINAL_GET_PRIVATE(self);

    VteTerminal* vte = priv->terminal = g_object_connect( vte_terminal_new(),
                                 /* Child dying */
                                 "signal::child-exited", on_eof, NULL,
                                 "signal::eof", on_eof, NULL,
                                 /* Child is trying to control the terminal */
                                 "signal::window-title-changed", on_window_title_changed, self, NULL);

    gtk_box_pack_start( GTK_BOX(self), vte, TRUE, TRUE, 0 );
    priv->scrollbar = gtk_vscrollbar_new( vte->adjustment );
    gtk_box_pack_start( GTK_BOX(self), priv->scrollbar, FALSE, FALSE, 0 );

    GdkColor fore = { 0, 0x0, 0x0, 0x0 };
    GdkColor back = { 0, 0xfffff, 0xfffff, 0xfffff };
    GdkColor colors[16];
    vte_terminal_set_colors( vte, &fore, &back, &colors, 16 );

    //vte_terminal_set_size( vte, 30, 30);
    vte_terminal_set_scrollback_lines( vte, 1000 );
    vte_terminal_set_font_from_string_full( vte, "Vera Sans Mono 5", 1 );
    vte_terminal_set_mouse_autohide( vte, TRUE );
    vte_terminal_set_cursor_blinks( vte, TRUE );

    vte_terminal_set_backspace_binding( vte, VTE_ERASE_ASCII_DELETE);

    vte_terminal_fork_command( vte,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               TRUE,
                               TRUE,
                               TRUE);
}
