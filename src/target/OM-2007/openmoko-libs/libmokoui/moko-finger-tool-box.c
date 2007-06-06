/*  moko-finger-tool-box.c
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
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
 *  Current Version: $Rev$ ($Date) [$Author: mickey $]
 */

#include "moko-application.h"
#include "moko-finger-tool-box.h"
#include "moko-finger-window.h"
#include "moko-pixmap-button.h"
#include "moko-window.h"

#include <gtk/gtkhbox.h>
#include <string.h>

//#define DEBUG_THIS_FILE
#ifdef DEBUG_THIS_FILE
#define moko_debug(fmt,...) g_debug(fmt,##__VA_ARGS__)
#define moko_debug_minder(predicate) moko_debug( __FUNCTION__ ); g_return_if_fail(predicate)
#else
#define moko_debug(fmt,...)
#endif

G_DEFINE_TYPE (MokoFingerToolBox, moko_finger_tool_box, MOKO_TYPE_ALIGNMENT)

#define MOKO_FINGER_TOOL_BOX_GET_PRIVATE(o)     (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_FINGER_TOOL_BOX, MokoFingerToolBoxPrivate))

#define INNER_PADDING 10
static void moko_finger_tool_box_show(GtkWidget* widget);
static void moko_finger_tool_box_hide(GtkWidget* widget);

static MokoAlignmentClass* parent_class = NULL;

typedef struct _MokoFingerToolBoxPrivate
{
    GdkBitmap* mask;
    GtkWidget* hbox;       /* GtkHBox          */
    GtkWidget* rightarrow; /* MokoPixmapButton */
    gboolean rightArrowVisible;

    guint maxButtonsPerPage;
    guint numberOfButtons;
    guint leftButton;
    guint buttonWidth;

    GtkWidget* popup; /* GtkWindow */
    GtkWidget* parent;

    GdkPixbuf* background_pixbuf;
    GdkPixbuf* button_pixbuf;
    GdkPixbuf* rightarrow_pixbuf;

} MokoFingerToolBoxPrivate;

static gchar* get_bg_pixmap_name (GtkWidget *widget)
{
   return widget->style->rc_style->bg_pixmap_name[GTK_STATE_NORMAL];
}

static void
moko_finger_tool_box_dispose (GObject *object)
{
    if (G_OBJECT_CLASS (moko_finger_tool_box_parent_class)->dispose)
        G_OBJECT_CLASS (moko_finger_tool_box_parent_class)->dispose (object);
}

static void
moko_finger_tool_box_finalize (GObject *object)
{
    G_OBJECT_CLASS (moko_finger_tool_box_parent_class)->finalize (object);
}

static void
moko_finger_tool_box_class_init(MokoFingerToolBoxClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    parent_class = g_type_class_peek_parent(klass);

    /* register private data */
    g_type_class_add_private (klass, sizeof (MokoFingerToolBoxPrivate));

    /* hook virtual methods */
    GtkWidgetClass *widget_class = (GtkWidgetClass*) klass;
    widget_class->show = moko_finger_tool_box_show;
    widget_class->hide = moko_finger_tool_box_hide;

    /* install properties */
    /* ... */

    object_class->dispose = moko_finger_tool_box_dispose;
    object_class->finalize = moko_finger_tool_box_finalize;
}

