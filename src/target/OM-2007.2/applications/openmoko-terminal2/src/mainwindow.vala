/*
 * mainwindow.vala
 *
 * Authored by Michael 'Mickey' Lauer <mickey@vanille-media.de>
 * Copyright (C) 2007-2008 OpenMoko, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 */

using GLib;
using Gtk;

// how to define a const here?

public class OpenMokoTerminal2.MainWindow : Window
{
    private VBox _vbox;
    private Toolbar _toolbar;
    private Notebook _notebook;

    private ToolButton _btn_new;
    private ToolButton _btn_delete;
    private ToolButton _btn_zoom_in;
    private ToolButton _btn_zoom_out;

    public MainWindow()
    {
        title = "Terminal";
    }

    construct
    {
        destroy += Gtk.main_quit;
        _vbox = new Gtk.VBox( false, 0 );
        add( _vbox );
        setup_toolbar();
        setup_notebook();
        update_toolbar();
    }

    public void setup_toolbar()
    {
        _toolbar = new Gtk.Toolbar();
        _vbox.add( _toolbar );

        _btn_new = new Gtk.ToolButton.from_stock( STOCK_NEW );
        _btn_new.clicked += on_new_clicked;
        _toolbar.insert( _btn_new, 0 );

        _btn_delete = new Gtk.ToolButton.from_stock( STOCK_DELETE );
        _btn_delete.clicked += on_delete_clicked;
        _toolbar.insert( _btn_delete, 1 );

        _toolbar.insert( new Gtk.SeparatorToolItem(), 2 );

        _btn_zoom_in = new Gtk.ToolButton.from_stock( STOCK_ZOOM_IN );
        _btn_zoom_in.clicked += on_zoom_in_clicked;
        _toolbar.insert( _btn_zoom_in, 3 );

        _btn_zoom_out = new Gtk.ToolButton.from_stock( STOCK_ZOOM_OUT );
        _btn_zoom_out.clicked += on_zoom_out_clicked;
        _toolbar.insert( _btn_zoom_out, 4 );
    }

    public void setup_notebook()
    {
        _notebook = new Gtk.Notebook();
        _notebook.set_tab_pos( PositionType.BOTTOM );
        _vbox.add( _notebook );

        var terminal = new OpenMokoTerminal2.MokoTerminal();
        _notebook.append_page( terminal, Image.from_stock( STOCK_INDEX, IconSize.LARGE_TOOLBAR ) );
        _notebook.child_set (terminal, "tab-expand", true, null );
    }

    private void on_new_clicked( Gtk.ToolButton b )
    {
        stdout.printf( "on_new_clicked\n" );
        var terminal = new OpenMokoTerminal2.MokoTerminal();
        _notebook.append_page( terminal, Image.from_stock( STOCK_INDEX, IconSize.LARGE_TOOLBAR ) );
        _notebook.child_set (terminal, "tab-expand", true, null );
        _notebook.show_all();
        update_toolbar();
    }

    private void on_delete_clicked( Gtk.ToolButton b )
    {
        stdout.printf( "on_delete_clicked\n" );
        var page = _notebook.get_nth_page( _notebook.get_current_page() );
        page.destroy();
        update_toolbar();
    }

    private void on_zoom_in_clicked( Gtk.ToolButton b )
    {
        stdout.printf( "on_zoom_in_clicked\n" );
        OpenMokoTerminal2.MokoTerminal terminal = _notebook.get_nth_page( _notebook.get_current_page() );
        terminal.zoom_in();
    }

    private void on_zoom_out_clicked( Gtk.ToolButton b )
    {
        stdout.printf( "on_zoom_out_clicked\n" );
        OpenMokoTerminal2.MokoTerminal terminal = _notebook.get_nth_page( _notebook.get_current_page() );
        terminal.zoom_out();
    }

    public void update_toolbar()
    {
        _btn_delete.set_sensitive( _notebook.get_n_pages() > 1 );
    }

    public void run()
    {
        show_all();
        Gtk.main();
    }

    static int main (string[] args) {
        Gtk.init(ref args);

        var window = new MainWindow();
        window.run();
        return 0;
    }

}
