/*  moko-dialog.c
 *
 *  Authored (in part) by Rob Bradford <rob@openedhand.com>
 *
 *  Copyright (C) 2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Also contains code directly derived from GTK+ (gtk/gtkdialog.c) with the
 *  following Copyright notice:
 *
 *  Copyright (C) 1995-1997 Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the
 *  Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 *
 *  Modified by the GTK+ Team and others 1997-2000.  See the AUTHORS
 *  file for a list of people on the GTK+ Team.  See the ChangeLog
 *  files for a list of changes.  These files are distributed with
 *  GTK+ at ftp://ftp.gtk.org/pub/gtk/. 
 *
 *  Derivation Copyright (C) 2007 OpenMoko Inc.
 *  Derivation Authored by Rob Bradford <rob@openedhand.com?
 */

#include "moko-dialog.h"
#include "moko-pixmap-button.h"
#include "moko-application.h"

#include <gtk/gtk.h>

G_DEFINE_TYPE (MokoDialog, moko_dialog, MOKO_TYPE_WINDOW)

#define DIALOG_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_DIALOG, MokoDialogPrivate))

typedef struct _MokoDialogPrivate MokoDialogPrivate;

struct _MokoDialogPrivate
{
  GtkWidget *hbox;
  GtkWidget *label;
  GtkWidget *eventbox;
  GtkWidget *closebutton;

  gint response_id;
  GMainLoop *loop;
  gboolean destroyed;
};

enum {
  RESPONSE,
  CLOSE,
  LAST_SIGNAL
};

static guint moko_dialog_signals[LAST_SIGNAL];

static void
moko_dialog_get_property (GObject *object, guint property_id,
                              GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
moko_dialog_set_property (GObject *object, guint property_id,
                              const GValue *value, GParamSpec *pspec)
{
  switch (property_id) {
  default:
    G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
  }
}

static void
moko_dialog_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (moko_dialog_parent_class)->dispose)
    G_OBJECT_CLASS (moko_dialog_parent_class)->dispose (object);
}

static void
moko_dialog_finalize (GObject *object)
{
  G_OBJECT_CLASS (moko_dialog_parent_class)->finalize (object);
}

static void
moko_dialog_class_init (MokoDialogClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  g_type_class_add_private (klass, sizeof (MokoDialogPrivate));

  object_class->get_property = moko_dialog_get_property;
  object_class->set_property = moko_dialog_set_property;
  object_class->dispose = moko_dialog_dispose;
  object_class->finalize = moko_dialog_finalize;

  moko_dialog_signals[RESPONSE] = g_signal_new (("response"),
      G_OBJECT_CLASS_TYPE (klass),
      G_SIGNAL_RUN_LAST,
      G_STRUCT_OFFSET (MokoDialogClass, response),
      NULL, NULL,
      g_cclosure_marshal_VOID__INT,
      G_TYPE_NONE, 1,
      G_TYPE_INT);
}

static void
moko_dialog_closebutton_cb (GtkButton *button, gpointer user_data)
{
  MokoDialog *self = MOKO_DIALOG (user_data);

  moko_dialog_response (self, GTK_RESPONSE_DELETE_EVENT);
}

static void
moko_dialog_init (MokoDialog *self)
{
  MokoDialogPrivate* priv = DIALOG_PRIVATE(self);
  GtkWidget* parent;

  /* Most of this was borrowed from GTK */

  /**
   * Set up a "Title Bar" - this should really be done in the window manager
   * theme...
   */

  /* The primary vbox holds the contents of the dialog and the action_area */
  self->vbox = gtk_vbox_new (FALSE, 0);
  gtk_container_add (GTK_CONTAINER (self), self->vbox);

  /* Create the hbox at the top */
  priv->hbox = gtk_hbox_new(FALSE, 0);

  /* Add an eventbox to said hbox */
  priv->eventbox = gtk_event_box_new ();
  gtk_box_pack_start (GTK_BOX(priv->hbox), priv->eventbox, TRUE, TRUE, 0);
  gtk_widget_set_name( priv->eventbox, "mokodialogwindow-title-labelbox" );

  /* Create the label for the top */
  priv->label = gtk_label_new(NULL);
  gtk_widget_set_name (priv->label, "mokodialogwindow-title-label");

  /* Add to the eventbox */
  gtk_container_add (GTK_CONTAINER(priv->eventbox), priv->label);

  /* Add close button */
  priv->closebutton = moko_pixmap_button_new ();
  g_signal_connect (G_OBJECT(priv->closebutton), "clicked", G_CALLBACK(moko_dialog_closebutton_cb), self);
  gtk_widget_set_name(priv->closebutton, "mokodialogwindow-closebutton");

  gtk_box_pack_start (GTK_BOX(priv->hbox), priv->closebutton, FALSE, FALSE, 0);

  /* Add this hbox to the start of vbox */
  gtk_box_pack_start (GTK_BOX (self->vbox), priv->hbox, FALSE, FALSE, 0 );

  /**
   * Now get back to the proper parts of the dialog
   */

  /* Create the action_area and put it into the main vbox */
  self->action_area = gtk_hbutton_box_new ();
  gtk_button_box_set_layout (GTK_BUTTON_BOX (self->action_area),
      GTK_BUTTONBOX_END);
  gtk_box_pack_start (GTK_BOX (self->vbox), self->action_area,
      FALSE, TRUE, 0);

  /* Mark it as a dialog window */
  gtk_window_set_type_hint (GTK_WINDOW (self), GDK_WINDOW_TYPE_HINT_DIALOG);

  /* Center on parent?, yessir */
  gtk_window_set_position (GTK_WINDOW (self), GTK_WIN_POS_CENTER_ON_PARENT);


  /* Setup the relationship between this window and its parent */
  parent = moko_application_get_main_window(moko_application_get_instance());

  if (parent)
  {
      gtk_window_set_transient_for(GTK_WINDOW(self), GTK_WINDOW (parent) );
      gtk_window_set_modal(GTK_WINDOW(self), TRUE );
      gtk_window_set_destroy_with_parent(GTK_WINDOW(self), TRUE );
  }


  gtk_widget_show_all (GTK_WIDGET (self->vbox));
}