static void
cb_size_allocate(GtkWidget* widget, GtkAllocation* allocation, MokoFingerToolBox* self)
{
    moko_debug( "size allocate %d, %d, %d, %d", allocation->x, allocation->y, allocation->width, allocation->height );

    MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(self);

    GtkAllocation* a = &(priv->hbox->allocation);

    //FIXME get from style
    priv->maxButtonsPerPage = a->width / ( priv->buttonWidth + (INNER_PADDING/2) );
    moko_debug( "-- width % buttonWidth = %d", a->width % priv->buttonWidth );

    guint numChild = 0;

    void checkstatus( GtkWidget* child, MokoFingerToolBox* self )
    {
        MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(self);
        guint maxButtonsPerPage = priv->maxButtonsPerPage;
        if ( priv->rightArrowVisible ) maxButtonsPerPage--;

        moko_debug( "maxButtonsPerPage = %d", maxButtonsPerPage );

        moko_debug( "child: '%s'", gtk_widget_get_name( child ) );
        if ( strcmp( "mokofingertoolbox-toolbutton", gtk_widget_get_name( child ) ) == 0 )
        {
            if ( numChild < priv->leftButton || numChild > priv->leftButton + maxButtonsPerPage-1 )
            {
                moko_debug( "hiding child %d", numChild );
                gtk_widget_hide( child );
            }
            else
            {
                moko_debug( "showing child %d", numChild );
                gtk_widget_show( child );
            }
        }
        numChild++;
    }

    priv->rightArrowVisible = priv->numberOfButtons > priv->maxButtonsPerPage;

    gtk_container_foreach( GTK_CONTAINER(priv->hbox), (GtkCallback) checkstatus, self );

    if ( priv->rightArrowVisible )
        gtk_widget_show( priv->rightarrow );
    else
        gtk_widget_hide( priv->rightarrow );

    moko_debug( "right button = %d", priv->rightArrowVisible );
}

