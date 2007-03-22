/**
 *  @file mokoiconview.c
 *  @brief The Main Menu in the Openmoko
 *  
 *  Authored by Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
 *
 *  Based on gtkiconview.c by Anders Carlsson <andersca@gnome.org>
 *  
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 *
 */

#include "mokoiconview.h"
#include "callbacks.h"

#define MOKO_MAX(arg1, arg2)	MAX(arg1, arg2)
#define MOKO_MIN(arg1, arg2)		MIN(arg1, arg2)
#define MOKO_ABS(arg1)			ABS(arg1)
#define P_(obj)					(obj)

#define MOKO_ICON_VIEW_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE ((obj), MOKO_TYPE_ICON_VIEW, MokoIconViewPrivate))

#define VALID_MODEL_AND_COLUMNS(obj) ((obj)->priv->model != NULL && \
                                      ((obj)->priv->pixbuf_column != -1 || \
				       (obj)->priv->text_column != -1 || \
				       (obj)->priv->markup_column != -1))

#define ICON_TEXT_PADDING 3

#define FRAME_OFFSET 10

typedef struct 
{
  GtkTreeIter iter;
  int index;
  
  gint row, col;

  /* Bounding boxes */
  gint x, y;
  gint width, height;

  gint pixbuf_x, pixbuf_y;
  gint pixbuf_height, pixbuf_width;

  gint layout_x, layout_y;
  gint layout_width, layout_height;

  guint selected : 1;
  guint selected_before_rubberbanding : 1;
} MokoIconViewItem;

struct _MokoIconViewPrivate
{
  gint width, height;

  gint text_column;
  gint markup_column;
  gint pixbuf_column;
  
  GtkSelectionMode selection_mode;

  GdkWindow *bin_window;

  GtkTreeModel *model;
  
  GList *items;
  
  GtkAdjustment *hadjustment;
  GtkAdjustment *vadjustment;

  guint layout_idle_id;
  
  gboolean doing_rubberband;
  gint rubberband_x1, rubberband_y1;
  gint rubberband_x2, rubberband_y2;

  guint scroll_timeout_id;
  gint scroll_value_diff;
  gint event_last_x, event_last_y;

  MokoIconViewItem *anchor_item;
  MokoIconViewItem *cursor_item;

  GdkPixbuf *bg_icon; //Decorated iocn background image.
  GdkPixbuf *bg_text;//Decorated text layout background image.
  gint max_text_len;
  gint decr_width;
  gboolean decorated;
  gint total_items;
  gboolean refresh;

  guint ctrl_pressed : 1;
  guint shift_pressed : 1;
  
  MokoIconViewItem *last_single_clicked;

#ifdef DND_WORKS
  /* Drag-and-drop. */
  gint pressed_button;
  gint press_start_x;
  gint press_start_y;
#endif

  /* Layout used to draw icon text */
  PangoLayout *layout;
  
  GtkOrientation orientation;

  gint columns;
  gint item_width;
  gint spacing;
  gint row_spacing;
  gint column_spacing;
  gint margin;
};

/* Signals */
enum
{
  ITEM_ACTIVATED,
  SELECTION_CHANGED,
  SELECT_ALL,
  UNSELECT_ALL,
  SELECT_CURSOR_ITEM,
  TOGGLE_CURSOR_ITEM,
  MOVE_CURSOR,
  ACTIVATE_CURSOR_ITEM,
  LAST_SIGNAL
};

/* Properties */
enum
{
  PROP_0,
  PROP_PIXBUF_COLUMN,
  PROP_TEXT_COLUMN,
  PROP_MARKUP_COLUMN,  
  PROP_SELECTION_MODE,
  PROP_ORIENTATION,
  PROP_MODEL,
  PROP_COLUMNS,
  PROP_ITEM_WIDTH,
  PROP_SPACING,
  PROP_ROW_SPACING,
  PROP_COLUMN_SPACING,
  PROP_MARGIN,
  PROP_BG_ICON,
  PROP_BG_TEXT,
  PROP_DECORATION_WIDTH,
  PROP_DECORATED,
  PROP_MAX_TEXT_LENGTH,
  PPOP_CURSOR_POSITION,
  PROP_TOTAL_ITEMS
};

/* GObject signals */
static void 
moko_icon_view_finalize     (GObject      *object);

static void 
moko_icon_view_set_property (GObject      *object,
					guint         prop_id,
					const GValue *value,
					GParamSpec   *pspec);

static void
moko_icon_view_get_property (GObject      *object,
					guint         prop_id,
					GValue       *value,
					GParamSpec   *pspec);


/* GtkObject signals */
static void 
moko_icon_view_destroy (GtkObject *object);

/* GtkWidget signals */
static void     
moko_icon_view_realize        (GtkWidget      *widget);

static void     
moko_icon_view_unrealize      (GtkWidget      *widget);

static void     
moko_icon_view_map            (GtkWidget      *widget);

static void     
moko_icon_view_size_request   (GtkWidget      *widget,
					      GtkRequisition *requisition);

static void     
moko_icon_view_size_allocate  (GtkWidget      *widget,
					      GtkAllocation  *allocation);

static gboolean 
moko_icon_view_expose         (GtkWidget      *widget,
					      GdkEventExpose *expose);

static gboolean 
moko_icon_view_motion         (GtkWidget      *widget,
					      GdkEventMotion *event);

static gboolean 
moko_icon_view_button_press   (GtkWidget      *widget,
					      GdkEventButton *event);

static gboolean
moko_icon_view_button_release (GtkWidget      *widget,
					      GdkEventButton *event);

/* MokoIconView signals */
static void     
moko_icon_view_set_adjustments           (MokoIconView   *icon_view,
							 GtkAdjustment *hadj,
							 GtkAdjustment *vadj);

static void     
moko_icon_view_real_select_all           (MokoIconView   *icon_view);

static void     
moko_icon_view_real_unselect_all         (MokoIconView   *icon_view);

static void     
moko_icon_view_real_select_cursor_item   (MokoIconView   *icon_view);

static void     
moko_icon_view_real_toggle_cursor_item   (MokoIconView   *icon_view);

static gboolean 
moko_icon_view_real_activate_cursor_item (MokoIconView   *icon_view);

/* Internal functions */
static void       
moko_icon_view_adjustment_changed          (GtkAdjustment   *adjustment,
							     MokoIconView     *icon_view);

static void       
moko_icon_view_layout                      (MokoIconView     *icon_view);

static void       
moko_icon_view_paint_item                  (MokoIconView     *icon_view,
							     MokoIconViewItem *item,
							     GdkRectangle    *area);

static void       
moko_icon_view_paint_rubberband            (MokoIconView     *icon_view,
							     GdkRectangle    *area);

static void       
moko_icon_view_queue_draw_item             (MokoIconView     *icon_view,
							     MokoIconViewItem *item);

static void       
moko_icon_view_queue_layout                (MokoIconView     *icon_view);

static void       
moko_icon_view_set_cursor_item             (MokoIconView     *icon_view,
							     MokoIconViewItem *item);

static void       
moko_icon_view_start_rubberbanding         (MokoIconView     *icon_view,
							     gint             x,
							     gint             y);

static void       
moko_icon_view_stop_rubberbanding          (MokoIconView     *icon_view);

static void       
moko_icon_view_update_rubberband_selection (MokoIconView     *icon_view);

static gboolean   
moko_icon_view_item_hit_test               (MokoIconViewItem *item,
							     gint             x,
							     gint             y,
							     gint             width,
							     gint             height);

#ifdef DND_WORKS
static gboolean   
moko_icon_view_maybe_begin_dragging_items  (MokoIconView     *icon_view,
							     GdkEventMotion  *event);
#endif

static gboolean   
moko_icon_view_unselect_all_internal       (MokoIconView     *icon_view);

static void       
moko_icon_view_calculate_item_size         (MokoIconView     *icon_view,
							     MokoIconViewItem *item,
							     gint             item_width);

static void       
moko_icon_view_update_rubberband           (gpointer         data);

static void       
moko_icon_view_item_invalidate_size        (MokoIconViewItem *item);

static void       
moko_icon_view_invalidate_sizes            (MokoIconView     *icon_view);

static void       
moko_icon_view_add_move_binding            (GtkBindingSet   *binding_set,
							     guint            keyval,
							     guint            modmask,
							     GtkMovementStep  step,
							     gint             count);

static gboolean   
moko_icon_view_real_move_cursor            (MokoIconView     *icon_view,
							     GtkMovementStep  step,
							     gint             count);

static void       
moko_icon_view_move_cursor_up_down         (MokoIconView     *icon_view,
							     gint             count);

static void       
moko_icon_view_move_cursor_page_up_down    (MokoIconView     *icon_view,
							     gint             count);

static void       
moko_icon_view_move_cursor_left_right      (MokoIconView     *icon_view,
							     gint             count);

static void      
moko_icon_view_move_cursor_start_end       (MokoIconView     *icon_view,
							     gint             count);

static void       
moko_icon_view_scroll_to_item              (MokoIconView     *icon_view,
							     MokoIconViewItem *item);

static GdkPixbuf *
moko_icon_view_get_item_icon               (MokoIconView     *icon_view,
							     MokoIconViewItem *item);

static void       
moko_icon_view_update_item_text            (MokoIconView     *icon_view,
							     MokoIconViewItem *item);

static void       
moko_icon_view_select_item                 (MokoIconView     *icon_view,
							     MokoIconViewItem *item);

static void       
moko_icon_view_unselect_item               (MokoIconView     *icon_view,
							     MokoIconViewItem *item);

static gboolean 
moko_icon_view_select_all_between            (MokoIconView     *icon_view,
							     MokoIconViewItem *anchor,
							     MokoIconViewItem *cursor);

static MokoIconViewItem *
moko_icon_view_get_item_at_pos (MokoIconView *icon_view,
						       gint         x,
						       gint         y);


void
moko_marshal_VOID__OBJECT_OBJECT (GClosure     *closure,
                                  GValue       *return_value,
                                  guint         n_param_values,
                                  const GValue *param_values,
                                  gpointer      invocation_hint,
                                  gpointer      marshal_data)
{
  typedef void (*GMarshalFunc_VOID__OBJECT_OBJECT) (gpointer     data1,
                                                    gpointer     arg_1,
                                                    gpointer     arg_2,
                                                    gpointer     data2);
  register GMarshalFunc_VOID__OBJECT_OBJECT callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;

  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_VOID__OBJECT_OBJECT) (marshal_data ? marshal_data : cc->callback);

  callback (data1,
            g_value_get_object (param_values + 1),
            g_value_get_object (param_values + 2),
            data2);
}

void
moko_marshal_BOOLEAN__ENUM_INT (GClosure     *closure,
                                GValue       *return_value,
                                guint         n_param_values,
                                const GValue *param_values,
                                gpointer      invocation_hint,
                                gpointer      marshal_data)
{
  typedef gboolean (*GMarshalFunc_BOOLEAN__ENUM_INT) (gpointer     data1,
                                                      gint         arg_1,
                                                      gint         arg_2,
                                                      gpointer     data2);
  register GMarshalFunc_BOOLEAN__ENUM_INT callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;
  gboolean v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 3);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_BOOLEAN__ENUM_INT) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       g_value_get_enum (param_values + 1),
                       g_value_get_int (param_values + 2),
                       data2);

  g_value_set_boolean (return_value, v_return);
}

void
moko_marshal_BOOLEAN__VOID (GClosure     *closure,
                            GValue       *return_value,
                            guint         n_param_values,
                            const GValue *param_values,
                            gpointer      invocation_hint,
                            gpointer      marshal_data)
{
  typedef gboolean (*GMarshalFunc_BOOLEAN__VOID) (gpointer     data1,
                                                  gpointer     data2);
  register GMarshalFunc_BOOLEAN__VOID callback;
  register GCClosure *cc = (GCClosure*) closure;
  register gpointer data1, data2;
  gboolean v_return;

  g_return_if_fail (return_value != NULL);
  g_return_if_fail (n_param_values == 1);

  if (G_CCLOSURE_SWAP_DATA (closure))
    {
      data1 = closure->data;
      data2 = g_value_peek_pointer (param_values + 0);
    }
  else
    {
      data1 = g_value_peek_pointer (param_values + 0);
      data2 = closure->data;
    }
  callback = (GMarshalFunc_BOOLEAN__VOID) (marshal_data ? marshal_data : cc->callback);

  v_return = callback (data1,
                       data2);

  g_value_set_boolean (return_value, v_return);
}


static GtkContainerClass *parent_class = NULL;

static guint moko_icon_view_signals[LAST_SIGNAL] = { 0 };

G_DEFINE_TYPE (MokoIconView, moko_icon_view, GTK_TYPE_CONTAINER);

