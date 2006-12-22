/*   openmoko-dialer-window-dialer.c
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

#include <libmokoui/moko-finger-tool-box.h>
#include <libmokoui/moko-finger-window.h>
#include <libmokoui/moko-finger-wheel.h>
#include <libmokoui/moko-pixmap-button.h>

#include <gtk/gtkalignment.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkvbox.h>
#include "contacts.h"
#include "openmoko-dialer-main.h"
#include "openmoko-dialer-window-dialer.h"

void cb_delete_button_clicked( GtkButton* button, MOKO_DIALER_APP_DATA * appdata)
{
g_debug( "delete button clicked" );

if(moko_dialer_autolist_has_selected(appdata->moko_dialer_autolist))
{
//first of all, we un-select the selection.
moko_dialer_autolist_set_select(appdata->moko_dialer_autolist,-1);

//fill the textview with ""
moko_dialer_textview_fill_it(appdata->moko_dialer_text_view,"");
//moko_dialer_textview_set_color(moko_dialer_textview);
}
else
{
moko_dialer_textview_delete(appdata->moko_dialer_text_view);
//refresh the autolist,but do not automaticall fill the textview
char codesinput[MOKO_DIALER_MAX_NUMBER_LEN];
moko_dialer_textview_get_input(appdata->moko_dialer_text_view,codesinput, 0);

if(strlen(codesinput)>=MOKO_DIALER_MIN_SENSATIVE_LEN)
{
moko_dialer_autolist_refresh_by_string(appdata->moko_dialer_autolist,codesinput,FALSE);
moko_dialer_textview_set_color(appdata->moko_dialer_text_view);
}
else
	moko_dialer_autolist_hide_all_tips(appdata->moko_dialer_autolist);


}

}

void cb_history_button_clicked( GtkButton* button, MOKO_DIALER_APP_DATA * appdata)
{
    g_debug( "history button clicked" );
}

void cb_dialer_button_clicked( GtkButton* button, MOKO_DIALER_APP_DATA * appdata)
{
    g_debug( "dialer button clicked" );
if(!appdata->window_outgoing)
	window_outgoing_init(appdata);

gtk_widget_show(appdata->window_outgoing);
}



void
on_dialer_autolist_user_selected(GtkWidget * widget,gpointer para_pointer,
                                        gpointer         user_data)
{
gchar codesinput[MOKO_DIALER_MAX_NUMBER_LEN];
gint lenstring=0;
gint leninput=0;
MOKO_DIALER_APP_DATA * appdata=(MOKO_DIALER_APP_DATA*)user_data;
MokoDialerTextview *moko_dialer_text_view=appdata->moko_dialer_text_view;
DIALER_READY_CONTACT * ready_contact=(DIALER_READY_CONTACT * )para_pointer;
DBG_MESSAGE("GOT THE MESSAGE OF SELECTED:%s",ready_contact->p_entry->content);
moko_dialer_textview_get_input(moko_dialer_text_view,codesinput, 0);
lenstring=strlen(ready_contact->p_entry->content);
leninput=strlen(codesinput);
if(lenstring>leninput)
{

moko_dialer_textview_fill_it(moko_dialer_text_view,&(ready_contact->p_entry->content[leninput]));

}

}

void
on_dialer_autolist_user_confirmed(GtkWidget * widget,gpointer para_pointer,
                                        gpointer         user_data)
{

MOKO_DIALER_APP_DATA * appdata=(MOKO_DIALER_APP_DATA*)user_data;
MokoDialerTextview *moko_dialer_text_view=appdata->moko_dialer_text_view;
DIALER_READY_CONTACT * ready_contact=(DIALER_READY_CONTACT * )para_pointer;
DBG_MESSAGE("GOT THE MESSAGE OF confirmed:%s",ready_contact->p_entry->content);
moko_dialer_textview_confirm_it(moko_dialer_text_view,ready_contact->p_entry->content);
DBG_MESSAGE("And here we are supposed to call out directly");

}
void
on_dialer_autolist_nomatch(GtkWidget * widget,gpointer         user_data)
{

MOKO_DIALER_APP_DATA * appdata=(MOKO_DIALER_APP_DATA*)user_data;
MokoDialerTextview *moko_dialer_text_view=appdata->moko_dialer_text_view;

DBG_MESSAGE("GOT THE MESSAGE OF no match");
moko_dialer_textview_fill_it(moko_dialer_text_view,"");

}


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

moko_dialer_textview_get_input(moko_dialer_text_view,codesinput, 0);
if(strlen(codesinput)>=MOKO_DIALER_MIN_SENSATIVE_LEN)
	moko_dialer_autolist_refresh_by_string(appdata->moko_dialer_autolist,codesinput,TRUE);
else
	moko_dialer_autolist_hide_all_tips(appdata->moko_dialer_autolist);

}
void
on_dialer_panel_user_hold(GtkWidget * widget,gchar parac,
                                        gpointer         user_data)
{

g_print("on_dialer_panel_user_hold:%c\n", parac);
}

#define WINDOW_DIALER_BUTTON_SIZE_X 100
#define WINDOW_DIALER_BUTTON_SIZE_Y 100
gint window_dialer_init( MOKO_DIALER_APP_DATA* p_dialer_data)
{

	GdkColor  color;
	gdk_color_parse("black",&color);

    GtkVBox* vbox = NULL;


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
    GtkHBox* hbox = gtk_hbox_new( FALSE, 10 );



  
	
	 GtkEventBox *eventbox1 = gtk_event_box_new ();
	 gtk_widget_show (eventbox1);
	//  gtk_widget_set_size_request (eventbox1, 480, 132);
  gtk_widget_set_name(GTK_WIDGET(eventbox1),"gtkeventbox-black");

    MokoDialerAutolist* autolist=moko_dialer_autolist_new();
    moko_dialer_autolist_set_data	(autolist,&(p_dialer_data->g_contactlist));
    p_dialer_data->moko_dialer_autolist=autolist;

   gtk_container_add (GTK_CONTAINER (eventbox1), autolist);
//    gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(autolist), FALSE, FALSE, 5 );
    gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(eventbox1), FALSE, FALSE, 0 );
 
    gtk_widget_modify_bg(eventbox1,GTK_STATE_NORMAL,&color);

    g_signal_connect (GTK_OBJECT (autolist), "user_selected",
			    G_CALLBACK (on_dialer_autolist_user_selected),p_dialer_data);

  
    g_signal_connect (GTK_OBJECT (autolist), "user_confirmed",
			    G_CALLBACK ( on_dialer_autolist_user_confirmed),p_dialer_data);

    g_signal_connect (GTK_OBJECT (autolist), "autolist_nomatch",
			    G_CALLBACK ( on_dialer_autolist_nomatch),p_dialer_data);





	eventbox1 = gtk_event_box_new ();
	gtk_widget_show (eventbox1);
	gtk_widget_set_name(GTK_WIDGET(eventbox1),"gtkeventbox-black");
      gtk_widget_modify_bg(eventbox1,GTK_STATE_NORMAL,&color);
//	  gtk_widget_set_size_request (eventbox1, 480, 132);

    MokoDialerTextview * mokotextview=moko_dialer_textview_new();
    p_dialer_data->moko_dialer_text_view=mokotextview;

   gtk_container_add (GTK_CONTAINER (eventbox1), mokotextview);
       gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(eventbox1), FALSE,FALSE, 0 );
//    gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(mokotextview), FALSE,FALSE, 5 );


    MokoDialerPanel* mokodialerpanel=moko_dialer_panel_new();

   gtk_widget_set_size_request (mokodialerpanel, 380, 384);


    g_signal_connect (GTK_OBJECT (mokodialerpanel), "user_input",
			    G_CALLBACK (on_dialer_panel_user_input),p_dialer_data);

  
    g_signal_connect (GTK_OBJECT (mokodialerpanel), "user_hold",
			    G_CALLBACK ( on_dialer_panel_user_hold),p_dialer_data);
   	
    gtk_box_pack_start( GTK_BOX(hbox), GTK_WIDGET(mokodialerpanel), TRUE, TRUE, 5 );



//the buttons

/*
    GtkWidget *iconimage = gtk_image_new();
   file_load_person_image_from_relative_path( iconimage,"unkown.png");
 //gtk_image_new_from_stock ("gtk-delete", GTK_ICON_SIZE_BUTTON);
 */
