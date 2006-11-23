/**
 * @file widgets.c 
 * @brief Save some widgets pointer.
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
 * @date 2006-10-09
 */

#include "support.h"
#include "widgets.h"
#include "errorcode.h"

static GtkWidget *mainwindow = NULL;
static GtkWidget *swpkglist = NULL;
static GtkWidget *treepkglist = NULL;
static GtkWidget *appimage = NULL;
static GtkWidget *menuappmenu = NULL;
static GtkWidget *menufilter = NULL;
static GtkWidget *menumark = NULL;
static GtkWidget *textappname = NULL;
static GtkWidget *textdetail = NULL;
static GtkWidget *bfilter = NULL;
static GtkWidget *entrysearch = NULL;
static GtkWidget *swdetails = NULL;
static GtkWidget *applyingdialog = NULL;
static GtkWidget *applyingtext = NULL;
static GtkWidget *fixedpkglist = NULL;
static GtkWidget *bsearch = NULL;
static GtkWidget *bsearchon = NULL;
static GtkWidget *bapply = NULL;
static GtkWidget *bmode = NULL;
static GtkWidget *bupgrade = NULL;
static GtkWidget *fixedtoolbar = NULL;

/**
 * @brief Save the main window widget and its child widgets
 *
 * @param widget The main window widget pointer
 */
void
save_main_window (GtkWidget *widget)
{
  mainwindow = widget;

  swpkglist = lookup_widget (widget, "swpkglist");
  if (swpkglist == NULL)
    {
      ERROR ("Can't find the scrolled window widget of package list");
      g_error ("Can't find the scrolled window widget of package list");
    }

  swdetails = lookup_widget (widget, "swdetails");
  if (swdetails == NULL)
    {
      ERROR ("Can't find the scrolled window widget of details");
      g_error ("Can't find the scrolled window widget of details");
    }

  appimage = lookup_widget (widget, "appimage");
  if (appimage == NULL)
    {
      ERROR ("Can't find the image widget that in the detail area");
      g_error ("Can't find the image widget that in the image area");
    }

  treepkglist = lookup_widget (widget, "treepkglist");
  if (treepkglist == NULL)
    {
      ERROR ("Can't find the tree view widget of package list");
      g_error ("Can't find the tree view widget of package list");
    }

  textappname = lookup_widget (widget, "textappname");
  if (textappname == NULL)
    {
      ERROR ("Can't find the textappname widget that in the detail area");
      g_error ("Can't find the textappname widget that in the image area");
    }

  textdetail = lookup_widget (widget, "textdetail");
  if (textdetail == NULL)
    {
      ERROR ("Can't find the textdetail widget that in the detail area");
      g_error ("Can't find the textdetail widget that in the image area");
    }

  bfilter = lookup_widget (widget, "bfilter");
  if (bfilter == NULL)
    {
      ERROR ("Can't find the button filter widget");
      g_error ("Can't find the button filter widget");
    }

  entrysearch = lookup_widget (widget, "entrysearch");
  if (entrysearch == NULL)
    {
      ERROR ("Can't find the entry search widget");
      g_error ("Can't find the entry search widget");
    }

  fixedpkglist = lookup_widget (widget, "fixedpkglist");
  if (fixedpkglist == NULL)
    {
      ERROR ("Can't find the fixed pkg list");
      g_error ("Can't find the fixed pkg list");
    }

  bsearch = lookup_widget (widget, "bsearch");
  if (bsearch == NULL)
    {
      ERROR ("Can't find the button search");
      g_error ("Can't find the button search");
    }

  bsearchon = lookup_widget (widget, "bsearchon");
  if (bsearchon == NULL)
    {
      ERROR ("Can't find the button search on");
      g_error ("Can't find the button search on");
    }

  bapply = lookup_widget (widget, "bapply");
  if (bapply == NULL)
    {
      ERROR ("Can't find the button apply");
      g_error ("Can't find the button apply");
    }

  bmode = lookup_widget (widget, "bmode");
  if (bmode == NULL)
    {
      ERROR ("Can't find the button mode");
      g_error ("Can't find the button mode");
    }

  bupgrade = lookup_widget (widget, "bupgrade");
  if (bupgrade == NULL)
    {
      ERROR ("Can't find the button upgrade");
      g_error ("Can't find the button upgrade");
    }

  fixedtoolbar = lookup_widget (widget, "fixedtoolbar");
  if (fixedtoolbar == NULL)
    {
      ERROR ("Can't find the button upgrade");
      g_error ("Can't find the button upgrade");
    }

}

/**
 * @brief Save the applying dialog widget
 */
void 
save_applying_dialog (GtkWidget *dialog)
{
  applyingdialog = dialog;
  applyingtext = lookup_widget (dialog, "applyingtext");
  if (applyingtext == NULL)
    {
      ERROR ("Can not find the applying text widget");
    }
}

/**
 * @brief Save the application menu widget
 *
 * @param menu The application menu widget
 */
void
save_application_menu (GtkWidget *menu)
{
  menuappmenu = menu;
}

/**
 * Save the filter menu widget.
 * @param menu The filter menu widget
 */
void 
save_filter_menu (GtkWidget *menu)
{
  menufilter = menu;
}

/**
 * Save the mark menu widget.
 * @param menu The mark menu widget
 */
void 
save_mark_menu (GtkWidget *menu)
{
  menumark = menu;
}

/**
 * @brief Get the specified widget pointer
 *
 * @param id The widget id
 * @return The widget pointer. If id is incorrect, return NULL
 */
GtkWidget *
get_widget_pointer (FIC_WIDEGT_ID id)
{
  switch (id)
    {
      case FIC_WIDGET_WINDOW_MAIN_WINDOW:
        return mainwindow;

      case FIC_WIDGET_DIALOG_APPLYING_DIALOG:
        return applyingdialog;

      case FIC_WIDGET_DIALOG_APPLYING_TEXT:
        return applyingtext;

      case FIC_WIDGET_FIX_PKG_LIST:
        return fixedpkglist;

      case FIC_WIDGET_SCROLLED_WINDOW_PKG_LIST:
        return swpkglist;

      case FIC_WIDGET_SCROLLED_WINDOW_DETAILS:
        return swdetails;

      case FIC_WIDGET_TREE_VIEW_PKG_LIST:
        return treepkglist;

      case FIC_WIDGET_IMAGE_IMAGE1:
        return appimage;

      case FIC_WIDGET_TEXT_VIEW_TEXTAPPNAME:
        return textappname;

      case FIC_WIDGET_TEXT_VIEW_DETAIL:
        return textdetail;

      case FIC_WIDGET_BUTTON_FILTER:
        return bfilter;

      case FIC_WIDGET_BUTTON_SEARCH:
        return bsearch;

      case FIC_WIDGET_BUTTON_SEARCHON:
        return bsearchon;

      case FIC_WIDGET_BUTTON_APPLY:
        return bapply;

      case FIC_WIDGET_BUTTON_MODE:
        return bmode;

      case FIC_WIDGET_BUTTON_UPGRADE:
        return bupgrade;

      case FIC_WIDGET_FIXED_TOOL_BAR:
        return fixedtoolbar;

      case FIC_WIDGET_ENTRY_SEARCH:
        return entrysearch;

      case FIC_WIDGET_MENU_APPLICATION_MENU:
        return menuappmenu;

      case FIC_WIDGET_MENU_FILTER_MENU:
        return menufilter;

      case FIC_WIDGET_MENU_SELECT_MENU:
        return menumark;

      default:
        return NULL;
    }
}

