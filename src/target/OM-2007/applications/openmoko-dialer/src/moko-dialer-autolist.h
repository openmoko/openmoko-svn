/* moko-dialer-autolist.h
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
 #ifndef _MOKO_DIALER_AUTOLIST_H_
#define _MOKO_DIALER_AUTOLIST_H_




#include <gdk/gdk.h>
#include <gtk/gtkhbox.h>
#include <glib-object.h>
#include <gtk/gtktable.h>
#include <gtk/gtkobject.h>
#include <gtk/gtksignal.h>
#include <gtk/gtktextview.h>
#include "moko-dialer-tip.h"
#include "moko-dialer-includes.h"
G_BEGIN_DECLS



#define MOKO_TYPE_DIALER_AUTOLIST                (moko_dialer_autolist_get_type())
#define MOKO_DIALER_AUTOLIST (obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_DIALER_AUTOLIST, MokoDialerAutolist))
#define MOKO_DIALER_AUTOLIST_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass),MOKO_TYPE_DIALER_AUTOLIST,MokoDialerAutolistClass))
#define MOKO_IS_DIALER_AUTOLIST(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_DIALER_AUTOLIST))
#define MOKO_IS_DIALER_AUTOLIST_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_DIALER_AUTOLIST))
#define MOKO_DIALER_AUTOLIST_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), MOKO_TYPE_DIALER_AUTOLIST, MokoDialerAutolistClass))



typedef struct _MokoDialerAutolistClass   MokoDialerAutolistClass;

typedef struct _MokoDialerAutolist        MokoDialerAutolist;
struct _MokoDialerAutolist
{
  GtkHBox hbox;
  DIALER_CONTACTS_LIST_HEAD* head;

  HISTORY_ENTRY * g_currentselected; ///<pointer to the history entry which in the GUI the user selects.

//static PangoFontDescription *font_desc=NULL; ///<the PangoFontDescription which a lot of widget will use it.

 DIALER_READY_CONTACT readycontacts[MOKO_DIALER_MAX_TIPS]; ///<the prepared contact list which will display to the user when he/she inputs part of the digits he/she wants to dial out

 gint g_selected;///<indicates the offset of the selected ready contacts list

 gint g_alternatecount;///<indicates how many alternative is ready in the ready list array.


  GtkWidget *tips[MOKO_DIALER_MAX_TIPS];

};

struct _MokoDialerAutolistClass
{
  GtkHBoxClass parent_class;
  void (* moko_dialer_autolist_selected) (MokoDialerAutolist *moko_dialer_autolist,gchar parac);
  void (* moko_dialer_autolist_confirmed) (MokoDialerAutolist *moko_dialer_autolist,gchar parac);
};


GType          moko_dialer_autolist_get_type         (void) ;

GtkWidget*      moko_dialer_autolist_new();




G_END_DECLS

#endif // 




