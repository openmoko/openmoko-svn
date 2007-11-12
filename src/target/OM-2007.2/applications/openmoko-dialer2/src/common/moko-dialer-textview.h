/* moko-dialer-textview.h
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
#ifndef _MOKO_DIALER_TEXTVIEW_H_
#define _MOKO_DIALER_TEXTVIEW_H_




#include <gdk/gdk.h>
#include <gtk/gtkvbox.h>
#include <glib-object.h>
#include <gtk/gtktable.h>
#include <gtk/gtkobject.h>
#include <gtk/gtksignal.h>
#include <gtk/gtktextview.h>

G_BEGIN_DECLS
#define MOKO_TYPE_DIALER_TEXTVIEW                (moko_dialer_textview_get_type())
#define MOKO_DIALER_TEXTVIEW(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_DIALER_TEXTVIEW, MokoDialerTextview))
#define MOKO_DIALER_TEXTVIEW_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass),MOKO_TYPE_DIALER_TEXTVIEW,MokoDialerTextviewClass))
#define MOKO_IS_DIALER_TEXTVIEW(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_DIALER_TEXTVIEW))
#define MOKO_IS_DIALER_TEXTVIEW_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_DIALER_TEXTVIEW))
#define MOKO_DIALER_TEXTVIEW_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), MOKO_TYPE_DIALER_TEXTVIEW, MokoDialerTextviewClass))
typedef struct _MokoDialerTextviewClass MokoDialerTextviewClass;

typedef struct _MokoDialerTextview MokoDialerTextview;

struct _MokoDialerTextview
{
  GtkTextView textview;         /* the main widget */
  PangoFontDescription *font_desc_textview;     /* the font description of this textview */
  GtkTextTag *tag_for_inputed;  /* the formating tag for the digits user already inputed */
  // GtkTextTag *tag_for_cursor;   /* the formatting tag  for the right digit user just inputed. */
  // GtkTextTag *tag_for_autofilled; /* the formatting tag for the autofilled digits if any. */
  gboolean sensed;
};

struct _MokoDialerTextviewClass
{
  GtkTextViewClass parent_class;
};


GType moko_dialer_textview_get_type (void);

GtkWidget *moko_dialer_textview_new ();

int moko_dialer_textview_insert (MokoDialerTextview * moko_dialer_textview,
                                 const gchar * number);

gchar *moko_dialer_textview_get_input (MokoDialerTextview *
                                       moko_dialer_textview,
                                       gboolean all_text);
int moko_dialer_textview_empty (MokoDialerTextview * moko_dialer_textview);
int moko_dialer_textview_fill_it (MokoDialerTextview * moko_dialer_textview,
                                  gchar * string);
int moko_dialer_textview_delete (MokoDialerTextview * moko_dialer_textview);
void moko_dialer_textview_set_color (MokoDialerTextview *
                                     moko_dialer_textview);
gint moko_dialer_textview_confirm_it (MokoDialerTextview *
                                      moko_dialer_textview,
                                      const gchar * string);

G_END_DECLS
#endif
