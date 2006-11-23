/**
 * @file initwidgets.c 
 * @brief Init all widgets with theme
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
 *
 * @author Chaowei Song(songcw@fic-sh.com.cn)
 * @date 2006-11-09
 */

#include "initwidgets.h"
#include "support.h"
#include "errorcode.h"

/**
 * @brief Init the widget in main window to fit the theme of openmoko.
 * @param widget A widget in main window.
 */
void 
init_main_window_widget (GtkWidget *widget)
{
  GtkWidget *bmainmenu;
  GtkWidget *bfilter;
  GtkWidget *viewportpkglist;
  GtkWidget *treepkglist;

  GtkWidget *fixedbarsep;
  GtkWidget *bbarsep;

  GtkWidget *fixedtoolbar;
  GtkWidget *entrysearch;
  GtkWidget *bapply;
  GtkWidget *bmode;
  GtkWidget *bupgrade;
  GtkWidget *bsearch;
  GtkWidget *bsearchon;

  GtkWidget *vboxapply;
  GtkWidget *alignapply1;
  GtkWidget *imageapply;
  GtkWidget *alignapply2;
  GtkWidget *labelapply;

  GtkWidget *vboxmode;
  GtkWidget *alignmode1;
  GtkWidget *imagemode;
  GtkWidget *alignmode2;
  GtkWidget *labelmode;

  GtkWidget *vboxupgrade;
  GtkWidget *alignupgrade1;
  GtkWidget *imageupgrade;
  GtkWidget *alignupgrade2;
  GtkWidget *labelupgrade;

  GtkWidget *viewportdetail;
  GtkWidget *bfullscreen;
  GtkWidget *textappname;
  GtkWidget *textdetail;

  if (widget == NULL)
    {
      ERROR ("The main window widget is NULL");
      return;
    }

  bmainmenu = lookup_widget (widget, "bmainmenu");
  if (bmainmenu != NULL)
    {
      gtk_widget_set_name (bmainmenu, "btn_menu_left");
    }

  bfilter = lookup_widget (widget, "bfilter");
  if (bfilter != NULL)
    {
      gtk_widget_set_name (bfilter, "btn_filter_menu");
    }

  viewportpkglist = lookup_widget (widget, "viewportpkglist");
  if (viewportpkglist != NULL)
    {
      gtk_widget_set_name (viewportpkglist, "bg_navigation_list");
    }

  treepkglist = lookup_widget (widget, "treepkglist");
  if (treepkglist != NULL)
    {
      gtk_widget_set_name (treepkglist, "treeview_navigation_list");
    }

  fixedbarsep = lookup_widget (widget, "fixedbarsep");
  if (fixedbarsep != NULL)
    {
      gtk_fixed_set_has_window (GTK_FIXED (fixedbarsep), TRUE);
      gtk_widget_set_name (fixedbarsep, "bg_bar_separate");
    }

  bbarsep = lookup_widget (widget, "bbarsep");
  if (bbarsep != NULL)
    {
      gtk_widget_set_name (bbarsep, "btn_bar_separate");
    }

  fixedtoolbar = lookup_widget (widget, "fixedtoolbar");
  if (fixedtoolbar != NULL)
    {
      gtk_fixed_set_has_window (GTK_FIXED (fixedtoolbar), TRUE);
      gtk_widget_set_name (fixedtoolbar, "bg_normal_toolbar");
    }

  entrysearch = lookup_widget (widget, "entrysearch");
  if (entrysearch != NULL)
    {
      gtk_entry_set_has_frame (GTK_ENTRY (entrysearch), FALSE);
      gtk_widget_set_name (entrysearch, "entry_search_toolbar");
      gtk_widget_hide (entrysearch);
    }

  bsearch = lookup_widget (widget, "bsearch");
  if (bsearch != NULL)
    {
      gtk_widget_set_name (bsearch, "btn_toolbar_search");
    }

  bsearchon = lookup_widget (widget, "bsearchon");
  if (bsearchon != NULL)
    {
      gtk_widget_hide (bsearchon);
      gtk_widget_set_name (bsearchon, "btn_toolbar_search_on");
    }

  bapply = lookup_widget (widget, "bapply");

  vboxapply = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vboxapply);
  gtk_container_add (GTK_CONTAINER (bapply), vboxapply);

  alignapply1 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignapply1);
  gtk_box_pack_start (GTK_BOX (vboxapply), alignapply1, TRUE, TRUE, 0);

  imageapply = gtk_image_new_from_stock ("icon_message", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (imageapply);
  gtk_container_add (GTK_CONTAINER (alignapply1), imageapply);

  alignapply2 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignapply2);
  gtk_box_pack_start (GTK_BOX (vboxapply), alignapply2, TRUE, TRUE, 0);

  labelapply = gtk_label_new (_("Apply"));
  gtk_widget_show (labelapply);
  gtk_container_add (GTK_CONTAINER (alignapply2), labelapply);
  gtk_widget_set_size_request (labelapply, 38, 23);
  gtk_misc_set_alignment (GTK_MISC (labelapply), 0.5, 0.84);

  bmode = lookup_widget (widget, "bmode");

  vboxmode = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vboxmode);
  gtk_container_add (GTK_CONTAINER (bmode), vboxmode);

  alignmode1 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignmode1);
  gtk_box_pack_start (GTK_BOX (vboxmode), alignmode1, TRUE, TRUE, 0);

  imagemode = gtk_image_new_from_stock ("icon_message", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (imagemode);
  gtk_container_add (GTK_CONTAINER (alignmode1), imagemode);

  alignmode2 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignmode2);
  gtk_box_pack_start (GTK_BOX (vboxmode), alignmode2, TRUE, TRUE, 0);

  labelmode = gtk_label_new (_("Mode"));
  gtk_widget_show (labelmode);
  gtk_container_add (GTK_CONTAINER (alignmode2), labelmode);
  gtk_widget_set_size_request (labelmode, 38, 23);
  gtk_misc_set_alignment (GTK_MISC (labelmode), 0.5, 0.84);

  bupgrade = lookup_widget (widget, "bupgrade");

  vboxupgrade = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vboxupgrade);
  gtk_container_add (GTK_CONTAINER (bupgrade), vboxupgrade);

  alignupgrade1 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignupgrade1);
  gtk_box_pack_start (GTK_BOX (vboxupgrade), alignupgrade1, TRUE, TRUE, 0);

  imageupgrade = gtk_image_new_from_stock ("icon_message", GTK_ICON_SIZE_BUTTON);
  gtk_widget_show (imageupgrade);
  gtk_container_add (GTK_CONTAINER (alignupgrade1), imageupgrade);

  alignupgrade2 = gtk_alignment_new (0.5, 0.5, 1, 1);
  gtk_widget_show (alignupgrade2);
  gtk_box_pack_start (GTK_BOX (vboxupgrade), alignupgrade2, TRUE, TRUE, 0);

  labelupgrade = gtk_label_new (_("Upgrade"));
  gtk_widget_show (labelupgrade);
  gtk_container_add (GTK_CONTAINER (alignupgrade2), labelupgrade);
  gtk_widget_set_size_request (labelupgrade, 38, 23);
  gtk_misc_set_alignment (GTK_MISC (labelupgrade), 0.5, 0.84);

  viewportdetail = lookup_widget (widget, "viewportdetail");
  if (viewportdetail != NULL)
    {
      gtk_widget_set_name (viewportdetail, "bg_detail_area");
      gtk_viewport_set_shadow_type (GTK_VIEWPORT (viewportdetail), GTK_SHADOW_NONE);
    }

  bfullscreen = lookup_widget (widget, "bfullscreen");
  if (bfullscreen != NULL)
    {
      gtk_widget_set_name (bfullscreen, "scroll_fullscreen_btn_on");
    }

  textappname = lookup_widget (widget, "textappname");
  if (textappname != NULL)
    {
      gtk_widget_set_name (textappname, "textview_detail_area");
    }

  textdetail = lookup_widget (widget, "textdetail");
  if (textdetail != NULL)
    {
      gtk_widget_set_name (textdetail, "textview_detail_area");
    }

}

