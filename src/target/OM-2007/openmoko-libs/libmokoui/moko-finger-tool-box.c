/*  moko-finger-tool-box.c
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006 Vanille-Media
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
 *  Current Version: $Rev$ ($Date: 2006/10/05 17:38:14 $) [$Author: mickey $]
 */

#include "moko-finger-tool-box.h"
#include "moko-pixmap-button.h"
#include "moko-window.h"

#include <gtk/gtkhbox.h>

G_DEFINE_TYPE (MokoFingerToolBox, moko_finger_tool_box, MOKO_TYPE_ALIGNMENT)

#define MOKO_FINGER_TOOL_BOX_GET_PRIVATE(o)     (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_FINGER_TOOL_BOX, MokoFingerToolBoxPrivate))

#define OUTER_PADDING 35
#define INNER_PADDING 10

static void moko_finger_tool_box_show(GtkWidget* widget);
static void moko_finger_tool_box_hide(GtkWidget* widget);

static MokoAlignmentClass *parent_class = NULL;

typedef struct _MokoFingerToolBoxPrivate
{
    GdkBitmap* mask;

    MokoPixmapButton* leftarrow;
    GtkHBox* hbox;
    MokoPixmapButton* rightarrow;

    guint maxButtonsPerPage;
    guint numberOfButtons;
    guint leftButton;

    gboolean leftArrowVisible;
    gboolean rightArrowVisible;

    GtkWindow* popup;

} MokoFingerToolBoxPrivate;

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
moko_finger_tool_box_class_init (MokoFingerToolBoxClass *klass)
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
    g_debug( "size allocate %d, %d, %d, %d", allocation->x, allocation->y, allocation->width, allocation->height );

    MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(self);

    GtkAllocation* a = &GTK_WIDGET(priv->hbox)->allocation;

    //FIXME get from style
    priv->maxButtonsPerPage = a->width / 71;

    GtkRequisition* r = &GTK_WIDGET(priv->hbox)->requisition;

    guint numChild = 0;

    void checkstatus( GtkWidget* child, MokoFingerToolBox* self )
    {
        MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(self);
        guint maxButtonsPerPage = priv->maxButtonsPerPage;
        if ( priv->rightArrowVisible || priv->leftArrowVisible ) maxButtonsPerPage--;

        g_debug( "child: '%s'", gtk_widget_get_name( child ) );
        if ( strcmp( "mokofingertoolbox-toolbutton", gtk_widget_get_name( child ) ) == 0 )
        {
            if ( numChild < priv->leftButton || numChild > priv->leftButton + maxButtonsPerPage )
            {
                g_debug( "hiding child %d", numChild );
                gtk_widget_hide( child );
            }
            else
            {
                g_debug( "showing child %d", numChild );
                gtk_widget_show( child );
            }
        }
        numChild++;
    }

    gboolean oldLeftArrowVisible = priv->leftArrowVisible;
    gboolean oldRightArrowVisible = priv->rightArrowVisible;

    if /* ( r->width > a->width ) */ ( priv->numberOfButtons > priv->maxButtonsPerPage )
    {
        priv->leftArrowVisible = priv->leftButton > 0;
        priv->rightArrowVisible = priv->leftButton + priv->maxButtonsPerPage < priv->numberOfButtons;
    }

    gtk_container_foreach( GTK_CONTAINER(priv->hbox), &checkstatus, self );

    if ( priv->leftArrowVisible )
        gtk_widget_show( GTK_WIDGET(priv->leftarrow) );
    else
        gtk_widget_hide( GTK_WIDGET(priv->leftarrow) );
    if ( priv->rightArrowVisible )
        gtk_widget_show( GTK_WIDGET(priv->rightarrow) );
    else
        gtk_widget_hide( GTK_WIDGET(priv->rightarrow) );

    g_debug( "left button = %d, right button = %d", priv->leftArrowVisible, priv->rightArrowVisible );


    //g_object_unref(G_OBJECT(alphapixbuf));
    //g_object_unref(G_OBJECT(pixbuf));
    //gtk_widget_shape_combine_mask(priv->popup, mask, 0, 0);

}

