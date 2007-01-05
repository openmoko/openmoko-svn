/*   openmoko-dialer-window-outgoing.c
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
#include "moko-dialer-status.h"
#include "openmoko-dialer-window-outgoing.h"

void cb_speaker_button_clicked( GtkButton* button, MOKO_DIALER_APP_DATA * appdata)
{
DBG_ENTER();
//moko_dialer_status_update_icon(appdata->status_outgoing);


appdata->g_state.callstate=STATE_TALKING;

gtk_widget_hide(appdata->window_outgoing);

if(!appdata->window_talking)
	window_talking_init(appdata);

//transfer the contact info
window_talking_prepare(appdata);

//start dialling.


gtk_widget_show(appdata->window_talking);


DBG_LEAVE();
}

void cb_redial_button_clicked( GtkButton* button, MOKO_DIALER_APP_DATA * appdata)
{
DBG_ENTER();
gtk_widget_hide(appdata->buttonRedial);
gtk_widget_show( appdata->buttonSpeaker);
moko_dialer_status_set_title_label(appdata->status_outgoing, "Outgoing call");
 moko_dialer_status_set_status_label(appdata->status_outgoing, "Calling ... (00:00:00)");
window_outgoing_setup_timer(appdata);

appdata->g_state.callstate=STATE_CALLING;
appdata->g_state.historytype=OUTGOING;	
gsm_dial(appdata->g_peer_info.number);

DBG_LEAVE();
}

void cb_cancel_button_clicked( GtkButton* button, MOKO_DIALER_APP_DATA * appdata)
{
DBG_ENTER();
gsm_hangup();
appdata->g_state.callstate=STATE_FAILED;
gtk_widget_hide(appdata->window_outgoing);
DBG_LEAVE();
}

void window_outgoing_prepare(MOKO_DIALER_APP_DATA * appdata)
{
if(appdata->window_outgoing==0)
	window_outgoing_init( appdata);

	
 moko_dialer_status_set_person_number(appdata->status_outgoing, appdata->g_peer_info.number);
if(appdata->g_peer_info.hasname)
{
   moko_dialer_status_set_person_image(appdata->status_outgoing, appdata->g_peer_info.picpath);  
   moko_dialer_status_set_person_name(appdata->status_outgoing, appdata->g_peer_info.name);
}
else
{
   moko_dialer_status_set_person_image(appdata->status_outgoing, MOKO_DIALER_DEFAULT_PERSON_IMAGE_PATH);  
   moko_dialer_status_set_person_name(appdata->status_outgoing, "");

}
strcpy(appdata->g_state.lastnumber,appdata->g_peer_info.number);


}

gint window_outgoing_fails(MOKO_DIALER_APP_DATA * appdata)
{
DBG_ENTER();
     moko_dialer_status_set_error(appdata->status_outgoing);
     moko_dialer_status_set_title_label(appdata->status_outgoing, "Call Failure");
     gtk_widget_hide( appdata->buttonSpeaker);
     gtk_widget_show(appdata->buttonRedial);
DBG_LEAVE();     
}

gint timer_outgoing_time_out(MOKO_DIALER_APP_DATA * appdata)
{
//DBG_ENTER();
TIMER_DATA* timer_data=&(appdata->g_timer_data);

	
timer_data->ticks++;
timer_data->hour=timer_data->ticks/3600;
timer_data->min=(timer_data->ticks-timer_data->hour*3600)/60;
timer_data->sec=timer_data->ticks%60;


sprintf(timer_data->timestring,"Calling ... (%02d:%02d:%02d)",timer_data->hour,timer_data->min,timer_data->sec);

//ok,we update the label now.


moko_dialer_status_set_status_label(appdata->status_outgoing,timer_data->timestring);
moko_dialer_status_update_icon(appdata->status_outgoing);

if(timer_data->stopsec!=0&&timer_data->ticks>=timer_data->stopsec)
{

timer_data->timeout=1;
gtk_timeout_remove(timer_data->ptimer);
timer_data->ptimer=0;
//maybe it failes
window_outgoing_fails(appdata);
return 0;//0 stops the timer.
}
else
return 1;
}



void
on_window_outgoing_hide                 (GtkWidget       *widget,
                                        MOKO_DIALER_APP_DATA * appdata)
{
if(appdata->g_timer_data.ptimer!=0)
{
gtk_timeout_remove(appdata->g_timer_data.ptimer);
appdata->g_timer_data.ptimer=0;
}
if(appdata->g_state.callstate!=STATE_TALKING)
{//	add_histroy_entry(g_state.historytype,g_state.contactinfo.name,g_state.contactinfo.number,g_state.contactinfo.picpath,g_state.starttime,0);

add_histroy_entry(appdata,appdata->g_state.historytype,
	appdata->g_peer_info.name,
	appdata->g_peer_info.number,
	appdata->g_peer_info.picpath,
	appdata->g_state.starttime,
	appdata->g_state.startdate,
	0);

}



}

gint window_outgoing_setup_timer(MOKO_DIALER_APP_DATA * appdata)
{
time_t timep;
struct tm *p;
time(&timep);
p=localtime(&timep);

sprintf(appdata->g_state.starttime,"%02d:%02d:%02d",p->tm_hour,p->tm_min,p->tm_sec);
sprintf(appdata->g_state.startdate,"%04d/%02d/%02d",p->tm_year,p->tm_mon,p->tm_mday);

if(appdata->g_timer_data.ptimer!=0)
{
gtk_timeout_remove(appdata->g_timer_data.ptimer);
appdata->g_timer_data.ptimer=0;
}

memset(&(appdata->g_timer_data),0,sizeof(appdata->g_timer_data));
// 1:30 timeout
appdata->g_timer_data.stopsec=90;

appdata->g_timer_data.ptimer=gtk_timeout_add(1000,timer_outgoing_time_out,appdata);


}

void
on_window_outgoing_show                  (GtkWidget       *widget,
                                        MOKO_DIALER_APP_DATA * appdata)
{


window_outgoing_setup_timer(appdata);


appdata->g_state.callstate=STATE_CALLING;
appdata->g_state.historytype=OUTGOING;	
gsm_dial(appdata->g_peer_info.number);


}


gint window_outgoing_init( MOKO_DIALER_APP_DATA* p_dialer_data)
{

DBG_ENTER();
MokoFingerWindow* window;
GtkWidget* vbox;
MokoDialerStatus * status;

if(p_dialer_data->window_outgoing==0)
{

  vbox = gtk_vbox_new( FALSE, 0 );
   status=moko_dialer_status_new();
   moko_dialer_status_add_status_icon(status,"status0.png");
   moko_dialer_status_add_status_icon(status,"status1.png");
   moko_dialer_status_add_status_icon(status,"status2.png");
   moko_dialer_status_set_icon_by_index(status,0);
   
   gtk_box_pack_start( GTK_BOX(vbox),status, FALSE, FALSE, 0 );

 	
    GtkHBox *  hbox2 = gtk_hbox_new( FALSE, 0 );
    GtkButton* button = gtk_button_new_with_label("Speaker");
    g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(cb_speaker_button_clicked), p_dialer_data );
p_dialer_data->buttonSpeaker=button;
//gtk_widget_set_size_request(button,100,32);
    gtk_box_pack_start( GTK_BOX(hbox2), GTK_WIDGET(button),TRUE, TRUE, 40 );

  button = gtk_button_new_with_label("Redial");
    p_dialer_data->buttonRedial=button;
  g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(cb_redial_button_clicked), p_dialer_data );
//gtk_widget_set_size_request(button,100,32);
  gtk_box_pack_start( GTK_BOX(hbox2), GTK_WIDGET(button), TRUE, TRUE, 40 );
//gtk_widget_set_size_request(button,60,24);


button = gtk_button_new_with_label("Cancel");
  p_dialer_data->buttonCancel=button;
 g_signal_connect( G_OBJECT(button), "clicked", G_CALLBACK(cb_cancel_button_clicked), p_dialer_data );
//gtk_widget_set_size_request(button,100,32);
 gtk_box_pack_start( GTK_BOX(hbox2), GTK_WIDGET(button),TRUE, TRUE, 40);




   gtk_box_pack_start( GTK_BOX(vbox),hbox2, FALSE, FALSE, 50 );


//currently     MokoDialogWindow is not finished, wating...
//   MokoDialogWindow* window = (MokoDialogWindow *)(moko_dialog_window_new());
//  moko_dialog_window_set_contents( window, GTK_WIDGET(vbox) );

    window = MOKO_FINGER_WINDOW(moko_finger_window_new());
    moko_finger_window_set_contents(window, GTK_WIDGET(vbox) );


     gtk_widget_show_all( GTK_WIDGET(window) );

     gtk_widget_hide( p_dialer_data->buttonRedial);


     gtk_widget_hide( GTK_WIDGET(window) );   


   moko_dialer_status_set_title_label(status, "Outgoing call");
   moko_dialer_status_set_status_label(status, "Calling ... (00:00:00)");

   p_dialer_data->window_outgoing=window;
   p_dialer_data->status_outgoing=status;
   
//   DBG_MESSAGE("p_dialer_data->status_outgoing=0X%x",p_dialer_data->status_outgoing);


  g_signal_connect ((gpointer) window, "show",
                    G_CALLBACK (on_window_outgoing_show),
                    p_dialer_data);
  g_signal_connect ((gpointer) window, "hide",
                    G_CALLBACK (on_window_outgoing_hide),
                    p_dialer_data);

}



// gtk_widget_show(p_dialer_data->window_outgoing);

DBG_LEAVE();
    return 1;
}