//    moko_pixmap_button_set_finger_toolbox_btn_center_image (button1, iconimage);
    GtkVBox *  vbox2 = gtk_vbox_new( FALSE, 0 );
    GtkButton* button1 = moko_pixmap_button_new();
    g_signal_connect( G_OBJECT(button1), "clicked", G_CALLBACK(cb_delete_button_clicked), p_dialer_data );
    gtk_widget_set_name( GTK_WIDGET(button1), "mokofingerbutton-orange" );
	moko_pixmap_button_set_action_btn_center_stock(button1,"gtk-delete");
	moko_pixmap_button_set_action_btn_lower_label(button1,"Delete");
  gtk_widget_set_size_request (button1, WINDOW_DIALER_BUTTON_SIZE_X, WINDOW_DIALER_BUTTON_SIZE_Y);
  
    gtk_box_pack_start( GTK_BOX(vbox2), GTK_WIDGET(button1),FALSE, FALSE, 5 );

    GtkButton* button3 = moko_pixmap_button_new();
    g_signal_connect( G_OBJECT(button3), "clicked", G_CALLBACK(cb_history_button_clicked), p_dialer_data );
    gtk_widget_set_name( GTK_WIDGET(button3), "mokofingerbutton-orange" );
moko_pixmap_button_set_action_btn_center_stock(button3,"gtk-refresh");
moko_pixmap_button_set_action_btn_lower_label(button3,"History");
  gtk_widget_set_size_request (button3, WINDOW_DIALER_BUTTON_SIZE_X, WINDOW_DIALER_BUTTON_SIZE_Y);	
    gtk_box_pack_start( GTK_BOX(vbox2), GTK_WIDGET(button3),FALSE, FALSE, 5 );


    GtkButton* button2 = moko_pixmap_button_new();
	
    g_signal_connect( G_OBJECT(button2), "clicked", G_CALLBACK(cb_dialer_button_clicked), p_dialer_data );
    gtk_widget_set_name( GTK_WIDGET(button2), "mokofingerbutton-black" );
	moko_pixmap_button_set_action_btn_center_stock(button2,"gtk-yes");
	moko_pixmap_button_set_action_btn_lower_label(button2,"Dial");
  gtk_widget_set_size_request (button2, WINDOW_DIALER_BUTTON_SIZE_X+20, WINDOW_DIALER_BUTTON_SIZE_Y+80);
  
    gtk_box_pack_start( GTK_BOX(vbox2), GTK_WIDGET(button2), FALSE, FALSE, 20 );


    gtk_box_pack_start( GTK_BOX(hbox), GTK_WIDGET(vbox2),TRUE, TRUE, 5 );



    gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(hbox), TRUE, TRUE, 5 );


    
    
    moko_finger_window_set_contents( window, GTK_WIDGET(vbox) );


    p_dialer_data-> window_dialer=window;

    gtk_widget_show_all( GTK_WIDGET(window) );
    

    return 1;
}