static void
cb_configure(GtkWidget* widget, GtkAllocation* a, MokoFingerToolBox* self)
{
    //FIXME unref all existing pixmaps, check whether we really need to draw new ones

    g_debug( "generating pixmaps for size = %d, %d", a->width, a->height );

    MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(self);

    //FIXME generate all possible combinations of mask images, not just one w/ 4 buttons

    gtk_widget_ensure_style( GTK_WIDGET(self) );
    GtkStyle* style = gtk_rc_get_style( GTK_WIDGET(self) );
    g_assert( style->rc_style );

    GdkPixbuf* background = gdk_pixbuf_new_from_file( style->rc_style->bg_pixmap_name[GTK_STATE_NORMAL], NULL);
    GdkPixbuf* pixbuf = gdk_pixbuf_scale_simple( background, a->width, a->height, GDK_INTERP_BILINEAR );
    GdkPixbuf* button = gdk_pixbuf_new_from_file( g_build_filename( moko_application_get_style_pixmap_dir(), "btn_type03.png", NULL ), NULL );
    guint w = gdk_pixbuf_get_width( button );
    guint h = gdk_pixbuf_get_height( button );
    guint x = OUTER_PADDING - 1;
    guint y = 0;

    gdk_pixbuf_copy_area( background, 0, 0, gdk_pixbuf_get_width( background ), gdk_pixbuf_get_height( background ), pixbuf, 0, 0 );

    for ( int i = 0; i < 4; ++i )
    {
        //gdk_pixbuf_copy_area( button, 0, 0, w, h, pixbuf, x, y );

        gdk_pixbuf_composite( button, pixbuf, x, y, w, h, x, y+2, 1, 1, GDK_INTERP_NEAREST, 255 );
        x += w + INNER_PADDING - 2;
    }
#ifdef CRAZY_DEBUG_CODE
    GtkWindow* window = gtk_window_new( GTK_WINDOW_TOPLEVEL );
    GtkImage* image = gtk_image_new_from_pixbuf( pixbuf );
    gtk_container_add( GTK_CONTAINER(window), GTK_WIDGET(image) );
    gtk_widget_show( image );
    gtk_widget_show( window );
#endif
    GdkPixmap* pixmap;
    gdk_pixbuf_render_pixmap_and_mask( pixbuf, &pixmap, &priv->mask, 1);
    g_object_unref( pixmap );
    gtk_widget_shape_combine_mask(priv->popup, priv->mask, 0, 0);
}

static void
cb_left_button_pressed(GtkWidget* widget, MokoFingerToolBox* self)
{
    g_debug( "left button pressed" );
    MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(self);
    priv->leftButton -= priv->maxButtonsPerPage;
    //FIXME force redraw
}

static void
cb_right_button_pressed(GtkWidget* widget, MokoFingerToolBox* self)
{
    g_debug( "right button pressed" );
    MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(self);
    priv->leftButton += priv->maxButtonsPerPage;
    // force redraw
    //FIXME force redraw
}

static void moko_finger_tool_box_show(GtkWidget* widget)
{
    //gtk_widget_ensure_style( widget ); //FIXME needed here?
    g_debug( "moko_finger_wheel_show" );
    GTK_WIDGET_CLASS(parent_class)->show(widget);
    MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(widget);
    if ( !priv->popup )
    {
        priv->popup = gtk_window_new(GTK_WINDOW_POPUP);
        //FIXME Setting it to transparent is probably not necessary since we issue a mask anyway, right?
        //gtk_widget_set_name( GTK_WIDGET(priv->popup), "transparent" );
        gtk_container_add( GTK_CONTAINER(priv->popup), widget );
        MokoWindow* window = moko_application_get_main_window( moko_application_get_instance() );
        GtkRequisition req;
        gtk_widget_size_request( widget, &req );
        //g_debug( "My requisition is %d, %d", req.width, req.height );
        int x, y, w, h;
        gdk_window_get_geometry( GTK_WIDGET(window)->window, &x, &y, &w, &h, NULL );
        //g_debug( "WINDOW geometry is %d, %d * %d, %d", x, y, w, h );
        int absx;
        int absy;

        g_signal_connect_after( G_OBJECT(widget), "size_allocate", G_CALLBACK(cb_size_allocate), widget );

        gdk_window_get_origin( GTK_WIDGET(window)->window, &absx, &absy );
        GtkAllocation* alloc = &GTK_WIDGET(window)->allocation;
        //g_debug( "WINDOW allocation is %d, %d * %d, %d", alloc->x, alloc->y, alloc->width, alloc->height );
        gtk_window_move( priv->popup, absx + w - req.width, absy + h - req.height );
    }
    gtk_widget_show( priv->popup );
}

