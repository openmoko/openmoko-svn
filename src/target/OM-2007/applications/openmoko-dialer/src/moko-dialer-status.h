/* moko-dialer-status.h
 *  to display the person name, picuter,number. etc.
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
#ifndef _MOKO_DIALER_STATUS_H_
#define _MOKO_DIALER_STATUS_H_




#include <gdk/gdk.h>
#include <gtk/gtkhbox.h>
#include <glib-object.h>
#include <gtk/gtktable.h>
#include <gtk/gtkobject.h>
#include <gtk/gtksignal.h>
#include <gtk/gtktextview.h>
#include "moko-dialer-tip.h"
#include "moko-dialer-declares.h"
#include "contacts.h"

//#include "moko-dialer-includes.h"

G_BEGIN_DECLS
#define MOKO_TYPE_DIALER_STATUS                (moko_dialer_status_get_type())
#define MOKO_DIALER_STATUS(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_DIALER_STATUS, MokoDialerStatus))
#define MOKO_DIALER_STATUS_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass),MOKO_TYPE_DIALER_STATUS,MokoDialerStatusClass))
#define MOKO_IS_DIALER_STATUS(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_DIALER_STATUS))
#define MOKO_IS_DIALER_STATUS_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_DIALER_STATUS))
#define MOKO_DIALER_STATUS_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), MOKO_TYPE_DIALER_STATUS, MokoDialerStatusClass))
typedef struct _MokoDialerStatusClass MokoDialerStatusClass;

typedef struct _MokoDialerStatus MokoDialerStatus;
struct _MokoDialerStatus
{
  GtkVBox vbox;
//upper section
  GtkWidget *labelStatusTitle;  ///<the topmost title bar of the status
  GtkWidget *icon;
  GdkPixbuf *iconStatus[MOKO_DIALER_MAX_STATUS_ICONS];
  GdkPixbuf *iconError;
  GdkPixbuf *iconSuccess;
//lower section  
  GtkWidget *imagePerson;       ///<the image of the person we care
  GtkWidget *labelStatus;       ///<the status label
  GtkWidget *labelPersonName;   ///<the person name
  GtkWidget *labelNumber;       ///<the number of the person

//private section
  gint number_of_the_icons;


};

struct _MokoDialerStatusClass
{
  GtkVBoxClass parent_class;
};


GType moko_dialer_status_get_type (void);

GtkWidget *moko_dialer_status_new ();

void moko_dialer_status_set_title_label (MokoDialerStatus *
                                         moko_dialer_status,
                                         const gchar * text);
void moko_dialer_status_set_person_name (MokoDialerStatus *
                                         moko_dialer_status,
                                         const gchar * text);
void moko_dialer_status_set_person_number (MokoDialerStatus *
                                           moko_dialer_status,
                                           const gchar * text);
void moko_dialer_status_set_person_image (MokoDialerStatus *
                                          moko_dialer_status,
                                          const gchar * id);
void moko_dialer_status_set_icons (MokoDialerStatus * moko_dialer_status,
                                   const gchar * text);
void moko_dialer_status_set_status_label (MokoDialerStatus *
                                          moko_dialer_status,
                                          const gchar * text);

void moko_dialer_status_set_error (MokoDialerStatus * moko_dialer_status);
void moko_dialer_status_update_icon (MokoDialerStatus * moko_dialer_status);
int moko_dialer_status_add_status_icon (MokoDialerStatus * moko_dialer_status,
                                        const gchar * text);
void moko_dialer_status_set_icon_by_index (MokoDialerStatus * moko_dialer_status,
                                           gint index);


G_END_DECLS
#endif //
