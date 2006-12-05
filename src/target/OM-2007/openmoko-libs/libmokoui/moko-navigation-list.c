/*  moko-navigation-list.c
 *
 *  Authored By Ken Zhao <ken_zhao@fic-sh.com.cn>
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
 *  Current Version: $Rev$ ($Date: 2006/11/28 17:38:14 $) [$Author: Ken $]
 */

#include "moko-navigation-list.h"

G_DEFINE_TYPE (MokoNavigationList, moko_navigation_list, GTK_TYPE_VIEWPORT)

#define NAVIGATION_LIST_PRIVATE(o)     (G_TYPE_INSTANCE_GET_PRIVATE ((o), MOKO_TYPE_NAVIGATION_LIST, MokoNavigationListPrivate))

typedef struct _MokoNavigationListPrivate
{
    GtkFixed* navigationcontainer;
    GtkScrolledWindow* navigationsw;
    MokoTreeView* treeview;
} MokoNavigationListPrivate;

/* forward declarations */
/* ... */

/* declare virtual methods */
static void
moko_navigation_list_size_request (GtkWidget *widget, GtkRequisition *requisition);

static void
moko_navigation_list_size_allocate (GtkWidget *widget, GtkAllocation *allocation);

static void
moko_navigation_list_get_view_allocation (GtkViewport *viewport, GtkAllocation *view_allocation);




static void
moko_navigation_list_reclamp_adjustment (GtkAdjustment *adjustment,
                             gboolean      *value_changed)
{
    gdouble value = adjustment->value;
    
    value = CLAMP (value, 0, adjustment->upper - adjustment->page_size);
    if (value != adjustment->value)
    {
        adjustment->value = value;
        if (value_changed)
            *value_changed = TRUE;
    }
    else if (value_changed)
        *value_changed = FALSE;
}

static void
moko_navigation_list_set_hadjustment_values (GtkViewport *viewport,
                                 gboolean    *value_changed)
{
    GtkBin *bin = GTK_BIN (viewport);
    GtkAllocation view_allocation;
    GtkAdjustment *hadjustment = gtk_viewport_get_hadjustment (viewport);
    gdouble old_page_size;
    gdouble old_upper;
    gdouble old_value;
    
    moko_navigation_list_get_view_allocation (viewport, &view_allocation);

    old_page_size = hadjustment->page_size;
    old_upper = hadjustment->upper;
    old_value = hadjustment->value;
    hadjustment->page_size = view_allocation.width;
    hadjustment->step_increment = view_allocation.width * 0.1;
    hadjustment->page_increment = view_allocation.width * 0.9;

    hadjustment->lower = 0;

    if (bin->child && GTK_WIDGET_VISIBLE (bin->child))
    {
        GtkRequisition child_requisition;
        
        gtk_widget_get_child_requisition (bin->child, &child_requisition);
        hadjustment->upper = MAX (child_requisition.width, view_allocation.width);
    }
    else
        hadjustment->upper = view_allocation.width;

    if (gtk_widget_get_direction (GTK_WIDGET (viewport)) == GTK_TEXT_DIR_RTL)
    {
        gdouble dist = old_upper - (old_value + old_page_size);
        hadjustment->value = hadjustment->upper - dist - hadjustment->page_size;
        moko_navigation_list_reclamp_adjustment (hadjustment, value_changed);
        *value_changed = (old_value != hadjustment->value);
    }
  else
      moko_navigation_list_reclamp_adjustment (hadjustment, value_changed);
}


static void
moko_navigation_list_get_view_allocation (GtkViewport   *viewport,
                              GtkAllocation *view_allocation)
{
    GtkWidget *widget = GTK_WIDGET (viewport);
    GtkAllocation *allocation = &widget->allocation;
    gint border_width = GTK_CONTAINER (viewport)->border_width;
    
    view_allocation->x = 0;
    view_allocation->y = 0;

    if (viewport->shadow_type != GTK_SHADOW_NONE)
    {
        view_allocation->x = widget->style->xthickness;
        view_allocation->y = widget->style->ythickness;
    }

    view_allocation->width = MAX (1, allocation->width - view_allocation->x * 2 - border_width * 2);
    view_allocation->height = MAX (1, allocation->height - view_allocation->y * 2 - border_width * 2);
  
  
}

