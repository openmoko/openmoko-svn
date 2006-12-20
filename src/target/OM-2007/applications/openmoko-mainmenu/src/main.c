/*
 *  openmoko-mainmenu
 *
 *  Authored by Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
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

int 
main( int argc, char** argv ) {
    MokoMainmenuApp *mma;
    int i;

    mma = g_malloc0 (sizeof (MokoMainmenuApp));
    if (!mma) {
    	fprintf (stderr, "openmoko-mainmenu application initialize FAILED");
    	exit (0);
    	}
    memset (mma, 0, sizeof (MokoMainmenuApp));

    gtk_init( &argc, &argv );
    
   /* application object */
    mma->app = MOKO_APPLICATION(moko_application_get_instance());
    g_set_application_name( "OpenMoko Main Menu" );
    
    /* main window */
    mma->window = MOKO_FINGER_WINDOW(moko_finger_window_new());
    gtk_widget_show (GTK_WIDGET (mma->window));
    mma->wheel = moko_finger_window_get_wheel (mma->window);
    mma->toolbox = moko_finger_window_get_toolbox(mma->window);

    for (i=0; i<4; i++)
    	{
    	    mma->history[i] =  moko_finger_tool_box_add_button (mma->toolbox);
           gtk_widget_show (mma->history[i]);
    	}
   
    mma->mm = MAINMENU(moko_main_menu_new());
    gtk_widget_show (mma->mm);
    
    //gtk_icon_view_selected_foreach (mm->icon_view, moko_item_select_cb, NULL);
   // g_signal_connect (mm->icon_view, "toggle-cursor-item", 
		//G_CALLBACK (moko_toggle_cursor_item_cb), NULL);
    moko_finger_window_set_contents( mma->window, GTK_WIDGET(mma->mm) );
    
    /* show everything and run main loop */
    gtk_widget_show_all( GTK_WIDGET(mma->window) );

    gtk_widget_show (GTK_WIDGET (mma->wheel));
    gtk_widget_show (GTK_WIDGET (mma->toolbox));

    moko_sample_hisory_app_fill (mma->history[0]);

    gtk_main();

    if (mma)
    	  g_free (mma);
    return 0;
}
