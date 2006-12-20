#ifndef _MOKO_ICON_VIEW_H__
#define _MOKO_ICON_VIEW_H__

#include <gtk/gtk.h>
#include <gtk/gtkmarshal.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>

G_BEGIN_DECLS
/*widgets property(s)*/
#define ICON_HEIGHT 	160
#define ICON_WIDTH 		160

#define MOKO_TYPE_ICON_VIEW				(moko_icon_view_get_type ())
#define MOKO_ICON_VIEW(obj)				(GTK_CHECK_CAST ((obj), MOKO_TYPE_ICON_VIEW, MokoIconView))
#define MOKO_ICON_VIEW_CLASS(klass)		(GTK_CHECK_CLASS_CAST ((klass), MOKO_TYPE_ICON_VIEW, MokoIconViewClass))
#define MOKO_IS_ICON_VIEW(obj)				(GTK_CHECK_TYPE ((obj), MOKO_TYPE_ICON_VIEW))
#define MOKO_IS_ICON_VIEW_CLASS(klass)		(GTK_CHECK_CAST ((klass), MOKO_TYPE_ICON_VIEW))
#define MOKO_ICON_VIEW_GET_CLASS(obj)    	(GTK_CHECK_CLASS_CAST ((obj), MOKO_TYPE_ICON_VIEW, MokoIconViewClass))

typedef struct _MokoIconView			MokoIconView;
typedef struct _MokoIconViewClass      	MokoIconViewClass;
typedef struct _MokoIconViewPrivate    	MokoIconViewPrivate;
//typedef struct _GtkIconViewPrivate    	GtkIconViewPrivate;

typedef void 
(* MokoIconViewForeachFunc) (MokoIconView *icon_view, 
						GtkTreePath *path, gpointer data);

struct _MokoIconView 
{
  //GtkVBox vbox;

  //GtkWidget *btns[3][3];
  //GtkIconView parent;
  GtkContainer parent;

  //GtkIconViewPrivate *priv;
  MokoIconViewPrivate *priv;
  
};

struct _MokoIconViewClass
{
  //GtkIconViewClass parent_class;
  GtkContainerClass parent_class;

  void(*moko_icon_view_function)(MokoIconView *self);

  void    (* set_scroll_adjustments) (MokoIconView      *icon_view,
				      GtkAdjustment    *hadjustment,
				      GtkAdjustment    *vadjustment);
  
  void    (* item_activated)         (MokoIconView      *icon_view,
				      GtkTreePath      *path);
  void    (* selection_changed)      (MokoIconView      *icon_view);

  /* Key binding signals */
  void    (* select_all)             (MokoIconView      *icon_view);
  void    (* unselect_all)           (MokoIconView      *icon_view);
  void    (* select_cursor_item)     (MokoIconView      *icon_view);
  void    (* toggle_cursor_item)     (MokoIconView      *icon_view);
  gboolean (* move_cursor)           (MokoIconView      *icon_view,
				      GtkMovementStep   step,
				      gint              count);
  gboolean (* activate_cursor_item)  (MokoIconView      *icon_view);



};

GType      
moko_icon_view_get_type (void) ;

GtkWidget *
moko_icon_view_new (void);

GtkWidget *
moko_icon_view_new_with_model (GtkTreeModel *model);

void
moko_icon_view_set_model (MokoIconView  *icon_view, GtkTreeModel *model);

GtkTreeModel *
moko_icon_view_get_model (MokoIconView *icon_view);

void
moko_icon_view_set_text_column (MokoIconView *icon_view, gint column);

gint          
moko_icon_view_get_text_column (MokoIconView *icon_view);

void
moko_icon_view_set_markup_column (MokoIconView *icon_view, gint column);

gint 
moko_icon_view_get_markup_column (MokoIconView  *icon_view);

void 
moko_icon_view_set_pixbuf_column (MokoIconView  *icon_view, gint column);

gint
moko_icon_view_get_pixbuf_column (MokoIconView *icon_view);

void
moko_icon_view_set_orientation (MokoIconView *icon_view, GtkOrientation orientation);

GtkOrientation 
moko_icon_view_get_orientation (MokoIconView *icon_view);

void
moko_icon_view_set_columns (MokoIconView *icon_view, gint columns);

gint
moko_icon_view_get_columns (MokoIconView *icon_view);

void
moko_icon_view_set_item_width (MokoIconView *icon_view, gint item_width);

gint
moko_icon_view_get_item_width (MokoIconView *icon_view);

void
moko_icon_view_set_spacing (MokoIconView *icon_view, gint spacing);

gint
moko_icon_view_get_spacing (MokoIconView *icon_view);

void
moko_icon_view_set_row_spacing (MokoIconView *icon_view, gint row_spacing);

gint
moko_icon_view_get_row_spacing (MokoIconView *icon_view);

void
moko_icon_view_set_column_spacing (MokoIconView *icon_view, gint column_spacing);

gint 
moko_icon_view_get_column_spacing (MokoIconView *icon_view);

void
moko_icon_view_set_margin (MokoIconView *icon_view, gint margin);

gint
moko_icon_view_get_margin (MokoIconView *icon_view);

GtkTreePath *
moko_icon_view_get_path_at_pos (MokoIconView *icon_view, gint x, gint y);

void
moko_icon_view_selected_foreach (MokoIconView *icon_view, 
					MokoIconViewForeachFunc  func, gpointer data);

void
moko_icon_view_set_selection_mode (MokoIconView *icon_view, GtkSelectionMode mode);

GtkSelectionMode 
moko_icon_view_get_selection_mode (MokoIconView *icon_view);

void 
moko_icon_view_select_path (MokoIconView *icon_view, GtkTreePath *path);

void
moko_icon_view_unselect_path (MokoIconView *icon_view, GtkTreePath *path);

gboolean
moko_icon_view_path_is_selected (MokoIconView *icon_view, GtkTreePath *path);

GList *
moko_icon_view_get_selected_items (MokoIconView *icon_view);

void
moko_icon_view_select_all (MokoIconView *icon_view);

void
moko_icon_view_unselect_all (MokoIconView *icon_view);

void 
moko_icon_view_item_activated (MokoIconView *icon_view, GtkTreePath *path);

G_END_DECLS

#endif /* mokoiconview.h */
