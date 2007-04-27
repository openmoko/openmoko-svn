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

G_DEFINE_TYPE (Footer, footer, GTK_TYPE_HBOX);

#define FOOTER_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_FOOTER, FooterPrivate))

typedef struct _FooterPrivate FooterPrivate;

struct _FooterPrivate
{
};

static void
footer_class_init (FooterClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (FooterPrivate));
}

static void footer_init (Footer *f) /* Instance Construction */
{
    PangoFontDescription *PangoFont = pango_font_description_new(); //get system default PangoFontDesc

    f->LeftEventBox = gtk_event_box_new (); 
    gtk_widget_show (GTK_WIDGET (f->LeftEventBox));
    gtk_event_box_set_visible_window (GTK_EVENT_BOX(f->LeftEventBox),FALSE);
    gtk_box_pack_start (GTK_BOX (f), GTK_WIDGET(f->LeftEventBox), FALSE, FALSE, BUTTON_PADDING);
    gtk_widget_set_events (GTK_WIDGET (f->LeftEventBox), GDK_BUTTON_PRESS_MASK);

    f->LeftImage = gtk_image_new_from_file (PKGDATADIR"/icon_app_history.png");
    gtk_widget_show (GTK_WIDGET (f->LeftImage));
    gtk_container_add ( GTK_CONTAINER (f->LeftEventBox), f->LeftImage);

/*
    f->CenterLabel = gtk_label_new("OpenMoko Taskmanager");
    gtk_widget_show (GTK_WIDGET (f->CenterLabel));
    gtk_widget_set_name (GTK_WIDGET (f->CenterLabel), "label_footer");
    gtk_misc_set_alignment (GTK_MISC (f->CenterLabel), LABEL_ALIGNMENT_X, LABEL_ALIGNMENT_Y);
    gtk_label_set_single_line_mode (GTK_LABEL (f->CenterLabel), TRUE);

    if (PangoFont){
        pango_font_description_set_size (PangoFont, FONT_SIZE);
        gtk_widget_modify_font (GTK_WIDGET (f->CenterLabel), PangoFont);
    }

    gtk_label_set_ellipsize (GTK_LABEL (f->CenterLabel), PANGO_ELLIPSIZE_END);
    gtk_box_pack_start (GTK_BOX (f), GTK_WIDGET (f->CenterLabel), TRUE, TRUE, LABEL_PADDING);
    gtk_label_set_text (GTK_LABEL (f->CenterLabel), "OpenMoko Task Manager");
*/

    f->ProgressBar = gtk_progress_bar_new();
    gtk_widget_show (f->ProgressBar);
    gtk_progress_bar_set_bar_style (f->ProgressBar, GTK_PROGRESS_CONTINUOUS);
    gtk_box_pack_start( f, GTK_WIDGET(f->ProgressBar), TRUE, TRUE, 0);
    gtk_progress_bar_set_fraction (f->ProgressBar, 0);
    gtk_progress_bar_set_text (f->ProgressBar, "OpenMoko TaskManager");
    gtk_progress_bar_set_ellipsize (GTK_PROGRESS_BAR (f->ProgressBar), PANGO_ELLIPSIZE_END);

    f->RightEventBox = gtk_event_box_new (); 
    gtk_widget_show (f->RightEventBox);
    gtk_event_box_set_visible_window (GTK_EVENT_BOX(f->RightEventBox),FALSE);
    gtk_box_pack_end (GTK_BOX (f), GTK_WIDGET(f->RightEventBox), FALSE, FALSE, BUTTON_PADDING);
    gtk_widget_set_events (f->RightEventBox,GDK_BUTTON_PRESS_MASK);

    f->RightImage = gtk_image_new_from_file (PKGDATADIR"/icon_app_toggle.png");
    gtk_widget_show (GTK_WIDGET (f->RightImage));
    gtk_container_add (GTK_CONTAINER (f->RightEventBox), f->RightImage);
}


GtkWidget* footer_new()
{
    return GTK_WIDGET(g_object_new(TYPE_FOOTER, NULL));
}

void footer_set_status_message (Footer *f, const gchar *text)
{
  gtk_progress_bar_set_text (GTK_PROGRESS_BAR (f->ProgressBar), text);
}

void footer_set_status_progress (Footer *f, gdouble fraction)
{
  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (f->ProgressBar), fraction);
}
