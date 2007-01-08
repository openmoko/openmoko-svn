/*
 *  Stylus Demo -- OpenMoko Demo Application
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006 First International Computer Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#include <libmokoui/moko-application.h>
#include <libmokoui/moko-details-window.h>
#include <libmokoui/moko-dialog-window.h>
#include <libmokoui/moko-paned-window.h>
#include <libmokoui/moko-tool-box.h>
#include <libmokoui/moko-navigation-list.h>

#include <gtk/gtkactiongroup.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtkcheckbutton.h>
#include <gtk/gtkcheckmenuitem.h>
#include <gtk/gtkcombobox.h>
#include <gtk/gtkframe.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkmessagedialog.h>
#include <gtk/gtkradiobutton.h>
#include <gtk/gtkspinbutton.h>
#include <gtk/gtktable.h>
#include <gtk/gtktreeview.h>


static GtkTreeModel *
create_model (void)
{

    GtkListStore *store;
    GtkTreeIter iter;
    gint i;
    gchar *stuff[19][2] = {
        { "Sean", "1111111111" },
        { "Mickey", "22222222222" },
        { "Harald", "33333333333" },
        { "Tom", "22222222222" },
        { "Steven", "02134567890" },
        { "Tony", "02178789999" },
        { "Gordon", "02122222222" },
        { "Gordon", "02122222222" },
        { "Gordon", "02122222222" },
        { "Gordon", "02122222222" },
        { "Gordon", "02122222222" },
        { "Gordon", "02122222222" },
        { "Gordon", "02122222222" },
        { "Gordon", "02122222222" },
        { "Gordon", "02122222222" },
        { "Gordon", "02122222222" },
        { "Gordon", "02122222222" },
        { "Gordon", "02122222222" },
        { "jeff", "02133333333" } };

        /* create list store */

        store = gtk_list_store_new (2,
                                    G_TYPE_STRING,
                                    G_TYPE_STRING);
        /* add data to the list store */
        for (i = 0; i < 19; i++)
        {
            gtk_list_store_append (store, &iter);
            gtk_list_store_set (store, &iter,
                                0, stuff[i][0],
                                1, stuff[i][1],
                                -1);
        }

        return GTK_TREE_MODEL (store);
}


void clist_insert(MokoTreeView *clist)
{
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Name",
            renderer,
            "text",
            0,
            NULL);


    gtk_tree_view_column_set_sort_column_id(column, 0);
    gtk_tree_view_append_column(clist, column);
    gtk_tree_view_column_set_min_width(column, 142);

    renderer = gtk_cell_renderer_text_new();

    column = gtk_tree_view_column_new_with_attributes("Cell Phone",
            renderer,
            "text",
            1,
            NULL);

    gtk_tree_view_column_set_sort_column_id(column, 1);
    gtk_tree_view_append_column(clist, column);
    gtk_tree_view_column_set_min_width(column, 156);
}

void cb_searchbox_visible(MokoToolBox* toolbox, gpointer user_data)
{
    g_debug( "openmoko-stylus-demo: searchbox now visible" );
    // populate the entry completion here and/or connect signals to entry
}

void cb_searchbox_invisible(MokoToolBox* toolbox, gpointer user_data)
{
    g_debug( "openmoko-stylus-demo: searchbox now invisible" );
    // free resources and/or disconnect signals
}

void cb_filter_changed(GtkMenu* menu, gchar* text, gpointer user_data )
{
    g_debug( "openmoko-stylus-demo: filter changed to '%s'", text );
}

