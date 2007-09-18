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
#include "go-page.h"

#include <glib/gi18n.h>
#include <strings.h>

enum {
    UrlEntry,
    SearchEntry,
    GoUrl,
    GoSearch,
    GoNewPageUrl,
    GoNewPageSearch
};

static const char* search_names[] = {
    N_("Url"),
    N_("Search"),
    N_("Open Url"),
    N_("Search"),
    N_("Open Url in a new page"),
    N_("Search in a new page")
};

/*
 * From the GdkLauncher of WebKit
 */
static gchar* autocorrect_url(const gchar* url)
{
    if (strncmp ("http://", url, 7) != 0 && strncmp ("https://", url, 8) != 0 && strncmp ("file://", url, 7) != 0 && strncmp ("ftp://", url, 6) != 0) {
        GString* string = g_string_new ("http://");
        g_string_append (string, url);
        return g_string_free (string, FALSE);
    }

    return g_strdup (url);
}

static gchar* prepare_search(const gchar* search_text)
{
    GString* string = g_string_new ("http://www.google.com/search?q=");
    g_string_append (string, search_text);
    return g_string_free (string, FALSE);
}

/*
 * Heavily inspired by openmoko-contacts
 */
static gboolean entry_focus_in(GtkEntry* entry, GdkEventFocus* even, gchar* field_name)
{
    if (!strcmp (gtk_entry_get_text (entry), field_name)) {
        gtk_entry_set_text (GTK_ENTRY (entry), "");
        gtk_widget_modify_text (GTK_WIDGET (entry), GTK_STATE_NORMAL, NULL);
    }

    return FALSE;
}

/*
 * Heavily inspired by openmoko-contacts
 */
static gboolean entry_focus_out(GtkEntry* entry, GdkEventFocus* event, gchar* field_name)
{
    if (gtk_entry_get_has_frame (entry) && strlen (gtk_entry_get_text (entry)) == 0) {
        gtk_entry_set_text (entry, field_name);
        gtk_widget_modify_text (GTK_WIDGET (entry), GTK_STATE_NORMAL, &GTK_WIDGET(entry)->style->text[GTK_STATE_INSENSITIVE]);
    }

    return FALSE;
}

static void search_url_entry_changed(GtkEntry* entry, GtkWidget* complementary_entry)
{
    const gchar* text = gtk_entry_get_text (entry);
    gtk_widget_set_sensitive(complementary_entry, !strcmp(text, search_names[SearchEntry]) || !strcmp(text, search_names[UrlEntry]) || strlen(text) == 0);
}

static void go_clicked(GtkButton* btn, struct BrowserData* data)
{
    g_return_if_fail (data->currentPage);

    gchar *url;
    if (strlen(gtk_entry_get_text (data->goSearchEntry)) != 0 && strcmp (gtk_entry_get_text (data->goSearchEntry), _(search_names[SearchEntry])) != 0)
        url = prepare_search (gtk_entry_get_text (data->goSearchEntry));
    else
        url = autocorrect_url (gtk_entry_get_text (data->goUrlEntry));

    webkit_gtk_page_open (data->currentPage->webKitPage, url);
    gtk_notebook_set_current_page (GTK_NOTEBOOK (data->mainNotebook), 0);
}

/*
 * Create a list
 */
static void create_new_page_clicked(GtkButton* btn, struct BrowserData* data)
{
    const gchar* url;
    if (strlen(gtk_entry_get_text (data->goSearchEntry)) != 0 && strcmp (gtk_entry_get_text (data->goSearchEntry), _(search_names[SearchEntry])) != 0)
        url = prepare_search (gtk_entry_get_text (data->goSearchEntry));
    else
        url = autocorrect_url (gtk_entry_get_text (data->goUrlEntry));

    struct BrowserPage* page = g_new(struct BrowserPage, 1);
    page->webKitPage = WEBKIT_GTK_PAGE (webkit_gtk_page_new ());

    data->browserPages = g_list_append (data->browserPages, page);
    webkit_gtk_page_open (page->webKitPage, url);
    set_current_page (page, data);
    g_object_ref (page->webKitPage);

    gtk_notebook_set_current_page (GTK_NOTEBOOK (data->mainNotebook), 0);
}

/*
 * If the "Go"-Page is shown we will do the following:
 *  1.) check if we have a open page and change the sensitivity of the "Open Bar"
 *  2.) make sure to remove the text from the search and set the default text to the buttons
 */
static void switched_notebook_tab(GtkNotebook* notebook, GtkNotebookPage* page, guint page_num, struct BrowserData* data)
{
    if (gtk_notebook_get_nth_page (notebook, page_num) == data->goBox) {
        gtk_entry_set_text (data->goUrlEntry, "");
        gtk_entry_set_text (data->goSearchEntry, "");
        entry_focus_out (data->goUrlEntry, NULL, _(search_names[UrlEntry]));
        entry_focus_out (data->goSearchEntry, NULL, _(search_names[SearchEntry]));
        gtk_button_set_label (GTK_BUTTON(data->goButton), _(search_names[GoUrl]));
        gtk_button_set_label (GTK_BUTTON(data->goButtonNewPage), _(search_names[GoNewPageUrl]));
        gtk_widget_set_sensitive(data->goButton, data->currentPage != 0);
    }
}

void setup_go_page(GtkBox* box, struct BrowserData* data)
{
    data->goBox = GTK_WIDGET (box);
    g_signal_connect (data->mainNotebook, "switch-page", G_CALLBACK(switched_notebook_tab), data);

    data->goUrlEntry = GTK_ENTRY (gtk_entry_new ());
    data->goSearchEntry = GTK_ENTRY (gtk_entry_new ());
    gtk_box_pack_start (box, GTK_WIDGET (data->goUrlEntry), FALSE, TRUE, 0);
    g_signal_connect(data->goUrlEntry, "changed", G_CALLBACK(search_url_entry_changed), data->goSearchEntry);
    g_signal_connect(data->goUrlEntry, "focus-in-event", G_CALLBACK(entry_focus_in), (gpointer)search_names[UrlEntry]);
    g_signal_connect(data->goUrlEntry, "focus-out-event", G_CALLBACK(entry_focus_out), (gpointer)search_names[UrlEntry]); 

    gtk_box_pack_start (box, GTK_WIDGET (data->goSearchEntry), FALSE, TRUE, 0);
    g_signal_connect(data->goSearchEntry, "changed", G_CALLBACK(search_url_entry_changed), data->goUrlEntry);
    g_signal_connect(data->goSearchEntry, "focus-in-event", G_CALLBACK(entry_focus_in), (gpointer)search_names[SearchEntry]);
    g_signal_connect(data->goSearchEntry, "focus-out-event", G_CALLBACK(entry_focus_out), (gpointer)search_names[SearchEntry]); 

    data->goButton = gtk_button_new_with_label (search_names[GoUrl]);
    gtk_box_pack_start (box, GTK_WIDGET (data->goButton), FALSE, TRUE, 0);
    g_signal_connect(data->goButton, "clicked", G_CALLBACK(go_clicked), data);

    data->goButtonNewPage = gtk_button_new_with_label (search_names[GoNewPageUrl]);
    gtk_box_pack_start (box, GTK_WIDGET (data->goButtonNewPage), FALSE, TRUE, 0);
    g_signal_connect(data->goButtonNewPage, "clicked", G_CALLBACK(create_new_page_clicked), data);
}