/**
*@brief initialize	MokoIconView class.
*@param klass	MokoIconView Class
*@return none
*/
static void 
moko_icon_view_class_init(MokoIconViewClass* klass) /* Class Initialization */
{
  GObjectClass *gobject_class;
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;
  GtkBindingSet *binding_set;
  
  parent_class = g_type_class_peek_parent (klass);
  binding_set = gtk_binding_set_by_class (klass);

  g_type_class_add_private (klass, sizeof (MokoIconViewPrivate));

  gobject_class = (GObjectClass *) klass;
  object_class = (GtkObjectClass *) klass;
  widget_class = (GtkWidgetClass *) klass;

  gobject_class->finalize = moko_icon_view_finalize;
  gobject_class->set_property = moko_icon_view_set_property;
  gobject_class->get_property = moko_icon_view_get_property;
  
  object_class->destroy = moko_icon_view_destroy;

  widget_class->realize = moko_icon_view_realize;
  widget_class->unrealize = moko_icon_view_unrealize;
  widget_class->map = moko_icon_view_map;
  widget_class->size_request = moko_icon_view_size_request;
  widget_class->size_allocate = moko_icon_view_size_allocate;
  widget_class->expose_event = moko_icon_view_expose;
  widget_class->motion_notify_event = moko_icon_view_motion;
  widget_class->button_press_event = moko_icon_view_button_press;
  widget_class->button_release_event = moko_icon_view_button_release;


  klass->set_scroll_adjustments = moko_icon_view_set_adjustments;
  //klass->select_all = moko_icon_view_real_select_all;
  //klass->unselect_all = moko_icon_view_real_unselect_all;
  //klass->select_cursor_item = moko_icon_view_real_select_cursor_item;
  //klass->toggle_cursor_item = moko_icon_view_real_toggle_cursor_item;
  klass->activate_cursor_item = moko_icon_view_real_activate_cursor_item;  
  klass->move_cursor = moko_icon_view_real_move_cursor;


  /* Properties */
/*New properties for MokoIconView*/
   /**
   * MokoIconView::bg-icon:
   *
   * Selected icon column background image, the decorated property should be set first. 
   * Default value NULL.
   *
   */
    g_object_class_install_property (gobject_class,
				   PROP_BG_ICON,
				   g_param_spec_object ("bg_icon",
                                                        P_("Decoration Background"),
                                                        P_("Background image used to decorated selected icon column."),
                                                        GDK_TYPE_PIXBUF,
                                                        G_PARAM_READWRITE)); 
 /**
   * MokoIconView::bg-text:
   *
   * The selected text column background image, the decorated property should be set first.
   *  Default value NULL.
   *
   */
    g_object_class_install_property (gobject_class,
				   PROP_BG_TEXT,
				   g_param_spec_object ("bg_text",
                                                        P_("text Layout Background"),
                                                        P_("Background image used to decorated selected text column."),
                                                        GDK_TYPE_PIXBUF,
                                                        G_PARAM_READWRITE)); 

/**
   * MokoIconView::decr-width:
   *
   * The decoration-width property specifies the decorated width to use for each item. 
   * 
   * Since: 2.6
   */
    g_object_class_install_property (gobject_class,
				   PROP_DECORATION_WIDTH,
				   g_param_spec_int ("decr_width",
						     P_("Width for Decoration"),
						     P_("The width used for scale icon and draw decoration"),
						     -1, G_MAXINT, -1,
						     G_PARAM_READWRITE)); 
/**
   * MokoIconView::decorated:
   *
   * Whether the selected item be decorate.
   * Default value is FALSE
   *
   */
    g_object_class_install_property (gobject_class,
				  PROP_DECORATED,
				  g_param_spec_boolean ("decorated",
                                                         P_("Decorated"),
                                                         P_("Whether decorated the icon and text with custom image when selected"),
                                                         FALSE,
                                                         G_PARAM_READWRITE));

/**
   * MokoIconView::max-text-len:
   *
   * Maximum text column width.
   *
   */
    g_object_class_install_property (gobject_class,
				   PROP_MAX_TEXT_LENGTH,
				   g_param_spec_int ("max_text_len",
						     P_("Maximum Text column Length"),
						     P_("The width used for scale icon and draw decoration"),
						     -1, G_MAXINT, -1,
						     G_PARAM_READWRITE)); 

/**
   * MokoIconViewItem::index:
   *
   * index of item.
   *
   */
    g_object_class_install_property (gobject_class,
				    PPOP_CURSOR_POSITION,
				    g_param_spec_int ("index",
						     P_("index of cursor item"),
						     P_(""),
						     -1, G_MAXINT, -1,
						     G_PARAM_READABLE)); 

/**
   * MokoIconView::total-items:
   *
   * total items of MokoIconView.
   *
   */
    g_object_class_install_property (gobject_class,
				     PROP_TOTAL_ITEMS,
				   g_param_spec_int ("total_items",
						     P_("total items number"),
						     P_(""),
						     -1, G_MAXINT, -1,
						     G_PARAM_READABLE)); 
     
/*Old properties of GtkIconView*/
  /**
   * MokoIconView:selection-mode:
   * 
   * The ::selection-mode property specifies the selection mode of
   * icon view. If the mode is #GTK_SELECTION_MULTIPLE, rubberband selection
   * is enabled, for the other modes, only keyboard selection is possible.
   *
   * Since: 2.6
   */
  g_object_class_install_property (gobject_class,
				   PROP_SELECTION_MODE,
				   g_param_spec_enum ("selection_mode",
						      P_("Selection mode"),
						      P_("The selection mode"),
						      GTK_TYPE_SELECTION_MODE,
						      GTK_SELECTION_SINGLE,
						      G_PARAM_READWRITE));

  /**
   * MokoIconView:pixbuf-column:
   *
   * The ::pixbuf-column property contains the number of the model column
   * containing the pixbufs which are displayed. The pixbuf column must be 
   * of type #GDK_TYPE_PIXBUF. Setting this property to -1 turns off the
   * display of pixbufs.
   *
   * Since: 2.6
   */
  g_object_class_install_property (gobject_class,
				   PROP_PIXBUF_COLUMN,
				   g_param_spec_int ("pixbuf_column",
						     P_("Pixbuf column"),
						     P_("Model column used to retrieve the icon pixbuf from"),
						     -1, G_MAXINT, -1,
						     G_PARAM_READWRITE));

  /**
   * MokoIconView:text-column:
   *
   * The ::text-column property contains the number of the model column
   * containing the texts which are displayed. The text column must be 
   * of type #G_TYPE_STRING. If this property and the :markup-column 
   * property are both set to -1, no texts are displayed.   
   *
   * Since: 2.6
   */
  g_object_class_install_property (gobject_class,
				   PROP_TEXT_COLUMN,
				   g_param_spec_int ("text_column",
						     P_("Text column"),
						     P_("Model column used to retrieve the text from"),
						     -1, G_MAXINT, -1,
						     G_PARAM_READWRITE));

  
  /**
   * MokoIconView:markup-column:
   *
   * The ::markup-column property contains the number of the model column
   * containing markup information to be displayed. The markup column must be 
   * of type #G_TYPE_STRING. If this property and the :text-column property 
   * are both set to column numbers, it overrides the text column.
   * If both are set to -1, no texts are displayed.   
   *
   * Since: 2.6
   */
  g_object_class_install_property (gobject_class,
				   PROP_MARKUP_COLUMN,
				   g_param_spec_int ("markup_column",
						     P_("Markup column"),
						     P_("Model column used to retrieve the text if using Pango markup"),
						     -1, G_MAXINT, -1,
						     G_PARAM_READWRITE));
  
  g_object_class_install_property (gobject_class,
                                   PROP_MODEL,
                                   g_param_spec_object ("model",
							P_("Icon View Model"),
							P_("The model for the icon view"),
							GTK_TYPE_TREE_MODEL,
							G_PARAM_READWRITE));
  
  /**
   * MokoIconView:columns:
   *
   * The columns property contains the number of the columns in which the
   * items should be displayed. If it is -1, the number of columns will
   * be chosen automatically to fill the available area.
   *
   * Since: 2.6
   */
  g_object_class_install_property (gobject_class,
				   PROP_COLUMNS,
				   g_param_spec_int ("columns",
						     P_("Number of columns"),
						     P_("Number of columns to display"),
						     -1, G_MAXINT, -1,
						     G_PARAM_READWRITE));
  

  /**
   * MokoIconView::item-width:
   *
   * The item-width property specifies the width to use for each item. 
   * If it is set to -1, the icon view will automatically determine a 
   * suitable item size.
   *
   * Since: 2.6
   */
  g_object_class_install_property (gobject_class,
				   PROP_ITEM_WIDTH,
				   g_param_spec_int ("item_width",
						     P_("Width for each item"),
						     P_("The width used for each item"),
						     -1, G_MAXINT, -1,
						     G_PARAM_READWRITE));  

  /**
   * MokoIconView::spacing:
   *
   * The spacing property specifies the space which is inserted between
   * the cells (i.e. the icon and the text) of an item.
   *
   * Since: 2.6
   */
  g_object_class_install_property (gobject_class,
                                   PROP_SPACING,
                                   g_param_spec_int ("spacing",
						     P_("Spacing"),
						     P_("Space which is inserted between cells of an item"),
						     0, G_MAXINT, 0,
						     G_PARAM_READWRITE));

  /**
   * MokoIconView::row-spacing:
   *
   * The row-spacing property specifies the space which is inserted between
   * the rows of the icon view.
   *
   * Since: 2.6
   */
  g_object_class_install_property (gobject_class,
                                   PROP_ROW_SPACING,
                                   g_param_spec_int ("row_spacing",
						     P_("Row Spacing"),
						     P_("Space which is inserted between grid rows"),
						     0, G_MAXINT, 6,
						     G_PARAM_READWRITE));

  /**
   * MokoIconView::column-spacing:
   *
   * The column-spacing property specifies the space which is inserted between
   * the columns of the icon view.
   *
   * Since: 2.6
   */
  g_object_class_install_property (gobject_class,
                                   PROP_COLUMN_SPACING,
                                   g_param_spec_int ("column_spacing",
						     P_("Column Spacing"),
						     P_("Space which is inserted between grid column"),
						     0, G_MAXINT, 6,
						     G_PARAM_READWRITE));

  /**
   * MokoIconView::margin:
   *
   * The margin property specifies the space which is inserted 
   * at the edges of the icon view.
   *
   * Since: 2.6
   */
  g_object_class_install_property (gobject_class,
                                   PROP_MARGIN,
                                   g_param_spec_int ("margin",
						     P_("Margin"),
						     P_("Space which is inserted at the edges of the icon view"),
						     0, G_MAXINT, 6,
						     G_PARAM_READWRITE));


  /**
   * MokoIconView::orientation:
   *
   * The orientation property specifies how the cells (i.e. the icon and 
   * the text) of the item are positioned relative to each other.
   *
   * Since: 2.6
   */
  g_object_class_install_property (gobject_class,
				   PROP_ORIENTATION,
				   g_param_spec_enum ("orientation",
						      P_("Orientation"),
						      P_("How the text and icon of each item are positioned relative to each other"),
						      GTK_TYPE_ORIENTATION,
						      GTK_ORIENTATION_VERTICAL,
						      G_PARAM_READWRITE));

  /* Style properties */
  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_boxed ("selection_box_color",
                                                               P_("Selection Box Color"),
                                                               P_("Color of the selection box"),
                                                               GDK_TYPE_COLOR,
                                                               G_PARAM_READABLE));

  gtk_widget_class_install_style_property (widget_class,
                                           g_param_spec_uchar ("selection_box_alpha",
                                                               P_("Selection Box Alpha"),
                                                               P_("Opacity of the selection box"),
                                                               0, 0xff,
                                                               0x40,
                                                               G_PARAM_READABLE));

/*signals*/
/*New signals for MokoIconView*/

/*old signals of GtkIconView widget*/
    widget_class->set_scroll_adjustments_signal =
    g_signal_new ("set_scroll_adjustments",
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (MokoIconViewClass, set_scroll_adjustments),
		  NULL, NULL, 
		  moko_marshal_VOID__OBJECT_OBJECT,
		  G_TYPE_NONE, 2,
		  GTK_TYPE_ADJUSTMENT, GTK_TYPE_ADJUSTMENT);

  moko_icon_view_signals[ITEM_ACTIVATED] =
    g_signal_new ("item_activated",
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_LAST,
		  G_STRUCT_OFFSET (MokoIconViewClass, item_activated),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__BOXED,
		  G_TYPE_NONE, 1,
		  GTK_TYPE_TREE_PATH);

    moko_icon_view_signals[MOVE_CURSOR] =
    g_signal_new ("move_cursor",
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (MokoIconViewClass, move_cursor),
		  NULL, NULL,
		  moko_marshal_BOOLEAN__ENUM_INT,
		  G_TYPE_BOOLEAN, 2,
		  GTK_TYPE_MOVEMENT_STEP,
		  G_TYPE_INT);

    moko_icon_view_signals[SELECTION_CHANGED] =
    g_signal_new ("selection_changed",
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (MokoIconViewClass, selection_changed),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

     moko_icon_view_signals[SELECT_CURSOR_ITEM] =
    g_signal_new ("select_cursor_item",
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (MokoIconViewClass, select_cursor_item),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

    moko_icon_view_signals[TOGGLE_CURSOR_ITEM] =
    g_signal_new ("toggle_cursor_item",
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (MokoIconViewClass, toggle_cursor_item),
		  NULL, NULL,
		  g_cclosure_marshal_VOID__VOID,
		  G_TYPE_NONE, 0);

    moko_icon_view_signals[ACTIVATE_CURSOR_ITEM] =
    g_signal_new ("activate_cursor_item",
		  G_TYPE_FROM_CLASS (gobject_class),
		  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
		  G_STRUCT_OFFSET (MokoIconViewClass, activate_cursor_item),
		  NULL, NULL,
		  moko_marshal_BOOLEAN__VOID,
		  G_TYPE_BOOLEAN, 0);
    
/*Only for test delete later*/
  gtk_binding_entry_add_signal (binding_set, GDK_a, GDK_CONTROL_MASK, "select_all", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_a, GDK_CONTROL_MASK | GDK_SHIFT_MASK, "unselect_all", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_space, 0, "select_cursor_item", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_space, GDK_CONTROL_MASK, "toggle_cursor_item", 0);

  gtk_binding_entry_add_signal (binding_set, GDK_Return, 0, "activate_cursor_item", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_ISO_Enter, 0, "activate_cursor_item", 0);
  gtk_binding_entry_add_signal (binding_set, GDK_KP_Enter, 0, "activate_cursor_item", 0);

    
/*Only for test delete later*/
  moko_icon_view_add_move_binding (binding_set, GDK_Up, 0,
				  GTK_MOVEMENT_DISPLAY_LINES, -1);
  moko_icon_view_add_move_binding (binding_set, GDK_KP_Up, 0,
				  GTK_MOVEMENT_DISPLAY_LINES, -1);

  moko_icon_view_add_move_binding (binding_set, GDK_Down, 0,
				  GTK_MOVEMENT_DISPLAY_LINES, 1);
  moko_icon_view_add_move_binding (binding_set, GDK_KP_Down, 0,
				  GTK_MOVEMENT_DISPLAY_LINES, 1);

  moko_icon_view_add_move_binding (binding_set, GDK_Right, 0, 
				  GTK_MOVEMENT_VISUAL_POSITIONS, 1);
  moko_icon_view_add_move_binding (binding_set, GDK_Left, 0, 
				  GTK_MOVEMENT_VISUAL_POSITIONS, -1);

  moko_icon_view_add_move_binding (binding_set, GDK_KP_Right, 0, 
				  GTK_MOVEMENT_VISUAL_POSITIONS, 1);
  moko_icon_view_add_move_binding (binding_set, GDK_KP_Left, 0, 
				  GTK_MOVEMENT_VISUAL_POSITIONS, -1);
				  
}

/*@brief initialize 	MokoIconView instance
 *@param mm	MokoIconView*
 *@return none
 */
static void
moko_icon_view_init(MokoIconView *icon_view)
{
  icon_view->priv = MOKO_ICON_VIEW_GET_PRIVATE (icon_view);
  
  icon_view->priv->width = 120;
  icon_view->priv->height = 120;
  icon_view->priv->selection_mode = GTK_SELECTION_SINGLE;
#ifdef DND_WORKS
  icon_view->priv->pressed_button = -1;
  icon_view->priv->press_start_x = -1;
  icon_view->priv->press_start_y = -1;
#endif
  icon_view->priv->text_column = -1;
  icon_view->priv->markup_column = -1;  
  icon_view->priv->pixbuf_column = -1;

  icon_view->priv->layout = gtk_widget_create_pango_layout (GTK_WIDGET (icon_view), NULL);

  icon_view->priv->max_text_len = 30;
  icon_view->priv->decr_width = 10;
  icon_view->priv->decorated = FALSE;
  
  pango_layout_set_wrap (icon_view->priv->layout, PANGO_WRAP_WORD_CHAR);

  GTK_WIDGET_SET_FLAGS (icon_view, GTK_CAN_FOCUS);
  
  moko_icon_view_set_adjustments (icon_view, NULL, NULL);

  icon_view->priv->orientation = GTK_ORIENTATION_VERTICAL;

  icon_view->priv->columns = -1;
  icon_view->priv->item_width = -1;
  icon_view->priv->spacing = 0;
  icon_view->priv->row_spacing = 6;
  icon_view->priv->column_spacing = 6;
  icon_view->priv->margin = 6;
  icon_view->priv->cursor_item = NULL;
  icon_view->priv->anchor_item = NULL;
}


/* Destruction */
void 
moko_icon_view_clear(MokoIconView *self)
{ 
  if (!self) g_free (self);
}

/* GtkWidget signals */
static void
moko_icon_view_realize (GtkWidget *widget)
{
  MokoIconView *icon_view;
  GdkWindowAttr attributes;
  gint attributes_mask;

  icon_view = MOKO_ICON_VIEW (widget);

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);

  /* Make the main, clipping window */
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);
  attributes.event_mask = GDK_VISIBILITY_NOTIFY_MASK;

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

  widget->window = gdk_window_new (gtk_widget_get_parent_window (widget),
				   &attributes, attributes_mask);
  gdk_window_set_user_data (widget->window, widget);

  /* Make the window for the icon view */
  attributes.x = 0;
  attributes.y = 0;
  attributes.width = MOKO_MAX (icon_view->priv->width, widget->allocation.width);
  attributes.height = MOKO_MAX (icon_view->priv->height, widget->allocation.height);
  attributes.event_mask = (GDK_EXPOSURE_MASK |
			   GDK_SCROLL_MASK |
			   GDK_POINTER_MOTION_MASK |
			   GDK_BUTTON_PRESS_MASK |
			   GDK_BUTTON_RELEASE_MASK |
			   GDK_KEY_PRESS_MASK |
			   GDK_KEY_RELEASE_MASK) |
  gtk_widget_get_events (widget);

  icon_view->priv->bin_window = gdk_window_new (widget->window,
						&attributes, attributes_mask);
  gdk_window_set_user_data (icon_view->priv->bin_window, widget);

  widget->style = gtk_style_attach (widget->style, widget->window);
  
  //SUNZY:draw window backgound with window BG_PIXBUF
  gtk_style_set_background (widget->style, icon_view->priv->bin_window, widget->state);
  gtk_style_set_background (widget->style, widget->window, widget->state);  
}

static void
moko_icon_view_map (GtkWidget *widget)
{
  MokoIconView *icon_view;

  icon_view = MOKO_ICON_VIEW (widget);

  GTK_WIDGET_SET_FLAGS (widget, GTK_MAPPED);

  gdk_window_show (icon_view->priv->bin_window);
  gdk_window_show (widget->window);

}

static void
moko_icon_view_unrealize (GtkWidget *widget)
{
  MokoIconView *icon_view;

  icon_view = MOKO_ICON_VIEW (widget);

  gdk_window_set_user_data (icon_view->priv->bin_window, NULL);
  gdk_window_destroy (icon_view->priv->bin_window);
  icon_view->priv->bin_window = NULL;

  /* GtkWidget::unrealize destroys children and widget->window */
  if (GTK_WIDGET_CLASS (parent_class)->unrealize)
    (* GTK_WIDGET_CLASS (parent_class)->unrealize) (widget);
}

static void
moko_icon_view_size_request (GtkWidget      *widget,
			    GtkRequisition *requisition)
{
  MokoIconView *icon_view;

  icon_view = MOKO_ICON_VIEW (widget);

  requisition->width = icon_view->priv->width;
  requisition->height = icon_view->priv->height;
}