/**
 * @brief Resize the navigation area and the detail area
 * @param widget A widget in main window
 * @param stepper The stepper size
 */
void 
resize_navigation_detail_area (GtkWidget *widget, gint stepper)
{
  GtkWidget *navigation_viewport;
  GtkWidget *navigation_fixed;
  GtkWidget *navigation_sw;
  GtkWidget *navigation_head_fixed;

  gint navigation_list_height;
  gint navigation_scrollwindow_height;
  gint navigation_sw_pos_y;
  gint x, y;

  GtkWidget *detail_viewport;
  GtkWidget *detail_fixed;
  GtkWidget *detail_sw;
  GtkWidget *button_fullscreen;

  gint detail_area_height;
  gint detail_sw_height;
  gint detail_sw_pos_y;
  gint button_fullscreen_pos_y;

  ///! Get the widget pointer in navigation area
  navigation_viewport = lookup_widget (widget, "viewportpkglist");
  if (navigation_viewport == NULL)
    {
      ERROR ("Can not find the widget named viewportpkglist");
      return;
    }

  navigation_fixed = lookup_widget (widget, "fixedpkglist");
  if (navigation_fixed == NULL)
    {
      ERROR ("Can not find the widget named fixedpkglist");
      return;
    }

  navigation_sw = lookup_widget (widget, "swpkglist");
  if (navigation_sw == NULL)
    {
      ERROR ("Can not find the widget named swpkglist");
      return;
    }

  navigation_head_fixed = lookup_widget (widget, "fixedpkghead");
  if (navigation_head_fixed == NULL)
    {
      ERROR ("Can not find the widget named fixedpkghead");
      return;
    }

  ///! Get the widget pointer in detail area
  detail_viewport = lookup_widget (widget, "viewportdetail");
  if (detail_viewport == NULL)
    {
      ERROR ("Can not find the widget named viewportdetail");
      return;
    }

  detail_fixed = lookup_widget (widget, "fixeddetail");
  if (detail_fixed == NULL)
    {
      ERROR ("Can not find the widget named fixeddetail");
      return;
    }

  detail_sw = lookup_widget (widget, "swdetails");
  if (detail_sw == NULL)
    {
      ERROR ("Can not find the widget named swdetails");
      return;
    }

  button_fullscreen = lookup_widget (widget, "bfullscreen");
  if (button_fullscreen == NULL)
    {
      ERROR ("Can not find the widget named swdetails");
      return;
    }

  ///! Count the sizes and positions of the widgets in the navigation area
  navigation_list_height = NAVIGATION_LIST_HEIGHT + stepper;
  if (stepper > 0)
    {
      navigation_scrollwindow_height = NAVIGATION_SW_HEIGHT + stepper * 0.88;
      navigation_sw_pos_y = NAVIGATION_SW_POS_Y + stepper * 0.07;
    }
  else
    {
      navigation_scrollwindow_height = NAVIGATION_SW_HEIGHT + stepper;
      navigation_sw_pos_y = NAVIGATION_SW_POS_Y;
    }

  ///! Resize and move the widgets in the navigation area
  gtk_fixed_move (GTK_FIXED (navigation_fixed), navigation_head_fixed, 
                  NAVIGATION_SW_POS_X, navigation_sw_pos_y);

  gtk_widget_get_size_request (navigation_sw, &x, &y);
  gtk_widget_set_size_request (navigation_sw, x, navigation_scrollwindow_height);
  gtk_fixed_move (GTK_FIXED (navigation_fixed), navigation_sw, 
                  NAVIGATION_SW_POS_X, navigation_sw_pos_y);

  gtk_widget_get_size_request (navigation_viewport, &x, &y);
  gtk_widget_set_size_request (navigation_viewport, x, navigation_list_height);

  ///! Count the sizes and positions of the widgets in the detail area
  detail_area_height = MAIN_WINDOW_HEIGHT - MENUBAR_HEIGHT - NAVIGATION_LIST_HEIGHT
                       - TOOLBAR_HEIGHT - BAR_SEPARATE_HEIGTH;
  detail_area_height -= stepper;

  if (stepper <= 0)
    {
      detail_sw_height = DETAIL_SW_HEIGHT + (-stepper) * 0.96;
      detail_sw_pos_y = DETAIL_SW_POS_Y + (-stepper) * 0.03;
    }
  else
    {
      detail_sw_height = DETAIL_SW_HEIGHT - stepper;
      detail_sw_pos_y = DETAIL_SW_POS_Y;
    }
  button_fullscreen_pos_y = detail_sw_pos_y - DETAIL_FULLSCREEN_BUTTON_HEIGHT;

  ///! Resize and move the widgets in the navigation area
  gtk_fixed_move (GTK_FIXED (detail_fixed), button_fullscreen, 
                  DETAIL_FULLSCREEN_BUTTON_POS_X, button_fullscreen_pos_y);
  gtk_fixed_move (GTK_FIXED (detail_fixed), detail_sw, DETAIL_SW_POS_X, 
                  detail_sw_pos_y);

  gtk_widget_get_size_request (detail_sw, &x, &y);
  gtk_widget_set_size_request (detail_sw, x, detail_sw_height);

  gtk_widget_get_size_request (detail_viewport, &x, &y);
  gtk_widget_set_size_request (detail_viewport, x, detail_area_height);
}

