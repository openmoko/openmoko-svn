/*
 *  RSS Reader, a simple RSS reader
 *
 *  Copyright (C) 2007 Holger Hans Peter Freyther
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 *  OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 *  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 *  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *  OTHER DEALINGS IN THE SOFTWARE.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#ifndef APPLICATION_DATA_H
#define APPLICATION_DATA_H


#include <libmokoui/moko-application.h>
#include <libmokoui/moko-paned-window.h>
#include <libmokoui/moko-tree-view.h>
#include <libmokoui/moko-tool-box.h>

#include <webkitgtkpage.h>

#include "moko_cache.h"

#include <gtk/gtk.h>

struct RSSReaderData {
    MokoApplication   *app;
    MokoCache         *cache;
    GtkMenu           *menu;
    GtkMenu           *filter;
    MokoPanedWindow   *window;
    MokoToolBox       *box;
    MokoMenuBox       *menubox;


    MokoTreeView      *treeView;
    GtkListStore      *feed_data;
    GtkTreeModelFilter *filter_model;
    GtkTreeModelSort  *sort_model;

    WebKitGtkPage     *textPage;

    gchar             *current_filter;
    int                is_all_filter;
    gchar             *current_search_text;
};

/*
 * Instead of having a real model we have this... as our feed
 * model
 * Either there is a link or text is included. And we contain
 * the sourcename of the feed. This will be used by the ModelFilter
 * to implement the FilterMenu
 */
enum {
    RSS_READER_COLUMN_AUTHOR,
    RSS_READER_COLUMN_SUBJECT,
    RSS_READER_COLUMN_DATE,
    RSS_READER_COLUMN_LINK,     /* Is this something like spiegel.de and only has a link */
    RSS_READER_COLUMN_TEXT,     /* Either link is NULL, or this contains the article     */
    RSS_READER_COLUMN_TEXT_TYPE,/* The Text Type of Atom feeds HTML, plain...            */
    RSS_READER_COLUMN_CATEGORY, /* The category as shown in the filter box               */
    RSS_READER_COLUMN_SOURCE,   /* the source of this entry, the URL of the feed, not the atom <source> */
    RSS_READER_NUM_COLS,
};

/**
 * text type for atom feeds, default is none
 */
enum {
    RSS_READER_TEXT_TYPE_NONE,
    RSS_READER_TEXT_TYPE_PLAIN,
    RSS_READER_TEXT_TYPE_HTML,
    RSS_READER_TEXT_TYPE_UNKNOWN
};

#endif
