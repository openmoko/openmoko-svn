/**
 * @file Footer.h
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
#ifndef OPENMOKO_FOOTER_H
#define OPENMOKO_FOOTER_H

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkprogressbar.h>
#include <gtk/gtkbutton.h>

G_BEGIN_DECLS

/*Footer property and position*/
#define FOOTER_PROPERTY_WIDTH 480
#define FOOTER_PROPERTY_HEIGHT 32
#define FOOTER_PROPERTY_X 0
#define FOOTER_PROPERTY_Y (640-FOOTER_PROPERTY_HEIGHT)
#define FOOTER_BUTTON_ICON_WIDTH 32 
#define FOOTER_BUTTON_ICON_HEIGHT 32
#define BUTTON_PADDING 15
#define LABEL_PADDING 25
#define LABEL_ALIGNMENT_X 0
#define LABEL_ALIGNMENT_Y 0.7

/*Pango Font spec*/
#define FONT_SIZE 12*PANGO_SCALE

#define TYPE_FOOTER            (footer_get_type())
#define FOOTER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), TYPE_FOOTER, Footer))
#define FOOTER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), TYPE_FOOTER, FooterClass))
#define IS_FOOTER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), TYPE_FOOTER))
#define IS_FOOTER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), TYPE_FOOTER))

typedef struct _Footer    Footer;
typedef struct _FooterClass    FooterClass;

struct _Footer
{
    GtkHBox hbox;

    GtkWidget *LeftEventBox;
    GtkWidget *LeftImage;
    GtkWidget *CenterLabel;
    GtkWidget *RightEventBox;
    GtkWidget *RightImage; 
    GtkWidget *ProgressBar;
};

struct _FooterClass
{
    GtkHBoxClass parent_class;
};

GType footer_get_type (void);
GtkWidget *footer_new (void);

void footer_set_status_message (Footer *f, const gchar *text);
void footer_set_status_progress (Footer *f, gdouble fraction);

G_END_DECLS

#endif /* OPENMOKO_FOOTER_H */
