/*   openmoko-dialer-window-dialer.c
 *
 *  Authored by Tony Guan<tonyguan@fic-sh.com.cn>
 *
 *  Copyright (C) 2006 FIC Shanghai Lab
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
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

#include "common.h"
#include "contacts.h"
#include "openmoko-dialer-main.h"
#include "openmoko-dialer-window-dialer.h"
#include "openmoko-dialer-window-history.h"
#include "openmoko-dialer-window-outgoing.h"

void
cb_delete_button_clicked (GtkButton * button, MOKO_DIALER_APP_DATA * appdata)
{
  g_debug ("delete button clicked");

  if (moko_dialer_autolist_has_selected (appdata->moko_dialer_autolist))
  {
//first of all, we un-select the selection.
    moko_dialer_autolist_set_select (appdata->moko_dialer_autolist, -1);

//fill the textview with ""
    moko_dialer_textview_fill_it (appdata->moko_dialer_text_view, "");
//moko_dialer_textview_set_color(moko_dialer_textview);
  }
  else
  {
    moko_dialer_textview_delete (appdata->moko_dialer_text_view);
//refresh the autolist,but do not automaticall fill the textview
    gchar *codesinput = 0;
    codesinput =
      g_strdup (moko_dialer_textview_get_input
                (appdata->moko_dialer_text_view, FALSE));

    if (codesinput)
    {
      DBG_MESSAGE ("input %s", codesinput);
      if (g_utf8_strlen (codesinput, -1) >= MOKO_DIALER_MIN_SENSATIVE_LEN)
      {
        moko_dialer_autolist_refresh_by_string (appdata->moko_dialer_autolist,
                                                codesinput, FALSE);
        moko_dialer_textview_set_color (appdata->moko_dialer_text_view);
      }
      else
        moko_dialer_autolist_hide_all_tips (appdata->moko_dialer_autolist);
      g_free (codesinput);
    }
    else
    {
      DBG_WARN ("No input now.");
    }


  }

}

void
cb_history_button_clicked (GtkButton * button, MOKO_DIALER_APP_DATA * appdata)
{
  if (!appdata->window_history)
    window_history_init (appdata);

//start dialling.
  gtk_widget_show_all (appdata->window_history);

}

void
window_dialer_dial_out (MOKO_DIALER_APP_DATA * appdata)
{
  gchar *codesinput;
  //get the input digits
  codesinput =
    g_strdup (moko_dialer_textview_get_input
              (appdata->moko_dialer_text_view, FALSE));
  DBG_TRACE();
  if ((!codesinput)||((codesinput!=NULL)&&g_utf8_strlen (codesinput, -1) )< 1)
  {
    //user didn't input anything, maybe it's a redial, so we just insert the last dialed number and return this time.
    if (g_utf8_strlen (appdata->g_state.lastnumber, -1) > 0)
    {
      moko_dialer_textview_insert (appdata->moko_dialer_text_view,
                                   appdata->g_state.lastnumber);
      moko_dialer_autolist_refresh_by_string (appdata->moko_dialer_autolist,
                                              appdata->g_state.lastnumber,
                                              TRUE);
    }
    return;
  }
//empty the textview
  moko_dialer_textview_empty (appdata->moko_dialer_text_view);

//and we set the selected autolist to be No
  moko_dialer_autolist_set_select (appdata->moko_dialer_autolist, -1);
  moko_dialer_autolist_hide_all_tips (appdata->moko_dialer_autolist);

//got the number;//FIXME:which function should I use if not g_strdup. & strcpy.
  //strcpy(appdata->g_peer_info.number, codesinput );
  g_stpcpy (appdata->g_peer_info.number, codesinput);

//retrieve the contact information if any.
  contact_get_peer_info_from_number (appdata->g_contactlist.contacts,
                                     &(appdata->g_peer_info));
// contact_get_peer_info_from_number

/*
if(!appdata->window_outgoing)
	window_incoming_init(appdata);

//transfer the contact info
window_incoming_prepare(appdata);

//start dialling.
gtk_widget_show(appdata->window_incoming);
*/

//transfer the contact info
  window_outgoing_prepare (appdata);

//start dialling.
  gtk_widget_show_all (appdata->window_outgoing);

  g_free (codesinput);

}