static GdkPixbuf *
create_colorized_pixbuf (GdkPixbuf *src, GdkColor *new_color)
{
	gint i, j;
	gint width, height, has_alpha, src_row_stride, dst_row_stride;
	gint red_value, green_value, blue_value;
	guchar *target_pixels;
	guchar *original_pixels;
	guchar *pixsrc;
	guchar *pixdest;
	GdkPixbuf *dest;

	red_value = new_color->red / 255.0;
	green_value = new_color->green / 255.0;
	blue_value = new_color->blue / 255.0;

	dest = gdk_pixbuf_new (gdk_pixbuf_get_colorspace (src),
			       gdk_pixbuf_get_has_alpha (src),
			       gdk_pixbuf_get_bits_per_sample (src),
			       gdk_pixbuf_get_width (src),
			       gdk_pixbuf_get_height (src));
	
	has_alpha = gdk_pixbuf_get_has_alpha (src);
	width = gdk_pixbuf_get_width (src);
	height = gdk_pixbuf_get_height (src);
	src_row_stride = gdk_pixbuf_get_rowstride (src);
	dst_row_stride = gdk_pixbuf_get_rowstride (dest);
	target_pixels = gdk_pixbuf_get_pixels (dest);
	original_pixels = gdk_pixbuf_get_pixels (src);

	for (i = 0; i < height; i++) {
		pixdest = target_pixels + i*dst_row_stride;
		pixsrc = original_pixels + i*src_row_stride;
		for (j = 0; j < width; j++) {		
			*pixdest++ = (*pixsrc++ * red_value) >> 8;
			*pixdest++ = (*pixsrc++ * green_value) >> 8;
			*pixdest++ = (*pixsrc++ * blue_value) >> 8;
			if (has_alpha) {
				*pixdest++ = *pixsrc++;
			}
		}
	}
	return dest;
}

static void
moko_icon_view_paint_item (MokoIconView     *icon_view,
			  MokoIconViewItem *item,
			  GdkRectangle    *area)
{
  gint focus_width, focus_pad;
  GdkPixbuf *pixbuf, *tmp, *scaled;
  GtkStateType state;

  if (!VALID_MODEL_AND_COLUMNS (icon_view))
    return;
  
  gtk_widget_style_get (GTK_WIDGET (icon_view),
			"focus-line-width", &focus_width,
			"focus-padding", &focus_pad,
			NULL);
 
  if (GTK_WIDGET_HAS_FOCUS (icon_view))
    state = GTK_STATE_SELECTED;
  else
    state = GTK_STATE_ACTIVE;

  if (icon_view->priv->pixbuf_column != -1) //FIXME: rewrite this function
    {
      pixbuf = moko_icon_view_get_item_icon (icon_view, item);

      gint decr_width = icon_view->priv->decr_width;
      gint scaled_w, scaled_h;
      gint scaled_x, scaled_y;
      scaled_w =  item->pixbuf_width - 2*decr_width;
      scaled_h = item->pixbuf_height - 2*decr_width;
      scaled_x = item->pixbuf_x + decr_width;
      scaled_y = item->pixbuf_y + decr_width;
      
      scaled = gdk_pixbuf_scale_simple (pixbuf, 
      				scaled_w, scaled_h, GDK_INTERP_NEAREST);
      
      if (item->selected && icon_view->priv->decorated) 
      	 {
	    tmp = gdk_pixbuf_scale_simple (icon_view->priv->bg_icon, 
	  					item->pixbuf_width, item->pixbuf_height,
	  					GDK_INTERP_NEAREST);

	    gdk_draw_pixbuf (icon_view->priv->bin_window, NULL, 
	  					tmp,
		       			0, 0,
		       			item->pixbuf_x, item->pixbuf_y,
		       			item->pixbuf_width,  item->pixbuf_height,
		       			GDK_RGB_DITHER_NORMAL,
		       			item->pixbuf_width,  item->pixbuf_height);

	    gdk_draw_pixbuf (icon_view->priv->bin_window, NULL, scaled,
		      			0, 0,
		      			scaled_x, scaled_y,
		       		scaled_w, scaled_h,
		       		GDK_RGB_DITHER_NORMAL,
		       		scaled_w, scaled_h);
	  
	    g_object_unref (tmp);
	    g_object_unref (scaled);
	 }
      else if (item->selected && !icon_view->priv->decorated)
      	{ 
      	    tmp = create_colorized_pixbuf (scaled,
					    &GTK_WIDGET (icon_view)->style->base[state]);
    	    gdk_draw_pixbuf (icon_view->priv->bin_window, NULL, tmp,
		      			0, 0,
		      			scaled_x, scaled_y,
		       		scaled_w, scaled_h,
		       		GDK_RGB_DITHER_NORMAL,
		       		scaled_w, scaled_h);
   	    g_object_unref (tmp);
      	}
      else
      	{
	    gdk_draw_pixbuf (icon_view->priv->bin_window, NULL, scaled,
		      			0, 0,
		      			scaled_x, scaled_y,
		       		scaled_w, scaled_h,
		       		GDK_RGB_DITHER_NORMAL,
		       		scaled_w, scaled_h);
	  
	    g_object_unref (scaled);
      	}

       g_object_unref (pixbuf);
       
    }

  if (icon_view->priv->text_column != -1 ||
      icon_view->priv->markup_column != -1)
    {
      if (item->selected && icon_view->priv->decorated)
	{
	    tmp = gdk_pixbuf_scale_simple (icon_view->priv->bg_text, 
	  					 item->layout_width + 2 * ICON_TEXT_PADDING,
	  					  item->layout_height + 2 * ICON_TEXT_PADDING,
	  					GDK_INTERP_NEAREST);

	    gdk_draw_pixbuf (icon_view->priv->bin_window, NULL, 
	  					tmp,
		       			0, 0,
		       			item->layout_x - ICON_TEXT_PADDING,
		       			item->layout_y - ICON_TEXT_PADDING,
		       			item->layout_width + 2 * ICON_TEXT_PADDING,
		       			item->layout_height + 2 * ICON_TEXT_PADDING,
		       			GDK_RGB_DITHER_NORMAL,
		       			item->layout_width + 2 * ICON_TEXT_PADDING,
		       			item->layout_height + 2 * ICON_TEXT_PADDING);
	 g_object_unref (tmp);
	}
      else if (item->selected && !icon_view->priv->decorated) 
      	{
      	    gdk_draw_rectangle (icon_view->priv->bin_window,
			      GTK_WIDGET (icon_view)->style->base_gc[state],
			      TRUE,
			      item->layout_x - ICON_TEXT_PADDING,
			      item->layout_y - ICON_TEXT_PADDING,
			      item->layout_width + 2 * ICON_TEXT_PADDING,
			      item->layout_height + 2 * ICON_TEXT_PADDING);
      	}

      moko_icon_view_update_item_text (icon_view, item);
      pango_layout_set_alignment (icon_view->priv->layout, PANGO_ALIGN_CENTER);
      pango_layout_set_width (icon_view->priv->layout, item->layout_width * PANGO_SCALE);
      gtk_paint_layout (GTK_WIDGET (icon_view)->style,
			icon_view->priv->bin_window,
			item->selected ? state : GTK_STATE_NORMAL,
			TRUE, area, GTK_WIDGET (icon_view), "moko_icon_view",
			item->layout_x,
			item->layout_y,
			icon_view->priv->layout);
    }

}

static guint32
moko_gdk_color_to_rgb (const GdkColor *color)
{
  guint32 result;
  result = (0xff0000 | (color->red & 0xff00));
  result <<= 8;
  result |= ((color->green & 0xff00) | (color->blue >> 8));
  return result;
}

static void
moko_icon_view_paint_rubberband (MokoIconView     *icon_view,
				GdkRectangle    *area)
{
  GdkRectangle rect;
  GdkPixbuf *pixbuf;
  GdkGC *gc;
  GdkRectangle rubber_rect;
  GdkColor *fill_color_gdk;
  guint fill_color;
  guchar fill_color_alpha;

  rubber_rect.x = MOKO_MIN (icon_view->priv->rubberband_x1, icon_view->priv->rubberband_x2);
  rubber_rect.y = MOKO_MIN (icon_view->priv->rubberband_y1, icon_view->priv->rubberband_y2);
  rubber_rect.width = MOKO_ABS (icon_view->priv->rubberband_x1 - icon_view->priv->rubberband_x2) + 1;
  rubber_rect.height = MOKO_ABS (icon_view->priv->rubberband_y1 - icon_view->priv->rubberband_y2) + 1;

  if (!gdk_rectangle_intersect (&rubber_rect, area, &rect))
    return;

  gtk_widget_style_get (GTK_WIDGET (icon_view),
                        "selection_box_color", &fill_color_gdk,
                        "selection_box_alpha", &fill_color_alpha,
                        NULL);

  if (!fill_color_gdk) {
    fill_color_gdk = gdk_color_copy (&GTK_WIDGET (icon_view)->style->base[GTK_STATE_SELECTED]);
  }

  fill_color = moko_gdk_color_to_rgb (fill_color_gdk) << 8 | fill_color_alpha;
#if 0
  if (!gdk_draw_rectangle_alpha_libgtk_only (icon_view->priv->bin_window,
					     rect.x, rect.y, rect.width, rect.height,
					     fill_color_gdk,
					     fill_color_alpha << 8 | fill_color_alpha))
#endif
    {
      pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, TRUE, 8, rect.width, rect.height);
      gdk_pixbuf_fill (pixbuf, fill_color);
      
      gdk_draw_pixbuf (icon_view->priv->bin_window, NULL, pixbuf,
		       0, 0, 
		       rect.x,rect.y,
		       rect.width, rect.height,
		       GDK_RGB_DITHER_NONE,
		       0, 0);
      g_object_unref (pixbuf);
    }

  gc = gdk_gc_new (icon_view->priv->bin_window);
  gdk_gc_set_rgb_fg_color (gc, fill_color_gdk);
  gdk_gc_set_clip_rectangle (gc, &rect);
  gdk_draw_rectangle (icon_view->priv->bin_window,
		      gc, FALSE,
		      rubber_rect.x, rubber_rect.y,
		      rubber_rect.width - 1, rubber_rect.height - 1);
  gdk_color_free (fill_color_gdk);
  g_object_unref (gc);
}

static gboolean
moko_icon_view_expose (GtkWidget *widget,
		      GdkEventExpose *expose)
{
  MokoIconView *icon_view;
  GList *icons;
  icon_view = MOKO_ICON_VIEW (widget);

  if (expose->window != icon_view->priv->bin_window)
    return FALSE;

  for (icons = icon_view->priv->items; icons; icons = icons->next) {
    MokoIconViewItem *item = icons->data;
    GdkRectangle item_rectangle;

    item_rectangle.x = item->x;
    item_rectangle.y = item->y;
    item_rectangle.width = item->width;
    item_rectangle.height = item->height;

    if (gdk_region_rect_in (expose->region, &item_rectangle) == GDK_OVERLAP_RECTANGLE_OUT)
      continue;

#ifdef DEBUG_ICON_VIEW
    gdk_draw_rectangle (icon_view->priv->bin_window,
			GTK_WIDGET (icon_view)->style->black_gc,
			FALSE,
			item->x, item->y, 
			item->width, item->height);
    gdk_draw_rectangle (icon_view->priv->bin_window,
			GTK_WIDGET (icon_view)->style->black_gc,
			FALSE,
			item->pixbuf_x, item->pixbuf_y, 
			item->pixbuf_width, item->pixbuf_height);
    gdk_draw_rectangle (icon_view->priv->bin_window,
			GTK_WIDGET (icon_view)->style->black_gc,
			FALSE,
			item->layout_x, item->layout_y, 
			item->layout_width, item->layout_height);
#endif
    moko_icon_view_paint_item (icon_view, item, &expose->area);

  }
  if (icon_view->priv->doing_rubberband)
    {
      GdkRectangle *rectangles;
      gint n_rectangles;
      
      gdk_region_get_rectangles (expose->region,
				 &rectangles,
				 &n_rectangles);
      
      while (n_rectangles--)
	moko_icon_view_paint_rubberband (icon_view, &rectangles[n_rectangles]);

      g_free (rectangles);
    }
  
  return TRUE;
}

static gboolean
scroll_timeout (gpointer data)
{
  MokoIconView *icon_view;
  gdouble value;

  GDK_THREADS_ENTER ();
  
  icon_view = data;

  value = MOKO_MIN (icon_view->priv->vadjustment->value +
	       icon_view->priv->scroll_value_diff,
	       icon_view->priv->vadjustment->upper -
	       icon_view->priv->vadjustment->page_size);

  gtk_adjustment_set_value (icon_view->priv->vadjustment,
			    value);

  moko_icon_view_update_rubberband (icon_view);
  
  GDK_THREADS_LEAVE ();

  return TRUE;
}

static gboolean
moko_icon_view_motion (GtkWidget      *widget,
		      GdkEventMotion *event)
{
  MokoIconView *icon_view;
  gint abs_y;

  icon_view = MOKO_ICON_VIEW (widget);
#ifdef DND_WORKS
  moko_icon_view_maybe_begin_dragging_items (icon_view, event);
#endif
  if (icon_view->priv->doing_rubberband)
    {
      moko_icon_view_update_rubberband (widget);
      
      abs_y = event->y - icon_view->priv->height *
	(icon_view->priv->vadjustment->value /
	 (icon_view->priv->vadjustment->upper -
	  icon_view->priv->vadjustment->lower));

      if (abs_y < 0 || abs_y > widget->allocation.height)
	{
	  if (icon_view->priv->scroll_timeout_id == 0)
	    icon_view->priv->scroll_timeout_id = g_timeout_add (30, scroll_timeout, icon_view);

	  if (abs_y < 0)
	    icon_view->priv->scroll_value_diff = abs_y;
	  else
	    icon_view->priv->scroll_value_diff = abs_y - widget->allocation.height;

	  icon_view->priv->event_last_x = event->x;
	  icon_view->priv->event_last_y = event->y;
	}
      else if (icon_view->priv->scroll_timeout_id != 0)
	{
	  g_source_remove (icon_view->priv->scroll_timeout_id);

	  icon_view->priv->scroll_timeout_id = 0;
	}
    }
  
  return TRUE;
}

static gboolean
moko_icon_view_button_press (GtkWidget      *widget,
			    GdkEventButton *event)
{
  MokoIconView *icon_view;
  MokoIconViewItem *item;
  gboolean dirty = FALSE;
  gboolean activate = FALSE;
  
  icon_view = MOKO_ICON_VIEW (widget);

  if (event->window != icon_view->priv->bin_window)
    return FALSE;

  if (!GTK_WIDGET_HAS_FOCUS (widget))
    gtk_widget_grab_focus (widget);

  if (event->button == 1 && event->type == GDK_BUTTON_PRESS)
    {
      item = moko_icon_view_get_item_at_pos (icon_view,
					    event->x, event->y);

      if (item != NULL)
	{
	  moko_icon_view_scroll_to_item (icon_view, item);
	  
	  if (icon_view->priv->selection_mode == GTK_SELECTION_NONE)
	    {
	      moko_icon_view_set_cursor_item (icon_view, item);
	    }
	  else if (icon_view->priv->selection_mode == GTK_SELECTION_MULTIPLE &&
		   (event->state & GDK_SHIFT_MASK))
	    {
	      moko_icon_view_unselect_all_internal (icon_view);

	      moko_icon_view_set_cursor_item (icon_view, item);
	      if (!icon_view->priv->anchor_item)
		    icon_view->priv->anchor_item = item;
	      else 
		    moko_icon_view_select_all_between (icon_view,
						  icon_view->priv->anchor_item,
						  item);
	      dirty = TRUE;
	    }
	  else 
	    {
	      if ((icon_view->priv->selection_mode == GTK_SELECTION_MULTIPLE ||
		  ((icon_view->priv->selection_mode == GTK_SELECTION_SINGLE) && item->selected)) &&
		  (event->state & GDK_CONTROL_MASK))
		{
		  item->selected = !item->selected;
		  moko_icon_view_queue_draw_item (icon_view, item);
		  dirty = TRUE;
		}
	      else
		{
		  if (!item->selected)
		    {
			  moko_icon_view_unselect_all_internal (icon_view);
		      item->selected = TRUE;
		      moko_icon_view_queue_draw_item (icon_view, item);
		      dirty = TRUE;
		    }
		  else 
		      activate = TRUE;
		  
		}
	      moko_icon_view_set_cursor_item (icon_view, item);
	      icon_view->priv->anchor_item = item;
	    }
#ifdef DND_WORKS
	  /* Save press to possibly begin a drag */
	  if (icon_view->priv->pressed_button < 0)
	    {
	      icon_view->priv->pressed_button = event->button;
	      icon_view->priv->press_start_x = event->x;
	      icon_view->priv->press_start_y = event->y;
	    }
#endif
	  if (!icon_view->priv->last_single_clicked)
	    icon_view->priv->last_single_clicked = item;
	}
      else
	{
	  if (icon_view->priv->selection_mode != GTK_SELECTION_BROWSE &&
	      !(event->state & GDK_CONTROL_MASK))
	    {
	      dirty = moko_icon_view_unselect_all_internal (icon_view);
	    }
	  
	  if (icon_view->priv->selection_mode == GTK_SELECTION_MULTIPLE)
	    moko_icon_view_start_rubberbanding (icon_view, event->x, event->y);
	}

    }
  if (activate)
	{
	   GtkTreePath *path;
	   path = gtk_tree_path_new_from_indices (item->index, -1);
	   moko_icon_view_item_activated (icon_view, path);
	   gtk_tree_path_free (path);

       icon_view->priv->last_single_clicked = NULL;
	}

  if (dirty)
    g_signal_emit (icon_view, moko_icon_view_signals[SELECTION_CHANGED], 0);

  return event->button == 1;
}