static void
moko_navigation_list_set_vadjustment_values (GtkViewport *viewport,
                                 gboolean    *value_changed)
{
    GtkBin *bin = GTK_BIN (viewport);
    GtkAllocation view_allocation;
    GtkAdjustment *vadjustment = gtk_viewport_get_vadjustment (viewport);
    
    moko_navigation_list_get_view_allocation (viewport, &view_allocation);
    
    vadjustment->page_size = view_allocation.height;
    vadjustment->step_increment = view_allocation.height * 0.1;
    vadjustment->page_increment = view_allocation.height * 0.9;
    
    vadjustment->lower = 0;

    if (bin->child && GTK_WIDGET_VISIBLE (bin->child))
    {
        GtkRequisition child_requisition;
        
        gtk_widget_get_child_requisition (bin->child, &child_requisition);
        vadjustment->upper = MAX (child_requisition.height, view_allocation.height);
    }
    else
        vadjustment->upper = view_allocation.height;

    moko_navigation_list_reclamp_adjustment (vadjustment, value_changed);
}



static void
moko_navigation_list_dispose (GObject *object)
{
    if (G_OBJECT_CLASS (moko_navigation_list_parent_class)->dispose)
        G_OBJECT_CLASS (moko_navigation_list_parent_class)->dispose (object);
}

static void
moko_navigation_list_finalize (GObject *object)
{
    G_OBJECT_CLASS (moko_navigation_list_parent_class)->finalize (object);
}


static void
moko_navigation_list_size_request(GtkWidget *widget, GtkRequisition *requisition)
{

    GtkViewport *viewport;
    GtkBin *bin;
    GtkRequisition child_requisition;
    
    gint width,height;
    gint navigationsw_width,navigationsw_height;
    
    viewport = GTK_VIEWPORT (widget);
    bin = GTK_BIN (widget);
    
    gtk_widget_get_size_request (widget, &width, &height);
    
    
    navigationsw_width  = width  - 6;
    navigationsw_height = height - 3;
    
    MokoNavigationListPrivate* priv = NAVIGATION_LIST_PRIVATE (widget);

    gtk_widget_set_size_request (GTK_WIDGET(priv->navigationsw), navigationsw_width, navigationsw_height);
  
    requisition->width = (GTK_CONTAINER (widget)->border_width +
                          GTK_WIDGET (widget)->style->xthickness) * 2;

    requisition->height = (GTK_CONTAINER (widget)->border_width * 2 +
                           GTK_WIDGET (widget)->style->ythickness) * 2;


    if (bin->child && GTK_WIDGET_VISIBLE (bin->child))
    {
        gtk_widget_size_request (bin->child, &child_requisition);
        requisition->width += child_requisition.width;
        requisition->height += child_requisition.height;
    }
  
  
}


static void
moko_navigation_list_size_allocate (GtkWidget     *widget,
                            GtkAllocation *allocation)
{
    GtkViewport *viewport = GTK_VIEWPORT (widget);
    GtkBin *bin = GTK_BIN (widget);
    gint border_width = GTK_CONTAINER (widget)->border_width;
    gboolean hadjustment_value_changed, vadjustment_value_changed;
    
    GtkAdjustment *hadjustment = gtk_viewport_get_hadjustment (viewport);
    GtkAdjustment *vadjustment = gtk_viewport_get_vadjustment (viewport);

    /* If our size changed, and we have a shadow, queue a redraw on widget->window to
     * redraw the shadow correctly.
     */
    if (GTK_WIDGET_MAPPED (widget) &&
        viewport->shadow_type != GTK_SHADOW_NONE &&
        (widget->allocation.width != allocation->width ||
         widget->allocation.height != allocation->height))
        gdk_window_invalidate_rect (widget->window, NULL, FALSE);
    
    widget->allocation = *allocation;

    moko_navigation_list_set_hadjustment_values (viewport, &hadjustment_value_changed);
    moko_navigation_list_set_vadjustment_values (viewport, &vadjustment_value_changed);

    if (GTK_WIDGET_REALIZED (widget))
    {
        GtkAllocation view_allocation;
        
        moko_navigation_list_get_view_allocation (viewport, &view_allocation);
        
        gdk_window_move_resize (widget->window,
                                allocation->x + border_width,
                                allocation->y + border_width,
                                allocation->width - border_width * 2,
                                allocation->height - border_width * 2);
        
        gdk_window_move_resize (viewport->view_window,
                                view_allocation.x,
                                view_allocation.y,
                                view_allocation.width,
                                view_allocation.height);
    }

    if (bin->child && GTK_WIDGET_VISIBLE (bin->child))
    {
        GtkAllocation child_allocation;
        
        child_allocation.x = 0;
        child_allocation.y = 0;
        child_allocation.width = hadjustment->upper;
        child_allocation.height = vadjustment->upper;
        
        if (GTK_WIDGET_REALIZED (widget))
            gdk_window_move_resize (viewport->bin_window,
                                    - hadjustment->value,
                                    - vadjustment->value,
                                    child_allocation.width,
                                    child_allocation.height);
        
        gtk_widget_size_allocate (bin->child, &child_allocation);
    }

    gtk_adjustment_changed (hadjustment);
    gtk_adjustment_changed (vadjustment);
    if (hadjustment_value_changed)
        gtk_adjustment_value_changed (hadjustment);
    if (vadjustment_value_changed)
        gtk_adjustment_value_changed (vadjustment);
    
    gtk_widget_set_size_request (widget, allocation->width, allocation->height);
}


