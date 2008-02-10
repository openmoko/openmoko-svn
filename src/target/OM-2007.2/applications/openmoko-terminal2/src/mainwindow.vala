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
//using Gdk;
using Gtk;

public class OpenMokoTerminal2.MainWindow : Window
{
    private VBox vbox;
    private Toolbar toolbar;
    private Notebook notebook;

    private ToolButton btn_new;
    private ToolButton btn_delete;
    private ToolButton btn_zoom_in;
    private ToolButton btn_zoom_out;
    private ToolButton btn_paste;

    private static string initial_command;
    private static string[] initial_command_line;

    public MainWindow()
    {
        title = "Terminal";
    }

    construct
    {
        destroy += Gtk.main_quit;
        vbox = new Gtk.VBox( false, 0 );
        add( vbox );
        setup_toolbar();
        setup_notebook();
        update_toolbar();
        idle_add( on_idle, this );
        idle_add( on_idle_first_command, this );
        //window.add_filter( on_gdk_filter, this );
    }

    public void setup_command( string command )
    {
        initial_command = command + "\n";
    }

    public void setup_toolbar()
    {
        toolbar = new Gtk.Toolbar();
        vbox.pack_start( toolbar, false, false, 0 );

        btn_new = new Gtk.ToolButton.from_stock( STOCK_NEW );
        btn_new.clicked += on_new_clicked;
        toolbar.insert( btn_new, 0 );

        btn_delete = new Gtk.ToolButton.from_stock( STOCK_DELETE );
        btn_delete.clicked += on_delete_clicked;
        toolbar.insert( btn_delete, 1 );

        toolbar.insert( new Gtk.SeparatorToolItem(), 2 );

        btn_zoom_in = new Gtk.ToolButton.from_stock( STOCK_ZOOM_IN );
        btn_zoom_in.clicked += on_zoom_in_clicked;
        toolbar.insert( btn_zoom_in, 3 );

        btn_zoom_out = new Gtk.ToolButton.from_stock( STOCK_ZOOM_OUT );
        btn_zoom_out.clicked += on_zoom_out_clicked;
        toolbar.insert( btn_zoom_out, 4 );

        toolbar.insert( new Gtk.SeparatorToolItem(), 5 );

        btn_paste = new Gtk.ToolButton.from_stock( STOCK_PASTE );
        btn_paste.clicked += on_paste_clicked;
        toolbar.insert( btn_paste, 6 );
    }

    public void setup_notebook()
    {
        notebook = new Gtk.Notebook();
        notebook.set_tab_pos( PositionType.BOTTOM );
        vbox.pack_start( notebook, true, true, 0 );

        var terminal = new OpenMokoTerminal2.MokoTerminal();
        notebook.append_page( terminal, Image.from_stock( STOCK_INDEX, IconSize.LARGE_TOOLBAR ) );
        notebook.child_set (terminal, "tab-expand", true, null );
    }

    /*
    [InstanceLast()]
    private Gdk.FilterReturn on_gdk_filter( Gdk.Event e, pointer xevent )
    {
        //stdout.printf( "gdk filter, event %d\n", e.type );
        if ( e.type == Gdk.EventType.PROPERTY_NOTIFY )
        {
            stdout.printf( "gdk filter, property notify event for atom %d, state %d\n", ((Gdk.EventProperty)e).atom, ((Gdk.EventProperty)e).state );
        }
        return Gdk.FilterReturn.CONTINUE;
    }
    */

    private bool on_idle()
    {
        stdout.printf( "on_idle\n" );
        notebook.switch_page += (o, page, num) => {
            btn_delete.set_sensitive( notebook.get_n_pages() > 1 );
            OpenMokoTerminal2.MokoTerminal terminal = notebook.get_nth_page( (int)num ); btn_zoom_in.set_sensitive( terminal.get_font_size() < 10 );
            btn_zoom_out.set_sensitive( terminal.get_font_size() > 1 );
        };
        notebook.page_removed += (o, page, num) => {
            stdout.printf( "on_page_removed\n");
            if ( notebook.get_n_pages() == 0 )
                Gtk.main_quit();
            else
                update_toolbar();
        };
        return false;
    }

