/**
 * @file winresize.h - Manager the size and allocation of each widget
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

#ifndef _FIC_WINRESIZE_H
#define _FIC_WINRESIZE_H

#define MAIN_WINDOW_MIN_SIZE_WIDTH      240
#define MAIN_WINDOW_MIN_SIZE_HEIGHT     240

#define STRING_SCROLLED_WINDOW_PACKAGE      "swpackage"
#define STRING_TREE_VIEW_PACKAGE            "tvpackage"
#define STRING_FIXED_MAIN                   "fixed1"
#define STRING_HBOX_TOP                     "hboxtop"
#define STRING_VBOX_PACKAGE                 "vboxpackage"
#define STRING_VBOX_SPLIT                   "vboxsplit"
#define STRING_VBOX_DETAIL_AREA             "vboxdetailarea"
#define STRING_VBOX_TOOL_BAR                "vboxtoolbar"
#define STRING_SCROLLED_WINDOW_MAIN         "scrolledwindow1"
#define STRING_TEXT_VIEW_SUMMARY            "tvsummary"
#define STRING_TEXT_VIEW_DEPEND             "tvdepend"
#define STRING_TREE_VIEW_DETAILS            "tvdetails"
#define STRING_ENTRY_SEARCH                 "entrysearch"
#define STRING_BUTTON_ACTION                "buttonaction"
#define STRING_BUTTON_PACKAGE               "buttonpackage"
#define STRING_BUTTON_PACKAGE_LABEL         "butpkglabel"
#define STRING_BUTTON_ALL_LABEL             "butalllabel"

gint main_window_resize(GtkWidget *window,
                        GdkRectangle    *allocation);


#endif
