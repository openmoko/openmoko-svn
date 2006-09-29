/*
 *  3 Part Demo -- OpenMoko Demo Application
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006 First International Computer Inc.
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

#include <mokoui/moko-application.h>
#include <mokoui/moko-window.h>
#include <mokoui/moko-menubar.h>
#include <mokoui/moko-toolbar.h>

#include <gtk/gtkactiongroup.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkcheckmenuitem.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkstock.h>
#include <gtk/gtkuimanager.h>
#include <gtk/gtkvpaned.h>
#include <gtk/gtkvbox.h>

#include <stdlib.h>

/* Obligatory basic callback */
static void print_hello( GtkWidget *w,
                         gpointer   data )
{
    g_message ("Hello, World!\n");
}

/* For the check button */
static void print_toggle( gpointer   callback_data,
                          guint      callback_action,
                          GtkWidget *menu_item )
{
    g_message ("Check button state - %d\n",
               GTK_CHECK_MENU_ITEM (menu_item)->active);
}

/* For the radio buttons */
static void print_selected( gpointer   callback_data,
                            guint      callback_action,
                            GtkWidget *menu_item )
{
    if(GTK_CHECK_MENU_ITEM(menu_item)->active)
        g_message ("Radio button %d selected\n", callback_action);
}

/* Normal items */
static const GtkActionEntry entries[] = {
    { "FileMenu", NULL, "_File" },
    { "ViewMenu", NULL, "_View" },
    { "Open", GTK_STOCK_OPEN, "_Open", "<control>O", "Open a file", print_hello },
    { "Exit", GTK_STOCK_QUIT, "E_xit", "<control>Q", "Exit the program", print_hello },
    { "ZoomIn", GTK_STOCK_ZOOM_IN, "Zoom _In", "plus", "Zoom into the image", print_hello },
    { "ZoomOut", GTK_STOCK_ZOOM_OUT, "Zoom _Out", "minus", "Zoom away from the image", print_hello },
};

/* Toggle items */
static const GtkToggleActionEntry toggle_entries[] = {
    { "FullScreen", NULL, "_Full Screen", "F11", "Switch between full screen and windowed mode", print_hello, FALSE }
};

/* Radio items */
static const GtkRadioActionEntry radio_entries[] = {
    { "HighQuality", "my-stock-high-quality", "_High Quality", NULL, "Display images in high quality, slow mode", 0 },
    { "NormalQuality", "my-stock-normal-quality", "_Normal Quality", NULL, "Display images in normal quality", 1 },
    { "LowQuality", "my-stock-low-quality", "_Low Quality", NULL, "Display images in low quality, fast mode", 2 }
};

static const char* application_menu_ui =
        "<ui>"
        "   <menu action='ApplicationMenu'>"
        "       <menuitem action='Open'/>"
        "       <menuitem action='Exit'/>"
        "   </menu>"
        "</ui>"
        ;

static const char *ui_description =
        "<ui>"
        "  <menubar name='MainMenu'>"
        "    <menu action='FileMenu'>"
        "      <menuitem action='Open'/>"
        "      <menuitem action='Exit'/>"
        "    </menu>"
        "    <menu action='ViewMenu'>"
        "      <menuitem action='ZoomIn'/>"
        "      <menuitem action='ZoomOut'/>"
        "      <separator/>"
        "      <menuitem action='FullScreen'/>"
        "      <separator/>"
        "      <menuitem action='HighQuality'/>"
        "      <menuitem action='NormalQuality'/>"
        "      <menuitem action='LowQuality'/>"
        "    </menu>"
        "  </menubar>"
        "</ui>";

static GtkWidget* get_menubar_menu( GtkWindow* window )
{
    GtkActionGroup* action_group = gtk_action_group_new ("MenuActions");
    gtk_action_group_add_actions (action_group, entries, G_N_ELEMENTS (entries), window);
    gtk_action_group_add_toggle_actions (action_group, toggle_entries, G_N_ELEMENTS (toggle_entries), window);
    gtk_action_group_add_radio_actions (action_group, radio_entries, G_N_ELEMENTS (radio_entries), 0, print_selected, window);
    
    GtkUIManager* ui_manager = gtk_ui_manager_new ();
    gtk_ui_manager_insert_action_group (ui_manager, action_group, 0);
    
    GtkAccelGroup* accel_group = gtk_ui_manager_get_accel_group (ui_manager);
    gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);
    
    GError* error = NULL;
    if (!gtk_ui_manager_add_ui_from_string (ui_manager, ui_description, -1, &error))
    {
        g_message ("building menus failed: s", error->message);
        g_error_free (error);
        exit (EXIT_FAILURE);
    }
    
    return gtk_ui_manager_get_widget (ui_manager, "/MainMenu");
}

int main( int argc, char** argv )
{
    /* Initialize GTK+ */
    gtk_init( &argc, &argv );

    MokoApplication* app = MOKO_APPLICATION( moko_application_get_instance() );
    g_set_application_name( "Hello OpenMoko!" );

    MokoWindow* window = MOKO_WINDOW(moko_window_new());
    // moko_application_set_main_window( window );

    /* Set up application menu */
    GtkMenu* appmenu = gtk_menu_new();
    GtkMenuItem* closeitem = gtk_menu_item_new_with_label( "Close" );
    g_signal_connect( G_OBJECT(closeitem), "activate", G_CALLBACK( gtk_main_quit ), NULL );
    gtk_menu_shell_append( appmenu, closeitem );

    GtkMenuItem* appitem = gtk_menu_item_new_with_label( "File" );
    gtk_menu_item_set_submenu( appitem, appmenu );

    gtk_widget_show( closeitem );

    /* Set up upper frame */
    GtkVBox* upperframe = gtk_vbox_new( FALSE, 0 );
    MokoMenuBar* menubar = GTK_MENU_BAR(moko_menu_bar_new());
    gtk_menu_shell_append( GTK_MENU_BAR(menubar), appitem );

    //MokoMenuBar* menubar = get_menubar_menu( window );

    gtk_box_pack_start( GTK_BOX(upperframe), GTK_WIDGET(menubar), TRUE, TRUE, 0 );
    GtkButton* navigationlist = gtk_button_new_with_label( "Hello Navigation Area!" );
    gtk_box_pack_start( GTK_BOX(upperframe), GTK_WIDGET(navigationlist), TRUE, TRUE, 0 );

    /* Set up lower frame */
    GtkVBox* lowerframe = gtk_vbox_new( FALSE, 0 );

    GtkButton* detailslist = gtk_button_new_with_label( "Hello Details Area!" );
    gtk_box_pack_start( GTK_BOX(lowerframe), GTK_WIDGET(detailslist), TRUE, TRUE, 0 );

    GtkVPaned* outerframe = gtk_vpaned_new();
    gtk_paned_add1( GTK_PANED(outerframe), GTK_WIDGET(upperframe) );
    gtk_paned_add2( GTK_PANED(outerframe), GTK_WIDGET(lowerframe) );
    
    gtk_container_add( GTK_CONTAINER(window), outerframe );

    g_signal_connect( G_OBJECT(window), "delete_event", G_CALLBACK( gtk_main_quit ), NULL );

    gtk_widget_show_all( GTK_WIDGET(window) );
    gtk_main();

    return 0;
}
