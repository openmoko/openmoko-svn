#ifndef _MOKO_STYLUS_MAIN_MENU_H
#define _MOKO_STYLUS_MAIN_MENU_H

#include <gtk/gtk.h>
#include "mokodesktop.h"

G_BEGIN_DECLS

#define STYLUSMENU_TYPE    (moko_stylus_menu_get_type())
#define STYLUSMENU(obj)    (G_TYPE_CHECK_INSTANCE_CAST((obj), STYLUSMENU_TYPE, MokoStylusMenu))
#define STYLUSMENUCALSS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), STYLUSMENU_TYPE, MokoStylusMenuClass))
#define IS_STYLUSMENU(obj)    (G_TYPE_CHECK_INSTANCE_CAST((obj), STYLUSMENU_TYPE))
#define IS_STYLUSMENUCALSS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), STYLUSMENU_TYPE))

typedef struct _MokoStylusMenu    MokoStylusMenu;
typedef struct _MokoStylusMenuPrivate    MokoStylusMenuPrivate;
typedef struct _MokoStylusMenuClass    MokoStylusMenuClass;

struct _MokoStylusMenu{
    GtkMenuShell parent;

    MokoStylusMenuPrivate *priv;
};

struct _MokoStylusMenuClass{
    GtkMenuShellClass parent_class;
};

GType moko_stylus_menu_get_type(void);

MokoStylusMenu *moko_stylus_menu_new ();

void moko_stylus_menu_build (GtkMenu *menu, MokoDesktopItem *dd_item);

void moko_menu_position_cb (GtkMenu *menu, int *x, int *y, gboolean *push_in, GtkWidget  *button);

G_END_DECLS
#endif /*_MOKO_STYLUS_MAIN_MENU_H*/
