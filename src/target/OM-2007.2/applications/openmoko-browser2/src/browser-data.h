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

#include "webkitwebview.h"

#include <glib.h>
#include <gtk/gtk.h>

/*
 * representation of one page
 */
#define BROWSER_TYPE_PAGE               (browser_page_get_type ())
#define BROWSER_PAGE(obj)               (G_TYPE_CHECK_INSTANCE_CAST((obj), BROWSER_TYPE_PAGE, BrowserPage))
#define BROWSER_PAGE_CLASS(klass)       (G_TYPE_CHECK_CLASS_CASS((klass),  BROWSER_TYPE_PAGE, BrowserPageClass))
#define BROWSER_IS_PAGE(obj)            (G_TYPE_CHECK_INSTANCE_TYPE((obj), BROWSER_TYPE_PAGE))
#define BROWSER_IS_PAGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_TYPE((klass),  BROWSER_TYPE_PAGE))
#define BROWSER_PAGE_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS((obj),  BROWSER_TYPE_PAGE, BrowserPageClass))
typedef struct _BrowserPage BrowserPage;
typedef struct _BrowserPageClass BrowserPageClass;

struct _BrowserPage {
    GObject parent;
    WebKitWebView* webKitPage;
};

struct _BrowserPageClass {
    GObjectClass parent_class;
};

GType browser_page_get_type (void);
BrowserPage* browser_page_new (WebKitWebView* page);

/*
 * The state of the Browser
 */
struct BrowserData {
    GtkWidget* mainWindow;
    GtkWidget* mainNotebook;
    GtkWidget* currentFingerScroll;

    GtkListStore* browserPages;
    BrowserPage* currentPage;
    GtkTreeIter currentPageIter;

    /**
     * Two special views for the Browser. The Overview
     * and Bookmark page are meant to be partly implemented
     * using HTML and JavaScript and binding the GObject(s) to
     * JavaScript.
     */
    WebKitWebView* pagesOverviewPage;
    WebKitWebView* bookmarkPage;


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
    GtkTreeView* pagesView;
};

#endif
