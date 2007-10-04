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

#include "config.h"
#include "browser-data.h"

G_DEFINE_TYPE(BrowserPage, browser_page, G_TYPE_OBJECT)

static void
browser_page_finalize (GObject* object)
{
    BrowserPage* page = BROWSER_PAGE (object);
    g_object_unref (page->webKitPage);
    page->webKitPage = 0;
}

static void
browser_page_init (BrowserPage* self)
{
    self->webKitPage = NULL;
}

static void
browser_page_class_init (BrowserPageClass *klass)
{
    G_OBJECT_CLASS(klass)->finalize = browser_page_finalize;
}

BrowserPage*
browser_page_new (WebKitPage* webKitPage)
{
    BrowserPage* page = BROWSER_PAGE (g_object_new (BROWSER_TYPE_PAGE, 0));
    page->webKitPage = webKitPage;
    g_object_ref (page->webKitPage);

    return page;
}
