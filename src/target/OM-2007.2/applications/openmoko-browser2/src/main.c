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
#include "current-page.h"

#include <gtk/gtk.h>
#include <glib/gi18n.h>

#include <webkitgtkglobal.h>

#include <moko-finger-scroll.h>
#include <moko-stock.h>

#include <stdlib.h>


static void setup_ui (struct BrowserData*);

int main (int argc, char** argv)
{
    g_debug ("openmoko-browser starting up");

    bindtextdomain (GETTEXT_PACKAGE, BROWSER_LOCALE_DIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);

    g_thread_init (NULL);
    gdk_threads_init ();
    gdk_threads_enter ();
    gtk_init (&argc, &argv);
    webkit_gtk_init ();
    moko_stock_register ();
    g_set_application_name (_("Browser"));

    struct BrowserData* data = g_new0 (struct BrowserData, 1);
    setup_ui (data);

    gtk_main ();
    gdk_threads_leave ();
    g_free (data);

    return EXIT_SUCCESS;
}

static void window_delete_event(GtkWidget* widget, GdkEvent* event, gpointer _data)
{
    gtk_main_quit ();
}

static void setup_ui (struct BrowserData* data)
{
    data->mainWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    g_signal_connect (data->mainWindow, "delete-event", G_CALLBACK(window_delete_event), NULL);

    data->mainNotebook = gtk_notebook_new ();
    gtk_notebook_set_tab_pos (GTK_NOTEBOOK (data->mainNotebook), GTK_POS_BOTTOM);
    gtk_container_add (GTK_CONTAINER (data->mainWindow), GTK_WIDGET (data->mainNotebook));

    /*
     * Current Page
     */
    GtkWidget* box = gtk_vbox_new (FALSE, 0);
    gtk_notebook_append_page (GTK_NOTEBOOK (data->mainNotebook), box, gtk_image_new_from_stock (GTK_STOCK_NETWORK, GTK_ICON_SIZE_LARGE_TOOLBAR));
    gtk_container_child_set (GTK_CONTAINER (data->mainNotebook), box, "tab-expand", TRUE, "tab-fill", TRUE, NULL);
    setup_current_page(GTK_BOX (box), data);



    gtk_widget_show_all (GTK_WIDGET (data->mainWindow));
}
