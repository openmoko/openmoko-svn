/*
 *  sms-dialog-window.c
 *  
 *  Authored By Alex Tang <alex@fic-sh.com.cn>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *  
 *  Current Version: $Rev$ ($Date: 2006/10/05 17:38:14 $) [$Author: alex $]
 */

#include "sms-dialog-window.h"
#include "main.h"
#include <libmokoui/moko-pixmap-button.h>
#include <libmokoui/moko-fixed.h>

#include <gtk/gtkeventbox.h>
#include <gtk/gtkdialog.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtktextview.h>
#include <gtk/gtkscrolledwindow.h>
#include <gtk/gtkviewport.h>

#include <pango/pango-font.h>

#include <glib/gmain.h>

G_DEFINE_TYPE (SmsDialogWindow, sms_dialog_window, MOKO_TYPE_WINDOW)

#define SMS_DIALOG_WINDOW_GET_PRIVATE(o)   (G_TYPE_INSTANCE_GET_PRIVATE ((o), SMS_TYPE_DIALOG_WINDOW, SmsDialogWindowPrivate))

typedef struct _SmsDialogWindowPrivate SmsDialogWindowPrivate;

struct _SmsDialogWindowPrivate
  {
    GtkWidget* vbox;
    GtkWidget* hbox;
    GtkWidget* eventbox;
    GtkWidget* titleLabel;
    GtkWidget* textInLabel;
    GtkWidget* toolbox;
    GtkWidget* closebutton;
    GtkWidget* txtView;
  };

typedef struct _SmsDialogRunInfo
  {
    SmsDialogWindow *dialog;
    gint response_id;
    GMainLoop *loop;
    gboolean destroyed;
  }
SmsDialogRunInfo;

static void sms_dialog_window_close(SmsDialogWindow* self);
gboolean on_sms_txtView_key_release_event       (GtkWidget       *widget,
    GdkEventKey     *event,
    SmsDialogWindow *self);


static void
shutdown_loop (SmsDialogRunInfo *ri)
{
  if (g_main_loop_is_running (ri->loop))
    g_main_loop_quit (ri->loop);
}

static void
run_unmap_handler (SmsDialogWindow* dialog, gpointer data)
{
  SmsDialogRunInfo *ri = data;

  shutdown_loop (ri);
}

static void
run_response_handler (SmsDialogWindow* dialog,
                      gint response_id,
                      gpointer data)
{
  SmsDialogRunInfo *ri;

  ri = data;

  ri->response_id = response_id;

  shutdown_loop (ri);
}

static gint
run_delete_handler (SmsDialogWindow* dialog,
                    GdkEventAny *event,
                    gpointer data)
{
  SmsDialogRunInfo *ri = data;

  shutdown_loop (ri);

  return TRUE; /* Do not destroy */
}

static void
run_destroy_handler (SmsDialogWindow* dialog, gpointer data)
{
  SmsDialogRunInfo *ri = data;

  /* shutdown_loop will be called by run_unmap_handler */

  ri->destroyed = TRUE;
}


static void
sms_dialog_window_dispose(GObject* object)
{
  if (G_OBJECT_CLASS (sms_dialog_window_parent_class)->dispose)
    G_OBJECT_CLASS (sms_dialog_window_parent_class)->dispose (object);
}

static void
sms_dialog_window_finalize(GObject* object)
{
  G_OBJECT_CLASS (sms_dialog_window_parent_class)->finalize (object);
}

static void
sms_dialog_window_class_init(SmsDialogWindowClass* klass)
{
  GObjectClass* object_class = G_OBJECT_CLASS(klass);

  g_type_class_add_private (klass, sizeof(SmsDialogWindowPrivate));

  object_class->dispose = sms_dialog_window_dispose;
  object_class->finalize = sms_dialog_window_finalize;
}

SmsDialogWindow*
sms_dialog_window_new(void)
{
  return g_object_new(SMS_TYPE_DIALOG_WINDOW, NULL);
}

