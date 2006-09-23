#ifndef __OPENMOKO_FOOTER_H__
#define __OPENMOKO_FOOTER_H__

#include <glib.h>
#include <glib-object.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtkstatusbar.h>
#include <gtk/gtkbutton.h>

G_BEGIN_DECLS

#define FOOTER_TYPE            (footer_get_type())
#define FOOTER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), FOOTER_TYPE, Footer))
#define FOOTER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), FOOTER_TYPE, FooterClass))
#define IS_FOOTER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), FOOTER_TYPE))
#define IS_FOOTER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), FOOTER_TYPE))

typedef struct _Footer       Footer;
typedef struct _FooterClass  FooterClass;

struct _Footer
{
    GtkHBox hbox;
    GtkButton* leftbutton;
    GtkStatusbar* statusbar;
    GtkButton* rightbutton;
};

struct _FooterClass
{
    GtkHBoxClass parent_class;
    void (*footer) (Footer *f);
};

GType          footer_get_type        (void);
GtkWidget*     footer_new             (void);
void           footer_clear           (Footer *f);

G_END_DECLS

#endif /* __OPENMOKO_FOOTER_H__ */
