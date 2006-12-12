/* moko-dialer-tip.h
 *  MokoDialerTip, for the autofill feature, this widget shows the current hints for the end user.
 *  Authored By Tony Guan<tonyguan@fic-sh.com.cn>
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
 #ifndef _MOKO_DIALER_TIP_H_
#define _MOKO_DIALER_TIP_H_




#include <gdk/gdk.h>
#include <gtk/gtkvbox.h>
#include <glib-object.h>
//#include <gtk/gtktable.h>
#include <gtk/gtkobject.h>
#include <gtk/gtksignal.h>
#include <gtk/gtklabel.h>
 #include <gtk/gtkeventbox.h>

G_BEGIN_DECLS


#define MOKO_TYPE_DIALER_TIP                (moko_dialer_tip_get_type())
#define MOKO_DIALER_TIP (obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_DIALER_TIP, MokoDialerTip))
#define MOKO_DIALER_TIP_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass),MOKO_TYPE_DIALER_TIP,MokoDialerTipClass))
#define MOKO_IS_DIALER_TIP(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_DIALER_TIP))
#define MOKO_IS_DIALER_TIP_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_DIALER_TIP))
#define MOKO_DIALER_TIP_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), MOKO_TYPE_DIALER_TIP, MokoDialerTipClass))



typedef struct _MokoDialerTipClass   MokoDialerTipClass;

typedef struct _MokoDialerTip        MokoDialerTip;

struct _MokoDialerTip
{
 GtkEventBox eventbox;
};

struct _MokoDialerTipClass
{
  GtkEventBoxClass parent_class;
  void (* moko_dialer_tip_input) (MokoDialerTip *moko_dialer_tip,gchar parac);
  void (* moko_dialer_tip_hold) (MokoDialerTip *moko_dialer_tip,gchar parac);
};


GType          moko_dialer_tip_get_type         (void) ;

GtkWidget*      moko_dialer_tip_new();

GtkWidget*      moko_dialer_tip_new_with_label_and_index(const gchar * stringname,const gint index);

gint moko_dialer_tip_get_index(MokoDialerTip* tip);
gboolean moko_dialer_tip_set_label(GtkWidget* widget,const gchar * stringname);
gboolean moko_dialer_tip_set_index(GtkWidget* widget,const gint index);

G_END_DECLS

#endif // 





