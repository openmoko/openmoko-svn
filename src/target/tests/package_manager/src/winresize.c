/**
 * @file winresize.c - Manager the size and allocation of each widget
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
 * @date 2006-7-28
 **/

#include <gtk/gtk.h>
#include "errorcode.h"
#include "winresize.h"
#include "support.h"

/**
 * @brief Resize the hbox widget
 *
 * @param fix The fix widget
 * @param hbox The hbox widget
 * @param allocation The widget new allocation
 */
void hbox_resize(GtkWidget *fix, GtkWidget *hbox, GdkRectangle *allocation)
{
  gtk_widget_set_size_request (hbox, allocation->width, allocation->height);
}

/**
 * @brief Resize the vboxpackage widget
 *
 * @param fix The fix widget
 * @param vboxpackage The vboxpackage widget
 * @param allocation The widget new allocation
 */
void vboxpackage_resize(GtkWidget *fix, GtkWidget *vboxpackage, GdkRectangle *allocation)
{
  gtk_fixed_move (GTK_FIXED (fix), vboxpackage, allocation->x, allocation->y);
  gtk_widget_set_size_request (vboxpackage, allocation->width, allocation->height);
}

/**
 * @brief Resize the vboxtoolbar widget
 *
 * @param fix The fix widget
 * @param vboxtoolbar The vboxtoolbar widget
 * @param allocation The widget new allocation
 */
void vboxtoolbar_resize(GtkWidget *fix, GtkWidget *vboxtoolbar, GdkRectangle *allocation)
{
  gtk_fixed_move (GTK_FIXED (fix), vboxtoolbar, allocation->x, allocation->y);
  gtk_widget_set_size_request (vboxtoolbar, allocation->width, allocation->height);
}

/**
 * @brief Resize the vboxsplit widget
 *
 * @param fix The fix widget
 * @param vboxsplit The vboxsplit widget
 * @param allocation The widget new allocation
 */
void vboxsplit_resize(GtkWidget *fix, GtkWidget *vboxsplit, GdkRectangle *allocation)
{
  gtk_fixed_move (GTK_FIXED (fix), vboxsplit, allocation->x, allocation->y);
  gtk_widget_set_size_request (vboxsplit, allocation->width, allocation->height);
}

/**
 * @brief Resize the vboxdetailarea widget
 *
 * @param fix The fix widget
 * @param vboxdetailarea The vboxdetailarea widget
 * @param allocation The widget new allocation
 */
void vboxdetailarea_resize(GtkWidget *fix, GtkWidget *vboxdetailarea, GdkRectangle *allocation)
{
  gtk_fixed_move (GTK_FIXED (fix), vboxdetailarea, allocation->x, allocation->y);
  gtk_widget_set_size_request (vboxdetailarea, allocation->width, allocation->height);
}

/**
 * @brief Resize and reallocation all widgets
 *
 * Count the size of widgets by the window size, and count the coordinate
 * of each widget.
 * @param window The main window widget
 * @param allocation The main window coordinate and size
 * @return Error code
 * @retval OP_SUCCESS Operation success
 * @retval OP_WINDOW_SIZE_WIDGET_NOT_FIND Some widget not find
 * @retval OP_WINDOW_SIZE_NEED_NOT_CHANGE The widget window don't need resize
 */
