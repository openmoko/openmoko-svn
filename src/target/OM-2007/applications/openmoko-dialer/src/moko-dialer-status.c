/* moko-dialer-autolist .c
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

#include "moko-dialer-status.h"
#include "error.h"
#include "common.h"
G_DEFINE_TYPE (MokoDialerStatus, moko_dialer_status, GTK_TYPE_VBOX)
     static void moko_dialer_status_class_init (MokoDialerStatusClass * class)
{
/*
  GtkVBoxClass* vbox_class;

  vbox_class= (GtkVBoxClass*) class;
*/

  GtkObjectClass *object_class;

  object_class = (GtkObjectClass *) class;


}


static void
moko_dialer_status_init (MokoDialerStatus * moko_dialer_status)
{
  DBG_ENTER ();
  moko_dialer_status->imagePerson = 0;

//upper section
  ;                             ///<the topmost title bar of the status
  moko_dialer_status->labelStatusTitle = gtk_label_new ("");

/*
for(gint i=0;i<MOKO_DIALER_MAX_STATUS_ICONS;i++)
	moko_dialer_status->iconStatus[i]=gtk_image_new();
*/

  moko_dialer_status->icon = gtk_image_new ();


//lower section  
  moko_dialer_status->imagePerson = gtk_image_new ();   ///<the image of the person we care
  gtk_widget_set_size_request (moko_dialer_status->imagePerson, 130, 130);
  gtk_misc_set_alignment (GTK_MISC (moko_dialer_status->imagePerson), 1, 0);


  moko_dialer_status->labelStatus = gtk_label_new (""); ///<the status label
  gtk_misc_set_alignment (GTK_MISC (moko_dialer_status->labelStatus), 0, 0);
//                gtk_misc_set_padding(GTK_MISC (moko_dialer_status->labelStatus), 20,0);
  moko_dialer_status->labelPersonName = gtk_label_new ("");     ///<the person name
  gtk_misc_set_alignment (GTK_MISC (moko_dialer_status->labelPersonName), 0,
                          0);
  moko_dialer_status->labelNumber = gtk_label_new (""); ///<the number of the person
  gtk_misc_set_alignment (GTK_MISC (moko_dialer_status->labelNumber), 0, 0);
//private section
  moko_dialer_status->number_of_the_icons = 0;

  gtk_widget_set_size_request (moko_dialer_status->icon, 64, 64);
  gtk_box_pack_start (GTK_BOX (moko_dialer_status),
                      moko_dialer_status->labelStatusTitle, FALSE, FALSE, 20);
  gtk_box_pack_start (GTK_BOX (moko_dialer_status), moko_dialer_status->icon,
                      FALSE, FALSE, 20);

//ok now, we arrange them in the vbox.

// we create another vbox to hold the status labels.
  GtkVBox *vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vbox2), moko_dialer_status->labelStatus, TRUE,
                      TRUE, 0);

  gtk_box_pack_start (GTK_BOX (vbox2), moko_dialer_status->labelPersonName,
                      TRUE, TRUE, 0);



  gtk_box_pack_start (GTK_BOX (vbox2), moko_dialer_status->labelNumber, TRUE,
                      TRUE, 0);
// a hbox to hold the image and the vbox2
  GtkVBox *hbox2 = gtk_hbox_new (FALSE, 0);



//   gtk_box_pack_start( GTK_BOX(hbox2),        moko_dialer_status->imagePerson, FALSE, FALSE, 0 );
  gtk_box_pack_start (GTK_BOX (hbox2), moko_dialer_status->imagePerson, TRUE,
                      TRUE, 0);

// gtk_box_pack_start( GTK_BOX(hbox2),vbox2, FALSE, FALSE, 0 );
  gtk_box_pack_start (GTK_BOX (hbox2), vbox2, TRUE, TRUE, 36);

  gtk_box_pack_start (GTK_BOX (moko_dialer_status), hbox2, TRUE, TRUE, 0);

//gtk_widget_set_size_request (GTK_WIDGET(moko_dialer_status), 320, 200); 

  DBG_LEAVE ();

}

