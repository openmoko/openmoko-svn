#include "mokoiconview.h"
#include "callbacks.h"

#define MOKO_MAX(arg1,arg2)		arg1>arg2?arg1:arg2

#define VALID_MODEL_AND_COLUMNS(obj) ((obj)->priv->model != NULL && \
                                      ((obj)->priv->pixbuf_column != -1 || \
				       (obj)->priv->text_column != -1 || \
				       (obj)->priv->markup_column != -1))

#define ICON_TEXT_PADDING 3

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
} GtkIconViewItem;

struct _GtkIconViewPrivate
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

  GtkIconViewItem *anchor_item;
  GtkIconViewItem *cursor_item;

  guint ctrl_pressed : 1;
  guint shift_pressed : 1;
  
  GtkIconViewItem *last_single_clicked;

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

enum {
    ICON_VIEW_SIGNAL,
    LAST_SIGNAL
};

G_DEFINE_TYPE (MokoIconView, moko_icon_view, GTK_TYPE_ICON_VIEW)

static void 
moko_icon_view_class_init(MokoIconViewClass *klass);

static void 
moko_icon_view_init(MokoIconView *self);

static void
moko_icon_view_realize (GtkWidget *widget);

static gboolean
moko_icon_view_expose (GtkWidget *widget,
		      GdkEventExpose *expose);

static guint icon_view_signals[LAST_SIGNAL] = { 0 };


static void
moko_icon_view_map (GtkWidget *widget)
{
  GtkIconView *icon_view;

  icon_view = GTK_ICON_VIEW (widget);

  GTK_WIDGET_SET_FLAGS (widget, GTK_MAPPED);

  gdk_window_show (icon_view->priv->bin_window);
  gdk_window_show (widget->window);
}

static void
moko_icon_view_unrealize (GtkWidget *widget)
{
  GtkIconView *icon_view;

  icon_view = GTK_ICON_VIEW (widget);

  gdk_window_set_user_data (icon_view->priv->bin_window, NULL);
  gdk_window_destroy (icon_view->priv->bin_window);
  icon_view->priv->bin_window = NULL;

  /* GtkWidget::unrealize destroys children and widget->window */
//  if (GTK_WIDGET_CLASS (parent_class)->unrealize)
    //(* GTK_WIDGET_CLASS (parent_class)->unrealize) (widget);
}

static void
moko_icon_view_size_request (GtkWidget      *widget,
			    GtkRequisition *requisition)
{
  GtkIconView *icon_view;

  icon_view = GTK_ICON_VIEW (widget);

  requisition->width = icon_view->priv->width;
  requisition->height = icon_view->priv->height;
}

static void
moko_icon_view_size_allocate (GtkWidget      *widget,
			     GtkAllocation  *allocation);



/**
*@brief initialize	MokoIconView class.
*@param klass	MokoIconView Class
*@return none
*/
static void 
moko_icon_view_class_init(MokoIconViewClass* klass) /* Class Initialization */
{
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
  //widget_class->realize = moko_icon_view_realize;
  //widget_class->expose_event = moko_icon_view_expose;
  //widget_class->unrealize = moko_icon_view_unrealize;
  //widget_class->map = moko_icon_view_map;
  //widget_class->size_request = moko_icon_view_size_request;
  widget_class->size_allocate = moko_icon_view_size_allocate;
    
   /* icon_view_signals[ICON_VIEW_SIGNAL] = g_signal_new ("MokoIconView",
            G_TYPE_FROM_CLASS (klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (MokoIconViewClass, moko_icon_view_function),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 
            0);
            */
}

/*@brief initialize 	MokoIconView instance
 *@param mm	MokoIconView*
 *@return none
 */
void
moko_icon_view_init(MokoIconView *self)
{
  PangoFontDescription* PangoFont = pango_font_description_new(); //get system default PangoFontDesc
  if (PangoFont)
    pango_font_description_free (PangoFont);
}


/* Construction */
GtkWidget* 
moko_icon_view_new()
{
  return GTK_WIDGET(g_object_new(moko_icon_view_get_type(), NULL));
}

/* Destruction */
void 
moko_icon_view_clear(MokoIconView *self)
{ 
  if (!self) g_free (self);
}

