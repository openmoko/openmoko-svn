#ifndef BUTTONACTIONS_H
#define BUTTONACTIONS_H

#include <glib.h>

gboolean panel_mainmenu_input_prepare( GSource* source, gint* timeout );
gboolean panel_mainmenu_input_check( GSource* source );
gboolean panel_mainmenu_input_dispatch( GSource* source, GSourceFunc callback, gpointer data );

gboolean panel_mainmenu_aux_timeout( guint timeout );
gboolean panel_mainmenu_power_timeout( guint timeout );

#endif