static void
sms_dialog_window_init(SmsDialogWindow* self)
{
  MokoWindow* parent = (MokoWindow*)moko_application_get_main_window( moko_application_get_instance() );
  if ( parent )
    {
      gtk_window_set_transient_for( GTK_WINDOW(self), GTK_WINDOW(parent) );
#ifndef DEBUG_THIS_FILE
      gtk_window_set_modal( GTK_WINDOW(self), TRUE );
#endif
      gtk_window_set_destroy_with_parent( GTK_WINDOW(self), TRUE );
    }
}

void sms_dialog_window_set_title(SmsDialogWindow* self, const gchar* title)
{
  SmsDialogWindowPrivate* priv = SMS_DIALOG_WINDOW_GET_PRIVATE(self);
  if ( !priv->titleLabel )
    {
      priv->titleLabel = gtk_label_new( title );
      priv->hbox = gtk_hbox_new( FALSE, 0 );
      gtk_window_set_title( GTK_WINDOW(self), title );
      gtk_widget_set_name( GTK_WIDGET(priv->titleLabel), "mokodialogwindow-title-label" );
      gtk_box_pack_start( GTK_BOX(priv->hbox), GTK_WIDGET(priv->titleLabel), TRUE, TRUE, 0 );
      PangoFontDescription* font_desc = pango_font_description_from_string ("Bold 10");
      priv->textInLabel = gtk_label_new( "160(1)" );
      gtk_widget_modify_font (priv->textInLabel, font_desc);
      gtk_widget_set_name( GTK_WIDGET(priv->textInLabel), "mokodialogwindow-title-label" );
      gtk_box_pack_start( GTK_BOX(priv->hbox), GTK_WIDGET(priv->textInLabel), FALSE, FALSE, 0);
      priv->eventbox = gtk_event_box_new();
      gtk_container_add( GTK_CONTAINER(priv->eventbox), GTK_WIDGET(priv->hbox) );
      gtk_widget_set_name( GTK_WIDGET(priv->eventbox), "mokodialogwindow-title-labelbox" );
      //FIXME get from theme
      gtk_misc_set_padding( GTK_MISC(priv->titleLabel), 0, 6 );
      gtk_widget_show( GTK_WIDGET(priv->titleLabel) );
      gtk_widget_show( GTK_WIDGET(priv->eventbox) );
    }
  else
    {
      gtk_label_set_text( GTK_LABEL(priv->titleLabel), title );
      gtk_window_set_title( GTK_WINDOW(self), title );
    }
  if ( !priv->vbox )
    {
      GtkWidget* image;
      GtkWidget* smsSendBtn;
      GtkWidget* emailBtn;

      priv->vbox = gtk_vbox_new( FALSE, 0 );
      gtk_box_pack_start( GTK_BOX(priv->vbox), GTK_WIDGET(priv->eventbox), FALSE, FALSE, 0 );

      //Add toolbox
      priv->hbox = gtk_hbox_new( FALSE, 0 );
      priv->toolbox = moko_tool_box_new();
      GtkWidget* btnBox = moko_tool_box_get_button_box (MOKO_TOOL_BOX(priv->toolbox));
      priv->closebutton = moko_pixmap_button_new();
      image = gtk_image_new_from_file (PKGDATADIR "/Cancel.png");
      moko_pixmap_button_set_center_image ( MOKO_PIXMAP_BUTTON(priv->closebutton),image);
      gtk_widget_set_name( GTK_WIDGET(priv->closebutton), "mokostylusbutton-black" );
      gtk_box_pack_end (GTK_BOX(btnBox),priv->closebutton,FALSE,FALSE,280);
      g_signal_connect_swapped( G_OBJECT(priv->closebutton), "clicked", G_CALLBACK(sms_dialog_window_close), self );

      self->addressBtn = moko_tool_box_add_action_button (MOKO_TOOL_BOX(priv->toolbox));
      gtk_widget_set_name( GTK_WIDGET(self->addressBtn ), "mokostylusbutton-white" );
      image = gtk_image_new_from_file (PKGDATADIR "/Address.png");
      moko_pixmap_button_set_center_image ( MOKO_PIXMAP_BUTTON(self->addressBtn ),image);

      smsSendBtn = moko_tool_box_add_action_button (MOKO_TOOL_BOX(priv->toolbox));
      gtk_widget_set_name( GTK_WIDGET(smsSendBtn), "mokostylusbutton-white" );
      image = gtk_image_new_from_file (PKGDATADIR "/Send.png");
      moko_pixmap_button_set_center_image ( MOKO_PIXMAP_BUTTON(smsSendBtn),image);

      gtk_box_pack_start( GTK_BOX(priv->vbox), GTK_WIDGET(priv->toolbox), FALSE, FALSE, 0 );
      gtk_container_add( GTK_CONTAINER(self), GTK_WIDGET(priv->vbox) );

      //Fill input entry
      GtkAlignment* alignment = GTK_ALIGNMENT(gtk_alignment_new (0.5, 0.5, 1, 1));
      gtk_alignment_set_padding (alignment, 10, 10, 50, 10);
      GtkWidget* entrybox = gtk_hbox_new(FALSE,0);
      GtkWidget* toLabel = gtk_label_new("To:");
      gtk_widget_set_size_request (toLabel, 40, -1);
      gtk_misc_set_alignment (GTK_MISC (toLabel),1,0.5);
      self->toEntry = gtk_entry_new();
      gtk_widget_set_size_request (self->toEntry, 320, -1);
      gtk_box_pack_start (GTK_BOX(entrybox),toLabel,FALSE,TRUE,0);
      gtk_box_pack_start (GTK_BOX(entrybox),self->toEntry,FALSE,TRUE,0);
      gtk_container_add (GTK_CONTAINER(alignment), entrybox);
      gtk_box_pack_start( GTK_BOX(priv->vbox), GTK_WIDGET(alignment), FALSE, FALSE, 0 );

      /* fill textview */
      priv->txtView = gtk_text_view_new();
      GtkWidget* viewAlign = gtk_alignment_new (0.5, 0.5, 1, 1);
      gtk_alignment_set_padding (GTK_ALIGNMENT(viewAlign),10,10,30,30);
      GtkWidget* scolwin = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scolwin), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
      GtkWidget*  viewport = gtk_viewport_new (NULL, NULL);
      gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(priv->txtView),GTK_WRAP_CHAR);
      gtk_container_add (GTK_CONTAINER(viewport), priv->txtView);
      gtk_container_add (GTK_CONTAINER (scolwin), viewport);
      gtk_container_add (GTK_CONTAINER(viewAlign),scolwin);
      gtk_box_pack_start( GTK_BOX(priv->vbox), GTK_WIDGET(viewAlign), TRUE, TRUE, 0 );
      gtk_widget_show_all( GTK_WIDGET(priv->vbox) );

      g_signal_connect ( G_OBJECT(priv->txtView), "key_release_event",
                         G_CALLBACK (on_sms_txtView_key_release_event),
                         self);
    }
}