static gboolean
moko_icon_view_button_release (GtkWidget      *widget,
			      GdkEventButton *event)
{
  MokoIconView *icon_view;

  icon_view = MOKO_ICON_VIEW (widget);

#ifdef DND_WORKS
  if (icon_view->priv->pressed_button == event->button)
  {
      icon_view->priv->pressed_button = -1;
  }
#endif
  moko_icon_view_stop_rubberbanding (icon_view);

  if (icon_view->priv->scroll_timeout_id != 0)
    {
      g_source_remove (icon_view->priv->scroll_timeout_id);
      icon_view->priv->scroll_timeout_id = 0;
    }

  return TRUE;
}

static void
moko_icon_view_update_rubberband (gpointer data)
{
  MokoIconView *icon_view;
  gint x, y;
  GdkRectangle old_area;
  GdkRectangle new_area;
  GdkRectangle common;
  GdkRegion *invalid_region;
  
  icon_view = MOKO_ICON_VIEW (data);

  gdk_window_get_pointer (icon_view->priv->bin_window, &x, &y, NULL);

  x = MOKO_MAX (x, 0);
  y = MOKO_MAX (y, 0);

  old_area.x = MOKO_MIN (icon_view->priv->rubberband_x1,
		    icon_view->priv->rubberband_x2);
  old_area.y = MOKO_MIN (icon_view->priv->rubberband_y1,
		    icon_view->priv->rubberband_y2);
  old_area.width = MOKO_ABS (icon_view->priv->rubberband_x2 -
			icon_view->priv->rubberband_x1) + 1;
  old_area.height = MOKO_ABS (icon_view->priv->rubberband_y2 -
			 icon_view->priv->rubberband_y1) + 1;
  
  new_area.x = MOKO_MIN (icon_view->priv->rubberband_x1, x);
  new_area.y = MOKO_MIN (icon_view->priv->rubberband_y1, y);
  new_area.width = MOKO_ABS (x - icon_view->priv->rubberband_x1) + 1;
  new_area.height = MOKO_ABS (y - icon_view->priv->rubberband_y1) + 1;

  invalid_region = gdk_region_rectangle (&old_area);
  gdk_region_union_with_rect (invalid_region, &new_area);

  gdk_rectangle_intersect (&old_area, &new_area, &common);
  if (common.width > 2 && common.height > 2)
    {
      GdkRegion *common_region;

      /* make sure the border is invalidated */
      common.x += 1;
      common.y += 1;
      common.width -= 2;
      common.height -= 2;
      
      common_region = gdk_region_rectangle (&common);

      gdk_region_subtract (invalid_region, common_region);
      gdk_region_destroy (common_region);
    }
  
  gdk_window_invalidate_region (icon_view->priv->bin_window, invalid_region, TRUE);
    
  gdk_region_destroy (invalid_region);

  icon_view->priv->rubberband_x2 = x;
  icon_view->priv->rubberband_y2 = y;  

  moko_icon_view_update_rubberband_selection (icon_view);
}

static void
moko_icon_view_start_rubberbanding (MokoIconView  *icon_view,
				   gint          x,
				   gint          y)
{
  GList *items;

  g_assert (!icon_view->priv->doing_rubberband);

  for (items = icon_view->priv->items; items; items = items->next)
    {
      MokoIconViewItem *item = items->data;

      item->selected_before_rubberbanding = item->selected;
    }
  
  icon_view->priv->rubberband_x1 = x;
  icon_view->priv->rubberband_y1 = y;
  icon_view->priv->rubberband_x2 = x;
  icon_view->priv->rubberband_y2 = y;

  icon_view->priv->doing_rubberband = TRUE;

  gtk_grab_add (GTK_WIDGET (icon_view));
}

static void
moko_icon_view_stop_rubberbanding (MokoIconView *icon_view)
{
  if (!icon_view->priv->doing_rubberband)
    return;

  icon_view->priv->doing_rubberband = FALSE;

  gtk_grab_remove (GTK_WIDGET (icon_view));
  
  gtk_widget_queue_draw (GTK_WIDGET (icon_view));
}

static void
moko_icon_view_update_rubberband_selection (MokoIconView *icon_view)
{
  GList *items;
  gint x, y, width, height;
  gboolean dirty = FALSE;
  
  x = MOKO_MIN (icon_view->priv->rubberband_x1,
	   icon_view->priv->rubberband_x2);
  y = MOKO_MIN (icon_view->priv->rubberband_y1,
	   icon_view->priv->rubberband_y2);
  width = MOKO_ABS (icon_view->priv->rubberband_x1 - 
	       icon_view->priv->rubberband_x2);
  height = MOKO_ABS (icon_view->priv->rubberband_y1 - 
		icon_view->priv->rubberband_y2);
  
  for (items = icon_view->priv->items; items; items = items->next)
    {
      MokoIconViewItem *item = items->data;
      gboolean is_in;
      gboolean selected;
      
      is_in = moko_icon_view_item_hit_test (item, x, y, width, height);

      selected = is_in ^ item->selected_before_rubberbanding;

      if (item->selected != selected)
	{
	  item->selected = selected;
	  dirty = TRUE;
	  moko_icon_view_queue_draw_item (icon_view, item);
	}
    }

  if (dirty)
    g_signal_emit (icon_view, moko_icon_view_signals[SELECTION_CHANGED], 0);
}

static gboolean
moko_icon_view_item_hit_test (MokoIconViewItem  *item,
			     gint              x,
			     gint              y,
			     gint              width,
			     gint              height)
{
  /* First try the pixbuf */
  if (MOKO_MIN (x + width, item->pixbuf_x + item->pixbuf_width) - MOKO_MAX (x, item->pixbuf_x) > 0 &&
      MOKO_MIN (y + height, item->pixbuf_y + item->pixbuf_height) - MOKO_MAX (y, item->pixbuf_y) > 0)
    return TRUE;

  /* Then try the text */
  if (MOKO_MIN (x + width, item->layout_x + item->layout_width) - MOKO_MAX (x, item->layout_x) > 0 &&
      MOKO_MIN (y + height, item->layout_y + item->layout_height) - MOKO_MAX (y, item->layout_y) > 0)
    return TRUE;
  
  return FALSE;
}

#ifdef DND_WORKS
static gboolean
moko_icon_view_maybe_begin_dragging_items (MokoIconView     *icon_view,
					  GdkEventMotion  *event)
{
  gboolean retval = FALSE;
  gint button;
  if (icon_view->priv->pressed_button < 0)
    return retval;

  if (!gtk_drag_check_threshold (GTK_WIDGET (icon_view),
				 icon_view->priv->press_start_x,
				 icon_view->priv->press_start_y,
				 event->x, event->y))
    return retval;

  button = icon_view->priv->pressed_button;
  icon_view->priv->pressed_button = -1;
  
  {
    static GtkTargetEntry row_targets[] = {
      { "MOKO_ICON_VIEW_ITEMS", GTK_TARGET_SAME_APP, 0 }
    };
    GtkTargetList *target_list;
    GdkDragContext *context;
    MokoIconViewItem *item;
    
    retval = TRUE;
    
    target_list = gtk_target_list_new (row_targets, G_N_ELEMENTS (row_targets));

    context = gtk_drag_begin (GTK_WIDGET (icon_view),
			      target_list, GDK_ACTION_MOVE,
			      button,
			      (GdkEvent *)event);

    item = moko_icon_view_get_item_at_pos (icon_view,
					  icon_view->priv->press_start_x,
					  icon_view->priv->press_start_y);
    g_assert (item != NULL);
    gtk_drag_set_icon_pixbuf (context, moko_icon_view_get_item_icon (icon_view, item),
			      event->x - item->x,
			      event->y - item->y);
  }
  
  return retval;
}
#endif

static gboolean
moko_icon_view_unselect_all_internal (MokoIconView  *icon_view)
{
  gboolean dirty = FALSE;
  GList *items;

  if (icon_view->priv->selection_mode == GTK_SELECTION_NONE)
    return FALSE;

  for (items = icon_view->priv->items; items; items = items->next)
    {
      MokoIconViewItem *item = items->data;

      if (item->selected)
	{
	  item->selected = FALSE;
	  dirty = TRUE;
	  moko_icon_view_queue_draw_item (icon_view, item);
	}
    }

  return dirty;
}
/* MokoIconView signals */
static void
moko_icon_view_set_adjustments (MokoIconView   *icon_view,
			       GtkAdjustment *hadj,
			       GtkAdjustment *vadj)
{
  gboolean need_adjust = FALSE;

  if (hadj)
    g_return_if_fail (GTK_IS_ADJUSTMENT (hadj));
  else
    hadj = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0));
  if (vadj)
    g_return_if_fail (GTK_IS_ADJUSTMENT (vadj));
  else
    vadj = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 0.0, 0.0, 0.0, 0.0));

  if (icon_view->priv->hadjustment && (icon_view->priv->hadjustment != hadj))
    {
      g_signal_handlers_disconnect_matched (icon_view->priv->hadjustment, G_SIGNAL_MATCH_DATA,
					   0, 0, NULL, NULL, icon_view);
      g_object_unref (icon_view->priv->hadjustment);
    }

  if (icon_view->priv->vadjustment && (icon_view->priv->vadjustment != vadj))
    {
      g_signal_handlers_disconnect_matched (icon_view->priv->vadjustment, G_SIGNAL_MATCH_DATA,
					    0, 0, NULL, NULL, icon_view);
      g_object_unref (icon_view->priv->vadjustment);
    }

  if (icon_view->priv->hadjustment != hadj)
    {
      icon_view->priv->hadjustment = hadj;
      g_object_ref (icon_view->priv->hadjustment);
      gtk_object_sink (GTK_OBJECT (icon_view->priv->hadjustment));

      g_signal_connect (icon_view->priv->hadjustment, "value_changed",
			G_CALLBACK (moko_icon_view_adjustment_changed),
			icon_view);
      need_adjust = TRUE;
    }

  if (icon_view->priv->vadjustment != vadj)
    {
      icon_view->priv->vadjustment = vadj;
      g_object_ref (icon_view->priv->vadjustment);
      gtk_object_sink (GTK_OBJECT (icon_view->priv->vadjustment));

      g_signal_connect (icon_view->priv->vadjustment, "value_changed",
			G_CALLBACK (moko_icon_view_adjustment_changed),
			icon_view);
      need_adjust = TRUE;
    }

  if (need_adjust)
    moko_icon_view_adjustment_changed (NULL, icon_view);
}

static void
moko_icon_view_real_select_all (MokoIconView *icon_view)
{
  moko_icon_view_select_all (icon_view);
}

static void
moko_icon_view_real_unselect_all (MokoIconView *icon_view)
{
  moko_icon_view_unselect_all (icon_view);
}

static void
moko_icon_view_real_select_cursor_item (MokoIconView *icon_view)
{
  moko_icon_view_unselect_all (icon_view);
  
  if (icon_view->priv->cursor_item != NULL)
    moko_icon_view_select_item (icon_view, icon_view->priv->cursor_item);
}

static gboolean
moko_icon_view_real_activate_cursor_item (MokoIconView *icon_view)
{
  GtkTreePath *path;
  
  if (!icon_view->priv->cursor_item)
    return FALSE;

  path = gtk_tree_path_new_from_indices (icon_view->priv->cursor_item->index, -1);
  
  moko_icon_view_item_activated (icon_view, path);

  gtk_tree_path_free (path);

  return TRUE;
}

static void
moko_icon_view_real_toggle_cursor_item (MokoIconView *icon_view)
{
  if (!icon_view->priv->cursor_item)
    return;

  switch (icon_view->priv->selection_mode)
    {
    case GTK_SELECTION_NONE:
      break;
    case GTK_SELECTION_BROWSE:
      moko_icon_view_select_item (icon_view, icon_view->priv->cursor_item);
      break;
    case GTK_SELECTION_SINGLE:
      if (icon_view->priv->cursor_item->selected)
	moko_icon_view_unselect_item (icon_view, icon_view->priv->cursor_item);
      else
	moko_icon_view_select_item (icon_view, icon_view->priv->cursor_item);
      break;
    case GTK_SELECTION_MULTIPLE:
      icon_view->priv->cursor_item->selected = !icon_view->priv->cursor_item->selected;
      g_signal_emit (icon_view, moko_icon_view_signals[SELECTION_CHANGED], 0); 
      
      moko_icon_view_queue_draw_item (icon_view, icon_view->priv->cursor_item);
      break;
    }
}

static void
moko_icon_view_calculate_item_size (MokoIconView     *icon_view,
				   MokoIconViewItem *item,
				   gint             item_width)
{
  gint focus_width, focus_pad;
  gint layout_width, layout_height;
  gint maximum_layout_width;
  gint spacing, padding;
  gint colspan;
  GdkPixbuf *pixbuf;
  
  if (item->width != -1 && item->height != -1) 
    return;

  gtk_widget_style_get (GTK_WIDGET (icon_view),
			"focus-line-width", &focus_width,
			"focus-padding", &focus_pad,
			NULL);

  spacing = icon_view->priv->spacing;

  if (icon_view->priv->pixbuf_column != -1)
    {
      pixbuf = moko_icon_view_get_item_icon (icon_view, item);
      item->pixbuf_width = gdk_pixbuf_get_width (pixbuf);
      item->pixbuf_height = gdk_pixbuf_get_height (pixbuf);
      g_object_unref (pixbuf);
    }
  else
    {
      item->pixbuf_width = 0;
      item->pixbuf_height = 0;
      spacing = 0;
    }
  
  if (icon_view->priv->orientation == GTK_ORIENTATION_HORIZONTAL &&
      item_width > 0)
    {
      colspan = item->pixbuf_width / item_width + 1;
      maximum_layout_width = MOKO_MAX (colspan * item_width - item->pixbuf_width - icon_view->priv->spacing - 2 * (ICON_TEXT_PADDING + focus_width + focus_pad), 50);
    }
  else
    maximum_layout_width = MOKO_MAX (item_width, item->pixbuf_width);
    
  if (icon_view->priv->markup_column != -1 ||
      icon_view->priv->text_column != -1)
    {
      moko_icon_view_update_item_text (icon_view, item);

      pango_layout_set_alignment (icon_view->priv->layout, PANGO_ALIGN_CENTER);
      pango_layout_set_width (icon_view->priv->layout, maximum_layout_width * PANGO_SCALE);
      
      pango_layout_get_pixel_size (icon_view->priv->layout, &layout_width, &layout_height);
      
      item->layout_width = layout_width;
      item->layout_height = layout_height;
      padding = 2 * (ICON_TEXT_PADDING + focus_width + focus_pad);
    }
  else
    {
      item->layout_width = 0;
      item->layout_height = 0;
      spacing = 0;
      padding = 0;
    }

  if (icon_view->priv->orientation == GTK_ORIENTATION_HORIZONTAL)
    {
      item->width = item->layout_width + padding + spacing + item->pixbuf_width;
      item->height = MOKO_MAX (item->layout_height + padding, item->pixbuf_height);
    }
  else
    {
      item->width = MOKO_MAX (item->layout_width + padding, item->pixbuf_width);
      item->height = item->layout_height + padding + spacing + item->pixbuf_height;
    }
}

/* Internal functions */
static void
moko_icon_view_adjustment_changed (GtkAdjustment *adjustment,
				  MokoIconView   *icon_view)
{
  if (GTK_WIDGET_REALIZED (icon_view))
    {
      gdk_window_move (icon_view->priv->bin_window,
		       - icon_view->priv->hadjustment->value,
		       - icon_view->priv->vadjustment->value);

      if (icon_view->priv->doing_rubberband)
	moko_icon_view_update_rubberband (GTK_WIDGET (icon_view));

       gdk_window_process_updates (icon_view->priv->bin_window, TRUE);
    }
}

