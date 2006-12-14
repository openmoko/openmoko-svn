/*   openmoko-dialer.c
 *
 *  Authored by Tony Guan<tonyguan@fic-sh.com.cn>
 *
 *  Copyright (C) 2006 FIC Shanghai Lab
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date) [$Author: Tony Guan $]
 */
#include <libmokoui/moko-application.h>
#include <libmokoui/moko-finger-tool-box.h>
#include <libmokoui/moko-finger-window.h>
#include <libmokoui/moko-finger-wheel.h>

#include <gtk/gtkalignment.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtktogglebutton.h>
#include <gtk/gtkvbox.h>

#include "openmoko-dialer-main.h"

void
on_dialer_panel_user_input(GtkWidget * widget,gchar parac,
                                        gpointer         user_data)
{
char input[2];
input[0]=parac;
input[1]=0;
char codesinput[MOKO_DIALER_MAX_NUMBER_LEN];

MOKO_DIALER_APP_DATA * appdata=(MOKO_DIALER_APP_DATA*)user_data;
MokoDialerTextview *moko_dialer_text_view=appdata->moko_dialer_text_view;

moko_dialer_textview_insert(moko_dialer_text_view, input);

moko_dialer_textview_get_input(moko_dialer_text_view,&codesinput, 0);

moko_dialer_autolist_refresh_by_string(appdata->moko_dialer_autolist,codesinput);

g_print("on_dialer_panel_user_input:%c\n", parac);
}
void
on_dialer_panel_user_hold(GtkWidget * widget,gchar parac,
                                        gpointer         user_data)
{

g_print("on_dialer_panel_user_hold:%c\n", parac);
}


int main( int argc, char** argv )
{

    MOKO_DIALER_APP_DATA* p_dialer_data;
    p_dialer_data=calloc(1,sizeof(MOKO_DIALER_APP_DATA));
    GtkVBox* vbox = NULL;
	static MokoFingerToolBox* tools = NULL;
    /* Initialize GTK+ */
    gtk_init( &argc, &argv );

   contact_init_contact_data(&(p_dialer_data->g_contactlist));

    /* application object */
    MokoApplication* app = MOKO_APPLICATION(moko_application_get_instance());
    g_set_application_name( "OpenMoko Dialer" );

    /* main window */
    MokoFingerWindow* window = MOKO_FINGER_WINDOW(moko_finger_window_new());

    /* application menu */
    GtkMenu* appmenu = GTK_MENU(gtk_menu_new());

    GtkMenuItem* closeitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Close" ));
    g_signal_connect( G_OBJECT(closeitem), "activate", G_CALLBACK(gtk_main_quit), NULL );
    
    gtk_menu_shell_append( appmenu, closeitem );
    moko_finger_window_set_application_menu( window, appmenu );

    /* connect close event */
    g_signal_connect( G_OBJECT(window), "delete_event", G_CALLBACK( gtk_main_quit ), NULL );

    /* contents */
    vbox = gtk_vbox_new( FALSE, 0 );


    GtkHBox* hbox = gtk_hbox_new( TRUE, 10 );


    MokoDialerTextview * mokotextview=moko_dialer_textview_new();
    p_dialer_data->moko_dialer_text_view=mokotextview;

    gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(mokotextview), FALSE,FALSE, 5 );


    MokoDialerPanel* mokodialerpanel=moko_dialer_panel_new();



    g_signal_connect (GTK_OBJECT (mokodialerpanel), "user_input",
			    G_CALLBACK (on_dialer_panel_user_input),p_dialer_data);

  
    g_signal_connect (GTK_OBJECT (mokodialerpanel), "user_hold",
			    G_CALLBACK ( on_dialer_panel_user_hold),p_dialer_data);
   	
    gtk_box_pack_start( GTK_BOX(hbox), GTK_WIDGET(mokodialerpanel), TRUE, TRUE, 5 );



    gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(hbox), TRUE, TRUE, 5 );
	

    MokoDialerAutolist* autolist=moko_dialer_autolist_new();
    gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(autolist), TRUE, TRUE, 5 );

    p_dialer_data->moko_dialer_autolist=autolist;
    
    moko_finger_window_set_contents( window, GTK_WIDGET(vbox) );

    /* show everything and run main loop */
    gtk_widget_show_all( GTK_WIDGET(window) );


    contact_release_contact_list(&(p_dialer_data->g_contactlist));
    
    gtk_main();
 

    return 0;
}