/*
*
*
*/
void
moko_icon_view_update(GtkListStore *store) {
    
}


/* GtkWidget signals */
static void
moko_icon_view_realize (GtkWidget *widget)
{
  GtkIconView *icon_view;
  GdkWindowAttr attributes;
  gint attributes_mask;
  
  icon_view = GTK_ICON_VIEW (widget);

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
  g_debug ("***************** #########");
 // if (widget->style->bg_pixmap[widget->state])
 //{
   //gdk_window_set_back_pixmap (icon_view->priv->bin_window, &widget->style->bg_pixmap[widget->state], FALSE);
   //gdk_window_set_back_pixmap (widget->window, &widget->style->bg_pixmap[widget->state], FALSE);
   
 //}else{
   //gdk_window_set_background (icon_view->priv->bin_window, &widget->style->base[widget->state]);
   //gdk_window_set_background (widget->window, &widget->style->base[widget->state]);
 //	}

 gtk_style_set_background (widget->style, widget->window, widget->state);

 g_debug ("***************** #########");
 sleep (2);
}

static void
gtk_icon_view_update_item_text (GtkIconView     *icon_view,
				GtkIconViewItem *item)
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
      pango_layout_set_text (icon_view->priv->layout, text, -1);
      g_free (text);        
    }
  else
      pango_layout_set_text (icon_view->priv->layout, "", -1);
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

static GdkPixbuf *
gtk_icon_view_get_item_icon (GtkIconView      *icon_view,
			     GtkIconViewItem  *item)
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

static void
gtk_icon_view_paint_item (GtkIconView     *icon_view,
			  GtkIconViewItem *item,
			  GdkRectangle    *area)
{
  gint focus_width, focus_pad;
  GdkPixbuf *pixbuf, *tmp, *bg_pixbuf;
  GtkStateType state;
  gboolean rtl = gtk_widget_get_direction (GTK_WIDGET (icon_view)) == GTK_TEXT_DIR_RTL;

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

  if (icon_view->priv->pixbuf_column != -1)
    {
      tmp = gtk_icon_view_get_item_icon (icon_view, item);
      if (item->selected)
	{
	  pixbuf = create_colorized_pixbuf (tmp,
					    &GTK_WIDGET (icon_view)->style->base[state]);
	  g_object_unref (tmp);
	}
      else
	pixbuf = tmp;
      
      gdk_draw_pixbuf (icon_view->priv->bin_window, NULL, pixbuf,
		       0, 0,
		       item->pixbuf_x, item->pixbuf_y,
		       item->pixbuf_width, item->pixbuf_height,
		       GDK_RGB_DITHER_NORMAL,
		       item->pixbuf_width, item->pixbuf_height);

	/*	       
	//pixbuf = gdk_pixbuf_get_from_drawable	   
	gdk_pixbuf_render_to_drawable_alpha (pixbuf,
                                             GdkDrawable *drawable,
                                             int src_x,
                                             int src_y,
                                             int dest_x,
                                             int dest_y,
                                             int width,
                                             int height,
                                             GdkPixbufAlphaMode alpha_mode,
                                             int alpha_threshold,
                                             GdkRgbDither dither,
                                             int x_dither,
                                             int y_dither);
                                             
      g_object_unref (pixbuf);
      */
    }

  if (icon_view->priv->text_column != -1 ||
      icon_view->priv->markup_column != -1)
    {
      if (item->selected)
	{
	  gdk_draw_rectangle (icon_view->priv->bin_window,
			      GTK_WIDGET (icon_view)->style->base_gc[state],
			      TRUE,
			      item->layout_x - ICON_TEXT_PADDING,
			      item->layout_y - ICON_TEXT_PADDING,
			      item->layout_width + 2 * ICON_TEXT_PADDING,
			      item->layout_height + 2 * ICON_TEXT_PADDING);
	}

      gtk_icon_view_update_item_text (icon_view, item);
      pango_layout_set_alignment (icon_view->priv->layout, rtl ? PANGO_ALIGN_RIGHT: PANGO_ALIGN_LEFT);
      pango_layout_set_width (icon_view->priv->layout, item->layout_width * PANGO_SCALE);
      gtk_paint_layout (GTK_WIDGET (icon_view)->style,
			icon_view->priv->bin_window,
			item->selected ? state : GTK_STATE_NORMAL,
			TRUE, area, GTK_WIDGET (icon_view), "icon_view",
			item->layout_x,
			item->layout_y,
			icon_view->priv->layout);

      if (GTK_WIDGET_HAS_FOCUS (icon_view) &&
	  item == icon_view->priv->cursor_item)
	gtk_paint_focus (GTK_WIDGET (icon_view)->style,
			 icon_view->priv->bin_window,
			 GTK_STATE_NORMAL,
			 area,
			 GTK_WIDGET (icon_view),
			 "icon_view",
			 item->layout_x - ICON_TEXT_PADDING - focus_width - focus_pad,
			 item->layout_y - ICON_TEXT_PADDING - focus_width - focus_pad,
			 item->layout_width + 2 * (ICON_TEXT_PADDING + focus_width + focus_pad),
			 item->layout_height + 2 * (ICON_TEXT_PADDING + focus_width + focus_pad));
			 
    }
}