static GList *
moko_icon_view_layout_single_row (MokoIconView *icon_view, 
				 GList       *first_item, 
				 gint         item_width,
				 gint         row,
				 gint        *y, 
				 gint        *maximum_width)
{
  gint focus_width, focus_pad;
  gint x, current_width, max_height, max_pixbuf_height;
  GList *items, *last_item;
  gint col;
  gint colspan;
  gboolean rtl = gtk_widget_get_direction (GTK_WIDGET (icon_view)) == GTK_TEXT_DIR_RTL;

  x = 0;
  col = 0;
  max_height = 0;
  max_pixbuf_height = 0;
  items = first_item;
  current_width = 0;

  gtk_widget_style_get (GTK_WIDGET (icon_view),
			"focus-line-width", &focus_width,
			"focus-padding", &focus_pad,
			NULL);

  x += icon_view->priv->margin;
  current_width += 2 * icon_view->priv->margin;
  items = first_item;

  while (items)
    {
      MokoIconViewItem *item = items->data;

      moko_icon_view_calculate_item_size (icon_view, item, item_width);

      colspan = 1 + (item->width - 1) / (item_width + icon_view->priv->column_spacing);
      current_width += colspan * (item_width + icon_view->priv->column_spacing);
	
      if (items != first_item)
	{
	  if ((icon_view->priv->columns <= 0 && current_width > GTK_WIDGET (icon_view)->allocation.width) ||
	      (icon_view->priv->columns > 0 && col >= icon_view->priv->columns))
	    break;
	}

      item->y = *y + icon_view->priv->row_spacing/2; //SUNZY:adjust y coordinate to center of item box at X coordinatioin.
      item->x = rtl ? GTK_WIDGET (icon_view)->allocation.width - MOKO_MAX (item_width, item->width) - x : x;

      if (icon_view->priv->orientation == GTK_ORIENTATION_HORIZONTAL)
	{
	  if (rtl)
	    {
	      item->layout_x = item->x + ICON_TEXT_PADDING + focus_width + focus_pad;
	      if (icon_view->priv->text_column != -1 ||
		  icon_view->priv->markup_column != -1)
		item->pixbuf_x = item->x + 2 * (ICON_TEXT_PADDING + focus_width + focus_pad) + icon_view->priv->spacing + item->layout_width;
	      else
		item->pixbuf_x = item->x;
	    }
	  else 
	    {
	      item->pixbuf_x = item->x;
	      if (icon_view->priv->pixbuf_column != -1)
		item->layout_x = item->x + item->pixbuf_width + icon_view->priv->spacing + ICON_TEXT_PADDING + focus_width + focus_pad;
	      else
		item->layout_x = item->x + ICON_TEXT_PADDING + focus_width + focus_pad;
	    }
	}
      else
	{
	  if (item->width < colspan * item_width + (colspan - 1) * icon_view->priv->column_spacing)
	    item->x += (colspan * item_width + (colspan - 1) * icon_view->priv->column_spacing - item->width) / 2;

	  item->pixbuf_x = item->x + (item->width - item->pixbuf_width) / 2;
	  item->layout_x = item->x + (item->width - item->layout_width) / 2;
	}

      x = current_width - icon_view->priv->margin; 

      max_height = MOKO_MAX (max_height, item->height);
      max_pixbuf_height = MOKO_MAX (max_pixbuf_height, item->pixbuf_height);
      
      if (current_width > *maximum_width)
	*maximum_width = current_width;

      item->row = row;
      item->col = col;

      col += colspan;
      icon_view->priv->total_items +=1;
      items = items->next;
    }

  last_item = items;

  *y += max_height + icon_view->priv->row_spacing;

  /* Now go through the row again and align the icons */
  for (items = first_item; items != last_item; items = items->next)
    {
      MokoIconViewItem *item = items->data;

      if (icon_view->priv->orientation == GTK_ORIENTATION_HORIZONTAL)
	{
	  item->pixbuf_y = item->y;
	  item->layout_y = item->y + ICON_TEXT_PADDING + focus_width + focus_pad;
	}
      else 
	{
	  item->pixbuf_y = item->y + (max_pixbuf_height - item->pixbuf_height);
	  if (icon_view->priv->pixbuf_column != -1)
	    item->layout_y = item->pixbuf_y + item->pixbuf_height + icon_view->priv->spacing + ICON_TEXT_PADDING + focus_width + focus_pad;
	  else
	    item->layout_y = item->y + ICON_TEXT_PADDING + focus_width + focus_pad;
      }
      /* Update the bounding box */
      item->y = item->pixbuf_y;

      /* We may want to readjust the new y coordinate. */
      if (item->y + item->height > *y)
	*y = item->y + item->height;

      if (rtl)
	item->col = col - 1 - item->col;
    }
  
  return last_item;
}

static void
moko_icon_view_item_invalidate_size (MokoIconViewItem *item)
{
  item->width = -1;
  item->height = -1;
}
static void
moko_icon_view_set_adjustment_upper (GtkAdjustment *adj,
				    gdouble        upper)
{
  if (upper != adj->upper)
    {
      gdouble min = MOKO_MAX (0.0, upper - adj->page_size);
      gboolean value_changed = FALSE;
      
      adj->upper = upper;

      if (adj->value > min)
	{
	  adj->value = min;
	  value_changed = TRUE;
	}
      
      gtk_adjustment_changed (adj);
      
      if (value_changed)
	gtk_adjustment_value_changed (adj);
    }
}

static void
moko_icon_view_layout (MokoIconView *icon_view)
{
  gint y = 0, maximum_width = 0;
  GList *icons;
  GtkWidget *widget;
  gint row;
  gint item_width;

  icon_view->priv->total_items = 0;

  if (!VALID_MODEL_AND_COLUMNS (icon_view))
    return;

  widget = GTK_WIDGET (icon_view);

  item_width = icon_view->priv->item_width;

  if (item_width < 0)
    {
      for (icons = icon_view->priv->items; icons; icons = icons->next)
	{
	  MokoIconViewItem *item = icons->data;
	  moko_icon_view_calculate_item_size (icon_view, item, -1);
	  item_width = MOKO_MAX(item_width, item->width);
	  moko_icon_view_item_invalidate_size (item);
	}
    }
  
  icons = icon_view->priv->items;
  row = 0;

  do
    {
      icons = moko_icon_view_layout_single_row (icon_view, icons, 
					       item_width, row,
					       &y, &maximum_width);
      row++;
    }
  while (icons != NULL);

  if (maximum_width != icon_view->priv->width)
    {
      icon_view->priv->width = maximum_width;
    }
  y += icon_view->priv->margin;
  
   icon_view->priv->height = y ;

  moko_icon_view_set_adjustment_upper (icon_view->priv->hadjustment, icon_view->priv->width);
  moko_icon_view_set_adjustment_upper (icon_view->priv->vadjustment, icon_view->priv->height);

  if (GTK_WIDGET_REALIZED (icon_view))
    {
      gdk_window_resize (icon_view->priv->bin_window,
			 MOKO_MAX (icon_view->priv->width, widget->allocation.width),
			 MOKO_MAX (icon_view->priv->height, widget->allocation.height));
    }

  if (icon_view->priv->layout_idle_id != 0)
    {
      g_source_remove (icon_view->priv->layout_idle_id);
      icon_view->priv->layout_idle_id = 0;
    }

  gtk_widget_queue_draw (GTK_WIDGET (icon_view));
}

static void
moko_icon_view_size_allocate (GtkWidget      *widget,
			     GtkAllocation  *allocation)
{
  MokoIconView *icon_view;

  widget->allocation = *allocation;

  icon_view = MOKO_ICON_VIEW (widget);

  if (GTK_WIDGET_REALIZED (widget))
    {
      gdk_window_move_resize (widget->window,
			      allocation->x, allocation->y,
			      allocation->width, allocation->height);
      gdk_window_resize (icon_view->priv->bin_window,
			 MOKO_MAX (icon_view->priv->width, allocation->width),
			 MOKO_MAX (icon_view->priv->height, allocation->height));
    }

  icon_view->priv->hadjustment->page_size = allocation->width;
  icon_view->priv->hadjustment->page_increment = allocation->width * 0.9;
  icon_view->priv->hadjustment->step_increment = allocation->width * 0.1;
  icon_view->priv->hadjustment->lower = 0;
  icon_view->priv->hadjustment->upper = MOKO_MAX (allocation->width, icon_view->priv->width);
  gtk_adjustment_changed (icon_view->priv->hadjustment);

  icon_view->priv->vadjustment->page_size = allocation->height;
  icon_view->priv->vadjustment->page_increment = allocation->height * 0.9;
  icon_view->priv->vadjustment->step_increment = allocation->width * 0.1;
  icon_view->priv->vadjustment->lower = 0;
  icon_view->priv->vadjustment->upper = MOKO_MAX (allocation->height, icon_view->priv->height);
  gtk_adjustment_changed (icon_view->priv->vadjustment);

  moko_icon_view_layout (icon_view);
}

static void
moko_icon_view_destroy (GtkObject *object)
{
  MokoIconView *icon_view;

  icon_view = MOKO_ICON_VIEW (object);
  
  moko_icon_view_set_model (icon_view, NULL);
  
  if (icon_view->priv->layout_idle_id != 0)
    g_source_remove (icon_view->priv->layout_idle_id);

  if (icon_view->priv->scroll_timeout_id != 0)
    g_source_remove (icon_view->priv->scroll_timeout_id);
  
  (GTK_OBJECT_CLASS (parent_class)->destroy) (object);
}

static void
moko_icon_view_finalize (GObject *object)
{
  MokoIconView *icon_view;

  icon_view = MOKO_ICON_VIEW (object);

  g_object_unref (icon_view->priv->layout);
  
  (G_OBJECT_CLASS (parent_class)->finalize) (object);
}

static void
moko_icon_view_set_property (GObject      *object,
			    guint         prop_id,
			    const GValue *value,
			    GParamSpec   *pspec)
{
  MokoIconView *icon_view;

  icon_view = MOKO_ICON_VIEW (object);

    switch (prop_id)
    {
    case PROP_SELECTION_MODE:
      moko_icon_view_set_selection_mode (icon_view, g_value_get_enum (value));
      break;
    case PROP_PIXBUF_COLUMN:
      moko_icon_view_set_pixbuf_column (icon_view, g_value_get_int (value));
      break;
    case PROP_TEXT_COLUMN:
      moko_icon_view_set_text_column (icon_view, g_value_get_int (value));
      break;
    case PROP_MARKUP_COLUMN:
      moko_icon_view_set_markup_column (icon_view, g_value_get_int (value));
      break;
    case PROP_MODEL:
      moko_icon_view_set_model (icon_view, g_value_get_object (value));
      break;
    case PROP_ORIENTATION:
      moko_icon_view_set_orientation (icon_view, g_value_get_enum (value));
      break;
    case PROP_COLUMNS:
      moko_icon_view_set_columns (icon_view, g_value_get_int (value));
      break;
    case PROP_ITEM_WIDTH:
      moko_icon_view_set_item_width (icon_view, g_value_get_int (value));
      break;
    case PROP_SPACING:
      moko_icon_view_set_spacing (icon_view, g_value_get_int (value));
      break;
    case PROP_ROW_SPACING:
      moko_icon_view_set_row_spacing (icon_view, g_value_get_int (value));
      break;
    case PROP_COLUMN_SPACING:
      moko_icon_view_set_column_spacing (icon_view, g_value_get_int (value));
      break;
    case PROP_MARGIN:
      moko_icon_view_set_margin (icon_view, g_value_get_int (value));
      break;
    case PROP_BG_ICON:
      moko_icon_view_set_icon_bg (icon_view, g_value_get_string (value));
      break;
    case PROP_BG_TEXT:
      moko_icon_view_set_text_bg (icon_view, g_value_get_string (value));
      break;
    case PROP_DECORATION_WIDTH:
      moko_icon_view_set_decoration_width (icon_view, g_value_get_int (value));
      break;
    case PROP_DECORATED:
      moko_icon_view_set_decorated (icon_view, g_value_get_boolean (value));
      break;
    case PROP_MAX_TEXT_LENGTH:
      moko_icon_view_set_max_text_length (icon_view, g_value_get_int (value));
      break;
    	
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
  
}

static void
moko_icon_view_get_property (GObject      *object,
			    guint         prop_id,
			    GValue       *value,
			    GParamSpec   *pspec)
{
  MokoIconView *icon_view;

  icon_view = MOKO_ICON_VIEW (object);

    switch (prop_id)
    {
    case PROP_SELECTION_MODE:
      g_value_set_enum (value, icon_view->priv->selection_mode);
      break;
    case PROP_PIXBUF_COLUMN:
      g_value_set_int (value, icon_view->priv->pixbuf_column);
      break;
    case PROP_TEXT_COLUMN:
      g_value_set_int (value, icon_view->priv->text_column);
      break;
    case PROP_MARKUP_COLUMN:
      g_value_set_int (value, icon_view->priv->markup_column);
      break;
    case PROP_MODEL:
      g_value_set_object (value, icon_view->priv->model);
      break;
    case PROP_ORIENTATION:
      g_value_set_enum (value, icon_view->priv->orientation);
      break;
    case PROP_COLUMNS:
      g_value_set_int (value, icon_view->priv->columns);
      break;
    case PROP_ITEM_WIDTH:
      g_value_set_int (value, icon_view->priv->item_width);
      break;
    case PROP_SPACING:
      g_value_set_int (value, icon_view->priv->spacing);
      break;
    case PROP_ROW_SPACING:
      g_value_set_int (value, icon_view->priv->row_spacing);
      break;
    case PROP_COLUMN_SPACING:
      g_value_set_int (value, icon_view->priv->column_spacing);
      break;
    case PROP_MARGIN:
      g_value_set_int (value, icon_view->priv->margin);
      break;
    case PROP_DECORATION_WIDTH:
      g_value_set_int (value, icon_view->priv->decr_width);
      break;
    case PROP_DECORATED:
      g_value_set_boolean (value, icon_view->priv->decorated);
      break;
    case PROP_MAX_TEXT_LENGTH:
      g_value_set_int (value, icon_view->priv->max_text_len);
      break;
    case PPOP_CURSOR_POSITION:
      g_value_set_int (value, icon_view->priv->cursor_item->index);
      break;
    case PROP_TOTAL_ITEMS:
      g_value_set_int (value, icon_view->priv->total_items);
      break;
      	
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }

}

static void
moko_icon_view_queue_draw_item (MokoIconView     *icon_view,
			       MokoIconViewItem *item)
{
  GdkRectangle rect;

  rect.x = item->x;
  rect.y = item->y;
  rect.width = item->width;
  rect.height = item->height;

  if (icon_view->priv->bin_window)
    gdk_window_invalidate_rect (icon_view->priv->bin_window, &rect, TRUE);
}
static void
moko_icon_view_invalidate_sizes (MokoIconView *icon_view)
{
  g_list_foreach (icon_view->priv->items,
		  (GFunc)moko_icon_view_item_invalidate_size, NULL);
}

static gboolean
layout_callback (gpointer user_data)
{
  MokoIconView *icon_view;

  GDK_THREADS_ENTER ();

  icon_view = MOKO_ICON_VIEW (user_data);
  
  icon_view->priv->layout_idle_id = 0;

  moko_icon_view_layout (icon_view);
  
  GDK_THREADS_LEAVE();

  return FALSE;
}

static void
moko_icon_view_queue_layout (MokoIconView *icon_view)
{
  if (icon_view->priv->layout_idle_id != 0)
    return;

  icon_view->priv->layout_idle_id = g_idle_add (layout_callback, icon_view);
}

static void
moko_icon_view_set_cursor_item (MokoIconView     *icon_view,
			       MokoIconViewItem *item)
{
  AtkObject *obj;
  AtkObject *item_obj;

  if (icon_view->priv->cursor_item == item)
    return;

  if (icon_view->priv->cursor_item != NULL)
    moko_icon_view_queue_draw_item (icon_view, icon_view->priv->cursor_item);
  
  icon_view->priv->cursor_item = item;
  moko_icon_view_queue_draw_item (icon_view, item);
  
  /* Notify that accessible focus object has changed */
  obj = gtk_widget_get_accessible (GTK_WIDGET (icon_view));
  item_obj = atk_object_ref_accessible_child (obj, item->index);

  if (item_obj != NULL)
    {
      atk_focus_tracker_notify (item_obj);
      g_object_unref (item_obj); 
    }
}


static MokoIconViewItem *
moko_icon_view_item_new (void)
{
  MokoIconViewItem *item;

  item = g_new0 (MokoIconViewItem, 1);

  item->width = -1;
  item->height = -1;
  
  return item;
}

static void
moko_icon_view_item_free (MokoIconViewItem *item)
{
  g_return_if_fail (item != NULL);

  g_free (item);
}

static void
moko_icon_view_update_item_text (MokoIconView     *icon_view,
				MokoIconViewItem *item)
{
  gboolean iters_persist;
  GtkTreeIter iter;
  GtkTreePath *path;
  gchar *text;
  
  iters_persist = gtk_tree_model_get_flags (icon_view->priv->model) & GTK_TREE_MODEL_ITERS_PERSIST;
  
  if (!iters_persist)
    {
      path = gtk_tree_path_new_from_indices (item->index, -1);
      gtk_tree_model_get_iter (icon_view->priv->model, &iter, path);
      gtk_tree_path_free (path);
    }
  else
    iter = item->iter;

  if (icon_view->priv->markup_column != -1)
    {
      gtk_tree_model_get (icon_view->priv->model, &iter,
			  icon_view->priv->markup_column, &text,
			  -1);
      pango_layout_set_markup (icon_view->priv->layout, text, -1);
      g_free (text);        
    }
  else if (icon_view->priv->text_column != -1)
    {
      gtk_tree_model_get (icon_view->priv->model, &iter,
			  icon_view->priv->text_column, &text,
			  -1);
      if (strlen(text) > icon_view->priv->max_text_len)
      	{
      	   pango_layout_set_text (icon_view->priv->layout, text, icon_view->priv->max_text_len);
      	}
      else
      	   pango_layout_set_text (icon_view->priv->layout, text, -1);
      g_free (text); 
    }
  else
      pango_layout_set_text (icon_view->priv->layout, "", -1);
}

static GdkPixbuf *
moko_icon_view_get_item_icon (MokoIconView      *icon_view,
			     MokoIconViewItem  *item)
{
  gboolean iters_persist;
  GtkTreeIter iter;
  GtkTreePath *path;
  GdkPixbuf *pixbuf;
  
  g_return_val_if_fail (item != NULL, NULL);

  iters_persist = gtk_tree_model_get_flags (icon_view->priv->model) & GTK_TREE_MODEL_ITERS_PERSIST;
  
  if (!iters_persist)
    {
      path = gtk_tree_path_new_from_indices (item->index, -1);
      gtk_tree_model_get_iter (icon_view->priv->model, &iter, path);
      gtk_tree_path_free (path);
    }
  else
    iter = item->iter;
  
  gtk_tree_model_get (icon_view->priv->model, &iter,
		      icon_view->priv->pixbuf_column, &pixbuf,
		      -1);

  return pixbuf;
}


static MokoIconViewItem *
moko_icon_view_get_item_at_pos (MokoIconView *icon_view,
			       gint         x,
			       gint         y)
{
  GList *items;
  
  for (items = icon_view->priv->items; items; items = items->next)
    {
      MokoIconViewItem *item = items->data;
      
      if (x > item->x && x < item->x + item->width &&
	  y > item->y && y < item->y + item->height)
	{
	  /* Check if the mouse is inside the icon or the label */
	  if ((x > item->pixbuf_x && x < item->pixbuf_x + item->pixbuf_width &&
	       y > item->pixbuf_y && y < item->pixbuf_y + item->pixbuf_height) ||
	      (x > item->layout_x - ICON_TEXT_PADDING &&
	       x < item->layout_x + item->layout_width + ICON_TEXT_PADDING &&
	       y > item->layout_y - ICON_TEXT_PADDING &&
	       y < item->layout_y + item->layout_height + ICON_TEXT_PADDING))
	    return item;
	}
    }

  return NULL;
}

static void
moko_icon_view_select_item (MokoIconView      *icon_view,
			   MokoIconViewItem  *item)
{
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));
  g_return_if_fail (item != NULL);

  if (item->selected)
    return;
  
  if (icon_view->priv->selection_mode == GTK_SELECTION_NONE)
    return;
  else if (icon_view->priv->selection_mode != GTK_SELECTION_MULTIPLE)
    moko_icon_view_unselect_all_internal (icon_view);

  item->selected = TRUE;

  moko_icon_view_queue_draw_item (icon_view, item);

  g_signal_emit (icon_view, moko_icon_view_signals[SELECTION_CHANGED], 0);
}


