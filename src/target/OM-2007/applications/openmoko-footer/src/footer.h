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
//#define FONT_STRING ("-*-*bold-r-normal-*-120-*-*-*-*-iso8859-1")
#define FONT_STRING ""
#define FONT_SIZE 12*PANGO_SCALE
#define FONT_MAX_LENGTH 3

#define FOOTER_TYPE            (footer_get_type())
#define FOOTER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FOOTER_TYPE, Footer))
#define FOOTER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FOOTER_TYPE, FooterClass))
#define IS_FOOTER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FOOTER_TYPE))
#define IS_FOOTER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FOOTER_TYPE))

typedef struct _Footer       Footer;
typedef struct _FooterClass  FooterClass;

/**
 * @typedef OMFooterApp
 *
 * Opaque structure used for representing an Openmoko footer app UI.
 */ 
/*struct _Footer
{
    GtkHBox hbox;
    GtkButton* leftbutton;
    GtkWidget* leftbtnalign;
    GtkHBox* leftbtnhbox;
    GtkImage* leftbtnimage;
    GtkProgressBar* progressbar;
    GtkButton* rightbutton;
    GtkWidget* rightbtnalign;
    GtkHBox* rightbtnhbox;
    GtkImage* rightbtnimage;
};*/
struct _Footer
{
    GtkHBox hbox;
    
    GtkEventBox* LeftEventBox;
    GtkImage* LeftImage;
    GtkLabel* CenterLabel;
    GtkEventBox* RightEventBox;
    GtkImage* RightImage; 
};

struct _FooterClass
{
    GtkHBoxClass parent_class;
    
    void (*footer) (Footer *f);
};

GType          footer_get_type        (void);
//GtkWidget*     footer_new             (void);
void           footer_clear           (Footer *f);

void           footer_set_status      (Footer *f, const char* s);

G_END_DECLS

#endif /* OPENMOKO_FOOTER_H */