void
cb_dialer_button_clicked (GtkButton * button, MOKO_DIALER_APP_DATA * appdata)
{
  window_dialer_dial_out (appdata);
}



void
on_dialer_autolist_user_selected (GtkWidget * widget, gpointer para_pointer,
                                  gpointer user_data)
{
  gchar *codesinput;
  gint lenstring = 0;
  gint leninput = 0;
  MOKO_DIALER_APP_DATA *appdata = (MOKO_DIALER_APP_DATA *) user_data;
  MokoDialerTextview *moko_dialer_text_view = appdata->moko_dialer_text_view;
  DIALER_READY_CONTACT *ready_contact = (DIALER_READY_CONTACT *) para_pointer;
  DBG_MESSAGE ("GOT THE MESSAGE OF SELECTED:%s",
               ready_contact->p_entry->content);
  codesinput = moko_dialer_textview_get_input (moko_dialer_text_view, FALSE);
  lenstring = g_utf8_strlen (ready_contact->p_entry->content, -1);
  leninput = g_utf8_strlen (codesinput, -1);
  if (lenstring > leninput)
  {

    moko_dialer_textview_fill_it (moko_dialer_text_view,
                                  &(ready_contact->p_entry->
                                    content[leninput]));

  }

  g_free (codesinput);

}

void
on_dialer_autolist_user_confirmed (GtkWidget * widget, gpointer para_pointer,
                                   gpointer user_data)
{

  MOKO_DIALER_APP_DATA *appdata = (MOKO_DIALER_APP_DATA *) user_data;
  MokoDialerTextview *moko_dialer_text_view = appdata->moko_dialer_text_view;
  DIALER_READY_CONTACT *ready_contact = (DIALER_READY_CONTACT *) para_pointer;
  DBG_MESSAGE ("GOT THE MESSAGE OF confirmed:%s",
               ready_contact->p_entry->content);
  moko_dialer_textview_confirm_it (moko_dialer_text_view,
                                   ready_contact->p_entry->content);
  DBG_MESSAGE ("And here we are supposed to call out directly");
  window_dialer_dial_out (appdata);


}

void
on_dialer_autolist_nomatch (GtkWidget * widget, gpointer user_data)
{

  MOKO_DIALER_APP_DATA *appdata = (MOKO_DIALER_APP_DATA *) user_data;
  MokoDialerTextview *moko_dialer_text_view = appdata->moko_dialer_text_view;

  DBG_MESSAGE ("GOT THE MESSAGE OF no match");
  moko_dialer_textview_fill_it (moko_dialer_text_view, "");

}

void
on_dialer_menu_close (GtkWidget * widget, gpointer user_data)
{
  MOKO_DIALER_APP_DATA *appdata = (MOKO_DIALER_APP_DATA *) user_data;
  g_main_loop_quit (appdata->mainloop);

}

void
on_dialer_menu_hide (GtkWidget * widget, MOKO_DIALER_APP_DATA * appdata)
{
  gtk_widget_hide (appdata->window_dialer);
}




void
on_dialer_panel_user_input (GtkWidget * widget, gchar parac,
                            gpointer user_data)
{
  char input[2];
  input[0] = parac;
  input[1] = 0;
  gchar *codesinput = NULL;

//DBG_TRACE();
  MOKO_DIALER_APP_DATA *appdata = (MOKO_DIALER_APP_DATA *) user_data;
  MokoDialerTextview *moko_dialer_text_view = appdata->moko_dialer_text_view;


  moko_dialer_textview_insert (moko_dialer_text_view, input);
//DBG_TRACE();


  codesinput =
    g_strdup (moko_dialer_textview_get_input (moko_dialer_text_view, FALSE));

  if (g_utf8_strlen (codesinput, -1) >= MOKO_DIALER_MIN_SENSATIVE_LEN)
  {
    moko_dialer_autolist_refresh_by_string (appdata->moko_dialer_autolist,
                                            codesinput, TRUE);
  }
  else
  {
    moko_dialer_autolist_hide_all_tips (appdata->moko_dialer_autolist);
  }

  if (codesinput)
    g_free (codesinput);

}

