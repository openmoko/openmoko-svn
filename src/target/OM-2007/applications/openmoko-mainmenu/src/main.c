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

    GError* err = NULL;
    GdkPixbuf *pixbuf;
    GtkStyle *style;    
    GdkPixmap *pixmap;
    GdkBitmap *bitmap;
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
    	    mma->history[i] =  moko_finger_tool_box_add_button( mma->toolbox );
           gtk_widget_show (mma->history[i]);
    	}
   
    mma->mm = MAINMENU(moko_main_menu_new());
    gtk_widget_show (mma->mm);
    
    //gtk_icon_view_selected_foreach (mm->icon_view, moko_item_select_cb, NULL);
   // g_signal_connect (mm->icon_view, "toggle-cursor-item", 
		//G_CALLBACK (moko_toggle_cursor_item_cb), NULL);
    moko_finger_window_set_contents( mma->window, GTK_WIDGET(mma->mm) );
    
    pixbuf = gdk_pixbuf_new_from_file ( PKGDATADIR"/bg_mainmenu.png", &err );
    gdk_pixbuf_render_pixmap_and_mask (pixbuf, &pixmap, &bitmap, NULL);
    //style = gtk_rc_get_style (mma->mm->scrolled);
    //style->bg_pixmap[GTK_STATE_NORMAL] = pixmap;
   // gtk_style_set_background (style, gtk_widget_get_parent_window (mma->mm->scrolled), GTK_STATE_NORMAL);
    /* show everything and run main loop */
    gtk_widget_show_all( GTK_WIDGET(mma->window) );

    gtk_widget_show (GTK_WIDGET (mma->wheel));
    //gtk_widget_reparent (GTK_WIDGET (mma->wheel), GTK_WIDGET (mma->window));
    gtk_widget_show (GTK_WIDGET (mma->toolbox));
    //gtk_widget_reparent (GTK_WIDGET (mma->toolbox), GTK_WIDGET (mma->window));

    g_debug ("**************");
    //gdk_window_set_back_pixmap (gtk_widget_get_parent_window (mma->mm->icon_view), pixmap, FALSE);
    style = gtk_rc_get_style (mma->mm->icon_view);
    //style->bg_pixmap[GTK_STATE_NORMAL] = pixmap;
    //gtk_style_set_background (style, mma->mm->icon_view->window, GTK_STATE_NORMAL);

    //if (GTK_WIDGET_NO_WINDOW(mma->mm->icon_view)) 
    	  //g_debug ("no window");
    g_debug ("**************");
    sleep (2);

    //g_debug( "openmoko-finger-demo entering main loop" );
    gtk_main();
    //g_debug( "openmoko-finger-demo left main loop" );

    if (mma)
    	  g_free (mma);
    return 0;
}
