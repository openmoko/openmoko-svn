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

#include <libmokoui/moko-finger-window.h>
#include <libmokoui/moko-pixmap-button.h>
#include <gtk/gtkalignment.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmenuitem.h>
#include <gtk/gtkvbox.h>
#include "dialer-main.h"

void
pin_delete_button_clicked (GtkButton * button, MokoDialerData * appdata)
{
  if (appdata->int_sim_pin_end_point)
  {
    moko_dialer_textview_delete (appdata->moko_pin_text_view);
    appdata->int_sim_pin_end_point--;
    appdata->str_sim_pin[appdata->int_sim_pin_end_point] = 0;
  }

}


void
pin_ok_button_clicked (GtkButton * button, MokoDialerData * appdata)
{
  //   gchar *codesinput;
//    codesinput =g_strdup(moko_dialer_textview_get_input (appdata->moko_pin_text_view, TRUE));

  if (!appdata->str_sim_pin || g_utf8_strlen (appdata->str_sim_pin, -1) < 1)
  {
    //user didn't input anything
    DBG_MESSAGE ("no input for pin");
  }
  else
  {                             //here send the pin codes and hide our window.

    DBG_MESSAGE ("here we send the pin:%s", appdata->str_sim_pin);
    //FIXME:why this call will cause segment fault?
    //lgsm_pin (appdata->lh, appdata->str_sim_pin);
    //lgsm_pin (appdata->lh, "1234");
    DBG_MESSAGE ("pin:%s sent", appdata->str_sim_pin);
    gtk_widget_hide (appdata->window_pin);
  }

}




void
on_pin_panel_user_input (GtkWidget * widget, gchar parac, gpointer user_data)
{
  char input[2];
  input[0] = parac;
  input[1] = 0;

//DBG_TRACE();
  MokoDialerData *appdata = (MokoDialerData *) user_data;
  MokoDialerTextview *moko_pin_text_view = appdata->moko_pin_text_view;
  if (appdata->int_sim_pin_end_point < MOKO_DIALER_MAX_NUMBER_LEN)
  {
    appdata->str_sim_pin[appdata->int_sim_pin_end_point] = parac;
    appdata->int_sim_pin_end_point++;
    moko_dialer_textview_insert (moko_pin_text_view, "*");
  }
  else
  {
    appdata->str_sim_pin[0] = parac;
    appdata->int_sim_pin_end_point = 1;
  }
//DBG_TRACE();

}

static void
on_window_pin_hide (GtkWidget * widget, MokoDialerData * appdata)
{
  appdata->window_present = 0;

}

static void
on_window_pin_show (GtkWidget * widget, MokoDialerData * appdata)
{
  DBG_ENTER ();
  appdata->window_present = widget;

  DBG_LEAVE ();
}




gint
window_pin_init (MokoDialerData * p_dialer_data)
{

  if (!p_dialer_data->window_pin)
  {

    g_stpcpy (p_dialer_data->str_sim_pin, "");
    p_dialer_data->int_sim_pin_end_point = 0;
    GdkColor color;
    gdk_color_parse ("black", &color);

    GtkVBox *vbox = NULL;


    MokoFingerWindow *window = MOKO_FINGER_WINDOW (moko_finger_window_new ());
    g_signal_connect ((gpointer) window, "show",
                      G_CALLBACK (on_window_pin_show), p_dialer_data);
    g_signal_connect ((gpointer) window, "hide",
                      G_CALLBACK (on_window_pin_hide), p_dialer_data);


    /* contents */
    vbox = gtk_vbox_new (FALSE, 0);
    GtkHBox *hbox = gtk_hbox_new (FALSE, 10);


    GtkEventBox *eventbox1 = gtk_event_box_new ();
    gtk_widget_show (eventbox1);
    gtk_widget_set_name (GTK_WIDGET (eventbox1), "gtkeventbox-black");
    gtk_widget_modify_bg (eventbox1, GTK_STATE_NORMAL, &color);
//        gtk_widget_set_size_request (eventbox1, 480, 132);

    MokoDialerTextview *mokotextview = moko_dialer_textview_new ();
    p_dialer_data->moko_pin_text_view = mokotextview;
//    moko_dialer_textview_fill_it(mokotextview , "Please input the pin:");

    gtk_container_add (GTK_CONTAINER (eventbox1), mokotextview);
    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (eventbox1), FALSE,
                        FALSE, 0);

    MokoDialerPanel *mokodialerpanel = moko_dialer_panel_new ();

    gtk_widget_set_size_request (mokodialerpanel, 380, 384);


    g_signal_connect (GTK_OBJECT (mokodialerpanel), "user_input",
                      G_CALLBACK (on_pin_panel_user_input), p_dialer_data);

    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (mokodialerpanel), TRUE,
                        TRUE, 5);



//the buttons

    GtkVBox *vbox2 = gtk_vbox_new (FALSE, 0);
    GtkButton *button1 = moko_pixmap_button_new ();
    g_signal_connect (G_OBJECT (button1), "clicked",
                      G_CALLBACK (pin_delete_button_clicked), p_dialer_data);
    gtk_widget_set_name (GTK_WIDGET (button1), "mokofingerbutton-orange");

    moko_pixmap_button_set_finger_toolbox_btn_center_image
      (MOKO_PIXMAP_BUTTON (button1),
       file_new_image_from_relative_path ("delete.png"));

    moko_pixmap_button_set_action_btn_lower_label (MOKO_PIXMAP_BUTTON
                                                   (button1), "Delete");
//    gtk_widget_set_size_request (button1, WINDOW_DIALER_BUTTON_SIZE_X,
    //                              WINDOW_DIALER_BUTTON_SIZE_Y);

    gtk_box_pack_start (GTK_BOX (vbox2), GTK_WIDGET (button1), FALSE, FALSE,
                        5);

    GtkButton *button2 = moko_pixmap_button_new ();

    g_signal_connect (G_OBJECT (button2), "clicked",
                      G_CALLBACK (pin_ok_button_clicked), p_dialer_data);
    gtk_widget_set_name (GTK_WIDGET (button1), "mokofingerbutton-orange");
    moko_pixmap_button_set_finger_toolbox_btn_center_image
      (MOKO_PIXMAP_BUTTON (button2),
       file_new_image_from_relative_path ("phone.png"));
    moko_pixmap_button_set_action_btn_lower_label (MOKO_PIXMAP_BUTTON
                                                   (button2), "OK");
    //gtk_widget_set_size_request (button2, WINDOW_DIALER_BUTTON_SIZE_X,
    //                         WINDOW_DIALER_BUTTON_SIZE_Y);

    gtk_box_pack_start (GTK_BOX (vbox2), GTK_WIDGET (button2), FALSE, FALSE,
                        20);

    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (vbox2), TRUE, TRUE, 5);



    gtk_box_pack_start (GTK_BOX (vbox), GTK_WIDGET (hbox), TRUE, TRUE, 5);


    moko_finger_window_set_contents (window, GTK_WIDGET (vbox));


    p_dialer_data->window_pin = window;

  }

  return 1;
}
