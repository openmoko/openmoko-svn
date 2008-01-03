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
    private string _fontname;
    private uint _fontsize;
    private Scrollbar _scrollbar;
    private Terminal _terminal;

    construct {
        stdout.printf( "moko-terminal constructed\n" );

        // may read from gconf at some point?
        _fontname = "LiberationMono";
        _fontsize = 5;

        _terminal = new Vte.Terminal();
        _terminal.child_exited += term => { stdout.printf( "unhandled eof\n" ); };
        _terminal.eof += term => { stdout.printf( "unhandled eof\n" ); };
        _terminal.window_title_changed += term => { Gtk.Window toplevel = get_toplevel(); toplevel.set_title( term.window_title ); };
        pack_start( _terminal, true, true, 0 );

        _scrollbar = new VScrollbar( _terminal.adjustment );
        pack_start( _scrollbar, false, false, 0 );

        var fore = new Gdk.Color() { pixel = 0, red = (ushort)0x0000, green = (ushort)0x0000, blue = (ushort)0x0000 };
        var back = new Gdk.Color() { pixel = 0, red = (ushort)0xffff, green = (ushort)0xffff, blue = (ushort)0xffff };
        var colors = new Gdk.Color[] {
            new Gdk.Color() { pixel = 0, red = (ushort)0x0000, green = (ushort)0x0000, blue = (ushort)0x0000 },
            new Gdk.Color() { pixel = 0, red = (ushort)0x8000, green = (ushort)0x0000, blue = (ushort)0x0000 },
            new Gdk.Color() { pixel = 0, red = (ushort)0x0000, green = (ushort)0x8000, blue = (ushort)0x0000 },
            new Gdk.Color() { pixel = 0, red = (ushort)0x8000, green = (ushort)0x8000, blue = (ushort)0x0000 },
            new Gdk.Color() { pixel = 0, red = (ushort)0x0000, green = (ushort)0x0000, blue = (ushort)0x8000 },
            new Gdk.Color() { pixel = 0, red = (ushort)0x8000, green = (ushort)0x0000, blue = (ushort)0x8000 },
            new Gdk.Color() { pixel = 0, red = (ushort)0x0000, green = (ushort)0x8000, blue = (ushort)0x8000 },
            new Gdk.Color() { pixel = 0, red = (ushort)0x8000, green = (ushort)0x8000, blue = (ushort)0x8000 }
        };

        _terminal.set_colors( ref fore, ref back, ref colors[0], 8 );

        update_font();
        _terminal.set_scrollback_lines( 1000 );
        _terminal.set_mouse_autohide( true );
        _terminal.set_cursor_blinks( true );
        _terminal.set_backspace_binding( TerminalEraseBinding.ASCII_DELETE);
        _terminal.fork_command( null, null, null, Environment.get_variable( "HOME" ), true, true, true );
    }

    public void update_font()
    {
        string font = "%s %d".printf( _fontname, _fontsize );
        _terminal.set_font_from_string_full( font, TerminalAntiAlias.FORCE_ENABLE );
    }

    public void zoom_in()
    {
        ++_fontsize;
        update_font();
    }

    public void zoom_out()
    {
        --_fontsize;
        update_font();
    }
}