static void
moko_icon_view_unselect_item (MokoIconView      *icon_view,
			     MokoIconViewItem  *item)
{
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));
  g_return_if_fail (item != NULL);

  if (!item->selected)
    return;
  
  if (icon_view->priv->selection_mode == GTK_SELECTION_NONE ||
      icon_view->priv->selection_mode == GTK_SELECTION_BROWSE)
    return;
  
  item->selected = FALSE;

  g_signal_emit (icon_view, moko_icon_view_signals[SELECTION_CHANGED], 0);

  moko_icon_view_queue_draw_item (icon_view, item);
}

static void
verify_items (MokoIconView *icon_view)
{
  GList *items;
  int i = 0;

  for (items = icon_view->priv->items; items; items = items->next)
    {
      MokoIconViewItem *item = items->data;

      if (item->index != i)
	g_error ("List item does not match its index: item index %d and list index %d\n", item->index, i);

      i++;
    }
}

static void
moko_icon_view_row_changed (GtkTreeModel *model,
			   GtkTreePath  *path,
			   GtkTreeIter  *iter,
			   gpointer      data)
{
  MokoIconViewItem *item;
  gint index;
  MokoIconView *icon_view;

  icon_view = MOKO_ICON_VIEW (data);
  
  index = gtk_tree_path_get_indices(path)[0];
  item = g_list_nth (icon_view->priv->items, index)->data;

  moko_icon_view_item_invalidate_size (item);
  moko_icon_view_queue_layout (icon_view);

  verify_items (icon_view);
}

static void
moko_icon_view_row_inserted (GtkTreeModel *model,
			    GtkTreePath  *path,
			    GtkTreeIter  *iter,
			    gpointer      data)
{
  gint length, index;
  MokoIconViewItem *item;
  gboolean iters_persist;
  MokoIconView *icon_view;
  GList *list;
  
  icon_view = MOKO_ICON_VIEW (data);
  iters_persist = gtk_tree_model_get_flags (icon_view->priv->model) & GTK_TREE_MODEL_ITERS_PERSIST;
  
  length = gtk_tree_model_iter_n_children (model, NULL);
  index = gtk_tree_path_get_indices(path)[0];

  item = moko_icon_view_item_new ();

  if (iters_persist)
    item->iter = *iter;

  item->index = index;

  /* FIXME: We can be more efficient here,
     we can store a tail pointer and use that when
     appending (which is a rather common operation)
  */
  icon_view->priv->items = g_list_insert (icon_view->priv->items,
					 item, index);
  
  list = g_list_nth (icon_view->priv->items, index + 1);
  for (; list; list = list->next)
    {
      item = list->data;

      item->index++;
    }
    
  verify_items (icon_view);
}

static void
moko_icon_view_row_deleted (GtkTreeModel *model,
			   GtkTreePath  *path,
			   gpointer      data)
{
  gint index;
  MokoIconView *icon_view;
  MokoIconViewItem *item;
  GList *list, *next;
  gboolean emit = FALSE;
  
  icon_view = MOKO_ICON_VIEW (data);

  index = gtk_tree_path_get_indices(path)[0];

  list = g_list_nth (icon_view->priv->items, index);
  item = list->data;

  if (item == icon_view->priv->anchor_item)
    icon_view->priv->anchor_item = NULL;

  if (item == icon_view->priv->cursor_item)
    icon_view->priv->cursor_item = NULL;

  if (item->selected)
    emit = TRUE;
  
  moko_icon_view_item_free (item);

  for (next = list->next; next; next = next->next)
    {
      item = next->data;

      item->index--;
    }
  
  icon_view->priv->items = g_list_delete_link (icon_view->priv->items, list);

  moko_icon_view_queue_layout (icon_view);

  verify_items (icon_view);  
  
  if (emit)
    g_signal_emit (icon_view, moko_icon_view_signals[SELECTION_CHANGED], 0);
}

static void
moko_icon_view_rows_reordered (GtkTreeModel *model,
			      GtkTreePath  *parent,
			      GtkTreeIter  *iter,
			      gint         *new_order,
			      gpointer      data)
{
  int i;
  int length;
  MokoIconView *icon_view;
  GList *items = NULL, *list;
  MokoIconViewItem **item_array;
  gint *order;
  
  icon_view = MOKO_ICON_VIEW (data);

  length = gtk_tree_model_iter_n_children (model, NULL);

  order = g_new (gint, length);
  for (i = 0; i < length; i++)
    order [new_order[i]] = i;

  item_array = g_new (MokoIconViewItem *, length);
  for (i = 0, list = icon_view->priv->items; list != NULL; list = list->next, i++)
    item_array[order[i]] = list->data;
  g_free (order);

  for (i = length - 1; i >= 0; i--)
    {
      item_array[i]->index = i;
      items = g_list_prepend (items, item_array[i]);
    }
  
  g_free (item_array);
  g_list_free (icon_view->priv->items);
  icon_view->priv->items = items;

  verify_items (icon_view);  
}

static void
moko_icon_view_build_items (MokoIconView *icon_view)
{
  GtkTreeIter iter;
  int i;
  gboolean iters_persist;
  GList *items = NULL;

  iters_persist = gtk_tree_model_get_flags (icon_view->priv->model) & GTK_TREE_MODEL_ITERS_PERSIST;
  
  if (!gtk_tree_model_get_iter_first (icon_view->priv->model,
				      &iter))
    return;

  i = 0;
  
  do
    {
      MokoIconViewItem *item = moko_icon_view_item_new ();

      if (iters_persist)
	item->iter = iter;

      item->index = i;
      
      i++;

      items = g_list_prepend (items, item);
      
    } while (gtk_tree_model_iter_next (icon_view->priv->model, &iter));

  icon_view->priv->items = g_list_reverse (items);
}

static void
moko_icon_view_add_move_binding (GtkBindingSet  *binding_set,
				guint           keyval,
				guint           modmask,
				GtkMovementStep step,
				gint            count)
{
  
  gtk_binding_entry_add_signal (binding_set, keyval, modmask,
                                "move_cursor", 2,
                                G_TYPE_ENUM, step,
                                G_TYPE_INT, count);

  gtk_binding_entry_add_signal (binding_set, keyval, GDK_SHIFT_MASK,
                                "move_cursor", 2,
                                G_TYPE_ENUM, step,
                                G_TYPE_INT, count);

  if ((modmask & GDK_CONTROL_MASK) == GDK_CONTROL_MASK)
   return;

  gtk_binding_entry_add_signal (binding_set, keyval, GDK_CONTROL_MASK | GDK_SHIFT_MASK,
                                "move_cursor", 2,
                                G_TYPE_ENUM, step,
                                G_TYPE_INT, count);

  gtk_binding_entry_add_signal (binding_set, keyval, GDK_CONTROL_MASK,
                                "move_cursor", 2,
                                G_TYPE_ENUM, step,
                                G_TYPE_INT, count);
}

