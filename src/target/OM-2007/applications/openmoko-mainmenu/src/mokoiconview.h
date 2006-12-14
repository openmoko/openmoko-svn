#ifndef _MOKO_ICON_VIEW_H__
#define _MOKO_ICON_VIEW_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS
/*widgets property(s)*/
#define ICON_HEIGHT 	160
#define ICON_WIDTH 		160

#define MOKO_TYPE_ICON_VIEW				(moko_icon_view_get_type ())
#define MOKO_ICON_VIEW(obj)				(G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_ICON_VIEW, MokoIconView))
#define MOKO_ICON_VIEW_CLASS(klass)		(G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_ICON_VIEW, MokoIconViewClass))
#define MOKO_IS_ICON_VIEW(obj)				(GTK_CHECK_TYPE ((obj), MOKO_TYPE_ICON_VIEW))
#define MOKO_IS_ICON_VIEW_CLASS(klass)		(G_TYPE_CHECK_INSTANCE_CAST ((klass), MOKO_TYPE_ICON_VIEW))
#define MOKO_ICON_VIEW_GET_CLASS(obj)    	(G_TYPE_CHECK_CLASS_CAST ((obj), MOKO_TYPE_ICON_VIEW, MokoIconViewClass))

typedef struct _MokoIconView			MokoIconView;
typedef struct _MokoIconViewClass      	MokoIconViewClass;
//typedef struct _MokoIconViewPrivate    	MokoIconViewPrivate;

typedef void 
(* MokoIconViewForeachFunc) (GtkIconView *icon_view, 
						GtkTreePath *path, gpointer data);

struct _MokoIconView 
{
  //GtkVBox vbox;

  //GtkWidget *btns[3][3];
  GtkIconView parent;
  
};

struct _MokoIconViewClass
{
  GtkIconViewClass parent_class;

  void(*moko_icon_view_function)(MokoIconView *self);

};

GType 
moko_icon_view_get_type (void);

G_END_DECLS

#endif /* mokoiconview.h */