void moko_dialog_set_title (MokoDialog* self, const gchar* title)
{
    MokoDialogPrivate* priv = DIALOG_PRIVATE(self);
    
    gtk_label_set_text (GTK_LABEL (priv->label), title);
    gtk_window_set_title (GTK_WINDOW(self), title);
}

static void
button_clicked_cb (GtkButton *button, gpointer *user_data)
{
  MokoDialog *self = MOKO_DIALOG (user_data);
  gint response_id;

  response_id = moko_dialog_get_button_response_id (self, button);

  moko_dialog_response (self, response_id);
}

static void
response_id_free_cb (gpointer data)
{
  g_slice_free (gint, data);
}

void
moko_dialog_set_button_response_id (MokoDialog *self, GtkButton *button, gint response_id)
{
  gint* data = g_slice_new (gint);
  
  *data = response_id;

  g_object_set_data_full (G_OBJECT (button),
      "moko-dialog-response-id",
      data,
      response_id_free_cb);
}

gint
moko_dialog_get_button_response_id (MokoDialog *self, GtkButton *button)
{
  gint *data = NULL;

  data = g_object_get_data (G_OBJECT (button), "moko-dialog-response-id");

  if (data == NULL)
    return GTK_RESPONSE_NONE;
  else
    return *data;
}

void
moko_dialog_add_button_widget (MokoDialog *self, GtkButton *button, gint response_id)
{
  gint cur_response_id;

  cur_response_id = moko_dialog_get_button_response_id (self, button);

  if (cur_response_id == GTK_RESPONSE_NONE)
    moko_dialog_set_button_response_id (self, button, response_id);

  g_signal_connect (button, "clicked", (GCallback)button_clicked_cb, self);

  gtk_box_pack_end (GTK_BOX (self->action_area),
      GTK_WIDGET (button), FALSE, TRUE, 0);
}

GtkWidget*
moko_dialog_add_button (MokoDialog *self, const gchar *text, gint response_id)
{
  GtkWidget *button;

  button = gtk_button_new_from_stock (text);
  moko_dialog_add_button_widget (self, GTK_BUTTON (button), response_id);
  gtk_widget_show (button);

  return button;
}

GtkWidget*
moko_dialog_add_button_secondary (MokoDialog *self, gchar *text, gint response_id)
{
  GtkWidget *button;

  button = gtk_button_new_from_stock (text);
  moko_dialog_add_button_widget (self, GTK_BUTTON (button), response_id);
  gtk_widget_show (button);
  moko_dialog_set_button_secondary (self, GTK_BUTTON (button), TRUE);

  return button;
}

void
moko_dialog_set_button_secondary (MokoDialog *self, GtkButton *button, gboolean is_secondary)
{
  gtk_button_box_set_child_secondary (GTK_BUTTON_BOX (self->action_area), GTK_WIDGET (button), is_secondary);
}

gboolean
moko_dialog_get_button_secondary (MokoDialog *self, GtkButton *button)
{
  return gtk_button_box_get_child_secondary (GTK_BUTTON_BOX (self->action_area), GTK_WIDGET (button));
}


void
moko_dialog_add_buttons (MokoDialog *self, gchar *first_button_text, ...)
{
  va_list args;
  const gchar* text;
  gint response_id;

  va_start (args, first_button_text);

  if (first_button_text == NULL)
    return;

  text = first_button_text;
  response_id = va_arg (args, gint);

  while (text != NULL)
    {
      moko_dialog_add_button (self, text, response_id);

      text = va_arg (args, gchar*);

      if (text == NULL)
        break;

      response_id = va_arg (args, int);
    }

  va_end (args);
}


