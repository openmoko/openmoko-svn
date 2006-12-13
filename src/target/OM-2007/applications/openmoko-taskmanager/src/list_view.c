/**
 * @file list_view.c
 * @brief list_view.c based on misc.c and gtk+-2.0.
 * @author Sun Zhiyong
 * @date 2006-10
 *
 * Copyright (C) 2006 FIC-SH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

 #include "list_view.h"

 enum {
    LIST_SIGNAL,
    LAST_SIGNAL
};

static void list_class_init          (ListClass *klass);
static void list_init                (List *l);

static guint list_signals[LAST_SIGNAL] = { 0 };

/**
*@brief retrun List type.
*@param none
*@return GType
*/
GType list_get_type (void) /* Typechecking */
{
    static GType list_type = 0;

    if (!list_type)
    {
        static const GTypeInfo list_info =
        {
            sizeof (ListClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) list_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (List),
            0,
            (GInstanceInitFunc) list_init,
            NULL
        };

        list_type = g_type_register_static (GTK_TYPE_VBOX, "List", &list_info, 0);
    }

    return list_type;
}

/**
*@brief initialize List class.
*@param klass	List Class
*@return none
*/
static void list_class_init(ListClass * Klass) /* Class Initialization */
{
    list_signals[LIST_SIGNAL] = g_signal_new ("list",
            G_TYPE_FROM_CLASS (Klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (ListClass, list),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 
            0);
}

/**
*@brief initialize List UI.
*@param l	List instance
*@return none
*/
static void /* Instance Construction */
list_init (List *l) { 
    GtkWidget *ico;
    l->list_store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_INT, G_TYPE_OBJECT);
    l->list_view = gtk_tree_view_new_with_model (GTK_TREE_MODEL (l->list_store));
    //l->mokolist_view = moko_tree_view_new_with_model (GTK_TREE_MODEL (l->list_store));
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (l->list_view), FALSE);
    //gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (l->mokolist_view), FALSE);
    
    l->renderer = gtk_cell_renderer_pixbuf_new ();
    l->column = gtk_tree_view_column_new_with_attributes ("Icon", l->renderer, "pixbuf", 2, NULL);
    gtk_tree_view_append_column (GTK_TREE_VIEW (l->list_view), l->column);
    //moko_tree_view_append_column (GTK_TREE_VIEW (l->mokolist_view), l->column);

    l->renderer = gtk_cell_renderer_text_new ();
    l->column = gtk_tree_view_column_new_with_attributes ("Running programs", l->renderer, 
    													"text", 0, NULL);
   gtk_tree_view_append_column (GTK_TREE_VIEW (l->list_view), l->column);
  	//	*/
   // moko_tree_view_append_column (GTK_TREE_VIEW (l->mokolist_view), l->column);
		
    l->scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (l->scrolled),
				  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_container_add (GTK_CONTAINER (l->scrolled), l->list_view);
    
    //l->scrolled = moko_tree_view_put_into_scrolled_window (l->mokolist_view);

    l->btn_close = gtk_button_new ();
    ico = gtk_image_new_from_stock(GTK_STOCK_CLOSE, GTK_ICON_SIZE_SMALL_TOOLBAR);
    gtk_widget_show (ico);
    gtk_container_add(GTK_CONTAINER(l->btn_close), ico);
    gtk_widget_show (l->btn_close);

    l->hbox = gtk_hbox_new (FALSE, 0);
    gtk_widget_show (l->hbox);

    gtk_box_pack_start (GTK_BOX (l), l->hbox, FALSE, FALSE, 0);
    gtk_box_pack_end (l->hbox, l->btn_close, FALSE, FALSE, 0);
    gtk_box_pack_end (GTK_BOX (l), l->scrolled, TRUE, TRUE, 0);

/*delete later*/
    l->tab = gtk_button_new_with_label ("tab");
    l->tabhold = gtk_button_new_with_label ("tab with hold");
    gtk_widget_show (l->tab);
    gtk_widget_show (l->tabhold);
    gtk_box_pack_start (GTK_BOX (l), l->tab, FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (l), l->tabhold, FALSE, FALSE, 0);
/*end deleter later*/
}

/* Construction */
GtkWidget* 
list_new() {
    return GTK_WIDGET(g_object_new(list_get_type(), NULL));
}

/* Destruction */
void 
list_clear(List *l) { 
    if (!l) g_free (l);
}

