/*
 *  openmoko-mainmenu
 *
 *  Authored By Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
 *
 *  Copyright (C) 2006 First International Computer Inc.
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
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#include "mainmenu.h"

enum {
    MENU_SIGNAL,
    LAST_SIGNAL
};

static void 
moko_main_menu_class_init(MokoMainMenuClass *klass);

static void 
moko_main_menu_init(MokoMainMenu *mm);

static guint menu_signals[LAST_SIGNAL] = { 0 };

/**
*@brief retrun List type.
*@param none
*@return GType
*/
GType 
moko_main_menu_get_type (void) /* Typechecking */
{
    static GType menu_type = 0;

    if (!menu_type)
    {
        static const GTypeInfo menu_info =
        {
            sizeof (MokoMainMenuClass),
            NULL, /* base_init */
            NULL, /* base_finalize */
            (GClassInitFunc) moko_main_menu_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (MokoMainMenu),
            0,
            (GInstanceInitFunc) moko_main_menu_init,
            NULL
        };

        menu_type = g_type_register_static (GTK_TYPE_VBOX, "MokoMainMenu", &menu_info, 0);
        //menu_type = g_type_register_static (GTK_TYPE_WIDGET, "MokoMainMenu", &menu_info, 0);

    }

    return menu_type;
}

/**
*@brief initialize	MokoMainMenu class.
*@param klass	MokoMainMenu Class
*@return none
*/
static void 
moko_main_menu_class_init(MokoMainMenuClass* Klass) /* Class Initialization */
{
    menu_signals[MENU_SIGNAL] = g_signal_new ("MokoMainMenu",
            G_TYPE_FROM_CLASS (Klass),
            G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION,
            G_STRUCT_OFFSET (MokoMainMenuClass, moko_main_menu_function),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE, 
            0);
}


/*@brief initialize 	MokoManiMenu instance
 *@param mm	MokoMainMenu*
 *@return none
 */
void
moko_main_menu_init(MokoMainMenu *mm) {
    PangoFontDescription* PangoFont = pango_font_description_new(); //get system default PangoFontDesc

    mm->section_name =  gtk_label_new ("Main Menu");
    gtk_widget_show (mm->section_name);
    gtk_widget_set_name (GTK_WIDGET (mm->section_name), "Section Name");
    gtk_label_set_single_line_mode (mm->section_name, TRUE);
    gtk_misc_set_alignment (GTK_MISC (mm->section_name), SECTION_ALG_X, SECTION_ALG_Y);
    gtk_misc_set_padding (GTK_MISC (mm->section_name), SECTION_X_PADDING, SECTION_Y_PADDING);
    gtk_label_set_ellipsize (mm->section_name, PANGO_ELLIPSIZE_END);
    /*if (PangoFont) {
    	  pango_font_description_set_size (PangoFont, FONT_SIZE_SECTION);
	  gtk_widget_modify_font (GTK_WIDGET (mm->section_name), PangoFont);
	  }
    else {
    	  g_debug("FAILED to load FONT ");
    	  }
*/
    mm->item_total = gtk_label_new ("11/22");
    gtk_widget_show (mm->item_total);
    gtk_label_set_width_chars (mm->item_total, ITME_TOTAL_WIDTH);
    gtk_misc_set_alignment (GTK_MISC (mm->item_total), ITEM_TOTAL_ALG_X, ITEM_TOTAL_ALG_Y);
    /*if (PangoFont) {
    	  pango_font_description_set_size (PangoFont, FONT_SIZE_ITEM);
	  gtk_widget_modify_font (GTK_WIDGET (mm->item_total), PangoFont);
	  }
    else {
    	  g_debug("FAILED to load FONT ");
    	  }
*/
    //mm->icon_view = gtk_icon_view_new ();
    mm->icon_view = MOKO_ICON_VIEW(moko_icon_view_new());
    gtk_widget_show (mm->icon_view);
    gtk_icon_view_set_columns (mm->icon_view, COLUMN_NUM);
    //gtk_icon_view_set_margin (mm->icon_view, ICON_MARGIN);
    //gtk_icon_view_set_row_spacing (mm->icon_view, ROW_SPACING);
    //gtk_icon_view_set_column_spacing (mm->icon_view, COLUMN_SPACING);
    //gtk_icon_view_set_selection_mode (mm->icon_view, GTK_SELECTION_SINGLE);
    gtk_widget_show (mm->icon_view);

    mm->list_store = gtk_list_store_new (3, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_POINTER);

    gtk_icon_view_set_pixbuf_column (GTK_ICON_VIEW (mm->icon_view), PIXBUF_COLUMN);
    gtk_icon_view_set_text_column (GTK_ICON_VIEW (mm->icon_view), TEXT_COLUMN);
    gtk_icon_view_set_model (GTK_ICON_VIEW (mm->icon_view), GTK_TREE_MODEL (mm->list_store));

    mm->scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (mm->scrolled),
				  GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_widget_show (mm->scrolled);
   // gtk_scrolled_window_add_with_viewport (GTK_CONTAINER (mm->scrolled),
    //						mm->icon_view);
    gtk_container_add (GTK_CONTAINER (mm->scrolled), mm->icon_view);

//    mm->icon_view = MOKO_ICON_VIEW(moko_icon_view_new());
  //  gtk_widget_show (mm->icon_view);
 
    mm->hbox = gtk_hbox_new (FALSE, FALSE);
    gtk_widget_show (mm->hbox);

    gtk_box_pack_start (mm, mm->hbox, FALSE, FALSE, 0);
    gtk_box_pack_start (mm->hbox, mm->section_name, TRUE, TRUE, 10);
    gtk_box_pack_end (mm->hbox, mm->item_total, FALSE, FALSE, 10);
    gtk_box_pack_end (mm, mm->scrolled, TRUE, TRUE, 0);
    //gtk_box_pack_end (mm, mm->icon_view, TRUE, TRUE, 0);


    moko_sample_model_fill(mm->list_store);
    gtk_widget_show (mm);

    if (PangoFont)
    	  pango_font_description_free (PangoFont);
}


/* Construction */
GtkWidget* 
moko_main_menu_new() {
    return GTK_WIDGET(g_object_new(moko_main_menu_get_type(), NULL));
}

/* Destruction */
void 
moko_main_menu_clear(MokoMainMenu *mm) { 
    if (!mm) g_free (mm);
}

/*
*
*
*/
void
moko_main_menu_update(GtkListStore *store) {
    
}