static void
cb_configure(GtkWidget* widget, GtkAllocation* a, MokoFingerToolBox* self)
{
    guint padding_top;
    guint padding_bottom;
    guint padding_left;
    guint padding_right;

    gtk_alignment_get_padding( GTK_ALIGNMENT(widget), &padding_top, &padding_bottom, &padding_left, &padding_right );
    
    //If padding defined in gtkrc, reset top/bottom/left/right.
    GtkBorder* padding = NULL;
    gtk_widget_style_get (widget, "padding", &padding, NULL);
    if ( padding )
    {
        padding_top    = padding->top;
        padding_bottom = padding->bottom;
        padding_left   = padding->left;
        padding_right  = padding->right;
    }    
    moko_debug( "my padding is %d, %d, %d, %d", padding_left, padding_top, padding_right, padding_bottom );


    //FIXME unref all existing pixmaps, check whether we really need to draw new ones

    moko_debug( "generating pixmaps for size = %d, %d", a->width, a->height );

    MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(self);
    guint maxButtonsPerPage = priv->maxButtonsPerPage;
    if ( priv->rightArrowVisible ) maxButtonsPerPage--;
    moko_debug( "max buttons per page is %d", maxButtonsPerPage );

    if ( !priv->background_pixbuf )
    {
        gchar *filename = NULL;
        if ((filename = get_bg_pixmap_name (GTK_WIDGET (self))))
        {
          priv->background_pixbuf = gdk_pixbuf_new_from_file( filename, NULL);
          if (!priv->background_pixbuf)
          {
            g_warning ("moko_finger_tool_box: could not load pixbuf for the finger tool box: %s", filename);
            return;
          }
        }
        else
        {
            g_warning ("moko_finger_tool_box: theme does not specify a background image for the finger tool box");
            return;
        }
        /* TODO: ideally this should inspect bg_pixmap of the child items */
        gchar *dirname = g_path_get_dirname (filename);
        gchar *buttonfile = g_build_filename (dirname, "btn_type03.png", NULL);
        priv->button_pixbuf = gdk_pixbuf_new_from_file( buttonfile, NULL);
        g_free (dirname);
        g_free (buttonfile);
    }
    GdkPixbuf* pixbuf = NULL;
    guint w = gdk_pixbuf_get_width( priv->button_pixbuf );
    guint h = gdk_pixbuf_get_height( priv->button_pixbuf );
    guint x = padding_left - 1;
    guint y = 0;

    if (GDK_IS_PIXBUF (priv->background_pixbuf))
    {
        pixbuf = gdk_pixbuf_scale_simple( priv->background_pixbuf, 
                                                     a->width, a->height,
                                                     GDK_INTERP_BILINEAR );
        gdk_pixbuf_copy_area( priv->background_pixbuf, 0, 0, 
                              gdk_pixbuf_get_width( priv->background_pixbuf ),
                              gdk_pixbuf_get_height( priv->background_pixbuf ),
                              pixbuf, 0, 0 );
    }
    guint composite_num;
    if ( maxButtonsPerPage == 0 )
        composite_num = priv->numberOfButtons - priv->leftButton;
    else if ( priv->numberOfButtons - priv->leftButton >= maxButtonsPerPage )
        composite_num = maxButtonsPerPage;
    else
        composite_num = priv->numberOfButtons % maxButtonsPerPage;
    
    for ( int i = 0; i < composite_num; ++i )
    {
        //gdk_pixbuf_copy_area( priv->button_pixbuf, 0, 0, w, h, pixbuf, x, y );

        gdk_pixbuf_composite( priv->button_pixbuf, pixbuf, x, y, w, h, x, y, 1, 1, GDK_INTERP_NEAREST, 255 );
        x += w + INNER_PADDING - 2;
    }

    if ( priv->rightArrowVisible )
    {
      /* FIXME: this really should be retrieved from the style */
        if ( !priv->rightarrow_pixbuf )
            priv->rightarrow_pixbuf = gdk_pixbuf_new_from_file( get_bg_pixmap_name (priv->rightarrow) , NULL);

        guint rightarrow_w = gdk_pixbuf_get_width( priv->rightarrow_pixbuf );
        guint rightarrow_h = gdk_pixbuf_get_height( priv->rightarrow_pixbuf );
        guint rightarrow_x = a->width - rightarrow_w;
        
        gdk_pixbuf_composite( priv->rightarrow_pixbuf, pixbuf, rightarrow_x, y, rightarrow_w, rightarrow_h, rightarrow_x, y, 1, 1, GDK_INTERP_NEAREST, 255 );
    }

#ifdef CRAZY_DEBUG_CODE
    GtkWindow* window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    GtkImage* image = gtk_image_new_from_pixbuf( pixbuf );
    gtk_container_add( GTK_CONTAINER(window), GTK_WIDGET(image) );
    gtk_widget_show( image );
    gtk_widget_show( window );
#endif
    gdk_pixbuf_render_pixmap_and_mask( pixbuf, NULL, &priv->mask, 1);
    gtk_widget_shape_combine_mask(priv->popup, priv->mask, 0, 0);

    priv->buttonWidth = w;
}

static void
cb_right_button_pressed(GtkWidget* widget, MokoFingerToolBox* self)
{
    moko_debug( "right button pressed" );
    MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(self);
    priv->leftButton += priv->maxButtonsPerPage -1 ; // % priv->numberOfButtons;
    if ( priv->leftButton >= priv->numberOfButtons ) priv->leftButton = 0;
    // force redraw
    //FIXME force redraw
}

