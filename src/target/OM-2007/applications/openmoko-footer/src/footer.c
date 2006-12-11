/**
 * @file Footer.c
 * @brief openmoko-taskmanager UI based on this file.
 * @author Sun Zhiyong
 * @date 2006-10
 *
 * Copyright (C) 2006 FIC-SH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */
#include "footer.h"

enum {
    FOOTER_SIGNAL,
    LAST_SIGNAL
};

static void footer_class_init          (FooterClass *klass);
static void footer_init                (Footer      *f);

static guint footer_signals[LAST_SIGNAL] = { 0 };

/**
*@brief retrun fooer type.
*@param none
*@return GType
*/
GType footer_get_type (void) /* Typechecking */
{
    static GType f_type = 0;

    if (!f_type)
    {
        static const GTypeInfo f_info =
        {
            sizeof (FooterClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) footer_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (Footer),
            0,
            (GInstanceInitFunc) footer_init,
            NULL
        };

        f_type = g_type_register_static(GTK_TYPE_HBOX, "Footer", &f_info, 0);
    }

    return f_type;
}

/**
*@brief initialize footer class.
*@param klass	FooterClass
*@return none
*/
static void footer_class_init (FooterClass *Klass) /* Class Initialization */
{
    footer_signals[FOOTER_SIGNAL] = g_signal_new ("footer",
            G_TYPE_FROM_CLASS (Klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (FooterClass, footer),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 0);
}

/**
*@brief initialize footer UI.
*@param f	Footer instance
*@return none
*/
static void footer_init (Footer *f) /* Instance Construction */
{
  PangoFontDescription* PangoFontDesc;
  //PangoLayout* PangoLayoud;
  
/*left image*/
    f->LeftEventBox = gtk_event_box_new (); 
    gtk_widget_show (GTK_WIDGET (f->LeftEventBox));
    gtk_event_box_set_visible_window (GTK_EVENT_BOX(f->LeftEventBox),FALSE);
    gtk_box_pack_start (GTK_BOX (f), GTK_WIDGET(f->LeftEventBox), FALSE, FALSE, BUTTON_PADDING);
    gtk_widget_set_events (GTK_EVENT_BOX (f->LeftEventBox), GDK_BUTTON_PRESS_MASK);
       

    f->LeftImage = gtk_image_new_from_file (PKGDATADIR"/icon_app_history.png");
    gtk_widget_show (GTK_WIDGET (f->LeftImage));
    gtk_container_add (f->LeftEventBox, f->LeftImage);

/*Label to show dbus message */
    f->CenterLabel = gtk_label_new("OpenMoko Taskmanager");
    gtk_widget_show (GTK_WIDGET (f->CenterLabel));
    gtk_widget_set_name (GTK_WIDGET (f->CenterLabel), "label_footer");
    gtk_misc_set_alignment (GTK_MISC (f->CenterLabel), LABEL_ALIGNMENT_X, LABEL_ALIGNMENT_Y);
    gtk_label_set_single_line_mode (f->CenterLabel, TRUE);
    if (PangoFontDesc = pango_font_description_from_string (FONT_STRING)){
	pango_font_description_set_size (PangoFontDesc, FONT_SIZE);
	gtk_widget_modify_font (GTK_WIDGET (f->CenterLabel), PangoFontDesc);
    }
    gtk_label_set_ellipsize (f->CenterLabel, PANGO_ELLIPSIZE_END);
    gtk_box_pack_start (GTK_BOX (f), GTK_WIDGET (f->CenterLabel), TRUE, TRUE, LABEL_PADDING);
    gtk_label_set_text (f->CenterLabel, "OpenMoko Task Manager");

/*right image*/
    f->RightEventBox = gtk_event_box_new (); 
    gtk_widget_show (f->RightEventBox);
    gtk_event_box_set_visible_window (GTK_EVENT_BOX(f->RightEventBox),FALSE);
    gtk_box_pack_end (GTK_BOX (f), GTK_WIDGET(f->RightEventBox), FALSE, FALSE, BUTTON_PADDING);
    gtk_widget_set_events (f->RightEventBox,GDK_BUTTON_PRESS_MASK);
       

    f->RightImage = gtk_image_new_from_file (PKGDATADIR"/icon_app_toggle.png");
    gtk_widget_show (GTK_WIDGET (f->RightImage));
    gtk_container_add (GTK_CONTAINER (f->RightEventBox), f->RightImage);

 
/*progressbar*/ 
/*
    f->progressbar = gtk_progress_bar_new();
    gtk_widget_show (f->progressbar);
    gtk_progress_bar_set_bar_style (f->progressbar, GTK_PROGRESS_CONTINUOUS);
    gtk_box_pack_start( f, GTK_WIDGET(f->progressbar), TRUE, TRUE, 0 );
    gtk_progress_bar_set_fraction (f->progressbar, 0.5);
    gtk_progress_bar_set_text (f->progressbar, "OpenMoko TaskManager");
    */

}

/**
*@brief create Footer widget object.
*@param none
*@return footer widget of GtkWidget type.
*/
GtkWidget* footer_new() /* Construction */
{
    return GTK_WIDGET(g_object_new(footer_get_type(), NULL));
}

/**
*@brief clear footer widget
*/
void footer_clear(Footer *f) /* Destruction */
{
     if (!f) g_free (f);
}

/**
*@brief set footer progressbar status.
*@param f	Footer reference
*@param s	string which is consist of status message and progressbar percent,
*			the string of message and percent is connected by symbol "@".
*@return none
*/
void footer_set_status(Footer *f, const char* s)
{
//    gtk_statusbar_push( f->statusbar, 1, s );
    gtk_label_set_text (f->CenterLabel, s);
    /*char message[128];
    char str_fraction[3];
    char* p_fraction;
    int StrLength;
    gdouble fraction;
    int i;
    
    strcpy(message,s);
    if(p_fraction = strrchr(s, '@'))
	StrLength = strlen(s)-strlen(p_fraction);
    else StrLength = strlen(s);

    memcpy(message,s,StrLength);
    message[StrLength] = '\0';
    for (i=0; i<4; i++)
         str_fraction[i] = s[StrLength+1+i];
    str_fraction[3] = '\0';
    
    fraction = atoi(str_fraction)/(double)100;
                                       
    g_print ("messsage is : %s\nthe char pointer is : %s\nlength of s and p_fraction: %d\nfraction is %lf:",message,p_fraction,StrLength,fraction );

    gtk_progress_bar_set_text (f->progressbar, message);

    if(fraction<=1 && fraction>=0)
         gtk_progress_bar_set_fraction (f->progressbar, fraction);
         */
         
}                   