static void moko_finger_tool_box_hide(GtkWidget* widget)
{
    g_debug( "moko_finger_tool_box_hide" );
    GTK_WIDGET_CLASS(parent_class)->hide(widget);
    MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(widget);
    gtk_widget_hide( priv->popup );
}

static void
moko_finger_tool_box_init (MokoFingerToolBox *self)
{
    MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(self);
    gtk_widget_set_name( GTK_WIDGET(self), "mokofingertoolbox" );
    //FIXME get from theme
    gtk_alignment_set_padding( GTK_ALIGNMENT(self), 0, 0, OUTER_PADDING, 0 );
    priv->leftarrow = MOKO_PIXMAP_BUTTON( moko_pixmap_button_new() );
    gtk_widget_set_name( GTK_WIDGET(priv->leftarrow), "mokofingertoolbox-leftarrow" );
    priv->rightarrow = MOKO_PIXMAP_BUTTON( moko_pixmap_button_new() );
    gtk_widget_set_name( GTK_WIDGET(priv->rightarrow), "mokofingertoolbox-rightarrow" );

    priv->hbox = gtk_hbox_new( FALSE, INNER_PADDING );
    gtk_container_add( GTK_CONTAINER(self), GTK_WIDGET(priv->hbox) );

    gtk_box_pack_start( GTK_BOX(priv->hbox), priv->leftarrow, FALSE, FALSE, 0 );
    gtk_box_pack_end( GTK_BOX(priv->hbox), priv->rightarrow, FALSE, FALSE, 0 );

    gtk_widget_show( GTK_WIDGET(priv->hbox) );

    g_signal_connect( G_OBJECT(priv->leftarrow), "clicked", G_CALLBACK(cb_left_button_pressed), self );
    g_signal_connect( G_OBJECT(priv->rightarrow), "clicked", G_CALLBACK(cb_right_button_pressed), self );

    g_signal_connect_after( G_OBJECT(self), "size-allocate", G_CALLBACK(cb_configure), self );

    //FIXME link with wheel
    gtk_widget_set_size_request( GTK_WIDGET(self), 350, 104 );
}

/* public API */

GtkWidget*
moko_finger_tool_box_new (void)
{
    return GTK_WIDGET(g_object_new(moko_finger_tool_box_get_type(), NULL));
}

GtkButton*
moko_finger_tool_box_add_button(MokoFingerToolBox* self)
{
    static gchar text[] = "0\0";
    MokoFingerToolBoxPrivate* priv = MOKO_FINGER_TOOL_BOX_GET_PRIVATE(self);

    MokoPixmapButton* b = moko_pixmap_button_new();
    gtk_button_set_label( GTK_BUTTON(b), &text );
    text[0]++;
    gtk_widget_set_name( GTK_WIDGET(b), "mokofingertoolbox-toolbutton" );

    priv->numberOfButtons++;

    gtk_box_pack_start( GTK_BOX(priv->hbox), b, FALSE, FALSE, 0 );
    gtk_widget_show( GTK_WIDGET(b) );
    // save button for inside the expose event we want to get its shape
    //if ( !priv->button ) priv->button = b;

    // force redraw
    gtk_widget_queue_draw( priv->hbox );

    return b;
}
