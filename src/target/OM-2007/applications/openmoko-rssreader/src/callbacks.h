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

#ifndef RSS_READER_CALLBACKS_H
#define RSS_READER_CALLBACKS_H

#include "application-data.h"


/*
 * filter callbacks
 */
gboolean cb_filter_changed(GtkWidget* widget, gchar* text, struct RSSReaderData* d);

/*
 * toolbox callbacks
 */
void cb_subscribe_button_clicked  ( GtkButton *btn, struct RSSReaderData *d);
void refresh_categories( struct RSSReaderData* );
void refresh_feeds( struct RSSReaderData *data );
void filter_feeds ( struct RSSReaderData *data );
void cb_refresh_all_button_clicked( GtkButton *btn, struct RSSReaderData *d);
void cb_searchbox_visible(MokoToolBox* toolbox, struct RSSReaderData* d);
void cb_searchbox_invisible(MokoToolBox* toolbox, struct RSSReaderData* d);

/*
 * changes to the treeview
 */
void cb_treeview_selection_changed( GtkTreeSelection *selection, struct RSSReaderData *d );
gboolean cb_treeview_keypress_event( GtkWidget *entry, GdkEventKey *key, struct RSSReaderData *d );
void cb_search_entry_changed      ( GtkWidget *entry, struct RSSReaderData *d );


#endif