static void moko_finger_tool_box_show(GtkWidget* widget)
{
    //gtk_widget_ensure_style( widget ); //FIXME needed here?
    moko_debug( "moko_finger_wheel_show" );
    GTK_WIDGET_CLASS(parent_class)->show(widget);
    MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(widget);
    if ( !priv->popup )
    {
        priv->popup = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_type_hint( GTK_WINDOW (priv->popup), GDK_WINDOW_TYPE_HINT_DIALOG );
        gtk_window_set_decorated( GTK_WINDOW (priv->popup), FALSE );
        gtk_container_add( GTK_CONTAINER(priv->popup), widget );
    }

    MokoWindow* window =(MokoWindow* )(priv->parent);
    
    g_return_if_fail( MOKO_IS_FINGER_WINDOW(window) );
    //FIXME set also transient for all other windows belonging to this app?
    gtk_window_set_transient_for( GTK_WINDOW (priv->popup), GTK_WINDOW (window) );
    GtkAllocation geometry;
    moko_finger_window_get_geometry_hint( MOKO_FINGER_WINDOW(window), widget, &geometry );
    g_signal_connect_after( G_OBJECT(widget), "size_allocate", G_CALLBACK(cb_size_allocate), widget );
    gtk_window_move( GTK_WINDOW (priv->popup), geometry.x, geometry.y );
    gtk_widget_set_size_request( GTK_WIDGET(widget), geometry.width, geometry.height );
    gtk_window_resize( GTK_WINDOW (priv->popup), geometry.width, geometry.height );

    gtk_widget_show( priv->popup );
    GtkWidget* wheel = moko_finger_window_get_wheel( MOKO_FINGER_WINDOW(window) );
    if ( wheel && GTK_WIDGET_VISIBLE(wheel) )
        moko_finger_wheel_set_transient_for( MOKO_FINGER_WHEEL (wheel), GTK_WINDOW (priv->popup) ); //moko_finger_wheel_raise( wheel );
}

static void moko_finger_tool_box_hide(GtkWidget* widget)
{
    moko_debug( "moko_finger_tool_box_hide" );
    GTK_WIDGET_CLASS(parent_class)->hide(widget);
    MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(widget);
    gtk_widget_hide( priv->popup );
}

static void
moko_finger_tool_box_init(MokoFingerToolBox *self)
{
    MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(self);
    gtk_widget_set_name( GTK_WIDGET(self), "mokofingertoolbox" );

    priv->rightarrow = moko_pixmap_button_new();
    gtk_widget_set_name( priv->rightarrow, "mokofingertoolbox-rightarrow" );

    priv->hbox = gtk_hbox_new( FALSE, INNER_PADDING );
    gtk_container_add( GTK_CONTAINER(self), priv->hbox );

    gtk_box_pack_end( GTK_BOX(priv->hbox), priv->rightarrow, FALSE, FALSE, 0 );

    gtk_widget_show( priv->hbox );

    g_signal_connect( G_OBJECT(priv->rightarrow), "clicked", G_CALLBACK(cb_right_button_pressed), self );
    g_signal_connect_after( G_OBJECT(self), "size-allocate", G_CALLBACK(cb_configure), self );

}

/* public API */
GtkWidget*
moko_finger_tool_box_new(GtkWidget* parent)
{
    MokoFingerToolBox* self=g_object_new(moko_finger_tool_box_get_type(), NULL);
    
    MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(self);
    priv->parent=parent;
    return GTK_WIDGET(self);
}

GtkWidget*
moko_finger_tool_box_add_button(MokoFingerToolBox* self)
{
    MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(self);

    GtkWidget* b = moko_pixmap_button_new();
    gtk_widget_set_name( b, "mokofingertoolbox-toolbutton" );

    priv->numberOfButtons++;

    gtk_box_pack_start( GTK_BOX(priv->hbox), b, FALSE, FALSE, 0 );
    gtk_widget_show( b );
    // save button for inside the expose event we want to get its shape
    //if ( !priv->button ) priv->button = b;

    // force redraw
    gtk_widget_queue_draw( priv->hbox );

    return b;
}


GtkWidget*
moko_finger_tool_box_add_button_without_label(MokoFingerToolBox* self)
{
    MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(self);

    GtkWidget* b = moko_pixmap_button_new();
    gtk_widget_set_name( b, "mokofingertoolbox-toolbutton" );

    priv->numberOfButtons++;

    gtk_box_pack_start( GTK_BOX(priv->hbox), b, FALSE, FALSE, 0 );
    gtk_widget_show( b );
    // save button for inside the expose event we want to get its shape
    //if ( !priv->button ) priv->button = b;

    // force redraw
    gtk_widget_queue_draw( priv->hbox );

    return b;
}