/**
 * @brief Hide the navigation list and resize the detail area
 * @param widget A widget in main window
 */
void 
full_screen_detail (GtkWidget *widget)
{
  GtkWidget *navigation_viewport;

  GtkWidget *detail_viewport;
  GtkWidget *detail_fixed;
  GtkWidget *detail_sw;
  GtkWidget *button_fullscreen;

  gint detail_area_height;
  gint detail_sw_height;
  gint detail_sw_pos_y;
  gint button_fullscreen_pos_y;
  gint x, y;

  navigation_viewport = lookup_widget (widget, "viewportpkglist");
  if (navigation_viewport == NULL)
    {
      ERROR ("Can not find the widget named viewportpkglist");
      return;
    }

  detail_viewport = lookup_widget (widget, "viewportdetail");
  if (detail_viewport == NULL)
    {
      ERROR ("Can not find the widget named viewportdetail");
      return;
    }

  detail_fixed = lookup_widget (widget, "fixeddetail");
  if (detail_fixed == NULL)
    {
      ERROR ("Can not find the widget named fixeddetail");
      return;
    }

  detail_sw = lookup_widget (widget, "swdetails");
  if (detail_sw == NULL)
    {
      ERROR ("Can not find the widget named swdetails");
      return;
    }

  button_fullscreen = lookup_widget (widget, "bfullscreen");
  if (button_fullscreen == NULL)
    {
      ERROR ("Can not find the widget named swdetails");
      return;
    }

  detail_area_height = MAIN_WINDOW_HEIGHT - MENUBAR_HEIGHT - TOOLBAR_HEIGHT
                       - BAR_SEPARATE_HEIGTH;
  detail_sw_height = DETAIL_SW_HEIGHT + NAVIGATION_LIST_HEIGHT * 0.96;
  detail_sw_pos_y = DETAIL_SW_POS_Y + NAVIGATION_LIST_HEIGHT * 0.03;
  button_fullscreen_pos_y = detail_sw_pos_y - DETAIL_FULLSCREEN_BUTTON_HEIGHT;

  ///! Resize and move the widgets in the navigation area
  gtk_fixed_move (GTK_FIXED (detail_fixed), button_fullscreen, 
                  DETAIL_FULLSCREEN_BUTTON_POS_X, button_fullscreen_pos_y);
  gtk_fixed_move (GTK_FIXED (detail_fixed), detail_sw, DETAIL_SW_POS_X, 
                  detail_sw_pos_y);

  gtk_widget_get_size_request (detail_sw, &x, &y);
  gtk_widget_set_size_request (detail_sw, x, detail_sw_height);

  gtk_widget_get_size_request (detail_viewport, &x, &y);
  gtk_widget_set_size_request (detail_viewport, x, detail_area_height);

  gtk_widget_hide (navigation_viewport);
}