void mail_dialog_window_set_title(SmsDialogWindow* self, const gchar* title)
{
  SmsDialogWindowPrivate* priv = SMS_DIALOG_WINDOW_GET_PRIVATE(self);
  if ( !priv->titleLabel )
    {
      priv->titleLabel = gtk_label_new( title );
      gtk_window_set_title( GTK_WINDOW(self), title );
      gtk_widget_set_name( GTK_WIDGET(priv->titleLabel), "mokodialogwindow-title-label" );
      priv->eventbox = gtk_event_box_new();
      gtk_widget_set_name( GTK_WIDGET(priv->eventbox), "mokodialogwindow-title-labelbox" );
      //FIXME get from theme
      gtk_misc_set_padding( GTK_MISC(priv->titleLabel), 0, 6 );
      gtk_container_add (GTK_CONTAINER(priv->eventbox), priv->titleLabel);
      gtk_widget_show( GTK_WIDGET(priv->titleLabel) );
      gtk_widget_show( GTK_WIDGET(priv->eventbox) );
    }
  else
    {
      gtk_label_set_text( GTK_LABEL(priv->titleLabel), title );
      gtk_window_set_title( GTK_WINDOW(self), title );
    }
  if ( !priv->vbox )
    {
      GtkWidget* image;
      GtkWidget* smsSendBtn;
      GtkWidget* emailBtn;

      priv->vbox = gtk_vbox_new( FALSE, 0 );
      gtk_box_pack_start( GTK_BOX(priv->vbox), GTK_WIDGET(priv->eventbox), FALSE, FALSE, 0 );

      //Add toolbox
      priv->hbox = gtk_hbox_new( FALSE, 0 );
      priv->toolbox = moko_tool_box_new();
      GtkWidget* btnBox = moko_tool_box_get_button_box (MOKO_TOOL_BOX(priv->toolbox));
      priv->closebutton = moko_pixmap_button_new();
      image = gtk_image_new_from_file (PKGDATADIR "/Cancel.png");
      moko_pixmap_button_set_center_image ( MOKO_PIXMAP_BUTTON(priv->closebutton),image);
      gtk_widget_set_name( GTK_WIDGET(priv->closebutton), "mokostylusbutton-black" );
      gtk_box_pack_end (GTK_BOX(btnBox),priv->closebutton,FALSE,FALSE, 200);
      g_signal_connect_swapped( G_OBJECT(priv->closebutton), "clicked", G_CALLBACK(sms_dialog_window_close), self );

      self->addressBtn = moko_tool_box_add_action_button (MOKO_TOOL_BOX(priv->toolbox));
      gtk_widget_set_name( GTK_WIDGET(self->addressBtn), "mokostylusbutton-white" );
      image = gtk_image_new_from_file (PKGDATADIR "/Address.png");
      moko_pixmap_button_set_center_image ( MOKO_PIXMAP_BUTTON(self->addressBtn),image);

      emailBtn = moko_tool_box_add_action_button (MOKO_TOOL_BOX(priv->toolbox));
      gtk_widget_set_name( GTK_WIDGET(emailBtn), "mokostylusbutton-white" );
      image = gtk_image_new_from_file (PKGDATADIR "/Attached.png");
      moko_pixmap_button_set_center_image ( MOKO_PIXMAP_BUTTON(emailBtn),image);

      smsSendBtn = moko_tool_box_add_action_button (MOKO_TOOL_BOX(priv->toolbox));
      gtk_widget_set_name( GTK_WIDGET(smsSendBtn), "mokostylusbutton-white" );
      image = gtk_image_new_from_file (PKGDATADIR "/Send.png");
      moko_pixmap_button_set_center_image ( MOKO_PIXMAP_BUTTON(smsSendBtn),image);

      gtk_box_pack_start( GTK_BOX(priv->vbox), GTK_WIDGET(priv->toolbox), FALSE, FALSE, 0 );
      gtk_container_add( GTK_CONTAINER(self), GTK_WIDGET(priv->vbox) );

      //Fill input entry
      GtkAlignment* alignment = GTK_ALIGNMENT(gtk_alignment_new (0.5, 0.5, 1, 1));
      gtk_alignment_set_padding (alignment, 5, 5, 10, 10);
      GtkWidget* hbox = gtk_hbox_new(FALSE,0);
      PangoFontDescription* font_desc;
      font_desc = pango_font_description_from_string ("Bold 12");
      GtkWidget* toLabel = gtk_label_new("To:");
      gtk_widget_modify_font (toLabel, font_desc);
      gtk_widget_set_size_request (toLabel, 110, -1);
      gtk_misc_set_alignment (GTK_MISC (toLabel),0.9,0.5);
      self->toEntry = gtk_entry_new();
      gtk_widget_set_size_request (self->toEntry, 320, -1);
      gtk_box_pack_start (GTK_BOX(hbox),toLabel,FALSE,TRUE,0);
      gtk_box_pack_start (GTK_BOX(hbox),self->toEntry,FALSE,TRUE,0);
      gtk_container_add (GTK_CONTAINER(alignment), hbox);
      gtk_box_pack_start( GTK_BOX(priv->vbox), GTK_WIDGET(alignment), FALSE, FALSE, 0 );

      alignment = GTK_ALIGNMENT(gtk_alignment_new (0.5, 0.5, 1, 1));
      gtk_alignment_set_padding (alignment, 5, 5, 10, 10);
      hbox = gtk_hbox_new(FALSE,0);
      GtkWidget* ccLabel = gtk_label_new("CC:");
      font_desc = pango_font_description_from_string ("Bold 12");
      gtk_widget_set_size_request (ccLabel, 110, -1);
      gtk_widget_modify_font (ccLabel, font_desc);
      gtk_misc_set_alignment (GTK_MISC (ccLabel),0.9,0.5);
      GtkWidget* ccEntry = gtk_entry_new();
      gtk_widget_set_size_request (ccEntry, 320, -1);
      gtk_box_pack_start (GTK_BOX(hbox),ccLabel,FALSE,TRUE,0);
      gtk_box_pack_start (GTK_BOX(hbox),ccEntry,FALSE,TRUE,0);
      gtk_container_add (GTK_CONTAINER(alignment), hbox);
      gtk_box_pack_start (GTK_BOX(priv->vbox),GTK_WIDGET(alignment),FALSE,TRUE,0);

      alignment = GTK_ALIGNMENT(gtk_alignment_new (0.5, 0.5, 1, 1));
      gtk_alignment_set_padding (alignment, 5, 5, 10, 10);
      hbox = gtk_hbox_new(FALSE,0);
      GtkWidget* bccLabel = gtk_label_new("Bcc:");
      font_desc = pango_font_description_from_string ("Bold 12");
      gtk_widget_set_size_request (bccLabel, 110, -1);
      gtk_widget_modify_font (bccLabel, font_desc);
      gtk_misc_set_alignment (GTK_MISC (bccLabel),0.9,0.5);
      GtkWidget* bccEntry = gtk_entry_new();
      gtk_widget_set_size_request (bccEntry, 320, -1);
      gtk_box_pack_start (GTK_BOX(hbox),bccLabel,FALSE,TRUE,0);
      gtk_box_pack_start (GTK_BOX(hbox),bccEntry,FALSE,TRUE,0);
      gtk_container_add (GTK_CONTAINER(alignment), hbox);
      gtk_box_pack_start (GTK_BOX(priv->vbox),GTK_WIDGET(alignment),FALSE,TRUE,0);

      alignment = GTK_ALIGNMENT(gtk_alignment_new (0.5, 0.5, 1, 1));
      gtk_alignment_set_padding (alignment, 5, 5, 10, 10);
      hbox = gtk_hbox_new(FALSE,0);
      GtkWidget* subjectLabel = gtk_label_new("Subject:");
      font_desc = pango_font_description_from_string ("Bold 12");
      gtk_widget_modify_font (subjectLabel, font_desc);
      gtk_widget_set_size_request (subjectLabel, 110, -1);
      gtk_misc_set_alignment (GTK_MISC (subjectLabel),0.9,0.5);
      GtkWidget* subjectEntry = gtk_entry_new();
      gtk_widget_set_size_request (subjectEntry, 320, -1);
      gtk_box_pack_start (GTK_BOX(hbox),subjectLabel,FALSE,TRUE,0);
      gtk_box_pack_start (GTK_BOX(hbox),subjectEntry,FALSE,TRUE,0);
      gtk_container_add (GTK_CONTAINER(alignment), hbox);
      gtk_box_pack_start (GTK_BOX(priv->vbox),GTK_WIDGET(alignment),FALSE,TRUE,0);

      /* fill textview */
      GtkWidget* txtView = gtk_text_view_new();
      GtkWidget* viewAlign = gtk_alignment_new (0.5, 0.5, 1, 1);
      gtk_alignment_set_padding (GTK_ALIGNMENT(viewAlign),10,10,30,30);
      GtkWidget* scolwin = gtk_scrolled_window_new (NULL, NULL);
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scolwin), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
      GtkWidget*  viewport = gtk_viewport_new (NULL, NULL);
      gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW(txtView),GTK_WRAP_CHAR);
      gtk_container_add (GTK_CONTAINER(viewport), txtView);
      gtk_container_add (GTK_CONTAINER (scolwin), viewport);
      gtk_container_add (GTK_CONTAINER(viewAlign),GTK_WIDGET(scolwin));
      gtk_box_pack_start( GTK_BOX(priv->vbox), GTK_WIDGET(viewAlign), TRUE, TRUE, 0 );
      gtk_widget_show_all( GTK_WIDGET(priv->vbox) );
    }
}

