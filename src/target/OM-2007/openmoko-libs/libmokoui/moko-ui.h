/*
 *  libmokoui -- OpenMoko Application Framework UI Library
 *
 *  Authored by OpenedHand Ltd <info@openedhand.com>
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
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#ifndef _MOKO_UI_H_
#define _MOKO_UI_H_

#include "moko-alignment.h"
#include "moko-application.h"
#include "moko-banner.h"
#include "moko-details-window.h"
#include "moko-dialog.h"
#include "moko-dialog-window.h"
#include "moko-finger-tool-box.h"
#include "moko-finger-wheel.h"
#include "moko-finger-window.h"
#include "moko-fixed.h"
#include "moko-menu-box.h"
#include "moko-message-dialog.h"
#include "moko-navigation-list.h"
#include "moko-paned-window.h"
#include "moko-panel-applet.h"
#include "moko-pixmap-button.h"
#include "moko-scrolled-pane.h"
#include "moko-stock.h"
#include "moko-tool-box.h"
#include "moko-tree-view.h"
#include "moko-window.h"

void moko_ui_banner_show_text( gint timeout, const gchar* message, ... )
{
    va_list a;
    g_return_if_fail( timeout ); // don't allow permanent banners using the simple interface
    va_start( a, message );
    const gchar* string = g_strdup_vprintf( message, a );
    va_end( a );
    MokoBanner* banner = moko_banner_new();
    moko_banner_show_text( banner, string, timeout );
    g_object_unref( banner );
    g_free( string );
}

#endif  /* _MOKO_UI_H_ */
