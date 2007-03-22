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

#include "application-data.h"
#include "callbacks.h"
#include <assert.h>

#include <libmokoui/moko-details-window.h>

/*
 * use gettext...
 */
#define _(x) (x)
#define ASSERT_X(x, error) assert(x)

/*
 * setup the toolbar
 */
static void setup_toolbar( struct RSSReaderData *data ) {
    GtkButton *a;
    GtkWidget *anImage;
    data->box = MOKO_TOOL_BOX(moko_tool_box_new_with_search());
    gtk_widget_grab_focus( GTK_WIDGET(data->box) );
    g_signal_connect( G_OBJECT(data->box), "searchbox_visible",   G_CALLBACK(cb_searchbox_visible), data );
    g_signal_connect( G_OBJECT(data->box), "searchbox_invisible", G_CALLBACK(cb_searchbox_invisible), data );


    a = GTK_BUTTON(moko_tool_box_add_action_button( MOKO_TOOL_BOX(data->box) ));
    anImage = gtk_image_new_from_file( PKGDATADIR "/rssreader_refresh_all.png" );
    moko_pixmap_button_set_center_image( MOKO_PIXMAP_BUTTON(a), anImage );
    g_signal_connect( G_OBJECT(a), "clicked", G_CALLBACK(cb_refresh_all_button_clicked), data );

    a = GTK_BUTTON(moko_tool_box_add_action_button( MOKO_TOOL_BOX(data->box) ));
    anImage = gtk_image_new_from_file( PKGDATADIR "/rssreader_subscribe.png" );
    moko_pixmap_button_set_center_image( MOKO_PIXMAP_BUTTON(a), anImage );
    g_signal_connect( G_OBJECT(a), "clicked", G_CALLBACK(cb_subscribe_button_clicked), data );

    a = GTK_BUTTON(moko_tool_box_add_action_button( MOKO_TOOL_BOX(data->box)) );
    gtk_button_set_label( GTK_BUTTON(a), _("Up for rent") );
    a = GTK_BUTTON(moko_tool_box_add_action_button( MOKO_TOOL_BOX(data->box)) );
    gtk_button_set_label( GTK_BUTTON(a), _("Buy more Mate") );

    moko_paned_window_add_toolbox( MOKO_PANED_WINDOW(data->window), MOKO_TOOL_BOX(data->box) );
}

static void create_navigaton_area( struct RSSReaderData *data ) {
    data->feed_data = gtk_list_store_new( RSS_READER_NUM_COLS,
                                          G_TYPE_STRING /* Author    */,
                                          G_TYPE_STRING /* Subject   */,
                                          G_TYPE_STRING /* Date      */,
                                          G_TYPE_STRING /* Link      */,
                                          G_TYPE_STRING /* Text      */,
                                          G_TYPE_INT    /* Text_Type */,
                                          G_TYPE_STRING /* Category  */,
                                          G_TYPE_STRING /* Source    */ );
    data->treeView = MOKO_TREE_VIEW(moko_tree_view_new_with_model(GTK_TREE_MODEL(data->feed_data)));
    moko_paned_window_set_upper_pane( MOKO_PANED_WINDOW(data->window), GTK_WIDGET(moko_tree_view_put_into_scrolled_window(data->treeView)) );

    /*
     * Only show the SUBJECT and DATE header
     */
    GtkCellRenderer *ren;
    GtkTreeViewColumn *column;
    ren = GTK_CELL_RENDERER(gtk_cell_renderer_text_new());
    column = GTK_TREE_VIEW_COLUMN(gtk_tree_view_column_new_with_attributes( _("Subject"), ren, "text", RSS_READER_COLUMN_SUBJECT, NULL));
    gtk_tree_view_column_set_expand( column, TRUE );
    gtk_tree_view_column_set_sizing( column, GTK_TREE_VIEW_COLUMN_FIXED );
    moko_tree_view_append_column( MOKO_TREE_VIEW(data->treeView), column );

    ren = GTK_CELL_RENDERER(gtk_cell_renderer_text_new());
    column = GTK_TREE_VIEW_COLUMN(gtk_tree_view_column_new_with_attributes( _("Date"), ren, "text", RSS_READER_COLUMN_DATE, NULL));
    gtk_tree_view_column_set_expand( column, TRUE );
    gtk_tree_view_column_set_sizing( column, GTK_TREE_VIEW_COLUMN_FIXED );
    moko_tree_view_append_column( MOKO_TREE_VIEW(data->treeView), column );

    /*
     * auto completion and selection updates
     */
    GtkTreeSelection *selection = GTK_TREE_SELECTION(gtk_tree_view_get_selection( GTK_TREE_VIEW(data->treeView) ));
    g_signal_connect( G_OBJECT(selection), "changed", G_CALLBACK(cb_treeview_selection_changed), data );

    GtkWidget *search_entry = GTK_WIDGET(moko_tool_box_get_entry(MOKO_TOOL_BOX(data->box)));
    g_signal_connect( G_OBJECT(data->treeView), "key_press_event", G_CALLBACK(cb_treeview_keypress_event), data );
    g_signal_connect( G_OBJECT(search_entry),   "changed", G_CALLBACK(cb_search_entry_changed), data );

}

