/*
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
 */
#ifndef BUTTONACTIONS_H
#define BUTTONACTIONS_H

#include <glib.h>
#include <gdk/gdk.h>

gboolean neod_buttonactions_input_prepare( GSource* source, gint* timeout );
gboolean neod_buttonactions_input_check( GSource* source );
gboolean neod_buttonactions_input_dispatch( GSource* source, GSourceFunc callback, gpointer data );

gboolean neod_buttonactions_touchscreen_cb( GIOChannel *source, GIOCondition condition, gpointer data );

gboolean neod_buttonactions_aux_timeout( guint timeout );
gboolean neod_buttonactions_power_timeout( guint timeout );

void neod_buttonactions_powersave_reset();

gboolean neod_buttonactions_powersave_timeout1( guint timeout );
gboolean neod_buttonactions_powersave_timeout2( guint timeout );
gboolean neod_buttonactions_powersave_timeout3( guint timeout );

void neod_buttonactions_sound_init();
void neod_buttonactions_set_display( int brightness );
void neod_buttonactions_sound_play( const gchar* samplename );

#endif