static gboolean
moko_icon_view_real_move_cursor (MokoIconView     *icon_view,
				GtkMovementStep  step,
				gint             count)
{
  GdkModifierType state;
  g_return_val_if_fail (MOKO_ICON_VIEW (icon_view), FALSE);
  g_return_val_if_fail (step == GTK_MOVEMENT_LOGICAL_POSITIONS ||
			step == GTK_MOVEMENT_VISUAL_POSITIONS ||
			step == GTK_MOVEMENT_DISPLAY_LINES ||
			step == GTK_MOVEMENT_PAGES ||
			step == GTK_MOVEMENT_BUFFER_ENDS, FALSE);

 //if (!GTK_WIDGET_HAS_FOCUS (GTK_WIDGET (icon_view)))
   //return FALSE;

  gtk_widget_grab_focus (GTK_WIDGET (icon_view));

  if (gtk_get_current_event_state (&state))
    {
      if ((state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK)
        icon_view->priv->ctrl_pressed = TRUE;
      if ((state & GDK_SHIFT_MASK) == GDK_SHIFT_MASK)
        icon_view->priv->shift_pressed = TRUE;
    }
  /* else we assume not pressed */
  switch (step)
    {
    case GTK_MOVEMENT_LOGICAL_POSITIONS:
    case GTK_MOVEMENT_VISUAL_POSITIONS:
      moko_icon_view_move_cursor_left_right (icon_view, count);
      break;
    case GTK_MOVEMENT_DISPLAY_LINES:
      moko_icon_view_move_cursor_up_down (icon_view, count);
      break;
    case GTK_MOVEMENT_PAGES:
      moko_icon_view_move_cursor_page_up_down (icon_view, count);
      break;
    case GTK_MOVEMENT_BUFFER_ENDS:
      moko_icon_view_move_cursor_start_end (icon_view, count);
      break;
    default:
      g_assert_not_reached ();
    }
  icon_view->priv->ctrl_pressed = FALSE;
  icon_view->priv->shift_pressed = FALSE;

  return TRUE;
}

static MokoIconViewItem *
find_item (MokoIconView     *icon_view,
	   MokoIconViewItem *current,
	   gint             row_ofs,
	   gint             col_ofs)
{
  gint row, col;
  GList *items;
  MokoIconViewItem *item;

  /* FIXME: this could be more efficient 
   */
  row = current->row + row_ofs;
  col = current->col + col_ofs;

  for (items = icon_view->priv->items; items; items = items->next)
    {
      item = items->data;
      if (item->row == row && item->col == col)
	return item;
    }
  
  return NULL;
}


static MokoIconViewItem *
find_item_page_up_down (MokoIconView     *icon_view,
			MokoIconViewItem *current,
			gint             count)
{
  GList *item, *next;
  gint y, col;
  
  col = current->col;
  y = current->y + count * icon_view->priv->vadjustment->page_size;

  item = g_list_find (icon_view->priv->items, current);
  if (count > 0)
    {
      while (item)
	{
	  for (next = item->next; next; next = next->next)
	    {
	      if (((MokoIconViewItem *)next->data)->col == col)
		break;
	    }
	  if (!next || ((MokoIconViewItem *)next->data)->y > y)
	    break;

	  item = next;
	}
    }
  else 
    {
      while (item)
	{
	  for (next = item->prev; next; next = next->prev)
	    {
	      if (((MokoIconViewItem *)next->data)->col == col)
		break;
	    }
	  if (!next || ((MokoIconViewItem *)next->data)->y < y)
	    break;

	  item = next;
	}
    }

  if (item)
    return item->data;

  return NULL;
}

static gboolean
moko_icon_view_select_all_between (MokoIconView     *icon_view,
				  MokoIconViewItem *anchor,
				  MokoIconViewItem *cursor)
{
  GList *items;
  MokoIconViewItem *item;
  gint row1, row2, col1, col2;
  gboolean dirty = FALSE;
  
  if (anchor->row < cursor->row)
    {
      row1 = anchor->row;
      row2 = cursor->row;
    }
  else
    {
      row1 = cursor->row;
      row2 = anchor->row;
    }

  if (anchor->col < cursor->col)
    {
      col1 = anchor->col;
      col2 = cursor->col;
    }
  else
    {
      col1 = cursor->col;
      col2 = anchor->col;
    }

  for (items = icon_view->priv->items; items; items = items->next)
    {
      item = items->data;

      if (row1 <= item->row && item->row <= row2 &&
	  col1 <= item->col && item->col <= col2)
	{
	  if (!item->selected)
	    dirty = TRUE;

	  item->selected = TRUE;
	  
	  moko_icon_view_queue_draw_item (icon_view, item);
	}
    }

  return dirty;
}

static void 
moko_icon_view_move_cursor_up_down (MokoIconView *icon_view,
				   gint         count)
{
  MokoIconViewItem *item;
  gboolean dirty = FALSE;
  
 // if (!GTK_WIDGET_HAS_FOCUS (icon_view)) 
    //return;

  if (icon_view->priv->cursor_item)
    {
	g_debug ("cursor item row = %d, col = %d", icon_view->priv->cursor_item->row, icon_view->priv->cursor_item->col);
     item = find_item (icon_view, 
		      icon_view->priv->cursor_item,
		      count, 0);

     while (!item && count >= 0) //SUNZY:Can't find next row item in the same column. 
        { 
           gint col = 0;
           for (col ; col + icon_view->priv->columns >0 && !item ; col--)
  	   	item = find_item (icon_view, 
		      icon_view->priv->cursor_item,
		      count, col);
	     count --;
	     col = icon_view->priv->columns;
    	    g_debug ("count = %d column = %d", count, col);
        }
    }
  else
    {
      GList *list;

      if (count > 0)
	list = icon_view->priv->items;
      else
	list = g_list_last (icon_view->priv->items);

      item = list ? list->data : NULL;
    }
 
  if (!item)
	return;

  if (icon_view->priv->ctrl_pressed ||
      !icon_view->priv->shift_pressed ||
      !icon_view->priv->anchor_item ||
      icon_view->priv->selection_mode != GTK_SELECTION_MULTIPLE)
    icon_view->priv->anchor_item = item;

  moko_icon_view_set_cursor_item (icon_view, item);

  if (!icon_view->priv->ctrl_pressed &&
      icon_view->priv->selection_mode != GTK_SELECTION_NONE)
    {
      moko_icon_view_unselect_all_internal (icon_view);
      dirty = moko_icon_view_select_all_between (icon_view, 
						icon_view->priv->anchor_item,
						item);
    }

  moko_icon_view_scroll_to_item (icon_view, item);

  if (dirty)
    g_signal_emit (icon_view, moko_icon_view_signals[SELECTION_CHANGED], 0);
}

static void 
moko_icon_view_move_cursor_page_up_down (MokoIconView *icon_view,
					gint         count)
{
  MokoIconViewItem *item;
  gboolean dirty = FALSE;
  
  if (!GTK_WIDGET_HAS_FOCUS (icon_view))
    return;
  
  if (!icon_view->priv->cursor_item)
    {
      GList *list;

      if (count > 0)
	list = icon_view->priv->items;
      else
	list = g_list_last (icon_view->priv->items);

      item = list ? list->data : NULL;
    }
  else
    item = find_item_page_up_down (icon_view, 
				   icon_view->priv->cursor_item,
				   count);

  if (!item)
    return;

  if (icon_view->priv->ctrl_pressed ||
      !icon_view->priv->shift_pressed ||
      !icon_view->priv->anchor_item ||
      icon_view->priv->selection_mode != GTK_SELECTION_MULTIPLE)
    icon_view->priv->anchor_item = item;

  moko_icon_view_set_cursor_item (icon_view, item);

  if (!icon_view->priv->ctrl_pressed &&
      icon_view->priv->selection_mode != GTK_SELECTION_NONE)
    {
      moko_icon_view_unselect_all_internal (icon_view);
      dirty = moko_icon_view_select_all_between (icon_view, 
						icon_view->priv->anchor_item,
						item);
    }

  moko_icon_view_scroll_to_item (icon_view, item);

  if (dirty)
    g_signal_emit (icon_view, moko_icon_view_signals[SELECTION_CHANGED], 0);  
}

static void 
moko_icon_view_move_cursor_left_right (MokoIconView *icon_view,
				      gint         count)
{
  MokoIconViewItem *item;
  gboolean dirty = FALSE;
  
  if (!GTK_WIDGET_HAS_FOCUS (icon_view))
    return;
  
  if (!icon_view->priv->cursor_item)
    {
      GList *list;

      if (count > 0)
	list = icon_view->priv->items;
      else
	list = g_list_last (icon_view->priv->items);

      item = list ? list->data : NULL;
    }
  else
    item = find_item (icon_view, 
		      icon_view->priv->cursor_item,
		      0, count);

  if (!item)
    return;

  if (icon_view->priv->ctrl_pressed ||
      !icon_view->priv->shift_pressed ||
      !icon_view->priv->anchor_item ||
      icon_view->priv->selection_mode != GTK_SELECTION_MULTIPLE)
    icon_view->priv->anchor_item = item;

  moko_icon_view_set_cursor_item (icon_view, item);

  if (!icon_view->priv->ctrl_pressed &&
      icon_view->priv->selection_mode != GTK_SELECTION_NONE)
    {
      moko_icon_view_unselect_all_internal (icon_view);
      dirty = moko_icon_view_select_all_between (icon_view, 
						icon_view->priv->anchor_item,
						item);
    }

  moko_icon_view_scroll_to_item (icon_view, item);

  if (dirty)
    g_signal_emit (icon_view, moko_icon_view_signals[SELECTION_CHANGED], 0);
}

static void 
moko_icon_view_move_cursor_start_end (MokoIconView *icon_view,
				     gint         count)
{
  MokoIconViewItem *item;
  GList *list;
  gboolean dirty = FALSE;
  
  if (!GTK_WIDGET_HAS_FOCUS (icon_view))
    return;
  
  if (count < 0)
    list = icon_view->priv->items;
  else
    list = g_list_last (icon_view->priv->items);
  
  item = list ? list->data : NULL;

  if (!item)
    return;

  if (icon_view->priv->ctrl_pressed ||
      !icon_view->priv->shift_pressed ||
      !icon_view->priv->anchor_item ||
      icon_view->priv->selection_mode != GTK_SELECTION_MULTIPLE)
    icon_view->priv->anchor_item = item;

  moko_icon_view_set_cursor_item (icon_view, item);

  if (!icon_view->priv->ctrl_pressed &&
      icon_view->priv->selection_mode != GTK_SELECTION_NONE)
    {
      moko_icon_view_unselect_all (icon_view);
      dirty = moko_icon_view_select_all_between (icon_view, 
						icon_view->priv->anchor_item,
						item);
    }

  moko_icon_view_scroll_to_item (icon_view, item);

  if (dirty)
    g_signal_emit (icon_view, moko_icon_view_signals[SELECTION_CHANGED], 0);
}

static void     
moko_icon_view_scroll_to_item (MokoIconView     *icon_view, 
			      MokoIconViewItem *item)
{
  gint y, height;
  gdouble value;
  MokoIconViewItem *above_item = find_item (icon_view, item, -1, 0);

  gdk_drawable_get_size (GDK_DRAWABLE (icon_view->priv->bin_window), NULL, &height);
  gdk_window_get_position (icon_view->priv->bin_window, NULL, &y);

  if (y + item->y < 0)
    {
      value = icon_view->priv->vadjustment->value + y + item->y;
      gtk_adjustment_set_value (icon_view->priv->vadjustment, value);
    }
  else if (above_item && y + item->y > 0)
    {      
      value = icon_view->priv->vadjustment->value + y + above_item->y;
      gtk_adjustment_set_value (icon_view->priv->vadjustment, value);
    }
 
}

/* Public API */


/**
 * moko_icon_view_new:
 * 
 * Creates a new #MokoIconView widget
 * 
 * Return value: A newly created #MokoIconView widget
 *
 **/
GtkWidget *
moko_icon_view_new (void)
{
  return g_object_new (MOKO_TYPE_ICON_VIEW, NULL);
}

/**
 * moko_icon_view_new_with_model:
 * @model: The model.
 * 
 * Creates a new #MokoIconView widget with the model @model.
 * 
 * Return value: A newly created #MokoIconView widget.
 *
 **/
GtkWidget *
moko_icon_view_new_with_model (GtkTreeModel *model)
{
  return g_object_new (MOKO_TYPE_ICON_VIEW, "model", model, NULL);
}


/**
 * moko_icon_view_get_path_at_pos:
 * @icon_view: A #MokoIconView.
 * @x: The x position to be identified
 * @y: The y position to be identified
 * 
 * Finds the path at the point (@x, @y), relative to widget coordinates.
 * 
 * Return value: The #GtkTreePath corresponding to the icon or %NULL
 * if no icon exists at that position.
 *
 **/
GtkTreePath *
moko_icon_view_get_path_at_pos (MokoIconView *icon_view,
			       gint         x,
			       gint         y)
{
  MokoIconViewItem *item;
  GtkTreePath *path;
  
  g_return_val_if_fail (MOKO_IS_ICON_VIEW (icon_view), NULL);

  item = moko_icon_view_get_item_at_pos (icon_view, x, y);

  if (!item)
    return NULL;

  path = gtk_tree_path_new_from_indices (item->index, -1);

  return path;
}

/**
 * moko_icon_view_selected_foreach:
 * @icon_view: A #MokoIconView.
 * @func: The funcion to call for each selected icon.
 * @data: User data to pass to the function.
 * 
 * Calls a function for each selected icon. Note that the model or
 * selection cannot be modified from within this function.
 *
 **/
void
moko_icon_view_selected_foreach (MokoIconView           *icon_view,
				MokoIconViewForeachFunc func,
				gpointer               data)
{
  GList *list;
  
  for (list = icon_view->priv->items; list; list = list->next)
    {
      MokoIconViewItem *item = list->data;
      GtkTreePath *path = gtk_tree_path_new_from_indices (item->index, -1);

      if (item->selected)
	(* func) (icon_view, path, data);

      gtk_tree_path_free (path);
    }
}

/**
 * moko_icon_view_set_selection_mode:
 * @icon_view: A #MokoIconView.
 * @mode: The selection mode
 * 
 * Sets the selection mode of the @icon_view.
 *
 **/
void
moko_icon_view_set_selection_mode (MokoIconView      *icon_view,
				  GtkSelectionMode  mode)
{
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));

  if (mode == icon_view->priv->selection_mode)
    return;
  
  if (mode == GTK_SELECTION_NONE ||
      icon_view->priv->selection_mode == GTK_SELECTION_MULTIPLE)
    moko_icon_view_unselect_all (icon_view);
  
  icon_view->priv->selection_mode = mode;

  g_object_notify (G_OBJECT (icon_view), "selection_mode");
}

/**
 * moko_icon_view_get_selection_mode:
 * @icon_view: A #MokoIconView.
 * 
 * Gets the selection mode of the @icon_view.
 *
 * Return value: the current selection mode
 *
 **/
GtkSelectionMode
moko_icon_view_get_selection_mode (MokoIconView *icon_view)
{
  g_return_val_if_fail (MOKO_IS_ICON_VIEW (icon_view), GTK_SELECTION_SINGLE);

  return icon_view->priv->selection_mode;
}

/**
 * moko_icon_view_set_model:
 * @icon_view: A #MokoIconView.
 * @model: The model.
 *
 * Sets the model for a #MokoIconView.  
 * If the @icon_view already has a model set, it will remove 
 * it before setting the new model.  If @model is %NULL, then
 * it will unset the old model.
 *
 **/
void
moko_icon_view_set_model (MokoIconView *icon_view,
			 GtkTreeModel *model)
{
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));
  g_return_if_fail (model == NULL || GTK_IS_TREE_MODEL (model));
  
  if (icon_view->priv->model == model)
    return;

  if (model)
    {
      GType column_type;
      
      g_return_if_fail (gtk_tree_model_get_flags (model) & GTK_TREE_MODEL_LIST_ONLY);

      if (icon_view->priv->pixbuf_column != -1)
	{
	  column_type = gtk_tree_model_get_column_type (model,
							icon_view->priv->pixbuf_column);	  

	  g_return_if_fail (column_type == GDK_TYPE_PIXBUF);
	}

      if (icon_view->priv->text_column != -1)
	{
	  column_type = gtk_tree_model_get_column_type (model,
							icon_view->priv->text_column);	  

	  g_return_if_fail (column_type == G_TYPE_STRING);
	}

      if (icon_view->priv->markup_column != -1)
	{
	  column_type = gtk_tree_model_get_column_type (model,
							icon_view->priv->markup_column);	  

	  g_return_if_fail (column_type == G_TYPE_STRING);
	}
      
    }
  
  if (icon_view->priv->model)
    {
      g_signal_handlers_disconnect_by_func (icon_view->priv->model,
					    moko_icon_view_row_changed,
					    icon_view);
      g_signal_handlers_disconnect_by_func (icon_view->priv->model,
					    moko_icon_view_row_inserted,
					    icon_view);
      g_signal_handlers_disconnect_by_func (icon_view->priv->model,
					    moko_icon_view_row_deleted,
					    icon_view);
      g_signal_handlers_disconnect_by_func (icon_view->priv->model,
					    moko_icon_view_rows_reordered,
					    icon_view);

      g_object_unref (icon_view->priv->model);
      
      g_list_foreach (icon_view->priv->items, (GFunc)moko_icon_view_item_free, NULL);
      g_list_free (icon_view->priv->items);
      icon_view->priv->items = NULL;
      icon_view->priv->anchor_item = NULL;
      icon_view->priv->cursor_item = NULL;
      icon_view->priv->last_single_clicked = NULL;
    }

  icon_view->priv->model = model;

  if (icon_view->priv->model)
    {
      g_object_ref (icon_view->priv->model);
      g_signal_connect (icon_view->priv->model,
			"row_changed",
			G_CALLBACK (moko_icon_view_row_changed),
			icon_view);
      g_signal_connect (icon_view->priv->model,
			"row_inserted",
			G_CALLBACK (moko_icon_view_row_inserted),
			icon_view);
      g_signal_connect (icon_view->priv->model,
			"row_deleted",
			G_CALLBACK (moko_icon_view_row_deleted),
			icon_view);
      g_signal_connect (icon_view->priv->model,
			"rows_reordered",
			G_CALLBACK (moko_icon_view_rows_reordered),
			icon_view);

      moko_icon_view_build_items (icon_view);
    }

  moko_icon_view_queue_layout (icon_view);

  g_object_notify (G_OBJECT (icon_view), "model");  
}

/**
 * moko_icon_view_get_model:
 * @icon_view: a #MokoIconView
 *
 * Returns the model the #MokoIconView is based on.  Returns %NULL if the
 * model is unset.
 *
 * Return value: A #GtkTreeModel, or %NULL if none is currently being used.
 *
 **/
GtkTreeModel *
moko_icon_view_get_model (MokoIconView *icon_view)
{
  g_return_val_if_fail (MOKO_IS_ICON_VIEW (icon_view), NULL);

  return icon_view->priv->model;
}

/**
 * moko_icon_view_set_text_column:
 * @icon_view: A #MokoIconView.
 * @column: A column in the currently used model.
 * 
 * Sets the column with text for @icon_view to be @column. The text
 * column must be of type #G_TYPE_STRING.
 *
 **/
void
moko_icon_view_set_text_column (MokoIconView *icon_view,
			       gint          column)
{
  if (column == icon_view->priv->text_column)
    return;
  
  if (column == -1)
    icon_view->priv->text_column = -1;
  else
    {
      if (icon_view->priv->model != NULL)
	{
	  GType column_type;
	  
	  column_type = gtk_tree_model_get_column_type (icon_view->priv->model, column);

	  g_return_if_fail (column_type == G_TYPE_STRING);
	}
      
      icon_view->priv->text_column = column;
    }

  moko_icon_view_invalidate_sizes (icon_view);
  moko_icon_view_queue_layout (icon_view);
  
  g_object_notify (G_OBJECT (icon_view), "text_column");
}

/**
 * moko_icon_view_get_text_column:
 * @icon_view: A #MokoIconView.
 *
 * Returns the column with text for @icon_view.
 *
 * Returns: the text column, or -1 if it's unset.
 *
 */
gint
moko_icon_view_get_text_column (MokoIconView  *icon_view)
{
  g_return_val_if_fail (MOKO_IS_ICON_VIEW (icon_view), -1);

  return icon_view->priv->text_column;
}

/**
 * moko_icon_view_set_markup_column:
 * @icon_view: A #MokoIconView.
 * @column: A column in the currently used model.
 * 
 * Sets the column with markup information for @icon_view to be
 * @column. The markup column must be of type #G_TYPE_STRING.
 * If the markup column is set to something, it overrides
 * the text column set by moko_icon_view_set_text_column().
 *
 **/
void
moko_icon_view_set_markup_column (MokoIconView *icon_view,
				 gint         column)
{
  if (column == icon_view->priv->markup_column)
    return;
  
  if (column == -1)
    icon_view->priv->markup_column = -1;
  else
    {
      if (icon_view->priv->model != NULL)
	{
	  GType column_type;
	  
	  column_type = gtk_tree_model_get_column_type (icon_view->priv->model, column);

	  g_return_if_fail (column_type == G_TYPE_STRING);
	}
      
      icon_view->priv->markup_column = column;
    }

  moko_icon_view_invalidate_sizes (icon_view);
  moko_icon_view_queue_layout (icon_view);
  
  g_object_notify (G_OBJECT (icon_view), "markup_column");
}

/**
 * moko_icon_view_get_markup_column:
 * @icon_view: A #MokoIconView.
 *
 * Returns the column with markup text for @icon_view.
 *
 * Returns: the markup column, or -1 if it's unset.
 *
 */
gint
moko_icon_view_get_markup_column (MokoIconView  *icon_view)
{
  g_return_val_if_fail (MOKO_IS_ICON_VIEW (icon_view), -1);

  return icon_view->priv->markup_column;
}

/**
 * moko_icon_view_set_pixbuf_column:
 * @icon_view: A #MokoIconView.
 * @column: A column in the currently used model.
 * 
 * Sets the column with pixbufs for @icon_view to be @column. The pixbuf
 * column must be of type #GDK_TYPE_PIXBUF
 *
 **/
void
moko_icon_view_set_pixbuf_column (MokoIconView *icon_view,
				 gint         column)
{
  if (column == icon_view->priv->pixbuf_column)
    return;
  
  if (column == -1)
    icon_view->priv->pixbuf_column = -1;
  else
    {
      if (icon_view->priv->model != NULL)
	{
	  GType column_type;
	  
	  column_type = gtk_tree_model_get_column_type (icon_view->priv->model, column);

	  g_return_if_fail (column_type == GDK_TYPE_PIXBUF);
	}
      
      icon_view->priv->pixbuf_column = column;
    }

  moko_icon_view_invalidate_sizes (icon_view);
  moko_icon_view_queue_layout (icon_view);
  
  g_object_notify (G_OBJECT (icon_view), "pixbuf_column");
  
}

/**
 * moko_icon_view_get_pixbuf_column:
 * @icon_view: A #MokoIconView.
 *
 * Returns the column with pixbufs for @icon_view.
 *
 * Returns: the pixbuf column, or -1 if it's unset.
 *
 */
