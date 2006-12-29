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

#include <gtk/gtkhbox.h>

#include "close-page.h"

static void 
moko_close_page_init(MokoClosePage *self);

/**
*@brief retrun MokoClosePage type.
*@param none
*@return GType
*/
GType 
moko_close_page_get_type (void) /* Typechecking */
{
    static GType close_page = 0;

    if (!close_page)
    {
        static const GTypeInfo type_info =
        {
            sizeof (MokoClosePageClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            NULL, /* class_init */
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (MokoClosePage),
            0,
            (GInstanceInitFunc) moko_close_page_init,
            NULL
        };

        close_page = g_type_register_static (GTK_TYPE_VBOX, "MokoClosePage", &type_info, 0);
    }

    return close_page;
}

static void 
moko_close_page_init(MokoClosePage *self){
  GtkHBox *hbox0 = gtk_hbox_new (FALSE, 0);
  GtkHBox *hbox1 = gtk_hbox_new (FALSE, 0);
  GtkHBox *hbox2 = gtk_hbox_new (FALSE, 0);
  GtkHBox *hbox3 = gtk_hbox_new (FALSE, 0);
  GtkWidget *image = gtk_image_new_from_file (PKGDATADIR"/Back_128x128.png");
  
  self ->close_btn = moko_pixmap_button_new();
  gtk_widget_set_name( GTK_WIDGET(self->close_btn), "mokofingerbutton-big" );
  moko_pixmap_button_set_center_image (self->close_btn, image);
  gtk_widget_show (self->close_btn);

  self->info[0] = gtk_label_new ("Click to close the main menu,");
  gtk_widget_show (self->info);
  gtk_label_set_justify (self->info, GTK_JUSTIFY_CENTER);
  gtk_label_set_line_wrap (self->info, TRUE);

  self->info[1] = gtk_label_new ("and turn to the previous application");
  gtk_widget_show (self->info);
  gtk_label_set_justify (self->info, GTK_JUSTIFY_CENTER);
  gtk_label_set_line_wrap (self->info, TRUE);

  gtk_box_pack_start (GTK_BOX (self), GTK_WIDGET (hbox0), FALSE, FALSE, 54);
  gtk_box_pack_start (GTK_BOX (self), GTK_WIDGET (hbox1), FALSE, FALSE, 34);
  gtk_box_pack_start (GTK_BOX (hbox1), GTK_WIDGET (self->close_btn), TRUE, TRUE, 140);
  gtk_box_pack_start (GTK_BOX (self), GTK_WIDGET (hbox2), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), GTK_WIDGET (self->info[0]), TRUE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (self), GTK_WIDGET (hbox3), FALSE, FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox3), GTK_WIDGET (self->info[1]), TRUE, FALSE, 0);

}

GtkWidget *
moko_close_page_new () {
  return g_object_new (MOKO_CLOSE_PAGE_TYPE, NULL);
}

