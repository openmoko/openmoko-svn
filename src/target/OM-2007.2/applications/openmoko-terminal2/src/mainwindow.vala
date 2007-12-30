/*
 * mainwindow.vala
 *
 * Authored by Michael 'Mickey' Lauer <mickey@vanille-media.de>
 * Copyright (C) 2007 OpenMoko, Inc.
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
    public MainWindow()
    {
        title = "Terminal";
    }

    construct
    {
        destroy += Gtk.main_quit;
        var notebook = new Gtk.Notebook();
        notebook.set_tab_pos( PositionType.BOTTOM );
        add( notebook );

        for ( int i = 0; i < 3; ++i )
        {
            var terminal = new OpenMokoTerminal2.MokoTerminal();
            notebook.append_page( terminal, Image.from_stock( STOCK_INDEX, IconSize.LARGE_TOOLBAR ) );
            notebook.child_set (terminal, "tab-expand", true, null );
        }
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
