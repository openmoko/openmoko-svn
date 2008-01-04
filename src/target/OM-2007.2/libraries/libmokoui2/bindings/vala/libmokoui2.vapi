/* libmokoui2.vapi
 *
 * Author: Michael 'Mickey' Lauer <mickey@vanille-media.de>
 * Copyright (C) 2008 OpenMoko, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 *
 */

[CCode (cprefix = "Moko", lower_case_cprefix = "moko_")]
namespace Moko {
	[CCode (cprefix = "MOKO_FINGER_SCROLL_MODE_", cheader_filename = "libmokoui2.h")]
	public enum FingerScrollMode {
		PUSH,
		ACCEL,
	}
	[CCode (cheader_filename = "moko-finger-scroll.h")]
	public class FingerScroll : Gtk.EventBox {
		public weak Gtk.EventBox parent;
		public void add_with_viewport (Gtk.Widget child);
		public FingerScroll ();
		public FingerScroll.full (int mode, bool enabled, double vel_min, double vel_max, double decel, uint sps);
	}
	[CCode (cheader_filename = "moko-finger-scroll.h")]
	public class FingerScrollClass {
		public pointer parent_class;
	}
	[CCode (cheader_filename = "moko-hint-entry.h")]
	public class HintEntry : Gtk.Entry {
		public weak Gtk.Entry parent;
		public bool is_empty ();
		public HintEntry (string hint);
		public void set_text (string text);
	}
	[CCode (cheader_filename = "moko-hint-entry.h")]
	public class HintEntryClass {
		public pointer parent_class;
	}
	[CCode (cheader_filename = "moko-search-bar.h")]
	public class SearchBar : Gtk.HBox {
		public weak Gtk.HBox parent;
		public weak Gtk.ComboBox get_combo_box ();
		public weak Gtk.Entry get_entry ();
		public SearchBar ();
		public SearchBar.with_combo (Gtk.ComboBox combo);
		public bool search_visible ();
		public void toggle ();
	}
	[CCode (cheader_filename = "moko-search-bar.h")]
	public class SearchBarClass {
		public pointer parent_class;
		public GLib.Callback toggled;
		public GLib.Callback text_changed;
		public GLib.Callback combo_changed;
	}
	public const string STOCK_CALL_ANSWER;
	public const string STOCK_CALL_DIAL;
	public const string STOCK_CALL_DIALED;
	public const string STOCK_CALL_HANGUP;
	public const string STOCK_CALL_HISTORY;
	public const string STOCK_CALL_HOLD;
	public const string STOCK_CALL_IGNORE;
	public const string STOCK_CALL_IN;
	public const string STOCK_CALL_MISSED;
	public const string STOCK_CALL_REDIAL;
	public const string STOCK_CALL_REJECT;
	public const string STOCK_CONTACT_ADDRESS;
	public const string STOCK_CONTACT_DELETE;
	public const string STOCK_CONTACT_EMAIL;
	public const string STOCK_CONTACT_GROUPS;
	public const string STOCK_CONTACT_MODE;
	public const string STOCK_CONTACT_NEW;
	public const string STOCK_CONTACT_PHONE;
	public const string STOCK_FOLDER_DELETE;
	public const string STOCK_FOLDER_NEW;
	public const string STOCK_HANDSET;
	public const string STOCK_HISTORY;
	public const string STOCK_MAIL_DELETE;
	public const string STOCK_MAIL_FORWARD;
	public const string STOCK_MAIL_MARK_READ;
	public const string STOCK_MAIL_NEW;
	public const string STOCK_MAIL_READ;
	public const string STOCK_MAIL_REPLY_ALL;
	public const string STOCK_MAIL_REPLY_SENDER;
	public const string STOCK_MAIL_SEND;
	public const string STOCK_MAIL_SEND_RECEIVE;
	public const string STOCK_PHONE_BOOK;
	public const string STOCK_SEARCH;
	public const string STOCK_SMS_NEW;
	public const string STOCK_SPEAKER;
	public const string STOCK_VIEW;
	public static void stock_register ();
}
