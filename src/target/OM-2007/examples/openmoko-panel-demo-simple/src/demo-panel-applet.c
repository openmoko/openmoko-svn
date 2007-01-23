/*
 *  Panel-Demo -- OpenMoko Demo Panel Applet
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#include <libmokoui/moko-panel-applet.h>

#include <gtk/gtk.h>
#include <glib.h>

static void button_callback( GtkButton* button, MokoPanelApplet* applet )
{
    moko_panel_applet_close_popup( applet );
}

G_MODULE_EXPORT GtkWidget* mb_panel_applet_create(const char* id, GtkOrientation orientation)
{
    g_debug( "openmoko-panel-demo-simple new" );

    // simple demo just uses the stock MokoPanelApplet class
    // more sophisticated way of using it is to derive from it
    MokoPanelApplet* applet = moko_panel_applet_new();

    moko_panel_applet_set_icon( applet, PKGDATADIR "/icon.png", TRUE );

    // you can add a menu
    GtkMenu* panelmenu = GTK_MENU(gtk_menu_new());
    GtkMenuItem* fooitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Foo" ));
    GtkMenuItem* baritem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Bar" ));
    gtk_widget_show( GTK_WIDGET(fooitem) );
    gtk_widget_show( GTK_WIDGET(baritem) );
    gtk_menu_shell_append( GTK_MENU_SHELL(panelmenu), GTK_WIDGET(fooitem) );
    gtk_menu_shell_append( GTK_MENU_SHELL(panelmenu), GTK_WIDGET(baritem) );
    gtk_widget_show_all( GTK_WIDGET(panelmenu) );

    moko_panel_applet_set_popup( applet, GTK_WIDGET(panelmenu), MOKO_PANEL_APPLET_TAP_HOLD_POPUP );

    // or something else
    GtkButton* button = GTK_BUTTON(gtk_button_new_with_label( "Hello Applet World!" ));
    g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(button_callback), applet );
    moko_panel_applet_set_popup( applet, GTK_WIDGET(button), MOKO_PANEL_APPLET_CLICK_POPUP );

    gtk_widget_show_all( GTK_WIDGET(applet) );
    return GTK_WIDGET(applet);
}

