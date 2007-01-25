/**
 *  @file mainmenu.c
 *  @brief The Main Menu in the Openmoko
 *  
 *  Authored by Sun Zhiyong <sunzhiyong@fic-sh.com.cn>
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
#include "mokodesktop.h"
#include "mokodesktop_item.h"

#include "mainmenu.h"

enum {
    MENU_SIGNAL = 0,
    LAST_SIGNAL
};

static void 
moko_main_menu_class_init(MokoMainMenuClass *klass);

static void 
moko_main_menu_init(MokoMainMenu *mm);

static guint menu_signals[LAST_SIGNAL] = { 0 };

/**
*@brief retrun MokoMainMenu type.
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
            NULL, /* class_init *///(GClassInitFunc) moko_main_menu_class_init,
            NULL, /* class_finalize */
            NULL, /* class_data */
            sizeof (MokoMainMenu),
            0,
            (GInstanceInitFunc) moko_main_menu_init,
            NULL
        };
        menu_type = g_type_register_static (GTK_TYPE_VBOX, "MokoMainMenu", &menu_info, 0);
    }

    return menu_type;
}

/**
*@brief initialize	MokoMainMenu class.
*@param klass	MokoMainMenu class
*@return none
*/
static void 
moko_main_menu_class_init(MokoMainMenuClass* Klass) 
{

}


/*@brief initialize 	MokoManiMenu instance
 *@param mm	MokoMainMenu*
 *@return none
 */
void
moko_main_menu_init(MokoMainMenu *mm) 
{
    PangoFontDescription* PangoFont = pango_font_description_new(); //get system default PangoFontDesc
    GtkEventBox *eventbox;
    int ret = 0 ;
    /* Buid Root item, don't display */
    mm->top_item  = mokodesktop_item_new_with_params ("Home", 
						       NULL,
						       NULL,
						       ITEM_TYPE_ROOT );

    /* Build Lists (parse .directory and .desktop files) */	
    ret = mokodesktop_init(mm->top_item, ITEM_TYPE_CNT);
    //make current item point to the top_item.
    mm->current = mm->top_item;

    /*center label of MokoMainMenu head*/
    mm->section_name =  gtk_label_new ("Main Menu");
    gtk_widget_show (mm->section_name);
    gtk_widget_set_name (GTK_WIDGET (mm->section_name), "Section Name");
    gtk_label_set_single_line_mode (mm->section_name, TRUE);
    gtk_misc_set_alignment (GTK_MISC (mm->section_name), SECTION_ALG_X, SECTION_ALG_Y);
    gtk_misc_set_padding (GTK_MISC (mm->section_name), SECTION_X_PADDING, SECTION_Y_PADDING);
    gtk_label_set_ellipsize (mm->section_name, PANGO_ELLIPSIZE_END);
   /* if (PangoFont) {
    	  pango_font_description_set_size (PangoFont, FONT_SIZE_SECTION);
	  gtk_widget_modify_font (GTK_WIDGET (mm->section_name), PangoFont);
	  }
    else {
    	  g_debug("FAILED to load FONT ");
    	  }
*/


    /*right side label of MokoMainMenu head*/
    mm->item_total = gtk_label_new ("");
    //gtk_widget_show (mm->item_total);
    gtk_label_set_justify (mm->item_total, GTK_JUSTIFY_RIGHT);
    gtk_label_set_width_chars (mm->item_total, ITME_TOTAL_WIDTH);
    gtk_misc_set_alignment (GTK_MISC (mm->item_total), ITEM_TOTAL_ALG_X, ITEM_TOTAL_ALG_Y);
    if (PangoFont) {
    	  pango_font_description_set_size (PangoFont, FONT_SIZE_ITEM);
	  gtk_widget_modify_font (GTK_WIDGET (mm->item_total), PangoFont);
	  }
    else {
    	  g_debug("FAILED to load FONT ");
    	  }

    //Only used to change background
    eventbox = gtk_event_box_new ();
    gtk_event_box_set_visible_window (eventbox, TRUE);
    gtk_widget_show (eventbox);
    gtk_widget_set_name (eventbox, "gtkeventbox-black");

    /*MokoIconView object initialize*/
    mm->icon_view = moko_icon_view_new();
    moko_icon_view_set_item_width(mm->icon_view, ITEM_WIDTH);
    moko_icon_view_set_columns (mm->icon_view, COLUMN_NUM);
    moko_icon_view_set_margin (mm->icon_view, ITEM_MARGIN);
    moko_icon_view_set_row_spacing (mm->icon_view, ROW_SPACING);
    moko_icon_view_set_column_spacing (mm->icon_view, COLUMN_SPACING);
    moko_icon_view_set_decoration_width (mm->icon_view, 20);
    moko_icon_view_set_icon_bg (mm->icon_view, PKGDATADIR"/main_menu_sel_icon.png");
    moko_icon_view_set_text_bg (mm->icon_view, PKGDATADIR"/main_menu_sel_text.png");
    moko_icon_view_set_decorated (mm->icon_view, TRUE);
    moko_icon_view_set_max_text_length(mm->icon_view, 20);
    gtk_widget_show (mm->icon_view);

    mm->list_store = gtk_list_store_new (3, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_POINTER);
    moko_icon_view_set_pixbuf_column (mm->icon_view, PIXBUF_COLUMN);
    moko_icon_view_set_text_column (mm->icon_view, TEXT_COLUMN);
    moko_icon_view_set_model (mm->icon_view, GTK_TREE_MODEL (mm->list_store));

    mm->scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (mm->scrolled),
				  GTK_POLICY_NEVER, GTK_POLICY_NEVER);
    gtk_widget_show (mm->scrolled);
    gtk_container_add (GTK_CONTAINER (mm->scrolled), mm->icon_view);

    mm->hbox = gtk_hbox_new (FALSE, FALSE);
    gtk_widget_show (mm->hbox);
    
    gtk_box_pack_start (mm, eventbox, FALSE, FALSE, 0);
    gtk_container_add (eventbox, mm->hbox); 
    gtk_box_pack_start (mm->hbox, mm->section_name, TRUE, TRUE, 0);
    gtk_box_pack_end (mm->hbox, mm->item_total, FALSE, FALSE, 0);
    gtk_box_pack_end (mm, mm->scrolled, TRUE, TRUE, 0);

    moko_main_menu_update_content (mm, mm->current);

    if (PangoFont)
    	  pango_font_description_free (PangoFont);
}