static gboolean
moko_icon_view_expose (GtkWidget *widget,
		      GdkEventExpose *expose)
{
  GtkIconView *icon_view;
  GList *icons;

    /*GError* err = NULL;
    GdkPixbuf *pixbuf;
    GtkStyle *style;    
    GdkPixmap *pixmap;
    GdkBitmap *bitmap;
    pixbuf = gdk_pixbuf_new_from_file ( PKGDATADIR"/bg_mainmenu.png", &err );
    gdk_pixbuf_render_pixmap_and_mask (pixbuf, &pixmap, &bitmap, NULL);
*/
  icon_view = GTK_ICON_VIEW (widget);

  //gdk_window_set_back_pixmap (icon_view->priv->bin_window, widget->style->bg_pixmap[widget->state], FALSE);

  //widget->style = gtk_style_attach (widget->style, widget->window);

  if (expose->window != icon_view->priv->bin_window)
    return FALSE;

  for (icons = icon_view->priv->items; icons; icons = icons->next) {
    GtkIconViewItem *item = icons->data;
    GdkRectangle item_rectangle;

    item_rectangle.x = item->x;
    item_rectangle.y = item->y;
    item_rectangle.width = item->width;
    item_rectangle.height = item->height;

    if (gdk_region_rect_in (expose->region, &item_rectangle) == GDK_OVERLAP_RECTANGLE_OUT)
      continue;

    gtk_icon_view_paint_item (icon_view, item, &expose->area);

  }

 /* if (icon_view->priv->doing_rubberband)
    {
      GdkRectangle *rectangles;
      gint n_rectangles;
      
      gdk_region_get_rectangles (expose->region,
				 &rectangles,
				 &n_rectangles);
      
      while (n_rectangles--)
	gtk_icon_view_paint_rubberband (icon_view, &rectangles[n_rectangles]);

      g_free (rectangles);
    }
*/
  return TRUE;
}