gint
moko_icon_view_get_pixbuf_column (MokoIconView  *icon_view)
{
  g_return_val_if_fail (MOKO_IS_ICON_VIEW (icon_view), -1);

  return icon_view->priv->pixbuf_column;
}

/**
 * moko_icon_view_select_path:
 * @icon_view: A #MokoIconView.
 * @path: The #GtkTreePath to be selected.
 * 
 * Selects the row at @path.
 *
 **/
void
moko_icon_view_select_path (MokoIconView *icon_view,
			   GtkTreePath *path)
{
  MokoIconViewItem *item;
  
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));
  g_return_if_fail (icon_view->priv->model != NULL);
  g_return_if_fail (path != NULL);

  item = g_list_nth (icon_view->priv->items,
		     gtk_tree_path_get_indices(path)[0])->data;

  if (!item)
    return;
  
  moko_icon_view_select_item (icon_view, item);
}

/**
 * moko_icon_view_unselect_path:
 * @icon_view: A #MokoIconView.
 * @path: The #GtkTreePath to be unselected.
 * 
 * Unselects the row at @path.
 *
 **/
void
moko_icon_view_unselect_path (MokoIconView *icon_view,
			     GtkTreePath *path)
{
  MokoIconViewItem *item;
  
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));
  g_return_if_fail (icon_view->priv->model != NULL);
  g_return_if_fail (path != NULL);

  item = g_list_nth (icon_view->priv->items,
		     gtk_tree_path_get_indices(path)[0])->data;

  if (!item)
    return;
  
  moko_icon_view_unselect_item (icon_view, item);
}

/**
 * moko_icon_view_get_selected_items:
 * @icon_view: A #MokoIconView.
 *
 * Creates a list of paths of all selected items. Additionally, if you are
 * planning on modifying the model after calling this function, you may
 * want to convert the returned list into a list of #GtkTreeRowReference<!-- -->s.
 * To do this, you can use gtk_tree_row_reference_new().
 *
 * To free the return value, use:
 * <informalexample><programlisting>
 * g_list_foreach (list, gtk_tree_path_free, NULL);
 * g_list_free (list);
 * </programlisting></informalexample>
 *
 * Return value: A #GList containing a #GtkTreePath for each selected row.
 *
 **/
GList *
moko_icon_view_get_selected_items (MokoIconView *icon_view)
{
  GList *list;
  GList *selected = NULL;
  
  g_return_val_if_fail (MOKO_IS_ICON_VIEW (icon_view), NULL);
  
  for (list = icon_view->priv->items; list != NULL; list = list->next)
    {
      MokoIconViewItem *item = list->data;

      if (item->selected)
	{
	  GtkTreePath *path = gtk_tree_path_new_from_indices (item->index, -1);

	  selected = g_list_prepend (selected, path);
	}
    }

  return selected;
}

/**
 * moko_icon_view_select_all:
 * @icon_view: A #MokoIconView.
 * 
 * Selects all the icons. @icon_view must has its selection mode set
 * to #GTK_SELECTION_MULTIPLE.
 *
 **/
void
moko_icon_view_select_all (MokoIconView *icon_view)
{
  GList *items;
  gboolean dirty = FALSE;
  
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));

  if (icon_view->priv->selection_mode != GTK_SELECTION_MULTIPLE)
    return;

  for (items = icon_view->priv->items; items; items = items->next)
    {
      MokoIconViewItem *item = items->data;
      
      if (!item->selected)
	{
	  dirty = TRUE;
	  item->selected = TRUE;
	  moko_icon_view_queue_draw_item (icon_view, item);
	}
    }

  if (dirty)
    g_signal_emit (icon_view, moko_icon_view_signals[SELECTION_CHANGED], 0);
}

/**
 * moko_icon_view_unselect_all:
 * @icon_view: A #MokoIconView.
 * 
 * Unselects all the icons.
 *
 **/
void
moko_icon_view_unselect_all (MokoIconView *icon_view)
{
  gboolean dirty = FALSE;
  
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));

  if (icon_view->priv->selection_mode == GTK_SELECTION_BROWSE)
    return;

  dirty = moko_icon_view_unselect_all_internal (icon_view);

  if (dirty)
    g_signal_emit (icon_view, moko_icon_view_signals[SELECTION_CHANGED], 0);
}

/**
 * moko_icon_view_path_is_selected:
 * @icon_view: A #MokoIconView.
 * @path: A #GtkTreePath to check selection on.
 * 
 * Returns %TRUE if the icon pointed to by @path is currently
 * selected. If @icon does not point to a valid location, %FALSE is returned.
 * 
 * Return value: %TRUE if @path is selected.
 *
 **/
gboolean
moko_icon_view_path_is_selected (MokoIconView *icon_view,
				GtkTreePath *path)
{
  MokoIconViewItem *item;
  
  g_return_val_if_fail (MOKO_IS_ICON_VIEW (icon_view), FALSE);
  g_return_val_if_fail (icon_view->priv->model != NULL, FALSE);
  g_return_val_if_fail (path != NULL, FALSE);
  
  item = g_list_nth (icon_view->priv->items,
		     gtk_tree_path_get_indices(path)[0])->data;

  if (!item)
    return FALSE;
  
  return item->selected;
}

/**
 * moko_icon_view_item_activated:
 * @icon_view: A #MokoIconView
 * @path: The #GtkTreePath to be activated
 * 
 * Activates the item determined by @path.
 *
 **/
void
moko_icon_view_item_activated (MokoIconView      *icon_view,
			      GtkTreePath      *path)
{
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));
  g_return_if_fail (path != NULL);
  
  g_signal_emit (icon_view, moko_icon_view_signals[ITEM_ACTIVATED], 0, path);
}

/**
 * moko_icon_view_set_orientation:
 * @icon_view: a #MokoIconView
 * @orientation: the relative position of texts and icons 
 * 
 * Sets the ::orientation property which determines whether the labels 
 * are drawn beside the icons instead of below.
 *
 **/
void 
moko_icon_view_set_orientation (MokoIconView    *icon_view,
			       GtkOrientation  orientation)
{
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));

  if (icon_view->priv->orientation != orientation)
    {
      icon_view->priv->orientation = orientation;

      moko_icon_view_invalidate_sizes (icon_view);
      moko_icon_view_queue_layout (icon_view);
      
      g_object_notify (G_OBJECT (icon_view), "orientation");
    }
}

/**
 * moko_icon_view_get_orientation:
 * @icon_view: a #MokoIconView
 * 
 * Returns the value of the ::orientation property which determines 
 * whether the labels are drawn beside the icons instead of below. 
 * 
 * Return value: the relative position of texts and icons 
 *
 **/
GtkOrientation
moko_icon_view_get_orientation (MokoIconView *icon_view)
{
  g_return_val_if_fail (MOKO_IS_ICON_VIEW (icon_view), 
			GTK_ORIENTATION_VERTICAL);

  return icon_view->priv->orientation;
}

void 
moko_icon_view_set_columns (MokoIconView *icon_view,
				gint         columns)
{
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));
  
  if (icon_view->priv->columns != columns)
    {
      icon_view->priv->columns = columns;

      moko_icon_view_queue_layout (icon_view);
      
      g_object_notify (G_OBJECT (icon_view), "columns");
    }  
}

gint
moko_icon_view_get_columns (MokoIconView *icon_view)
{
  g_return_val_if_fail (MOKO_IS_ICON_VIEW (icon_view), -1);

  return icon_view->priv->columns;
}

void moko_icon_view_set_item_width (MokoIconView *icon_view,
				   gint         item_width)
{
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));
  
  if (icon_view->priv->item_width != item_width)
    {
      icon_view->priv->item_width = item_width;

      moko_icon_view_invalidate_sizes (icon_view);
      moko_icon_view_queue_layout (icon_view);
      
      g_object_notify (G_OBJECT (icon_view), "item-width");
    }  
}

gint
moko_icon_view_get_item_width (MokoIconView *icon_view)
{
  g_return_val_if_fail (MOKO_IS_ICON_VIEW (icon_view), -1);

  return icon_view->priv->item_width;
}


void moko_icon_view_set_spacing (MokoIconView *icon_view,
				gint         spacing)
{
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));
  
  if (icon_view->priv->spacing != spacing)
    {
      icon_view->priv->spacing = spacing;

      moko_icon_view_invalidate_sizes (icon_view);
      moko_icon_view_queue_layout (icon_view);
      
      g_object_notify (G_OBJECT (icon_view), "spacing");
    }  
}

gint
moko_icon_view_get_spacing (MokoIconView *icon_view)
{
  g_return_val_if_fail (MOKO_IS_ICON_VIEW (icon_view), -1);

  return icon_view->priv->spacing;
}

void moko_icon_view_set_row_spacing (MokoIconView *icon_view,
				    gint         row_spacing)
{
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));
  
  if (icon_view->priv->row_spacing != row_spacing)
    {
      icon_view->priv->row_spacing = row_spacing;

      moko_icon_view_invalidate_sizes (icon_view);
      moko_icon_view_queue_layout (icon_view);
      
      g_object_notify (G_OBJECT (icon_view), "row-spacing");
    }  
}

gint
moko_icon_view_get_row_spacing (MokoIconView *icon_view)
{
  g_return_val_if_fail (MOKO_IS_ICON_VIEW (icon_view), -1);

  return icon_view->priv->row_spacing;
}

void moko_icon_view_set_column_spacing (MokoIconView *icon_view,
				       gint         column_spacing)
{
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));
  
  if (icon_view->priv->column_spacing != column_spacing)
    {
      icon_view->priv->column_spacing = column_spacing;

      moko_icon_view_invalidate_sizes (icon_view);
      moko_icon_view_queue_layout (icon_view);
      
      g_object_notify (G_OBJECT (icon_view), "column-spacing");
    }  
}

gint
moko_icon_view_get_column_spacing (MokoIconView *icon_view)
{
  g_return_val_if_fail (MOKO_IS_ICON_VIEW (icon_view), -1);

  return icon_view->priv->column_spacing;
}

void moko_icon_view_set_margin (MokoIconView *icon_view,
			       gint         margin)
{
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));
  
  if (icon_view->priv->margin != margin)
    {
      icon_view->priv->margin = margin;

      moko_icon_view_invalidate_sizes (icon_view);
      moko_icon_view_queue_layout (icon_view);
      
      g_object_notify (G_OBJECT (icon_view), "margin");
    }  
}

gint
moko_icon_view_get_margin (MokoIconView *icon_view)
{
  g_return_val_if_fail (MOKO_IS_ICON_VIEW (icon_view), -1);

  return icon_view->priv->margin;
}

/**
 *moko_icon_view_set_icon_bg:
 *@icon_view A #MokoIconView
 *@bg_icon A gchar* 
 *
 *Set selected icon column background image, the ::decorated property
 *must be set TRUE first, more see moko_icon_view_set_decorated ().
 *
 */
void
moko_icon_view_set_icon_bg (MokoIconView *icon_view, const gchar *bg_icon)
{
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));
  
  if (bg_icon)
    {
      icon_view->priv->bg_icon = gdk_pixbuf_new_from_file (bg_icon, NULL);
          if (!icon_view->priv->bg_icon)

      moko_icon_view_invalidate_sizes (icon_view);
      moko_icon_view_queue_layout (icon_view);
      
      g_object_notify (G_OBJECT (icon_view), "bg_icon");
    }  
}

/**
 *moko_icon_view_set_text_bg:
 *@icon_view 	a #MokoIconView
 *@bg_text 	gchar*
 *
 *Set selected text column background image, the ::decorated property
 *must be set TRUE first, more see moko_icon_view_set_decorated ().
 *
 */
void
moko_icon_view_set_text_bg (MokoIconView *icon_view, const gchar *bg_text)
{
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));
  
  if (bg_text)
    {
      icon_view->priv->bg_text = gdk_pixbuf_new_from_file (bg_text, NULL);
          if (!icon_view->priv->bg_text)
          
      moko_icon_view_invalidate_sizes (icon_view);
      moko_icon_view_queue_layout (icon_view);
      g_object_notify (G_OBJECT (icon_view), "bg_text");
    }  
}

/**
 *moko_icon_view_set_decoration_width:
 *@icon_view		a #MokoIconView
 *@decr_width		gint
 *
 *Set icon decoration width(pixels), the icon will be scaled with this argumnet.
 *This argument will be efficient when #decorated set.
 */
void 
moko_icon_view_set_decoration_width (MokoIconView *icon_view, gint decr_width)
{
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));
  
  if (icon_view->priv->decr_width!= decr_width)
    {
      icon_view->priv->decr_width = decr_width;

      moko_icon_view_invalidate_sizes (icon_view);
      moko_icon_view_queue_layout (icon_view);
      g_object_notify (G_OBJECT (icon_view), "decr_width");
    }  
}

/**
 *moko_icon_view_get_decoration_width:
 *icon_view a #MokoIconView
 *
 *Returns decoration width.
 *
 */
gint 
moko_icon_view_get_decoration_width (MokoIconView *icon_view)
{
   g_return_val_if_fail (MOKO_IS_ICON_VIEW (icon_view), -1);

   return icon_view->priv->decr_width;
}

/**
 *moko_icon_view_set_decorated:
 *@icon_view 		a #MokoIconView
 *@decorated		gboolean
 *
 *Changer ::decorated property, default value is FALSE.
 *whether use the custom background image to decorate selected item, if you want
 *use this function, you must set icon and text background image first, more
 *see moko_icon_view_set_icon_bg () and moko_icon_view_set_text_bg ().
 *
 */
void
moko_icon_view_set_decorated (MokoIconView *icon_view, gboolean decorated)
{
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));

  decorated = decorated != FALSE;
  
  if (icon_view->priv->decorated != decorated)
    {
      icon_view->priv->decorated = decorated;

        moko_icon_view_invalidate_sizes (icon_view);
        moko_icon_view_queue_layout (icon_view);
      
      g_object_notify (G_OBJECT (icon_view), "decorated");
    } 
    
}

/**
 *moko_icon_view_get_decorated:
 *@icon_view 		a #MokoIconView
 *
 *Return ::decorated property.
 *TRUE	Use costom image to decorated selected item.
 *FALSE 	UnUse custom image to decorated selected item.
 *
 */
gboolean
moko_icon_view_get_decorated (MokoIconView *icon_view)
{
  g_return_val_if_fail (MOKO_IS_ICON_VIEW (icon_view), -1);

    return icon_view->priv->decorated;
}

/**
 *moko_icon_view_set_max_text_length:
 *@icon_view 			a #MokoIconView
 *@max_text_length	gint
 *
 *Set maximum text column length.
 *
 */
void
moko_icon_view_set_max_text_length (MokoIconView *icon_view, gint max_text_length)
{
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));
  
  if (icon_view->priv->max_text_len!= max_text_length)
    {
      icon_view->priv->max_text_len = max_text_length;

      moko_icon_view_invalidate_sizes (icon_view);
      moko_icon_view_queue_layout (icon_view);
      g_object_notify (G_OBJECT (icon_view), "max_text_len");
    }  

}

/**
 *moko_icon_view_get_max_text_length:
 *@icon_view		a #MokoIconView
 *
 *Return Value: ::max_text_len property.
 *
 */
gint
moko_icon_view_get_max_text_length (MokoIconView *icon_view)
{
  g_return_val_if_fail (MOKO_IS_ICON_VIEW (icon_view), -1);

    return icon_view->priv->max_text_len;
}

/**
 *moko_icon_view_get_cursor_positon:
 *@icon_view		a #MokoIconView
 *
 *Return Value: ::cursor_item::index.
 *
 */
gint
moko_icon_view_get_cursor_positon (MokoIconView *icon_view)
{
  g_return_val_if_fail (MOKO_IS_ICON_VIEW (icon_view), -1);
  if (!icon_view->priv->cursor_item)
  	return -1;
  return (icon_view->priv->cursor_item->index + 1);
}

/**
 *moko_icon_view_get_total_items:
 *@icon_view		a #MokoIconView
 *
 *Return Value: ::total_items.
 *
 */
gint
moko_icon_view_get_total_items (MokoIconView *icon_view)
{
  g_return_val_if_fail (MOKO_IS_ICON_VIEW (icon_view), -1);

    return icon_view->priv->total_items;
}

void
moko_icon_view_move_cursor_line_up (MokoIconView *icon_view)
{
    g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));
    
    moko_icon_view_move_cursor_up_down (icon_view, -1);

}

void
moko_icon_view_move_cursor_line_down (MokoIconView *icon_view)
{
    g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));
    
    moko_icon_view_move_cursor_up_down (icon_view, 1);

}
void
moko_icon_view_refresh (MokoIconView *icon_view)
{
  g_return_if_fail (MOKO_IS_ICON_VIEW (icon_view));

  icon_view->priv->cursor_item = NULL;
  if (icon_view->priv->items)
  {
     MokoIconViewItem *item = icon_view->priv->items->data;
     moko_icon_view_scroll_to_item (icon_view, item);     
  }
}
