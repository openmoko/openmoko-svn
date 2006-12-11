/*   openmoko-dialer.c
 *
 *  Authored By Tony Guan<tonyguan@fic-sh.com.cn>
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
#include "moko-digit-button.h"
#include "moko-dialer-panel.h"
#include "openmoko-dialer-main.h"
#include "moko-dialer-textview.h"
#include "moko-dialer-autolist.h"
void
on_dialer_panel_user_input(GtkWidget * widget,gchar parac,
                                        gpointer         user_data)
{
char input[2];
input[0]=parac;
input[1]=0;

MokoDialerTextview *moko_dialer_text_view=(MokoDialerTextview *)user_data;

moko_dialer_textview_insert(moko_dialer_text_view, input);

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

	GtkVBox* vbox = NULL;
	static MokoFingerToolBox* tools = NULL;
    /* Initialize GTK+ */
    gtk_init( &argc, &argv );

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

/*
    GtkLabel* label1 = gtk_label_new( "Populate this area with finger widgets\n \nThere are three types of finger buttons:" );

    GtkLabel* label2 = gtk_label_new( "Orange button toggles finger scrolling wheel\nBlack button toggles finger toolbar\nDialer Button does nothing :)" );



    GtkButton* button1 = gtk_button_new();
    g_signal_connect( G_OBJECT(button1), "clicked", G_CALLBACK(cb_orange_button_clicked), window );
    gtk_widget_set_name( GTK_WIDGET(button1), "mokofingerbutton-orange" );
    gtk_box_pack_start( GTK_BOX(hbox), GTK_WIDGET(button1), TRUE, TRUE, 5 );

    GtkButton* button2 = gtk_button_new();
    //FIXME toggle buttons look odd... needs working on styling
    //gtk_toggle_button_set_mode (GTK_TOGGLE_BUTTON (button2), TRUE);
    g_signal_connect( G_OBJECT(button2), "clicked", G_CALLBACK(cb_dialer_button_clicked), window );
    gtk_widget_set_name( GTK_WIDGET(button2), "mokofingerbutton-dialer" );
    gtk_box_pack_start( GTK_BOX(hbox), GTK_WIDGET(button2), TRUE, TRUE, 5 );

    GtkButton* button3 = gtk_button_new();
    g_signal_connect( G_OBJECT(button3), "clicked", G_CALLBACK(cb_black_button_clicked), window );
    gtk_widget_set_name( GTK_WIDGET(button3), "mokofingerbutton-black" );
    gtk_box_pack_start( GTK_BOX(hbox), GTK_WIDGET(button3), TRUE, TRUE, 5 );

    MokoDigitButton*  mokobutton=moko_digit_button_new_with_labels("1","ABC");
    moko_digit_button_set_numbers(mokobutton,'1', '*');
    gtk_box_pack_start( GTK_BOX(hbox), GTK_WIDGET(mokobutton), TRUE, TRUE, 5 );


*/

    GtkHBox* hbox = gtk_hbox_new( TRUE, 10 );


    MokoDialerTextview * mokotextview=moko_dialer_textview_new();
    gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(mokotextview), FALSE,FALSE, 5 );


    MokoDialerPanel* mokodialerpanel=moko_dialer_panel_new();

    g_signal_connect (GTK_OBJECT (mokodialerpanel), "user_input",
			    G_CALLBACK (on_dialer_panel_user_input),mokotextview);

  
    g_signal_connect (GTK_OBJECT (mokodialerpanel), "user_hold",
			    G_CALLBACK ( on_dialer_panel_user_hold),mokotextview);
   	
    gtk_box_pack_start( GTK_BOX(hbox), GTK_WIDGET(mokodialerpanel), TRUE, TRUE, 5 );



    gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(hbox), TRUE, TRUE, 5 );
	

    MokoDialerAutolist* autolist=moko_dialer_autolist_new();
    gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(autolist), TRUE, TRUE, 5 );
    
    moko_finger_window_set_contents( window, GTK_WIDGET(vbox) );
/*

    MokoDigitButton*  mokobutton=moko_digit_button_new_with_labels("1","ABC");
    moko_digit_button_set_numbers(mokobutton,'1', '*');
    gtk_box_pack_start( GTK_BOX(hbox), GTK_WIDGET(mokobutton), TRUE, TRUE, 5 );

 
    gtk_box_pack_start( vbox, GTK_WIDGET(label1), TRUE, TRUE, 0 );
    gtk_box_pack_start( vbox, GTK_WIDGET(hbox), TRUE, TRUE, 0 );
    gtk_box_pack_start( vbox, GTK_WIDGET(label2), TRUE, TRUE, 0 );


*/
    /* show everything and run main loop */
    gtk_widget_show_all( GTK_WIDGET(window) );

    gtk_main();
 

    return 0;
}