void
moko_dialog_response (MokoDialog *self, gint response_id)
{
  g_signal_emit (self, moko_dialog_signals[RESPONSE], 0, response_id);
}

GtkWidget*
moko_dialog_new (void)
{
  return g_object_new (MOKO_TYPE_DIALOG, NULL);
}

/* Check all buttons have the correct style */
static void
check_button_style_cb (GtkWidget *widget, GtkButtonBox *box)
{
     if (GTK_IS_BUTTON (widget)
        && gtk_button_box_get_child_secondary (GTK_BUTTON_BOX (box), widget))
      gtk_widget_set_name (GTK_WIDGET (widget), "mokostylusbutton-white");
    else
      gtk_widget_set_name (GTK_WIDGET (widget), "mokostylusbutton-black");
}

static void
check_button_styles (MokoDialog *self)
{
  gtk_container_forall (GTK_CONTAINER (self->action_area), (GtkCallback)
      (check_button_style_cb), self->action_area);
#if 0

  GList *children, *l;
  /*
   * the button box doesn't seem to return secondary children from
   * gtk_container_get_children
   *
   * maybe a gtk+ bug?
   */

  children = gtk_container_get_children (GTK_CONTAINER (self->action_area));

  for (l = children; (l = g_list_next (l)); )
  {

    if (GTK_IS_BUTTON (l->data)
        && gtk_button_box_get_child_secondary (GTK_BUTTON_BOX (self->action_area), l->data))
      gtk_widget_set_name (GTK_WIDGET (l->data), "mokostylusbutton-white");
    else
      gtk_widget_set_name (GTK_WIDGET (l->data), "mokostylusbutton-black");

    if (GTK_IS_CONTAINER (l->data))
      l = g_list_concat (l, gtk_container_get_children (GTK_CONTAINER (l->data)));
  }
#endif
}

/* Code beyond this point directly derived from gtk+ (gtkdialog.c) */
static void
shutdown_loop (MokoDialog *self)
{
  MokoDialogPrivate *priv = DIALOG_PRIVATE (self);

  if (g_main_loop_is_running (priv->loop))
    g_main_loop_quit (priv->loop);
}

static void
run_unmap_handler (MokoDialog *self, gpointer data)
{
  shutdown_loop (self);
}

static void
run_response_handler (MokoDialog *self, gint response_id, gpointer data)
{
  MokoDialogPrivate *priv = DIALOG_PRIVATE (self);
  priv->response_id = response_id;

  shutdown_loop (self);
}

static gint
run_delete_handler (MokoDialog *self, GdkEventAny *event, gpointer data)
{
  shutdown_loop (self);

  return TRUE; /* Do not destroy */
}

static void
run_destroy_handler (MokoDialog *self, gpointer data)
{
  MokoDialogPrivate *priv = DIALOG_PRIVATE (self);
  /* shutdown_loop will be called by run_unmap_handler */
  
  priv->destroyed = TRUE;
}

gint
moko_dialog_run (MokoDialog *self)
{
  MokoDialogPrivate *priv = DIALOG_PRIVATE (self);
  priv->response_id = GTK_RESPONSE_NONE;
  priv->loop = NULL;
  priv->destroyed = FALSE;

  gboolean was_modal;
  gulong response_handler;
  gulong unmap_handler;
  gulong destroy_handler;
  gulong delete_handler;

  g_object_ref (self);

  /* check we have the correct styles for OpenMoko */
  check_button_styles (self);

  was_modal = GTK_WINDOW (self)->modal;
  if (!was_modal)
    gtk_window_set_modal (GTK_WINDOW (self), TRUE);

  if (!GTK_WIDGET_VISIBLE (self))
    gtk_widget_show (GTK_WIDGET (self));
  
  response_handler =
    g_signal_connect (self,
                      "response",
                      G_CALLBACK (run_response_handler),
                      NULL);
  
  unmap_handler =
    g_signal_connect (self,
                      "unmap",
                      G_CALLBACK (run_unmap_handler),
                      NULL);
  
  delete_handler =
    g_signal_connect (self,
                      "delete-event",
                      G_CALLBACK (run_delete_handler),
                      NULL);
  
  destroy_handler =
    g_signal_connect (self,
                      "destroy",
                      G_CALLBACK (run_destroy_handler),
                      NULL);

  priv->loop = g_main_loop_new (NULL, FALSE);

  GDK_THREADS_LEAVE ();  
  g_main_loop_run (priv->loop);
  GDK_THREADS_ENTER ();  

  g_main_loop_unref (priv->loop);

  priv->loop = NULL;
  
  if (!priv->destroyed)
    {
      if (!was_modal)
        gtk_window_set_modal (GTK_WINDOW(self), FALSE);
      
      g_signal_handler_disconnect (self, response_handler);
      g_signal_handler_disconnect (self, unmap_handler);
      g_signal_handler_disconnect (self, delete_handler);
      g_signal_handler_disconnect (self, destroy_handler);
    }

  g_object_unref (self);

  return priv->response_id;
}