static void
moko_set_label_content(GtkLabel *label, const char *content)
{
    if (label)
    	  gtk_label_set_text (label, content);
}

/**
*@brief fill model
*@param store		GtkListSrore*
*@param icon_path	const char*
*@param icon_name	const char*
*@return Bool
*/
static gboolean
moko_fill_model(GtkListStore *store, const char* icon_path, 
						const char* icon_name, MokoDesktopItem *item)
{
    if (!icon_path && !icon_name)
        return FALSE;

    GtkTreeIter iter;
    GdkPixbuf *pixbuf;

    gtk_list_store_append (store, &iter);
    pixbuf = gdk_pixbuf_new_from_file_at_size (icon_path, PIXBUF_WIDTH, PIXBUF_HEIGHT, NULL);// ADD Gerro handle later
    gtk_list_store_set (store, &iter, PIXBUF_COLUMN, pixbuf, TEXT_COLUMN, icon_name, OBJECT_COLUMN, item, -1);
    g_object_unref (pixbuf);
    return TRUE;
}

/*public functions*/

/**
 * moko_main_menu_new:
 * 
 * Creates a new #MokoMainMenu widget
 * 
 * Return value: A newly created #MokoMainMenu widget
 *
 **/
GtkWidget* 
moko_main_menu_new () 
{
    return GTK_WIDGET(g_object_new(moko_main_menu_get_type(), NULL));
}

/**
 * moko_main_menu_clear:
 * @mm #MokoMainMenu instance.
 * 
 * clear a #MokoMainMenu widget.
 * 
 **/
void 
moko_main_menu_clear(MokoMainMenu *mm) 
{ 
    if (mm->top_item)
    	{
    	  /* Free Lists (free .directory and .desktop files) */
	  mokodesktop_item_folder_contents_free(mm->top_item, mm->top_item);
	  /* Free Root item */
	  mokodesktop_item_free(mm->top_item);
    	}
    if (mm) g_free (mm);
}

/**
 * moko_main_menu_update_content:
 * @mm #MokoMainMenu.
 * @item #MokoDesktopItem list.
 * 
 * Reinstall a #MokoMainMenu widget content.
 *
 * Return value: %TRUE if @update successful is selected.
 * 
 **/
gboolean
moko_main_menu_update_content (MokoMainMenu *mm, MokoDesktopItem *item)
{
  MokoDesktopItem *item_new;
  gint count = 0;
  char total_item[6];
  //g_debug("mokodesktop: item [%d][%s][%s]\n", item->type, item->name, item->icon_name);
    
  item_new = item->item_child;
  //g_debug("mokodesktop: item [%d][%s][%s]\n", item_new->type, item_new->name, item_new->icon_name);
 // g_debug ("test");

  if (item->type == ITEM_TYPE_ROOT)
  	{
  	moko_set_label_content (mm->section_name, "Main Menu");
  	}
  else if (item->type == ITEM_TYPE_FOLDER)
  	{
  	moko_set_label_content (mm->section_name, item->name);
  	}
  else 
  	return FALSE; // neither ROOT nor FOLDER

  if (mm->list_store)
  	gtk_list_store_clear (mm->list_store);

  mokodesktop_items_enumerate_siblings(item->item_child, item_new)
  { 
     count +=1;
     
     if (access (item_new->icon_name, 0) == 0)
     {
      	moko_fill_model(mm->list_store, item_new->icon_name, item_new->name, item_new);
     }
     else 
     {
       char path[512];
       snprintf (path, 512, "%s/%s", PIXMAP_PATH, item_new->icon_name);

       if (access (path, 0) == 0)
          moko_fill_model(mm->list_store, path, item_new->name, item_new);
       else
         {
	     snprintf (path, 512, "%s/%s", PKGDATADIR, "default-app-icon.xpm");
      	     moko_fill_model(mm->list_store, path, item_new->name, item_new);
         }
      }
    }

  snprintf (total_item, 6, "00/%.2d", count);
  moko_set_label_content(mm->item_total, total_item);

  return TRUE;
}

/**
 * moko_main_menu_update_item_total_label:
 * @mm A #MokoMainMenu.
 *
 * Update right side label infomation.
 *
 **/
void
moko_main_menu_update_item_total_label (MokoMainMenu *mm)
{  
  gint total = 0, cursor = 0;
  char item_total[6];

  total = moko_icon_view_get_total_items (mm->icon_view);
  cursor = moko_icon_view_get_cursor_positon (mm->icon_view);

  if (cursor <0)
  	return;

  snprintf (item_total, 6, "%.2d/%.2d", cursor, total);
  moko_set_label_content (mm->item_total, item_total);
}

/**
 * moko_main_menu_set_setction_name_label:
 * @mm A #MokoMainMenu.
 * @str const char*
 *
 * Set Center label content of #MokoMainMenu head.
 *
 **/
void
moko_main_menu_set_section_name_label (MokoMainMenu *mm, const char *str)
{
    moko_set_label_content (mm->section_name, str);
}
