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

void panel_mainmenu_set_display( int brightness );
void panel_mainmenu_play_stylus_click();

#endif