    private bool on_idle_first_command()
    {
        stdout.printf( "on_idle_first_command\n" );
        OpenMokoTerminal2.MokoTerminal terminal = notebook.get_nth_page( 0 );
        if ( initial_command != null )
            terminal.paste_command( initial_command );
        return false;
    }

    private void on_new_clicked( Gtk.ToolButton b )
    {
        stdout.printf( "on_new_clicked\n" );
        var terminal = new OpenMokoTerminal2.MokoTerminal();
        notebook.append_page( terminal, Image.from_stock( STOCK_INDEX, IconSize.LARGE_TOOLBAR ) );
        notebook.child_set (terminal, "tab-expand", true, null );
        notebook.show_all();
        update_toolbar();
    }

    private void on_delete_clicked( Gtk.ToolButton b )
    {
        stdout.printf( "on_delete_clicked\n" );
        var page = notebook.get_nth_page( notebook.get_current_page() );
        page.destroy();
        // update_toolbar will be called through the page-removed signal handler
    }

    private void on_zoom_in_clicked( Gtk.ToolButton b )
    {
        stdout.printf( "on_zoom_in_clicked\n" );
        OpenMokoTerminal2.MokoTerminal terminal = notebook.get_nth_page( notebook.get_current_page() );
        terminal.zoom_in();
        update_toolbar();
    }

    private void on_zoom_out_clicked( Gtk.ToolButton b )
    {
        stdout.printf( "on_zoom_out_clicked\n" );
        OpenMokoTerminal2.MokoTerminal terminal = notebook.get_nth_page( notebook.get_current_page() );
        terminal.zoom_out();
        update_toolbar();
    }

    private void on_paste_clicked( Gtk.ToolButton b )
    {
        stdout.printf( "on_paste_clicked\n" );
        OpenMokoTerminal2.MokoTerminal terminal = notebook.get_nth_page( notebook.get_current_page() );
        terminal.paste();
        update_toolbar();
    }

    public void update_toolbar()
    {
        stdout.printf( "update_toolbar\n" );
        if ( null == notebook )
        {
            stdout.printf( "notebook no longer present\n" );
            return;
        }
        btn_delete.set_sensitive( notebook.get_n_pages() > 1 );
        OpenMokoTerminal2.MokoTerminal terminal = notebook.get_nth_page( notebook.get_current_page() );
        stdout.printf( "current font size for terminal is %d\n", terminal.get_font_size() );
        btn_zoom_in.set_sensitive( terminal.get_font_size() < 10 );
        btn_zoom_out.set_sensitive( terminal.get_font_size() > 1 );
    }

    public void run()
    {
        show_all();
        Gtk.main();
    }

    const OptionEntry[] options = {
        { "command", 'e', 0, OptionArg.STRING, out initial_command, "Execute COMMAND inside the terminal.", "COMMAND" },
        { "", 'x', 0, OptionArg.STRING_ARRAY, out initial_command_line, "Execute remainder of command line inside the terminal.", "COMMANDS" },
        { null }
    };

    static int main (string[] args) {
        try {
            Gtk.init_with_args(ref args.length, ref args, " - a lightweight terminal for the OpenMoko environment", options, null);
        } catch (Error e)
        {
            stderr.printf("Error: %s\n", e.message);
            return 1; 
        }

        var window = new MainWindow();
        if ( initial_command != null )
        {
            window.setup_command( initial_command );
        }
        else if ( initial_command_line != null )
        {
            initial_command = string.joinv( " ", initial_command_line );
            window.setup_command( initial_command );
        }

        window.run();

        return 0;
    }

}
