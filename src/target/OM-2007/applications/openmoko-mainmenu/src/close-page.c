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

 enum {
   close_page_signal,
   last_signal
 };

static void 
moko_close_page_class_init(MokoClosePageClass *self);

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
            (GClassInitFunc) moko_close_page_class_init,
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
moko_close_page_class_init(MokoClosePageClass *self) {

}

static void 
moko_close_page_init(MokoClosePage *self){
  GtkHBox *hbox1 = gtk_hbox_new (FALSE, 0);
  GtkHBox *hbox2 = gtk_hbox_new (FALSE, 0);
  
  self->close_btn = gtk_button_new ();
  gtk_widget_set_name( GTK_WIDGET(self->close_btn), "mokofingerbutton-black" );
  gtk_widget_show (self->close_btn);

  self->info = gtk_label_new ("Close main menu and turn to the last application");
  gtk_widget_show (self->info);
  gtk_label_set_justify (self->info, GTK_JUSTIFY_CENTER);
  gtk_label_set_line_wrap (self->info, TRUE);
  
  gtk_misc_set_alignment (GTK_MISC (self->close_btn), 0.5, 1);
  gtk_misc_set_alignment (GTK_MISC (self->info), 0.5, 0);
  
  gtk_box_pack_start (GTK_BOX (self), GTK_WIDGET (hbox1), TRUE, TRUE, 150);
  gtk_box_pack_start (GTK_BOX (hbox1), GTK_WIDGET (self->close_btn), TRUE, TRUE, 150);
  gtk_box_pack_start (GTK_BOX (self), GTK_WIDGET (hbox2), TRUE, TRUE, 0);
  gtk_box_pack_start (GTK_BOX (hbox2), GTK_WIDGET (self->info), TRUE, FALSE, 100);


}

GtkWidget *
moko_close_page_new () {
  return g_object_new (MOKO_CLOSE_PAGE_TYPE, NULL);
}