static void
moko_navigation_list_class_init (MokoNavigationListClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS (klass);
    GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

    /* register private data */
    g_type_class_add_private (klass, sizeof (MokoNavigationListPrivate));

    /* hook virtual methods */
    widget_class->size_request = moko_navigation_list_size_request;
    widget_class->size_allocate = moko_navigation_list_size_allocate;
    
    /* install properties */
    /* ... */

    object_class->dispose = moko_navigation_list_dispose;
    object_class->finalize = moko_navigation_list_finalize;
}

static void
moko_navigation_list_init (MokoNavigationList *self)
{
    MokoNavigationListPrivate* priv = NAVIGATION_LIST_PRIVATE (self);
    gtk_widget_set_name ( GTK_WIDGET (self), "mokonavigationlist-background" );
    
    gtk_tree_view_set_rules_hint ( GTK_TREE_VIEW (priv->treeview), TRUE );
    gtk_tree_view_set_headers_visible ( GTK_TREE_VIEW (priv->treeview), TRUE );
}

MokoNavigationList*
moko_navigation_list_new (void)
{
    MokoNavigationList* self = MOKO_NAVIGATION_LIST ( g_object_new (MOKO_TYPE_NAVIGATION_LIST, NULL));
    MokoNavigationListPrivate* priv = NAVIGATION_LIST_PRIVATE (self);

    gtk_viewport_set_shadow_type ( GTK_VIEWPORT (self), GTK_SHADOW_NONE );
    
    priv->navigationcontainer = (GtkFixed *) gtk_fixed_new();

    gtk_container_add ( GTK_CONTAINER (self), GTK_WIDGET (priv->navigationcontainer) );
    
    priv->treeview = (MokoTreeView *) moko_tree_view_new ();
    priv->navigationsw = moko_tree_view_put_into_scrolled_window (priv->treeview);
    
    
    gtk_widget_set_size_request ( GTK_WIDGET (self), 458, 160 );
    gtk_fixed_put ( GTK_FIXED (priv->navigationcontainer), GTK_WIDGET (priv->navigationsw), 4, 2 );
    
    
    return self;
}

MokoNavigationList*
moko_navigation_list_new_with_model (GtkTreeModel *model)
{

    MokoNavigationList* self = MOKO_NAVIGATION_LIST ( g_object_new (MOKO_TYPE_NAVIGATION_LIST, NULL));
    MokoNavigationListPrivate* priv = NAVIGATION_LIST_PRIVATE (self);

    gtk_viewport_set_shadow_type ( GTK_VIEWPORT (self), GTK_SHADOW_NONE );
    
    priv->navigationcontainer = (GtkFixed *) gtk_fixed_new();

    gtk_container_add ( GTK_CONTAINER (self), GTK_WIDGET (priv->navigationcontainer) );
    
    priv->treeview = (MokoTreeView *) moko_tree_view_new_with_model (model);
    priv->navigationsw = moko_tree_view_put_into_scrolled_window (priv->treeview);
    
    gtk_widget_set_size_request ( GTK_WIDGET (self), 458, 160 );
    gtk_fixed_put ( GTK_FIXED (priv->navigationcontainer), GTK_WIDGET (priv->navigationsw), 4, 2 );
    
    
    return self;
}


void moko_navigation_list_append_column (MokoNavigationList* self, GtkTreeViewColumn* column)
{
    MokoNavigationListPrivate* priv = NAVIGATION_LIST_PRIVATE (self);
    moko_tree_view_append_column( MOKO_TREE_VIEW (priv->treeview), column );
}

MokoTreeView* moko_navigation_list_get_tree_view (MokoNavigationList* self)
{
    MokoNavigationListPrivate* priv = NAVIGATION_LIST_PRIVATE (self);
    return priv->treeview;
}