gint main_window_resize(GtkWidget *window,
                        GdkRectangle    *allocation)
{
  static GdkRectangle  orsize = {0, 0, 0, 0};

  GtkWidget *fixed1;
  GtkWidget *hboxtop;
  GtkWidget *vboxpackage;
  GtkWidget *vboxtoolbar;
  GtkWidget *vboxsplit;
  GtkWidget *vboxdetailarea;
  GtkWidget *swbg;

  GdkRectangle lo;

  /** if width small then minimum size, set the width to minimum size */
  if (allocation->width < MAIN_WINDOW_MIN_SIZE_WIDTH)
    {
      allocation->width = MAIN_WINDOW_MIN_SIZE_WIDTH;
    }

  /** if height small then minimum size, set the height to minimum size */
  if (allocation->height < MAIN_WINDOW_MIN_SIZE_HEIGHT)
    {
      allocation->height = MAIN_WINDOW_MIN_SIZE_HEIGHT;
    }

  if ( (orsize.width == 0) || (orsize.height == 0) )
    {
      orsize.width = allocation->width;
      orsize.height = allocation->height;
    }

  if ( (orsize.width == allocation->width)
      && (orsize.height == allocation->height) )
    {
      return OP_WINDOW_SIZE_NEED_NOT_CHANGE;
    }
  else
    {
      orsize.width = allocation->width;
      orsize.height = allocation->height;
    }

  DBG("Begin resize the window\n");
  DBG("allocation111 w=%d,h=%d\n", allocation->width, allocation->height);
  DBG("window-size: w=%d,h=%d\n", window->allocation.width, window->allocation.height);

  if ( (allocation->width %2) == 0)
    allocation->width -= 2;
  else
    allocation->width -= 1;

  if ( (allocation->height %2) == 0)
    allocation->height -= 2;
  else
    allocation->height -= 1;

  fixed1 = lookup_widget (window, STRING_FIXED_MAIN);
  hboxtop = lookup_widget (window, STRING_HBOX_TOP);
  vboxpackage = lookup_widget (window, STRING_VBOX_PACKAGE);
  vboxtoolbar = lookup_widget (window, STRING_VBOX_TOOL_BAR);
  vboxsplit = lookup_widget (window, STRING_VBOX_SPLIT);
  vboxdetailarea = lookup_widget (window, STRING_VBOX_DETAIL_AREA);
  swbg = lookup_widget (window, STRING_SCROLLED_WINDOW_MAIN);


  gtk_widget_set_size_request(fixed1, allocation->width, allocation->height);
  //gtk_widget_set_size_request(swbg, orsize.width, orsize.height);


  gtk_widget_hide(swbg);

  if ( (fixed1 == NULL)
      || (hboxtop == NULL)
      || (vboxpackage == NULL)
      || (vboxtoolbar == NULL)
      || (vboxsplit == NULL)
      || (vboxdetailarea == NULL)
      || (swbg == NULL) )
    {
      ERROR ("Some widget not find\n");
      return OP_WINDOW_SIZE_WIDGET_NOT_FIND;
    }

  lo.x = 0;
  lo.y = 0;
  lo.width = allocation->width;
  lo.height = hboxtop->allocation.height;

  hbox_resize (fixed1, hboxtop, &lo);

  lo.y = lo.y + lo.height;
  lo.height = (gint) (allocation->height/2) - hboxtop->allocation.height \
              - vboxtoolbar->allocation.height - vboxsplit->allocation.height;

  vboxpackage_resize(fixed1, vboxpackage, &lo);

  lo.y = lo.y + lo.height;
  lo.height = vboxtoolbar->allocation.height;

  vboxtoolbar_resize(fixed1, vboxtoolbar, &lo);

  lo.y = lo.y + lo.height;
  lo.height = vboxsplit->allocation.height;

  vboxsplit_resize(fixed1, vboxsplit, &lo);

  lo.y = lo.y + lo.height;
  lo.height = allocation->height - lo.y;

  vboxdetailarea_resize(fixed1, vboxdetailarea, &lo);

  /*
  if (allocation->width < MAIN_WINDOW_MIN_SIZE_WIDTH)
    {
      gtk_widget_hide (GTK_SCROLLED_WINDOW (swbg)->hscrollbar);
    }
  else
    {
      gtk_widget_show (GTK_SCROLLED_WINDOW (swbg)->hscrollbar);
    }

  if (allocation->height < MAIN_WINDOW_MIN_SIZE_HEIGHT)
    {
      gtk_widget_hide (GTK_SCROLLED_WINDOW (swbg)->vscrollbar);
    }
  else
    {
      gtk_widget_show (GTK_SCROLLED_WINDOW (swbg)->vscrollbar);
    }
  */
  DBG("allocation w=%d,h=%d\n", allocation->width, allocation->height);
  gtk_widget_show(swbg);

  DBG("swbg-size: w=%d,h=%d\n", swbg->allocation.width, swbg->allocation.height);
  DBG("fixed1-size: w=%d,h=%d\n", fixed1->allocation.width, fixed1->allocation.height);

  return OP_SUCCESS;
}
