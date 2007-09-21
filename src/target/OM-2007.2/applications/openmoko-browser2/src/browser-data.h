/*
 * A simple WebBrowser
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

#ifndef OPENMOKO_BROWSER_DATA_H
#define OPENMOKO_BROWSER_DATA_H

#include "webkitgtkpage.h"

#include <glib.h>
#include <gtk/gtk.h>

/*
 * representation of one page
 */
struct BrowserPage {
    WebKitGtkPage* webKitPage;
};

/*
 * The state of the Browser
 */
struct BrowserData {
    GtkWidget* mainWindow;
    GtkWidget* mainNotebook;
    GtkWidget* currentFingerScroll;

    GList* browserPages;
    struct BrowserPage* currentPage;

    /**
     * Two special views for the Browser. The Overview
     * and Bookmark page are meant to be partly implemented
     * using HTML and JavaScript and binding the GObject(s) to
     * JavaScript.
     */
    WebKitGtkPage* pagesOverviewPage;
    WebKitGtkPage* bookmarkPage;


    /*
     * Current
     */
    GtkToolItem* currentBack;
    GtkToolItem* currentForward;
    GtkToolItem* currentStop;
    GtkToolItem* currentAdd;
    GtkToolItem* currentClose;

    /*
     * Go-Page
     */
    GtkWidget* goBox;
    GtkEntry* goUrlEntry;
    GtkEntry* goSearchEntry;
    GtkWidget* goButton;
    GtkWidget* goButtonNewPage;

    /*
     * Open-Pages-Page
     */
    GtkWidget* pagesBox;
    GtkListStore* pagesStore;
    GtkTreeView* pagesView;
};

#endif
