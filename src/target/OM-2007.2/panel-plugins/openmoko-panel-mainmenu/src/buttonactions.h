/*
 *  openmoko-panel-mainmenu: handle action buttons
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
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
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */
#ifndef BUTTONACTIONS_H
#define BUTTONACTIONS_H

#include <glib.h>
#include <gdk/gdk.h>

gboolean panel_mainmenu_input_prepare( GSource* source, gint* timeout );
gboolean panel_mainmenu_input_check( GSource* source );
gboolean panel_mainmenu_input_dispatch( GSource* source, GSourceFunc callback, gpointer data );

gboolean panel_mainmenu_touchscreen_cb( GIOChannel *source, GIOCondition condition, gpointer data );

gboolean panel_mainmenu_aux_timeout( guint timeout );
gboolean panel_mainmenu_power_timeout( guint timeout );

void panel_mainmenu_powersave_reset();

gboolean panel_mainmenu_powersave_timeout1( guint timeout );
gboolean panel_mainmenu_powersave_timeout2( guint timeout );
gboolean panel_mainmenu_powersave_timeout3( guint timeout );

void panel_mainmenu_sound_init();
void panel_mainmenu_set_display( int brightness );
void panel_mainmenu_sound_play( const gchar* samplename );

#endif