static void
gtk_icon_view_calculate_item_size (GtkIconView     *icon_view,
				   GtkIconViewItem *item,
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
      pixbuf = gtk_icon_view_get_item_icon (icon_view, item);
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
      maximum_layout_width = MAX (colspan * item_width - item->pixbuf_width - icon_view->priv->spacing - 2 * (ICON_TEXT_PADDING + focus_width + focus_pad), 50);
    }
  else
    maximum_layout_width = MAX (item_width, item->pixbuf_width);
    
  if (icon_view->priv->markup_column != -1 ||
      icon_view->priv->text_column != -1)
    {
      gtk_icon_view_update_item_text (icon_view, item);

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
static GList *
gtk_icon_view_layout_single_row (GtkIconView *icon_view, 
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
      GtkIconViewItem *item = items->data;

      gtk_icon_view_calculate_item_size (icon_view, item, item_width);

      colspan = 1 + (item->width - 1) / (item_width + icon_view->priv->column_spacing);
      current_width += colspan * (item_width + icon_view->priv->column_spacing);
	
      if (items != first_item)
	{
	  if ((icon_view->priv->columns <= 0 && current_width > GTK_WIDGET (icon_view)->allocation.width) ||
	      (icon_view->priv->columns > 0 && col >= icon_view->priv->columns))
	    break;
	}

      item->y = *y;
      item->x = rtl ? GTK_WIDGET (icon_view)->allocation.width - MAX (item_width, item->width) - x : x;

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

      max_height = MAX (max_height, item->height);
      max_pixbuf_height = MAX (max_pixbuf_height, item->pixbuf_height);
      
      if (current_width > *maximum_width)
	*maximum_width = current_width;

      item->row = row;
      item->col = col;

      col += colspan;
      items = items->next;
    }

  last_item = items;

  *y += max_height + icon_view->priv->row_spacing;

  /* Now go through the row again and align the icons */
  for (items = first_item; items != last_item; items = items->next)
    {
      GtkIconViewItem *item = items->data;

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
gtk_icon_view_item_invalidate_size (GtkIconViewItem *item)
{
  item->width = -1;
  item->height = -1;
}
static void
gtk_icon_view_set_adjustment_upper (GtkAdjustment *adj,
				    gdouble        upper)
{
  if (upper != adj->upper)
    {
      gdouble min = MAX (0.0, upper - adj->page_size);
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
gtk_icon_view_layout (GtkIconView *icon_view)
{
  gint y = 0, maximum_width = 0;
  GList *icons;
  GtkWidget *widget;
  gint row;
  gint item_width;

  if (!VALID_MODEL_AND_COLUMNS (icon_view))
    return;

  widget = GTK_WIDGET (icon_view);

  item_width = icon_view->priv->item_width;

  if (item_width < 0)
    {
      for (icons = icon_view->priv->items; icons; icons = icons->next)
	{
	  GtkIconViewItem *item = icons->data;
	  gtk_icon_view_calculate_item_size (icon_view, item, -1);
	  item_width = MAX (item_width, item->width);
	  gtk_icon_view_item_invalidate_size (item);
	}
    }

  icons = icon_view->priv->items;
  y += icon_view->priv->margin;
  row = 0;
  
  do
    {
      icons = gtk_icon_view_layout_single_row (icon_view, icons, 
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
  
  if (y != icon_view->priv->height)
    {
      icon_view->priv->height = y;
    }

  gtk_icon_view_set_adjustment_upper (icon_view->priv->hadjustment, icon_view->priv->width);
  gtk_icon_view_set_adjustment_upper (icon_view->priv->vadjustment, icon_view->priv->height);

  if (GTK_WIDGET_REALIZED (icon_view))
    {
      gdk_window_resize (icon_view->priv->bin_window,
			 MAX (icon_view->priv->width, widget->allocation.width),
			 MAX (icon_view->priv->height, widget->allocation.height));
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
  g_debug ("Call moko icon view size allocate");
  sleep (2);
  GtkIconView *icon_view;

  widget->allocation = *allocation;
  
  icon_view = GTK_ICON_VIEW (widget);
  
  if (GTK_WIDGET_REALIZED (widget))
    {
      gdk_window_move_resize (widget->window,
			      allocation->x, allocation->y,
			      allocation->width, allocation->height);
      gdk_window_resize (icon_view->priv->bin_window,
			 MAX (icon_view->priv->width, allocation->width),
			 MAX (icon_view->priv->height, allocation->height));
    }
  gdk_window_set_back_pixmap (icon_view->priv->bin_window, widget->style->bg_pixmap[widget->state], FALSE);

  icon_view->priv->hadjustment->page_size = allocation->width;
  icon_view->priv->hadjustment->page_increment = allocation->width * 0.9;
  icon_view->priv->hadjustment->step_increment = allocation->width * 0.1;
  icon_view->priv->hadjustment->lower = 0;
  icon_view->priv->hadjustment->upper = MAX (allocation->width, icon_view->priv->width);
  gtk_adjustment_changed (icon_view->priv->hadjustment);

  icon_view->priv->vadjustment->page_size = allocation->height;
  icon_view->priv->vadjustment->page_increment = allocation->height * 0.9;
  icon_view->priv->vadjustment->step_increment = allocation->width * 0.1;
  icon_view->priv->vadjustment->lower = 0;
  icon_view->priv->vadjustment->upper = MAX (allocation->height, icon_view->priv->height);
  gtk_adjustment_changed (icon_view->priv->vadjustment);

  gtk_icon_view_layout (icon_view);
}


