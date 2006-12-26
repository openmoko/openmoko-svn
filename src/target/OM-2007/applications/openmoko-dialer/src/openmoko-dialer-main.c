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
#include "contacts.h"
#include "openmoko-dialer-main.h"
#include "openmoko-dialer-window-dialer.h"
#include "openmoko-dialer-window-outgoing.h"
int main( int argc, char** argv )
{

    MOKO_DIALER_APP_DATA* p_dialer_data;
    p_dialer_data=calloc(1,sizeof(MOKO_DIALER_APP_DATA));
    /* Initialize GTK+ */
    gtk_init( &argc, &argv );


    //init application data
   contact_init_contact_data(&(p_dialer_data->g_contactlist));

    /* application object */
    MokoApplication* app = MOKO_APPLICATION(moko_application_get_instance());
    g_set_application_name( "OpenMoko Dialer" );

//init the dialer window

   window_outgoing_init(p_dialer_data);


   window_dialer_init(p_dialer_data); 

    gtk_main();
    
    contact_release_contact_list(&(p_dialer_data->g_contactlist)); 

    return 0;
}