GtkWidget *
moko_dialer_status_new ()
{
  DBG_ENTER ();

  MokoDialerStatus *dp;

  dp = (MokoDialerStatus *) g_object_new (MOKO_TYPE_DIALER_STATUS, NULL);
  return GTK_WIDGET (dp);

}

void
moko_dialer_status_set_title_label (MokoDialerStatus * moko_dialer_status,
                                    const gchar * text)
{

  gtk_label_set_text (GTK_LABEL (moko_dialer_status->labelStatusTitle), text);
}

void
moko_dialer_status_set_status_label (MokoDialerStatus * moko_dialer_status,
                                     const gchar * text)
{

//DBG_MESSAGE("moko_dialer_status_set_status_label:moko_dialer_status=0X%x",moko_dialer_status);
  gtk_label_set_text (GTK_LABEL (moko_dialer_status->labelStatus), text);
}

void
moko_dialer_status_set_person_name (MokoDialerStatus * moko_dialer_status,
                                    const gchar * text)
{
  gtk_label_set_text (GTK_LABEL (moko_dialer_status->labelPersonName), text);
}

void
moko_dialer_status_set_person_number (MokoDialerStatus * moko_dialer_status,
                                      const gchar * text)
{
  gtk_label_set_text (GTK_LABEL (moko_dialer_status->labelNumber), text);
}

void
moko_dialer_status_set_person_image (MokoDialerStatus * moko_dialer_status,
                                     const gchar * id)
{
//file_load_person_image_scalable_from_relative_path(moko_dialer_status->imagePerson,path);
//file_load_person_image_from_relative_path(moko_dialer_status->imagePerson,path);

  contact_load_contact_photo (moko_dialer_status->imagePerson, id);
//file_load_person_image_scalable_from_relative_path(moko_dialer_status->imagePerson,path);
}

int
moko_dialer_status_add_status_icon (MokoDialerStatus * moko_dialer_status,
                                    const gchar * text)
{
  if (moko_dialer_status->number_of_the_icons < MOKO_DIALER_MAX_STATUS_ICONS)
  {
    moko_dialer_status->iconStatus[moko_dialer_status->number_of_the_icons] =
      create_pixbuf (text);

    moko_dialer_status->number_of_the_icons++;
    return 1;
  }
  else
  {
    DBG_ERROR
      ("MOKO_DIALER_MAX_STATUS_ICONS has reached, add no more icons.");
    return 0;
  }
}

void
moko_dialer_status_set_error_icon (MokoDialerStatus * moko_dialer_status,
                                   const gchar * text)
{
  moko_dialer_status->iconError = create_pixbuf (text);
//      file_load_person_image_from_relative_path(moko_dialer_status->iconError,text);
}

void
moko_dialer_status_set_success_icon (MokoDialerStatus * moko_dialer_status,
                                     const gchar * text)
{
  moko_dialer_status->iconError = create_pixbuf (text);
//      file_load_person_image_from_relative_path(moko_dialer_status->iconSuccuss,text);
}

void
moko_dialer_status_set_icon_by_index (MokoDialerStatus * moko_dialer_status,
                                      gint index)
{
  if (moko_dialer_status->iconStatus[index] != 0
      && index < moko_dialer_status->number_of_the_icons)
    gtk_image_set_from_pixbuf (moko_dialer_status->icon,
                               moko_dialer_status->iconStatus[index]);
}

void
moko_dialer_status_update_icon (MokoDialerStatus * moko_dialer_status)
{
  static gint currenticon = 0;
  currenticon = (currenticon + 1) % moko_dialer_status->number_of_the_icons;
  if (moko_dialer_status->iconStatus[currenticon])
    moko_dialer_status_set_icon_by_index (moko_dialer_status, currenticon);
}


void
moko_dialer_status_set_error (MokoDialerStatus * moko_dialer_status)
{
  if (moko_dialer_status->iconError)
    gtk_image_set_from_pixbuf (moko_dialer_status->icon,
                               moko_dialer_status->iconError);
  moko_dialer_status_set_status_label (moko_dialer_status, "");
}

void
moko_dialer_status_set_success (MokoDialerStatus * moko_dialer_status)
{
  if (moko_dialer_status->iconSuccess)
    gtk_image_set_from_pixbuf (moko_dialer_status->icon,
                               moko_dialer_status->iconSuccess);

}
