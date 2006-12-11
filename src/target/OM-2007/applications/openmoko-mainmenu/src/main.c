/*
 *  openmoko-mainmenu
 *
 *  Authored By Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
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

#include "main.h"

static GtkVBox* vbox = NULL;
static MokoFingerToolBox* tools = NULL;

void cb_orange_button_clicked( GtkButton* button, MokoFingerWindow* window )
{
    g_debug( "openmoko-finger-demo: orange button clicked" );
    static gboolean show = TRUE;
    static MokoFingerWheel* wheel = NULL;

    if (!wheel) wheel = moko_finger_wheel_new();

    if ( show )
        gtk_widget_show( GTK_WIDGET(wheel) );
    else
        gtk_widget_hide( GTK_WIDGET(wheel) );

    show = !show;
}

void cb_black_button_clicked( GtkButton* button, MokoFingerWindow* window )
{
    g_debug( "openmoko-finger-demo: black button clicked" );
    static gboolean show = TRUE;

    if (!tools)
    {
        tools = moko_finger_tool_box_new();
        for ( int i = 0; i < 1; ++i )
            moko_finger_tool_box_add_button( tools );
    }

    if ( show )
        gtk_widget_show( GTK_WIDGET(tools) );
    else
        gtk_widget_hide( GTK_WIDGET(tools) );

    show = !show;
}

void cb_dialer_button_clicked( GtkButton* button, MokoFingerWindow* window )
{
    if (!tools) return;
    moko_finger_tool_box_add_button( tools );
}

int main( int argc, char** argv ) {
    MokoMainmenuApp *mma;
    mma = g_malloc0 (sizeof (MokoMainmenuApp));
    if (!mma) {
    	fprintf (stderr, "openmoko-mainmenu application initialize FAILED");
    	exit (0);
    	}
    memset (mma, 0, sizeof (MokoMainmenuApp));

    gtk_init( &argc, &argv );
    
    //add_pixmap_directory (PACKAGE_DATA_DIR "/pixmaps");
    //add_pixmap_directory (PACKAGE_DATA_DIR "/" PACKAGE "/pixmaps");
    //add_pixmap_directory ("/usr/share/pixmaps");
    /* application object */
    mma->app = MOKO_APPLICATION(moko_application_get_instance());
    g_set_application_name( "OpenMoko Main Menu" );
    
    /* main window */
    mma->window = MOKO_FINGER_WINDOW(moko_finger_window_new());
    
    mma->mm = MAINMENU(moko_main_menu_new());
    //gtk_icon_view_selected_foreach (mm->icon_view, moko_item_select_cb, NULL);
   // g_signal_connect (mm->icon_view, "toggle-cursor-item", 
		//G_CALLBACK (moko_toggle_cursor_item_cb), NULL);
    moko_finger_window_set_contents( mma->window, GTK_WIDGET(mma->mm) );
    /* show everything and run main loop */
    gtk_widget_show_all( GTK_WIDGET(mma->window) );
    //g_debug( "openmoko-finger-demo entering main loop" );
    gtk_main();
    //g_debug( "openmoko-finger-demo left main loop" );

    if (mma)
    	  g_free (mma);
    return 0;
}
