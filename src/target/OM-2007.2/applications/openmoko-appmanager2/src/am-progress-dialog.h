/* am-progress-dialog.h */

#ifndef _AM_PROGRESS_DIALOG
#define _AM_PROGRESS_DIALOG

#include <glib-object.h>

G_BEGIN_DECLS

#define AM_TYPE_PROGRESS_DIALOG am_progress_dialog_get_type()

#define AM_PROGRESS_DIALOG(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
  AM_TYPE_PROGRESS_DIALOG, AmProgressDialog))

#define AM_PROGRESS_DIALOG_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST ((klass), \
  AM_TYPE_PROGRESS_DIALOG, AmProgressDialogClass))

#define AM_IS_PROGRESS_DIALOG(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
  AM_TYPE_PROGRESS_DIALOG))

#define AM_IS_PROGRESS_DIALOG_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE ((klass), \
  AM_TYPE_PROGRESS_DIALOG))

#define AM_PROGRESS_DIALOG_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS ((obj), \
  AM_TYPE_PROGRESS_DIALOG, AmProgressDialogClass))

typedef struct {
  GtkDialog parent;
} AmProgressDialog;

typedef struct {
  GtkDialogClass parent_class;
} AmProgressDialogClass;

GType am_progress_dialog_get_type (void);

GtkWidget* am_progress_dialog_new (void);
GtkWidget* am_progress_dialog_new_full (gchar *title, gchar *message, gdouble fraction);

void am_progress_dialog_append_details_text (AmProgressDialog *dialog, gchar *text);
void am_progress_dialog_set_progress (AmProgressDialog *dialog, gdouble fraction);
void am_progress_dialog_set_label_text (AmProgressDialog *dialog, gchar *text);

G_END_DECLS

#endif /* _AM_PROGRESS_DIALOG */
