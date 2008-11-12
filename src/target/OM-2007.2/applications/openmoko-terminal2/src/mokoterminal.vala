/*
 * mokoterminal.vala
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
using Gdk;
using Gtk;
using Vte;

public class OpenMokoTerminal2.MokoTerminal : HBox
{
    private string fontname;
    private uint fontsize;
    private Scrollbar scrollbar;
    private Terminal terminal;

    construct {
        stdout.printf( "moko-terminal constructed\n" );

        // may read from gconf at some point?
        fontname = "LiberationMono";
        fontsize = 5;

        terminal = new Vte.Terminal();
        // auto-exit may become a preference at some point?
        terminal.child_exited += term => { destroy(); };
        terminal.eof += term => { destroy(); };
        terminal.window_title_changed += term => { Gtk.Window toplevel = (Gtk.Window) get_toplevel(); toplevel.set_title( term.window_title ); };
        pack_start( terminal, true, true, 0 );

        scrollbar = new VScrollbar( terminal.adjustment );
        pack_start( scrollbar, false, false, 0 );

        var fore = Gdk.Color() { pixel = 0, red = 0x0000, green = 0x0000, blue = 0x0000 };
        var back = Gdk.Color() { pixel = 0, red = 0xffff, green = 0xffff, blue = 0xffff };
        var colors = new Gdk.Color[] {
            Gdk.Color() { pixel = 0, red = 0x0000, green = 0x0000, blue = 0x0000 },
            Gdk.Color() { pixel = 0, red = 0x8000, green = 0x0000, blue = 0x0000 },
            Gdk.Color() { pixel = 0, red = 0x0000, green = 0x8000, blue = 0x0000 },
            Gdk.Color() { pixel = 0, red = 0x8000, green = 0x8000, blue = 0x0000 },
            Gdk.Color() { pixel = 0, red = 0x0000, green = 0x0000, blue = 0x8000 },
            Gdk.Color() { pixel = 0, red = 0x8000, green = 0x0000, blue = 0x8000 },
            Gdk.Color() { pixel = 0, red = 0x0000, green = 0x8000, blue = 0x8000 },
            Gdk.Color() { pixel = 0, red = 0x8000, green = 0x8000, blue = 0x8000 }
        };

        terminal.set_colors( fore, back, colors );

        update_font();
        terminal.set_scrollback_lines( 1000 );
        terminal.set_mouse_autohide( true );
        terminal.set_cursor_blinks( true );
        terminal.set_backspace_binding( TerminalEraseBinding.ASCII_DELETE);
        // work around bug in VTE. FIXME: Clear with upstream
        terminal.fork_command( (string) 0, (string[]) 0, new string[]{}, Environment.get_variable( "HOME" ), true, true, true );
    }

    public uint get_font_size()
    {
        return fontsize;
    }

    public void update_font()
    {
        string font = "%s %u".printf( fontname, fontsize );
        terminal.set_font_from_string_full( font, TerminalAntiAlias.FORCE_ENABLE );
    }

    public void zoom_in()
    {
        ++fontsize;
        update_font();
    }

    public void zoom_out()
    {
        --fontsize;
        update_font();
    }

    public void paste()
    {
        terminal.paste_primary();
    }

    public void paste_command( string command )
    {
        terminal.feed_child( command, command.size() );
    }
}