static void create_details_area( struct RSSReaderData* data ) {
    data->tagTable   = GTK_TEXT_TAG_TABLE(gtk_text_tag_table_new());
    data->textBuffer = GTK_TEXT_BUFFER(gtk_text_buffer_new(data->tagTable));
    data->textView   = GTK_TEXT_VIEW(gtk_text_view_new_with_buffer(GTK_TEXT_BUFFER(data->textBuffer)));

    GValue value = { 0, };
    g_value_init( &value, G_TYPE_BOOLEAN );
    g_value_set_boolean( &value, FALSE );
    g_object_set_property( G_OBJECT(data->textView), "editable",       &value );
    g_object_set_property( G_OBJECT(data->textView), "cursor-visible", &value );
    gtk_text_view_set_wrap_mode( data->textView, GTK_WRAP_WORD_CHAR );

    GtkWidget *scrollWindow = GTK_WIDGET(moko_details_window_new());
    gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrollWindow), GTK_WIDGET (data->textView));
    moko_paned_window_set_lower_pane( MOKO_PANED_WINDOW(data->window), GTK_WIDGET(moko_details_window_put_in_box(MOKO_DETAILS_WINDOW(scrollWindow))) );
}

/*
 * create the mainwindow
 */
static void setup_ui( struct RSSReaderData* data ) {
    data->window = MOKO_PANED_WINDOW(moko_paned_window_new());
    g_signal_connect( G_OBJECT(data->window), "delete_event", G_CALLBACK( gtk_main_quit ), NULL );

    /*
     * menu
     */
    data->menu = GTK_MENU(gtk_menu_new());
    GtkMenuItem *closeitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( _("Close")));
    g_signal_connect( G_OBJECT(closeitem), "activate", G_CALLBACK(gtk_main_quit), NULL );
    gtk_menu_shell_append( GTK_MENU_SHELL(data->menu), GTK_WIDGET(closeitem) );
    moko_paned_window_set_application_menu( MOKO_PANED_WINDOW(data->window), GTK_MENU(data->menu) );

    /*
     * filter
     */
    data->filter = GTK_MENU(gtk_menu_new());
    moko_paned_window_set_filter_menu( MOKO_PANED_WINDOW(data->window), GTK_MENU(data->filter) );
    data->menubox = MOKO_MENU_BOX(moko_paned_window_get_menubox( MOKO_PANED_WINDOW(data->window) ) );
    g_signal_connect( G_OBJECT(data->menubox), "filter_changed", G_CALLBACK(cb_filter_changed), data );


    /*
     * tool bar
     */
    setup_toolbar( data );
    create_navigaton_area( data );
    create_details_area( data );
}

int main( int argc, char** argv )
{
    /*
     * boiler plate code
     */
    g_debug( "openmoko-rssreader starting up" );

    /*
     * initialize threads for fetching the RSS in the background
     */
    g_thread_init( NULL );
    gdk_threads_init();
    gdk_threads_enter();
    gtk_init( &argc, &argv );

    struct RSSReaderData *data = g_new0( struct RSSReaderData, 1 );


    data->app = MOKO_APPLICATION( moko_application_get_instance() );
    g_set_application_name( _("FeedReader") );

    setup_ui( data );

    /*
     * load data
     */
    refresh_categories( data );
    moko_menu_box_set_active_filter( data->menubox, _("All") );

    gtk_widget_show_all( GTK_WIDGET(data->window) );
    gtk_main();
    gdk_threads_leave();

    return 0;
}