void
on_dialer_panel_user_hold (GtkWidget * widget, gchar parac,
                           gpointer user_data)
{
  g_print ("on_dialer_panel_user_hold:%c\n", parac);
}

void
on_window_dialer_hide (GtkWidget * widget, MOKO_DIALER_APP_DATA * appdata)
{
  appdata->window_present = 0;
}

void
on_window_dialer_show (GtkWidget * widget, MOKO_DIALER_APP_DATA * appdata)
{
  DBG_ENTER ();
  appdata->window_present = widget;
  DBG_LEAVE ();
}






#define WINDOW_DIALER_BUTTON_SIZE_X 100
#define WINDOW_DIALER_BUTTON_SIZE_Y 100
gint
window_dialer_init (MOKO_DIALER_APP_DATA * p_dialer_data)
{

  if (!p_dialer_data->window_dialer)
  {

    GdkColor color;
    gdk_color_parse ("black", &color);

    GtkWidget *vbox = NULL;


    GtkWidget *window = moko_finger_window_new ();
    gtk_window_set_decorated(GTK_WINDOW(window ),FALSE);

    GtkMenu *appmenu = GTK_MENU (gtk_menu_new ());
    GtkWidget *closeitem = gtk_menu_item_new_with_label ("Close");
    g_signal_connect (G_OBJECT (closeitem), "activate",
                      G_CALLBACK (on_dialer_menu_close), p_dialer_data);
    gtk_menu_shell_append (GTK_MENU_SHELL (appmenu), closeitem);

    GtkMenuItem *hideitem =
      GTK_MENU_ITEM (gtk_menu_item_new_with_label ("Hide"));
    g_signal_connect (G_OBJECT (hideitem), "activate",
                      G_CALLBACK (on_dialer_menu_hide), p_dialer_data);
    gtk_menu_shell_append (GTK_MENU_SHELL (appmenu), GTK_WIDGET (hideitem));


    moko_finger_window_set_application_menu (MOKO_FINGER_WINDOW (window),
                                             appmenu);

    g_signal_connect (G_OBJECT (window), "delete_event",
                      G_CALLBACK (gtk_widget_hide_on_delete), NULL);
    g_signal_connect (G_OBJECT (window), "show",
                      G_CALLBACK (on_window_dialer_show), p_dialer_data);
    g_signal_connect (G_OBJECT (window), "hide",
                      G_CALLBACK (on_window_dialer_hide), p_dialer_data);



    /* contents */
    vbox = gtk_vbox_new (FALSE, 0);
    GtkWidget *hbox = gtk_hbox_new (FALSE, 10);


    GtkWidget *eventbox1 = gtk_event_box_new ();
    gtk_widget_set_name (eventbox1, "gtkeventbox-black");

    GtkWidget *autolist = moko_dialer_autolist_new ();
    moko_dialer_autolist_set_data (MOKO_DIALER_AUTOLIST (autolist),
                                   &(p_dialer_data->g_contactlist));
    p_dialer_data->moko_dialer_autolist = MOKO_DIALER_AUTOLIST (autolist);

    gtk_container_add (GTK_CONTAINER (eventbox1), autolist);
//    gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(autolist), FALSE, FALSE, 5 );
    gtk_box_pack_start (GTK_BOX (vbox), eventbox1, FALSE, FALSE, 0);

    gtk_widget_modify_bg (eventbox1, GTK_STATE_NORMAL, &color);

    g_signal_connect (GTK_OBJECT (autolist), "user_selected",
                      G_CALLBACK (on_dialer_autolist_user_selected),
                      p_dialer_data);


    g_signal_connect (GTK_OBJECT (autolist), "user_confirmed",
                      G_CALLBACK (on_dialer_autolist_user_confirmed),
                      p_dialer_data);

    g_signal_connect (GTK_OBJECT (autolist), "autolist_nomatch",
                      G_CALLBACK (on_dialer_autolist_nomatch), p_dialer_data);





    eventbox1 = gtk_event_box_new ();
    gtk_widget_set_name (eventbox1, "gtkeventbox-black");
    gtk_widget_modify_bg (eventbox1, GTK_STATE_NORMAL, &color);
//        gtk_widget_set_size_request (eventbox1, 480, 132);

    GtkWidget *mokotextview = moko_dialer_textview_new ();
    p_dialer_data->moko_dialer_text_view =
      MOKO_DIALER_TEXTVIEW (mokotextview);

    gtk_container_add (GTK_CONTAINER (eventbox1), mokotextview);
    gtk_box_pack_start (GTK_BOX (vbox), eventbox1, FALSE, FALSE, 0);
//    gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(mokotextview), FALSE,FALSE, 5 );


    GtkWidget *mokodialerpanel = moko_dialer_panel_new ();

    gtk_widget_set_size_request (mokodialerpanel, 380, 384);


    g_signal_connect (GTK_OBJECT (mokodialerpanel), "user_input",
                      G_CALLBACK (on_dialer_panel_user_input), p_dialer_data);


    g_signal_connect (GTK_OBJECT (mokodialerpanel), "user_hold",
                      G_CALLBACK (on_dialer_panel_user_hold), p_dialer_data);

    gtk_box_pack_start (GTK_BOX (hbox), mokodialerpanel, TRUE, TRUE, 5);



//the buttons

    GtkWidget *vbox2 = gtk_vbox_new (FALSE, 0);
    GtkWidget *button1 = moko_pixmap_button_new ();
    g_signal_connect (G_OBJECT (button1), "clicked",
                      G_CALLBACK (cb_delete_button_clicked), p_dialer_data);
    gtk_widget_set_name (button1, "mokofingerbutton-orange");
    moko_pixmap_button_set_finger_toolbox_btn_center_image (MOKO_PIXMAP_BUTTON
                                                            (button1),
                                                            file_new_image_from_relative_path
                                                            ("delete.png"));
    moko_pixmap_button_set_action_btn_lower_label (MOKO_PIXMAP_BUTTON
                                                   (button1), "Delete");
    gtk_widget_set_size_request (button1, WINDOW_DIALER_BUTTON_SIZE_X,
                                 WINDOW_DIALER_BUTTON_SIZE_Y);

    gtk_box_pack_start (GTK_BOX (vbox2), button1, FALSE, FALSE, 5);

    GtkWidget *button3 = moko_pixmap_button_new ();
    g_signal_connect (G_OBJECT (button3), "clicked",
                      G_CALLBACK (cb_history_button_clicked), p_dialer_data);
    gtk_widget_set_name (button3, "mokofingerbutton-orange");
//moko_pixmap_button_set_center_stock(button3,"gtk-refresh");
    moko_pixmap_button_set_finger_toolbox_btn_center_image (MOKO_PIXMAP_BUTTON
                                                            (button3),
                                                            file_new_image_from_relative_path
                                                            ("history.png"));
    moko_pixmap_button_set_action_btn_lower_label (MOKO_PIXMAP_BUTTON
                                                   (button3), "History");
    gtk_widget_set_size_request (button3, WINDOW_DIALER_BUTTON_SIZE_X,
                                 WINDOW_DIALER_BUTTON_SIZE_Y);
    gtk_box_pack_start (GTK_BOX (vbox2), button3, FALSE, FALSE, 5);


    GtkWidget *button2 = moko_pixmap_button_new ();

    g_signal_connect (G_OBJECT (button2), "clicked",
                      G_CALLBACK (cb_dialer_button_clicked), p_dialer_data);
    gtk_widget_set_name (button2, "mokofingerbutton-black");
    moko_pixmap_button_set_finger_toolbox_btn_center_image (MOKO_PIXMAP_BUTTON
                                                            (button2),
                                                            file_new_image_from_relative_path
                                                            ("phone.png"));
    moko_pixmap_button_set_action_btn_lower_label (MOKO_PIXMAP_BUTTON
                                                   (button2), "Dial");
    gtk_widget_set_size_request (button2, WINDOW_DIALER_BUTTON_SIZE_X + 20,
                                 WINDOW_DIALER_BUTTON_SIZE_Y + 80);

    gtk_box_pack_start (GTK_BOX (vbox2), button2, FALSE, FALSE, 20);


    gtk_box_pack_start (GTK_BOX (hbox), vbox2, TRUE, TRUE, 5);



    gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 5);




    moko_finger_window_set_contents (MOKO_FINGER_WINDOW (window), vbox);

    p_dialer_data->window_dialer = window;

  }

  return 1;
}