void cb_button1_clicked(GtkButton *button, gpointer user_data)
{
    g_debug( "openmoko-stylus-demo: button1 clicked" );

    GtkMessageDialog* dialog = gtk_message_dialog_new( moko_application_get_main_window( moko_application_get_instance() ),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_ERROR,
                                  GTK_BUTTONS_YES_NO,
                                  "Simple question dialog" );
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

void cb_button2_clicked(GtkButton *button, gpointer user_data)
{
    g_debug( "openmoko-stylus-demo: button2 clicked" );

    /* prepare contents of dialog. Yes, eventually we want to use libglade for the dialogs */

    GtkVBox* controls = gtk_vbox_new( FALSE, 0 );
    GtkCheckButton* check1 = gtk_check_button_new_with_label( "Use GtkCheckButton for on/off options" );
    GtkCheckButton* check2 = gtk_check_button_new_with_label( "Be concise, but don't abbreviate" );
    GtkCheckButton* check3 = gtk_check_button_new_with_label( "Less Options are less confusing = good!" );

    gtk_box_pack_start_defaults( GTK_BOX(controls), GTK_WIDGET(check1) );
    gtk_box_pack_start_defaults( GTK_BOX(controls), GTK_WIDGET(check2) );
    gtk_box_pack_start_defaults( GTK_BOX(controls), GTK_WIDGET(check3) );

    GtkFrame* frame = gtk_frame_new( "Choose your favourite gift" );
    GtkVBox* framevbox = gtk_vbox_new( TRUE, 0 );
    g_object_set( G_OBJECT(frame), "border-width", 10, NULL );
    gtk_container_add( GTK_CONTAINER(frame), GTK_WIDGET(framevbox) );

    GSList* group = NULL;
    GtkRadioButton* radio1 = gtk_radio_button_new_with_label( group, "Use Radio Buttons only for few(!) options" );
    group = gtk_radio_button_get_group( radio1 );
    GtkRadioButton* radio2 = gtk_radio_button_new_with_label( group, "Use GtkComboBoxes for many options" );
    group = gtk_radio_button_get_group( radio2 );
    GtkRadioButton* radio3 = gtk_radio_button_new_with_label( group, "Always Group Radio Buttons in a Frame" );

    gtk_box_pack_start_defaults( GTK_BOX(framevbox), GTK_WIDGET(radio1) );
    gtk_box_pack_start_defaults( GTK_BOX(framevbox), GTK_WIDGET(radio2) );
    gtk_box_pack_start_defaults( GTK_BOX(framevbox), GTK_WIDGET(radio3) );

    gtk_box_pack_start_defaults( GTK_BOX(controls), GTK_WIDGET(frame) );

    GtkComboBox* combo = gtk_combo_box_new_text();
    gtk_combo_box_append_text( combo, "Use" );
    gtk_combo_box_append_text( combo, "the" );
    gtk_combo_box_append_text( combo, "GtkComboBox" );
    gtk_combo_box_append_text( combo, "widget" );
    gtk_combo_box_append_text( combo, "if you have" );
    gtk_combo_box_append_text( combo, "a lot of" );
    gtk_combo_box_append_text( combo, "different options" );
    gtk_combo_box_append_text( combo, "for one setting" );
    gtk_box_pack_start_defaults( GTK_BOX(controls), GTK_WIDGET(combo) );

    GtkHBox* hb1 = gtk_hbox_new( FALSE, 0 );
    GtkLabel* l1 = gtk_label_new( "Text Entry: " );
    gtk_misc_set_alignment( GTK_MISC(l1), 0, 0.5 );
    gtk_misc_set_padding( GTK_MISC(l1), 8, 8 );
    GtkEntry* e1 = gtk_entry_new();
    g_object_set( G_OBJECT(e1), "has-frame", FALSE, NULL );
    gtk_box_pack_start( GTK_BOX(hb1), GTK_WIDGET(l1), FALSE, FALSE, 0 );
    gtk_box_pack_start( GTK_BOX(hb1), GTK_WIDGET(e1), TRUE, TRUE, 10 );
    gtk_box_pack_start_defaults( GTK_BOX(controls), GTK_WIDGET(hb1) );

    GtkHBox* hb2 = gtk_hbox_new( FALSE, 0 );
    GtkLabel* l2 = gtk_label_new( "SpinBox Entry: " );
    gtk_misc_set_alignment( GTK_MISC(l2), 0, 0.5 );
    gtk_misc_set_padding( GTK_MISC(l2), 8, 8 );
    GtkSpinButton* e2 = gtk_spin_button_new_with_range( -20, 20, 5 );
    g_object_set( G_OBJECT(e2), "has-frame", FALSE, NULL );
    gtk_box_pack_start( GTK_BOX(hb2), GTK_WIDGET(l2), FALSE, FALSE, 0 );
    gtk_box_pack_start( GTK_BOX(hb2), GTK_WIDGET(e2), TRUE, TRUE, 10 );
    gtk_box_pack_start_defaults( GTK_BOX(controls), GTK_WIDGET(hb2) );

    gtk_widget_show_all( GTK_WIDGET(controls) );

    /* run dialog */
    MokoDialogWindow* dialog = moko_application_execute_dialog( moko_application_get_instance(),
            "Example Full Screen Dialog Window",
            GTK_WIDGET(controls) );
    /* process results */

    gtk_widget_destroy( GTK_WIDGET(dialog) );
}

void cb_button3_clicked(GtkButton *button, gpointer user_data)
{
    g_debug( "openmoko-stylus-demo: button3 clicked" );
}

void cb_button4_clicked(GtkButton *button, gpointer user_data)
{
    g_debug( "openmoko-stylus-demo: button4 clicked" );

    GtkMessageDialog* dialog = gtk_message_dialog_new( moko_application_get_main_window( moko_application_get_instance() ),
                                  GTK_DIALOG_DESTROY_WITH_PARENT,
                                  GTK_MESSAGE_QUESTION,
                                  GTK_BUTTONS_CLOSE,
                                  "Simple confirmation dialog" );
    gtk_dialog_run (GTK_DIALOG (dialog));
    gtk_widget_destroy (dialog);
}

static gboolean searchmode = TRUE;

int main( int argc, char** argv )
{
    g_debug( "openmoko-stylus-demo starting up" );
    /* Initialize GTK+ */
    gtk_init( &argc, &argv );

    if ( argc > 1 && strcmp( argv[1], "-no-search" ) == 0)
    {
        g_debug( "disabling search mode" );
        searchmode = FALSE;
    }

    /* application object */
    MokoApplication* app = MOKO_APPLICATION(moko_application_get_instance());
    g_set_application_name( "Stylus Demo" );

    /* main window */
    MokoPanedWindow* window = MOKO_PANED_WINDOW(moko_paned_window_new());

    /* application menu */
    GtkMenu* appmenu = GTK_MENU(gtk_menu_new());
    GtkMenuItem* item = gtk_menu_item_new_with_label( "Some" );
    gtk_menu_shell_append( GTK_MENU_SHELL(appmenu), item );
    item = gtk_menu_item_new_with_label( "Sample" );
    gtk_menu_shell_append( GTK_MENU_SHELL(appmenu), item );
    item = gtk_menu_item_new_with_label( "Menu" );
    gtk_menu_shell_append( GTK_MENU_SHELL(appmenu), item );
    item = gtk_menu_item_new_with_label( "Entries" );
    gtk_menu_shell_append( GTK_MENU_SHELL(appmenu), item );
    item = gtk_separator_menu_item_new();
    gtk_menu_shell_append( GTK_MENU_SHELL(appmenu), item );
    GtkMenuItem* closeitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Close" ));
    g_signal_connect( G_OBJECT(closeitem), "activate", G_CALLBACK(gtk_main_quit), NULL );
    gtk_menu_shell_append( appmenu, closeitem );
    moko_paned_window_set_application_menu( window, appmenu );

    /* filter menu */
    GtkMenu* filtmenu = GTK_MENU(gtk_menu_new());
    GtkMenuItem* item1 = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Some" ));
    gtk_menu_shell_append( GTK_MENU_SHELL(filtmenu), item1 );
    GtkMenuItem* item2 = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Sample" ));
    gtk_menu_shell_append( GTK_MENU_SHELL(filtmenu), item2 );
    GtkMenuItem* item3 = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Filter" ));
    gtk_menu_shell_append( GTK_MENU_SHELL(filtmenu), item3 );
    GtkMenuItem* item4 = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Entries" ));
    gtk_menu_shell_append( GTK_MENU_SHELL(filtmenu), item4 );
    moko_paned_window_set_filter_menu( window, filtmenu );
    MokoMenuBox* menubox = moko_paned_window_get_menubox( window );
    g_signal_connect( G_OBJECT(menubox), "filter_changed", G_CALLBACK(cb_filter_changed), NULL );

    /* connect close event */
    g_signal_connect( G_OBJECT(window), "delete_event", G_CALLBACK(gtk_main_quit), NULL );

    /* navigation area */
    //GtkLabel* navigation = gtk_label_new( "Add your widget for navigating\nthrough appplication specific\ndata here" );

    MokoTreeView *moko_treeview;
    GtkTreeModel *model = create_model ();
    MokoNavigationList *moko_navigation_list;

    moko_navigation_list = moko_navigation_list_new();
    moko_treeview = moko_navigation_list_get_tree_view (moko_navigation_list);
    gtk_tree_view_set_model (moko_treeview, GTK_TREE_MODEL (model) );
    clist_insert(moko_treeview);

    //moko_paned_window_set_upper_pane( window, GTK_WIDGET(navigation) );
    moko_paned_window_set_upper_pane( window, GTK_WIDGET(moko_navigation_list) );

    GtkButton* button1;
    GtkButton* button2;
    GtkButton* button3;
    GtkButton* button4;

    /* tool bar */
    MokoToolBox* toolbox;
    if (!searchmode)
    {
        toolbox = MOKO_TOOL_BOX(moko_tool_box_new());
    } else {
        toolbox = MOKO_TOOL_BOX(moko_tool_box_new_with_search());
    }

    g_signal_connect( G_OBJECT(toolbox), "searchbox_visible", G_CALLBACK(cb_searchbox_visible), NULL );
    g_signal_connect( G_OBJECT(toolbox), "searchbox_invisible", G_CALLBACK(cb_searchbox_invisible), NULL );

    button1 = moko_tool_box_add_action_button( toolbox );
    //gtk_button_set_label( button1, "Action 1" );
    //moko_pixmap_button_set_action_btn_upper_stock (button1, "openmoko-action-button-message-icon");
    //moko_pixmap_button_set_action_btn_lower_label (button1, "Edit");
    moko_pixmap_button_set_center_stock (button1, "openmoko-action-button-group-icon");
    button2 = moko_tool_box_add_action_button( toolbox );
    //gtk_button_set_label( button2, "Dialog" );
    moko_pixmap_button_set_center_stock (button2, "openmoko-action-button-history-icon");
    button3 = moko_tool_box_add_action_button( toolbox );
    //gtk_button_set_label( button3, "ActMenu" );
    moko_pixmap_button_set_center_stock (button3, "openmoko-action-button-edit-icon");
    button4 = moko_tool_box_add_action_button( toolbox );
    //gtk_button_set_label( button4, "Action 4" );
    moko_pixmap_button_set_center_stock (button4, "openmoko-action-button-view-icon");
    moko_paned_window_add_toolbox( window, toolbox );

    g_signal_connect( G_OBJECT(button1), "clicked", G_CALLBACK(cb_button1_clicked), NULL );
    g_signal_connect( G_OBJECT(button2), "clicked", G_CALLBACK(cb_button2_clicked), NULL );
    g_signal_connect( G_OBJECT(button3), "clicked", G_CALLBACK(cb_button3_clicked), NULL );
    g_signal_connect( G_OBJECT(button4), "clicked", G_CALLBACK(cb_button4_clicked), NULL );

    GtkMenu* actionmenu = GTK_MENU(gtk_menu_new());
    GtkMenuItem* fooitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Foo" ));
    GtkMenuItem* baritem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( "Bar" ));
    gtk_widget_show( GTK_WIDGET(fooitem) );
    gtk_widget_show( GTK_WIDGET(baritem) );
    gtk_menu_shell_append( actionmenu, fooitem );
    gtk_menu_shell_append( actionmenu, baritem );
    moko_pixmap_button_set_menu( MOKO_PIXMAP_BUTTON(button3), actionmenu );
    gtk_widget_show_all( actionmenu );

    /* details area */
    GtkLabel* details = gtk_label_new( "\n \n \nAdd your widget for showing\n \ndetails for the selected\n"
            "\ndata entry here\n \n \n \n \n \n \n \nThis particular label\n \nis very long\n"
            "\nto make the fullscreen\n \ntrigger more interesting\n \n \n" );

    MokoDetailsWindow* detailswindow = moko_details_window_new();
    gtk_scrolled_window_add_with_viewport( GTK_SCROLLED_WINDOW(detailswindow), GTK_WIDGET(details) );

    moko_paned_window_set_lower_pane( window, GTK_WIDGET(moko_details_window_put_in_box(detailswindow) ) );

    /* show everything and run main loop */
    gtk_widget_show_all( GTK_WIDGET(window) );
    g_debug( "openmoko-stylus-demo entering main loop" );
    gtk_main();
    g_debug( "openmoko-stylus-demo left main loop" );

    return 0;
}
