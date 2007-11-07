/* am-progress-dialog.c */

#include <gtk/gtk.h>
#include "am-progress-dialog.h"

G_DEFINE_TYPE (AmProgressDialog, am_progress_dialog, GTK_TYPE_DIALOG)

#define PROGRESS_DIALOG_PRIVATE(o) \
  (G_TYPE_INSTANCE_GET_PRIVATE ((o), AM_TYPE_PROGRESS_DIALOG, AmProgressDialogPrivate))

typedef struct _AmProgressDialogPrivate AmProgressDialogPrivate;

struct _AmProgressDialogPrivate
{
  GtkWidget *label;
  GtkWidget *details;
  GtkWidget *pbar;
  gint       pulse_source;
};

static void
am_progress_dialog_class_init (AmProgressDialogClass *klass)
{
  /* GObjectClass *object_class = G_OBJECT_CLASS (klass); */

  g_type_class_add_private (klass, sizeof (AmProgressDialogPrivate));
}

static void
am_progress_dialog_init (AmProgressDialog *self)
{
  GtkWidget *vbox, *w, *sw;
  
  AmProgressDialogPrivate *priv = PROGRESS_DIALOG_PRIVATE (self);

  gtk_dialog_add_button (GTK_DIALOG (self), GTK_STOCK_OK, GTK_RESPONSE_OK);
  gtk_dialog_set_has_separator (GTK_DIALOG (self), FALSE);
  gtk_widget_set_sensitive (GTK_WIDGET (GTK_DIALOG (self)->action_area), FALSE);
  
  vbox = gtk_vbox_new (FALSE, 6);
  gtk_container_set_border_width (GTK_CONTAINER (vbox), 6);
  gtk_box_pack_start_defaults (GTK_BOX (GTK_DIALOG (self)->vbox), vbox);

  priv->label = gtk_label_new (NULL);
  gtk_misc_set_alignment (GTK_MISC (priv->label), 0, 0);
  gtk_box_pack_start (GTK_BOX (vbox), priv->label, FALSE, FALSE, 0);
  
  priv->pbar = gtk_progress_bar_new ();
  gtk_box_pack_start (GTK_BOX (vbox), priv->pbar, FALSE, FALSE, 0);
  
  w = gtk_expander_new ("Details");
  gtk_box_pack_start_defaults (GTK_BOX (vbox), w);
  
  sw = gtk_scrolled_window_new (NULL, NULL);
  gtk_container_add (GTK_CONTAINER (w), sw);
  
  priv->details = gtk_text_view_new ();
  gtk_container_add (GTK_CONTAINER (sw), priv->details);
  
  gtk_widget_show_all (vbox);
}

static gboolean
progress_bar_pulse (GtkProgressBar *pbar)
{
  if (GTK_IS_PROGRESS_BAR (pbar))
  {
    gtk_progress_bar_pulse (pbar);
    return TRUE;
  }
  
  return FALSE;
}



/* public functions */

GtkWidget*
am_progress_dialog_new (void)
{
  return g_object_new (AM_TYPE_PROGRESS_DIALOG, NULL);
}

GtkWidget*
am_progress_dialog_new_full (gchar *title, gchar *message, gdouble fraction)
{
  GtkWidget *w;
  AmProgressDialogPrivate *priv;
  
  w = am_progress_dialog_new ();
  
  priv = PROGRESS_DIALOG_PRIVATE (w);
  
  gtk_window_set_title (GTK_WINDOW (w), title);
  
  gtk_label_set_text (GTK_LABEL (priv->label), message);
  
  am_progress_dialog_set_progress (AM_PROGRESS_DIALOG (w), fraction);

  return w;
}

void
am_progress_dialog_set_progress (AmProgressDialog *dialog, gdouble fraction)
{
  AmProgressDialogPrivate *priv;
  priv = PROGRESS_DIALOG_PRIVATE (dialog);
  
  if (priv->pulse_source)
    g_source_remove (priv->pulse_source);
  
  if (fraction < 0)
    priv->pulse_source = g_timeout_add (250, (GSourceFunc) progress_bar_pulse, priv->pbar);
  else
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (priv->pbar), fraction);
  
  gtk_widget_set_sensitive (GTK_WIDGET (GTK_DIALOG (dialog)->action_area), 
                            (fraction == 1));
  
}

void
am_progress_dialog_append_details_text (AmProgressDialog *dialog, gchar *text)
{
  GtkTextBuffer *buf;
  GtkTextIter iter;
  AmProgressDialogPrivate *priv;
  priv = PROGRESS_DIALOG_PRIVATE (dialog);

  
  buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (priv->details));
  
  gtk_text_buffer_get_end_iter (buf, &iter);
  gtk_text_buffer_insert (buf, &iter, text, -1);
}

void
am_progress_dialog_set_label_text (AmProgressDialog *dialog, gchar *text)
{
  AmProgressDialogPrivate *priv;
  priv = PROGRESS_DIALOG_PRIVATE (dialog);

  gtk_label_set_text (GTK_LABEL (priv->label), text);
}