void sms_dialog_window_set_contents(SmsDialogWindow* self, GtkWidget* contents)
{
  SmsDialogWindowPrivate* priv = SMS_DIALOG_WINDOW_GET_PRIVATE(self);
  g_return_if_fail( priv->vbox );
  gtk_box_pack_start( GTK_BOX(priv->vbox), contents, TRUE, TRUE, 0 );
}

static void sms_dialog_window_close(SmsDialogWindow* self)
{
  /* Synthesize delete_event to close dialog. */

  GtkWidget *widget = GTK_WIDGET(self);
  GdkEvent *event;

  event = gdk_event_new( GDK_DELETE );

  event->any.window = g_object_ref(widget->window);
  event->any.send_event = TRUE;

  gtk_main_do_event( event );
  gdk_event_free( event );
}

void sms_dialog_reply_message(SmsDialogWindow* self, message* msg)
{
  g_assert (msg != NULL);
  SmsDialogWindowPrivate* priv = SMS_DIALOG_WINDOW_GET_PRIVATE(self);
  gtk_entry_set_text (GTK_ENTRY(self->toEntry), msg->name);
  gtk_widget_grab_focus (priv->txtView);
}

void sms_dialog_forward_message(SmsDialogWindow* self, message* msg)
{
  g_assert (msg != NULL);
  SmsDialogWindowPrivate* priv = SMS_DIALOG_WINDOW_GET_PRIVATE(self);
  gtk_entry_set_text (GTK_ENTRY(self->toEntry), msg->name);
  g_assert (priv->txtView != NULL);
  GtkTextBuffer* buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(priv->txtView));
  gchar* text = g_strdup_printf("\n\n\n>%s",msg->content);
  gtk_text_buffer_set_text (buffer, text, strlen(text));
  gtk_widget_grab_focus (priv->txtView);
}

gboolean on_sms_txtView_key_release_event       (GtkWidget       *widget,
                                                 GdkEventKey     *event,
                                                 SmsDialogWindow *self)
{
  SmsDialogWindowPrivate* priv = SMS_DIALOG_WINDOW_GET_PRIVATE(self);
  GtkTextBuffer * buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW(priv->txtView));
  gint n = 160 - gtk_text_buffer_get_char_count(buffer)%160;
  gint m = gtk_text_buffer_get_char_count(buffer)/160 + 1;
  gtk_label_set_text(GTK_LABEL(priv->textInLabel),g_strdup_printf("%d(%d)",n,m));
  return FALSE;
}


