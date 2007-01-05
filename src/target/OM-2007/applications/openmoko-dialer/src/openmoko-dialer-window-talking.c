/*   openmoko-dialer-window-talking.c
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
#include "openmoko-dialer-window-talking.h"

void
openmoko_wheel_press_left_up_cb(GtkWidget *widget, MOKO_DIALER_APP_DATA * appdata)
{DBG_ENTER();
}

void
openmoko_wheel_press_right_down_cb(GtkWidget *widget, MOKO_DIALER_APP_DATA * appdata)
{
DBG_ENTER();
}


void cb_tool_button_speaker_clicked( GtkButton* button, MOKO_DIALER_APP_DATA * appdata)
{
  DBG_ENTER();
}

void cb_tool_button_dtmf_talk_clicked( GtkButton* button, MOKO_DIALER_APP_DATA * appdata)
{
	DBG_ENTER();
	
	appdata->dtmf_in_talking_window=!appdata->dtmf_in_talking_window;
	if(appdata->dtmf_in_talking_window)
		{
		
			
			moko_pixmap_button_set_finger_toolbox_btn_center_image(button, appdata->imageTALK);
		       gtk_widget_hide(appdata->content_talk);
			gtk_widget_show(appdata->content_dtmf);
		}
	else
		{
			moko_pixmap_button_set_finger_toolbox_btn_center_image(button, appdata->imageDTMF);
			gtk_widget_hide(appdata->content_dtmf);
		       gtk_widget_show(appdata->content_talk);
		       

		}

  
}

void cb_tool_button_hangup_clicked( GtkButton* button, MOKO_DIALER_APP_DATA * appdata)
{
     gsm_hangup();
     gtk_widget_hide(appdata->window_talking);

}


void
on_dtmf_panel_user_input(GtkWidget * widget,gchar parac,
                                        gpointer         user_data)
{
char input[2];
input[0]=parac;
input[1]=0;
char codesinput[MOKO_DIALER_MAX_NUMBER_LEN+1];


MOKO_DIALER_APP_DATA * appdata=(MOKO_DIALER_APP_DATA*)user_data;
MokoDialerTextview *moko_dtmf_text_view=appdata->moko_dtmf_text_view;

moko_dialer_textview_insert(moko_dtmf_text_view, input);

}



void window_talking_prepare(MOKO_DIALER_APP_DATA * appdata)
{
DBG_ENTER();
   moko_dialer_status_set_person_number(appdata->status_talking, appdata->g_peer_info.number);
if(appdata->g_peer_info.hasname)
{
   moko_dialer_status_set_person_image(appdata->status_talking, appdata->g_peer_info.picpath);  
   moko_dialer_status_set_person_name(appdata->status_talking, appdata->g_peer_info.name);
}
else
{
   moko_dialer_status_set_person_image(appdata->status_talking, MOKO_DIALER_DEFAULT_PERSON_IMAGE_PATH);  
   moko_dialer_status_set_person_name(appdata->status_talking, "");

}

DBG_LEAVE();

}

gint timer_talking_time_out(MOKO_DIALER_APP_DATA * appdata)
{
//DBG_ENTER();
TIMER_DATA* timer_data=&(appdata->g_timer_data);

	
timer_data->ticks++;
timer_data->hour=timer_data->ticks/3600;
timer_data->min=(timer_data->ticks-timer_data->hour*3600)/60;
timer_data->sec=timer_data->ticks%60;


sprintf(timer_data->timestring,"Talking (%02d:%02d:%02d)",timer_data->hour,timer_data->min,timer_data->sec);

//ok,we update the label now.


moko_dialer_status_set_status_label(appdata->status_talking,timer_data->timestring);
moko_dialer_status_update_icon(appdata->status_talking);

if(timer_data->stopsec!=0&&timer_data->ticks>=timer_data->stopsec)
{

timer_data->timeout=1;
gtk_timeout_remove(timer_data->ptimer);
timer_data->ptimer=0;
return 0;//0 stops the timer.
}
else
return 1;
}

void
on_window_talking_hide                 (GtkWidget       *widget,
                                        MOKO_DIALER_APP_DATA * appdata)
{
if(appdata->g_timer_data.ptimer!=0)
{
gtk_timeout_remove(appdata->g_timer_data.ptimer);
appdata->g_timer_data.ptimer=0;
}

gtk_widget_hide(appdata->wheel_talking);
gtk_widget_hide(appdata->toolbox_talking);

//DBG_MESSAGE("%s, %s",appdata->g_state.starttime,appdata->g_state.startdate);
add_histroy_entry(appdata,appdata->g_state.historytype,
	appdata->g_peer_info.name,
	appdata->g_peer_info.number,
	appdata->g_peer_info.picpath,
	appdata->g_state.starttime,
	appdata->g_state.startdate,
	appdata->g_timer_data.ticks);


}

void
on_window_talking_show                  (GtkWidget       *widget,
                                        MOKO_DIALER_APP_DATA * appdata)
{
DBG_ENTER();

appdata->dtmf_in_talking_window=TRUE;
	//hide the talking button in talking mode.

time_t timep;
struct tm *p;
time(&timep);
p=localtime(&timep);

sprintf(appdata->g_state.starttime,"%02d:%02d:%02d",p->tm_hour,p->tm_min,p->tm_sec);
sprintf(appdata->g_state.startdate,"%04d/%02d/%02d",p->tm_year+1900,p->tm_mon,p->tm_mday);

memset(&(appdata->g_timer_data),0,sizeof(appdata->g_timer_data));

appdata->g_timer_data.stopsec=0;

appdata->g_timer_data.ptimer=gtk_timeout_add(1000,timer_talking_time_out,appdata);

if(appdata->wheel_talking)
gtk_widget_show(appdata->wheel_talking);

if(appdata->toolbox_talking)
gtk_widget_show(appdata->toolbox_talking);


   
DBG_LEAVE();
}




gint window_talking_init( MOKO_DIALER_APP_DATA* p_dialer_data)
{

DBG_ENTER();


if(p_dialer_data->window_talking==0)
{

	MokoFingerWindow* window=NULL;
	GtkWidget* vbox=gtk_vbox_new( FALSE, 0 );;
	MokoDialerStatus * status=NULL;
	GtkWidget* content_talk= NULL;
	GtkWidget* content_dtmf= NULL;
      MokoFingerToolBox *tools = NULL;
      MokoDialerPanel* mokodialerpanel=NULL;
      GtkWidget *button;
      GtkWidget* image;


//first, the talking content.

   content_talk = gtk_vbox_new( FALSE, 0 );
   status=moko_dialer_status_new();
   moko_dialer_status_add_status_icon(status,"status0.png");
   moko_dialer_status_add_status_icon(status,"status1.png");
   moko_dialer_status_add_status_icon(status,"status2.png");
   moko_dialer_status_set_icon_by_index(status,0);
   
   
   moko_dialer_status_set_title_label(status, "In Call");
   moko_dialer_status_set_status_label(status, "Talking ...(00:00:00)");

   gtk_box_pack_start( GTK_BOX(content_talk),status, FALSE, FALSE, 0 );


   p_dialer_data->status_talking=status;
   p_dialer_data->content_talk= content_talk;

   gtk_box_pack_start( GTK_BOX(vbox),content_talk, FALSE, FALSE, 0 );
   
  
//now the dtmf content
  	content_dtmf= gtk_vbox_new( FALSE, 0 );
  	GtkEventBox* eventbox1 = gtk_event_box_new ();
	gtk_widget_show (eventbox1);
	gtk_widget_set_name(GTK_WIDGET(eventbox1),"gtkeventbox-black");

   	MokoDialerTextview * mokotextview=moko_dialer_textview_new();

  	gtk_container_add (GTK_CONTAINER (eventbox1), mokotextview);

   	p_dialer_data->moko_dtmf_text_view=mokotextview;
    
  	gtk_box_pack_start( GTK_BOX(content_dtmf), GTK_WIDGET(eventbox1), FALSE,FALSE, 2 );



	 mokodialerpanel=moko_dialer_panel_new();

   	gtk_widget_set_size_request (mokodialerpanel, 380, 384);
   	g_signal_connect (GTK_OBJECT (mokodialerpanel), "user_input",  G_CALLBACK (on_dtmf_panel_user_input),p_dialer_data);
    	gtk_box_pack_start( GTK_BOX(content_dtmf), GTK_WIDGET(mokodialerpanel), TRUE, TRUE, 5 );
  
   	gtk_box_pack_start( GTK_BOX(vbox),content_dtmf, FALSE, FALSE, 0 );
   	p_dialer_data->content_dtmf=content_dtmf;


//now the container--window
     window = MOKO_FINGER_WINDOW(moko_finger_window_new());
     p_dialer_data->window_talking=window;
     moko_finger_window_set_contents(window, GTK_WIDGET(vbox) );
    g_signal_connect ((gpointer) window, "show",
	                    G_CALLBACK (on_window_talking_show),
	                    p_dialer_data);
    g_signal_connect ((gpointer) window, "hide",
	                    G_CALLBACK (on_window_talking_hide),
	                    p_dialer_data);

     
     gtk_widget_show_all( GTK_WIDGET(window) );

//the gtk_widget_show_all is really bad, cause i have to call it and then hide some widgets.

   gtk_widget_hide(content_dtmf);


 //now the wheel and tool box, why should the wheel and toolbox created after the gtk_widget_show_all???
   gtk_widget_show(GTK_WIDGET(moko_finger_window_get_wheel(window)));
    
    g_signal_connect(G_OBJECT(moko_finger_window_get_wheel(window)),
		    "press_left_up",
		    G_CALLBACK(openmoko_wheel_press_left_up_cb),
		    p_dialer_data);
    g_signal_connect(G_OBJECT(moko_finger_window_get_wheel(window)),
		    "press_right_down",
		    G_CALLBACK(openmoko_wheel_press_right_down_cb),
		    p_dialer_data);



        tools = moko_finger_window_get_toolbox(window);
     
	button = moko_finger_tool_box_add_button_without_label(tools);
       image = file_new_image_from_relative_path("speaker.png");
	moko_pixmap_button_set_finger_toolbox_btn_center_image(button, image);
        g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(cb_tool_button_speaker_clicked), p_dialer_data);
	
	button = moko_finger_tool_box_add_button_without_label(tools);
       image = file_new_image_from_relative_path("dtmf.png");
	moko_pixmap_button_set_finger_toolbox_btn_center_image(button, image);
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(cb_tool_button_dtmf_talk_clicked), p_dialer_data);
	p_dialer_data->imageDTMF=image;
	p_dialer_data->imageTALK=file_new_image_from_relative_path("talking.png");
	

	button = moko_finger_tool_box_add_button_without_label(tools);
       image = file_new_image_from_relative_path("tony.png");
	moko_pixmap_button_set_finger_toolbox_btn_center_image(button, image);
	g_signal_connect(G_OBJECT(button), "clicked",
			G_CALLBACK(cb_tool_button_hangup_clicked), p_dialer_data);
	gtk_widget_show(GTK_WIDGET(tools));

	p_dialer_data->dtmf_in_talking_window=0;
	p_dialer_data->wheel_talking=moko_finger_window_get_wheel(window);
	p_dialer_data->toolbox_talking=tools;


	DBG_LEAVE();
}
    return 1;
}

