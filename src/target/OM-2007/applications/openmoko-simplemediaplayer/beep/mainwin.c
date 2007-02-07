/*  BMP - Cross-platform multimedia player
 *  Copyright (C) 2003-2004  BMP development team.
 *
 *  Based on XMMS:
 *  Copyright (C) 1998-2003  XMMS development team.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif


#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <gdk/gdkkeysyms.h>

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>

#include "textbox.h"
#include "mainwin.h"
#include "pixmaps.h"

#include "main.h"

#include "controlsocket.h"
#include "pluginenum.h"

#include "about.h"
#include "dnd.h"
#include "dock.h"
#include "equalizer.h"
#include "hints.h"
#include "input.h"
#include "playlistwin.h"
#include "prefswin.h"
#include "skinwin.h"

#include "hslider.h"
#include "menurow.h"
#include "monostereo.h"
#include "pbutton.h"
#include "playback.h"
#include "playlist.h"
#include "playlist_list.h"
#include "playstatus.h"
#include "sbutton.h"
#include "svis.h"
#include "textbox.h"
#include "urldecode.h"
#include "util.h"
#include "vis.h"
#include "visualization.h"
#include "libbeep/configdb.h"

//added by lijiang
#define DBUS_API_SUBJECT_TO_CHANGE

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>

//openmoko header
#include <libmokoui/moko-application.h>
#include <libmokoui/moko-finger-tool-box.h>
#include <libmokoui/moko-finger-window.h>
#include <libmokoui/moko-finger-wheel.h>
#include <libmokoui/moko-pixmap-button.h>

//id3lib
#include <id3.h>

//vorbis
//#include "vorbis/codec.h"
//#include "vorbisfile.h"

//vfs
#include <libbeep/vfs.h>

//mp4
//#include <mp4.h>
//added end

#define ITEM_SEPARATOR {"/-", NULL, NULL, 0, "<Separator>"}

/*
 * If you change the menu above change these defines also
 */

#define MAINWIN_VIS_MENU_VIS_MODE               1
#define MAINWIN_VIS_MENU_NUM_VIS_MODE           3
#define MAINWIN_VIS_MENU_ANALYZER_MODE          5
#define MAINWIN_VIS_MENU_NUM_ANALYZER_MODE      3
#define MAINWIN_VIS_MENU_ANALYZER_TYPE          9
#define MAINWIN_VIS_MENU_NUM_ANALYZER_TYPE      2
#define MAINWIN_VIS_MENU_ANALYZER_PEAKS         12
#define MAINWIN_VIS_MENU_SCOPE_MODE             14
#define MAINWIN_VIS_MENU_NUM_SCOPE_MODE         3
#define MAINWIN_VIS_MENU_WSHADEVU_MODE          18
#define MAINWIN_VIS_MENU_NUM_WSHADEVU_MODE      2
#define MAINWIN_VIS_MENU_REFRESH_RATE           21
#define MAINWIN_VIS_MENU_NUM_REFRESH_RATE       4
#define MAINWIN_VIS_MENU_AFALLOFF               26
#define MAINWIN_VIS_MENU_NUM_AFALLOFF           5
#define MAINWIN_VIS_MENU_PFALLOFF               32
#define MAINWIN_VIS_MENU_NUM_PFALLOFF           5

#define VOLSET_DISP_TIMES 5


enum {
    MAINWIN_SONGNAME_FILEINFO,
    MAINWIN_SONGNAME_JTF,
    MAINWIN_SONGNAME_JTT,
    MAINWIN_SONGNAME_SCROLL
};

enum {
    MAINWIN_OPT_SKIN, MAINWIN_OPT_RELOADSKIN,
    MAINWIN_OPT_REPEAT, MAINWIN_OPT_SHUFFLE, MAINWIN_OPT_NPA,
    MAINWIN_OPT_TELAPSED, MAINWIN_OPT_TREMAINING,
    MAINWIN_OPT_ALWAYS,
    MAINWIN_OPT_STICKY,
    MAINWIN_OPT_WS,
    MAINWIN_OPT_PWS,
    MAINWIN_OPT_EQWS
};

enum {
    MAINWIN_VIS_ANALYZER, MAINWIN_VIS_SCOPE, MAINWIN_VIS_OFF,
    MAINWIN_VIS_ANALYZER_NORMAL, MAINWIN_VIS_ANALYZER_FIRE,
    MAINWIN_VIS_ANALYZER_VLINES,
    MAINWIN_VIS_ANALYZER_LINES, MAINWIN_VIS_ANALYZER_BARS,
    MAINWIN_VIS_ANALYZER_PEAKS,
    MAINWIN_VIS_SCOPE_DOT, MAINWIN_VIS_SCOPE_LINE, MAINWIN_VIS_SCOPE_SOLID,
    MAINWIN_VIS_VU_NORMAL, MAINWIN_VIS_VU_SMOOTH,
    MAINWIN_VIS_REFRESH_FULL, MAINWIN_VIS_REFRESH_HALF,
    MAINWIN_VIS_REFRESH_QUARTER, MAINWIN_VIS_REFRESH_EIGHTH,
    MAINWIN_VIS_AFALLOFF_SLOWEST, MAINWIN_VIS_AFALLOFF_SLOW,
    MAINWIN_VIS_AFALLOFF_MEDIUM, MAINWIN_VIS_AFALLOFF_FAST,
    MAINWIN_VIS_AFALLOFF_FASTEST,
    MAINWIN_VIS_PFALLOFF_SLOWEST, MAINWIN_VIS_PFALLOFF_SLOW,
    MAINWIN_VIS_PFALLOFF_MEDIUM, MAINWIN_VIS_PFALLOFF_FAST,
    MAINWIN_VIS_PFALLOFF_FASTEST,
    MAINWIN_VIS_PLUGINS
};

enum {
    MAINWIN_VIS_ACTIVE_MAINWIN, MAINWIN_VIS_ACTIVE_PLAYLISTWIN
};


typedef struct _PlaybackInfo PlaybackInfo;

struct _PlaybackInfo {
    gchar *title;
    gint bitrate;
    gint frequency;
    gint n_channels;
};


GtkWidget *mainwin = NULL;

static GdkBitmap *nullmask;
static gint balance;

GtkWidget *mainwin_jtf = NULL;
static GtkWidget *mainwin_jtt = NULL;

GtkItemFactory *mainwin_songname_menu, *mainwin_vis_menu;
GtkItemFactory *mainwin_general_menu, *mainwin_play_menu, *mainwin_add_menu;
GtkItemFactory *mainwin_view_menu;


GdkGC *mainwin_gc;
static GdkPixmap *mainwin_bg = NULL;

GtkAccelGroup *mainwin_accel = NULL;

static PButton *mainwin_menubtn;
static PButton *mainwin_minimize, *mainwin_shade, *mainwin_close;

static PButton *mainwin_rew, *mainwin_fwd;
static PButton *mainwin_eject;
static PButton *mainwin_play, *mainwin_pause, *mainwin_stop;

TButton *mainwin_shuffle, *mainwin_repeat, *mainwin_eq, *mainwin_pl;
TextBox *mainwin_info;

static TextBox *mainwin_rate_text, *mainwin_freq_text;
static TextBox *mainwin_stime_min, *mainwin_stime_sec;

PlayStatus *mainwin_playstatus;

static Number *mainwin_minus_num, *mainwin_10min_num, *mainwin_min_num;
static Number *mainwin_10sec_num, *mainwin_sec_num;

static gboolean setting_volume = FALSE;

Vis *active_vis;
Vis *mainwin_vis;
SVis *mainwin_svis;

static MenuRow *mainwin_menurow;
static HSlider *mainwin_volume, *mainwin_balance, *mainwin_position;
static HSlider *mainwin_sposition = NULL;
static MonoStereo *mainwin_monostereo;
static SButton *mainwin_srew, *mainwin_splay, *mainwin_spause;
static SButton *mainwin_sstop, *mainwin_sfwd, *mainwin_seject, *mainwin_about;

static GList *mainwin_wlist = NULL;

static gint mainwin_timeout_id;

G_LOCK_DEFINE_STATIC(mainwin_title);

static gboolean mainwin_force_redraw = FALSE;
static gchar *mainwin_title_text = NULL;
static gboolean mainwin_info_text_locked = FALSE;


static void mainwin_songname_menu_callback(gpointer user_data,
                                           guint action,
                                           GtkWidget * widget);

static void mainwin_vis_menu_callback(gpointer user_data,
                                      guint action,
                                      GtkWidget * widget);

static void mainwin_view_menu_callback(gpointer user_data,
                                       guint action,
                                       GtkWidget * widget);

static void mainwin_play_menu_callback(gpointer user_data,
                                       guint action,
                                       GtkWidget * widget);

/* Song name area menu */

static GtkItemFactoryEntry mainwin_songname_menu_entries[] = {
    {N_("/View Track Details"), "<alt>i", mainwin_general_menu_callback,
     MAINWIN_GENERAL_FILEINFO, "<ImageItem>", my_pixbuf},
    {"/-", NULL, NULL, 0, "<Separator>"},
    {N_("/Autoscroll Songname"), NULL, mainwin_songname_menu_callback,
     MAINWIN_SONGNAME_SCROLL, "<ToggleItem>"},
};

static gint mainwin_songname_menu_entries_num =
    G_N_ELEMENTS(mainwin_songname_menu_entries);

/* Mini-visualizer area menu */

static GtkItemFactoryEntry mainwin_vis_menu_entries[] = {
    {N_("/Visualization Mode"), NULL, NULL, 0, "<Branch>"},
    {N_("/Visualization Mode/Analyzer"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_ANALYZER, "<RadioItem>"},
    {N_("/Visualization Mode/Scope"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_SCOPE, "/Visualization Mode/Analyzer"},
    {N_("/Visualization Mode/Off"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_OFF, "/Visualization Mode/Analyzer"},
    {N_("/Analyzer Mode"), NULL, NULL, 0, "<Branch>"},
    {N_("/Analyzer Mode/Normal"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_ANALYZER_NORMAL, "<RadioItem>"},
    {N_("/Analyzer Mode/Fire"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_ANALYZER_FIRE, "/Analyzer Mode/Normal"},
    {N_("/Analyzer Mode/Vertical Lines"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_ANALYZER_VLINES, "/Analyzer Mode/Normal"},
    {"/Analyzer Mode/-", NULL, NULL, 0, "<Separator>"},
    {N_("/Analyzer Mode/Lines"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_ANALYZER_LINES, "<RadioItem>"},
    {N_("/Analyzer Mode/Bars"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_ANALYZER_BARS, "/Analyzer Mode/Lines"},
    {"/Analyzer Mode/-", NULL, NULL, 0, "<Separator>"},
    {N_("/Analyzer Mode/Peaks"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_ANALYZER_PEAKS, "<ToggleItem>"},
    {N_("/Scope Mode"), NULL, NULL, 0, "<Branch>"},
    {N_("/Scope Mode/Dot Scope"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_SCOPE_DOT, "<RadioItem>"},
    {N_("/Scope Mode/Line Scope"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_SCOPE_LINE, "/Scope Mode/Dot Scope"},
    {N_("/Scope Mode/Solid Scope"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_SCOPE_SOLID, "/Scope Mode/Dot Scope"},
    {N_("/WindowShade VU Mode"), NULL, NULL, 0, "<Branch>"},
    {N_("/WindowShade VU Mode/Normal"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_VU_NORMAL, "<RadioItem>"},
    {N_("/WindowShade VU Mode/Smooth"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_VU_SMOOTH, "/WindowShade VU Mode/Normal"},
    {N_("/Refresh Rate"), NULL, NULL, 0, "<Branch>"},
    {N_("/Refresh Rate/Full (~50 fps)"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_REFRESH_FULL, "<RadioItem>"},
    {N_("/Refresh Rate/Half (~25 fps)"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_REFRESH_HALF, "/Refresh Rate/Full (~50 fps)"},
    {N_("/Refresh Rate/Quarter (~13 fps)"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_REFRESH_QUARTER, "/Refresh Rate/Full (~50 fps)"},
    {N_("/Refresh Rate/Eighth (~6 fps)"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_REFRESH_EIGHTH, "/Refresh Rate/Full (~50 fps)"},
    {N_("/Analyzer Falloff"), NULL, NULL, 0, "<Branch>"},
    {N_("/Analyzer Falloff/Slowest"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_AFALLOFF_SLOWEST, "<RadioItem>"},
    {N_("/Analyzer Falloff/Slow"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_AFALLOFF_SLOW, "/Analyzer Falloff/Slowest"},
    {N_("/Analyzer Falloff/Medium"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_AFALLOFF_MEDIUM, "/Analyzer Falloff/Slowest"},
    {N_("/Analyzer Falloff/Fast"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_AFALLOFF_FAST, "/Analyzer Falloff/Slowest"},
    {N_("/Analyzer Falloff/Fastest"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_AFALLOFF_FASTEST, "/Analyzer Falloff/Slowest"},
    {N_("/Peaks Falloff"), NULL, NULL, 0, "<Branch>"},
    {N_("/Peaks Falloff/Slowest"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_PFALLOFF_SLOWEST, "<RadioItem>"},
    {N_("/Peaks Falloff/Slow"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_PFALLOFF_SLOW, "/Peaks Falloff/Slowest"},
    {N_("/Peaks Falloff/Medium"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_PFALLOFF_MEDIUM, "/Peaks Falloff/Slowest"},
    {N_("/Peaks Falloff/Fast"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_PFALLOFF_FAST, "/Peaks Falloff/Slowest"},
    {N_("/Peaks Falloff/Fastest"), NULL, mainwin_vis_menu_callback,
     MAINWIN_VIS_PFALLOFF_FASTEST, "/Peaks Falloff/Slowest"}
};

static const gint mainwin_vis_menu_entries_num =
    G_N_ELEMENTS(mainwin_vis_menu_entries);

/* Playback menu (now used only for accelerators) */

static GtkItemFactoryEntry mainwin_playback_menu_entries[] = {
/*
    {N_("/Play CD"), "<alt>C", mainwin_general_menu_callback,
     MAINWIN_GENERAL_PLAYCD, "<StockItem>", GTK_STOCK_CDROM},
    {"/-", NULL, NULL, 0, "<Separator>"},
*/
    {N_("/Repeat"), "R", mainwin_play_menu_callback,
     MAINWIN_OPT_REPEAT, "<ToggleItem>"},
    {N_("/Shuffle"), "S", mainwin_play_menu_callback,
     MAINWIN_OPT_SHUFFLE, "<ToggleItem>"},
/*
    {N_("/No Playlist Advance"), "<control>N", mainwin_play_menu_callback,
     MAINWIN_OPT_NPA, "<ToggleItem>"},
*/
    {"/-", NULL, NULL, 0, "<Separator>"},
    {N_("/Play"), "x", mainwin_general_menu_callback,
     MAINWIN_GENERAL_PLAY, "<Item>"},
    {N_("/Pause"), "c", mainwin_general_menu_callback,
     MAINWIN_GENERAL_PAUSE, "<Item>"},
    {N_("/Stop"), "v", mainwin_general_menu_callback,
     MAINWIN_GENERAL_STOP, "<StockItem>", GTK_STOCK_STOP},
    {N_("/Previous"), "z", mainwin_general_menu_callback,
     MAINWIN_GENERAL_PREV, "<StockItem>", GTK_STOCK_GO_BACK},
    {N_("/Next"), "b", mainwin_general_menu_callback,
     MAINWIN_GENERAL_NEXT, "<StockItem>", GTK_STOCK_GO_FORWARD},
    {"/-", NULL, NULL, 0, "<Separator>"},
    {N_("/Jump to Playlist Start"), "<control>Z", mainwin_general_menu_callback,
     MAINWIN_GENERAL_START, "<StockItem>", GTK_STOCK_GOTO_TOP},
    {"/-", NULL, NULL, 0, "<Separator>"},
    {N_("/Jump to File"), "J", mainwin_general_menu_callback,
     MAINWIN_GENERAL_JTF, "<StockItem>", GTK_STOCK_JUMP_TO},
    {N_("/Jump to Time"), "<control>J", mainwin_general_menu_callback,
     MAINWIN_GENERAL_JTT, "<StockItem>", GTK_STOCK_JUMP_TO},
};

static const gint mainwin_playback_menu_entries_num =
    G_N_ELEMENTS(mainwin_playback_menu_entries);

/* Main menu */

static GtkItemFactoryEntry mainwin_general_menu_entries[] = {
    { N_("/View Track Details"), "<alt>I", mainwin_general_menu_callback,
     MAINWIN_GENERAL_FILEINFO, "<ImageItem>", my_pixbuf},
    {"/-", NULL, NULL, 0, "<Separator>"},
    {N_("/Preferences"), "<control>P", mainwin_general_menu_callback,
     MAINWIN_GENERAL_PREFS, "<StockItem>", GTK_STOCK_PREFERENCES},
    {N_("/_View"), NULL, NULL, 0, "<Item>"},
    {"/-", NULL, NULL, 0, "<Separator>"},
    {N_("/About BMP"), NULL, mainwin_general_menu_callback,
     MAINWIN_GENERAL_ABOUT, "<StockItem>", GTK_STOCK_DIALOG_INFO},
    {N_("/_Quit"), NULL, mainwin_general_menu_callback,
     MAINWIN_GENERAL_EXIT, "<StockItem>", GTK_STOCK_QUIT}
};

static const gint mainwin_general_menu_entries_num =
    G_N_ELEMENTS(mainwin_general_menu_entries);

/* Add submenu */

static GtkItemFactoryEntry mainwin_add_menu_entries[] = {
    {N_("/Files..."), "f", mainwin_general_menu_callback,
     MAINWIN_GENERAL_PLAYFILE, "<StockItem>", GTK_STOCK_OPEN},
/*
    {N_("/Folders..."), "d",
     mainwin_general_menu_callback,
     MAINWIN_GENERAL_PLAYDIRECTORY, "<StockItem>", GTK_STOCK_OPEN},
*/
    {N_("/Internet location..."), "<control>h", mainwin_general_menu_callback,
     MAINWIN_GENERAL_PLAYLOCATION, "<StockItem>", GTK_STOCK_NETWORK},
};

static const gint mainwin_add_menu_entries_num =
    G_N_ELEMENTS(mainwin_add_menu_entries);


/*
*/

/* View submenu */

static GtkItemFactoryEntry mainwin_view_menu_entries[] = {
    {N_("/Show Playlist Editor"), "<alt>E", mainwin_general_menu_callback,
     MAINWIN_GENERAL_SHOWPLWIN, "<ToggleItem>"},
    {N_("/Show Equalizer"), "<alt>G", mainwin_general_menu_callback,
     MAINWIN_GENERAL_SHOWEQWIN, "<ToggleItem>"},
    {"/-", NULL, NULL, 0, "<Separator>"},
    {N_("/Time Elapsed"), "<control>E", mainwin_view_menu_callback,
     MAINWIN_OPT_TELAPSED, "<RadioItem>"},
    {N_("/Time Remaining"), "<control>R", mainwin_view_menu_callback,
     MAINWIN_OPT_TREMAINING, "/Time Elapsed"},
    {"/-", NULL, NULL, 0, "<Separator>"},
    {N_("/Always On Top"), "<control>o", mainwin_view_menu_callback,
     MAINWIN_OPT_ALWAYS, "<ToggleItem>"},
    {N_("/Put on All Workspaces"), "<control>S",
     mainwin_view_menu_callback, MAINWIN_OPT_STICKY, "<ToggleItem>"},
    {"/-", NULL, NULL, 0, "<Separator>"},
    {N_("/Roll up Player"), "<control>W", mainwin_view_menu_callback,
     MAINWIN_OPT_WS, "<ToggleItem>"},
    {N_("/Roll up Playlist Editor"), "<control><shift>W", mainwin_view_menu_callback,
     MAINWIN_OPT_PWS, "<ToggleItem>"},
    {N_("/Roll up Equalizer"), "<control><alt>W", mainwin_view_menu_callback,
     MAINWIN_OPT_EQWS, "<ToggleItem>"}
};

static const gint mainwin_view_menu_entries_num =
    G_N_ELEMENTS(mainwin_view_menu_entries);


static PlaybackInfo playback_info = { NULL, 0, 0, 0 };


static gint mainwin_idle_func(gpointer data);

static void set_timer_mode_menu_cb(TimerMode mode);
static void set_timer_mode(TimerMode mode);


/* FIXME: placed here for now */
void
playback_get_sample_params(gint * bitrate,
                           gint * frequency,
                           gint * n_channels)
{
    if (bitrate)
        *bitrate = playback_info.bitrate;

    if (frequency)
        *frequency = playback_info.frequency;

    if (n_channels)
        *n_channels = playback_info.n_channels;
}

static void
playback_set_sample_params(gint bitrate,
                           gint frequency,
                           gint n_channels)
{
    if (bitrate >= 0)
        playback_info.bitrate = bitrate;

    if (frequency >= 0)
        playback_info.frequency = frequency;

    if (n_channels >= 0)
        playback_info.n_channels = n_channels;
}

static void
mainwin_set_title_scroll(gboolean scroll)
{
    cfg.autoscroll = scroll;
    textbox_set_scroll(mainwin_info, cfg.autoscroll);
}


void
mainwin_set_always_on_top(gboolean always)
{
    GtkWidget *widget = gtk_item_factory_get_widget(mainwin_view_menu,
                                                    "/Always On Top");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widget),
                                   mainwin_menurow->mr_always_selected);
}

static void
mainwin_set_shape_mask(void)
{
    GdkBitmap *mask;

    if (!cfg.player_visible)
        return;

    mask = skin_get_mask(bmp_active_skin, SKIN_MASK_MAIN + cfg.player_shaded);
    gtk_widget_shape_combine_mask(mainwin, mask, 0, 0);
}

static void
mainwin_set_shade(gboolean shaded)
{
    GtkWidget *widget;
    widget = gtk_item_factory_get_widget(mainwin_view_menu,
                                         "/Roll up Player");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widget), shaded);
}

static void
mainwin_set_shade_menu_cb(gboolean shaded)
{
    cfg.player_shaded = shaded;

    mainwin_set_shape_mask();

    if (shaded) {
        dock_shade(dock_window_list, GTK_WINDOW(mainwin),
                   MAINWIN_SHADED_HEIGHT);

        widget_show(WIDGET(mainwin_svis));
        vis_clear_data(mainwin_vis);

        widget_show(WIDGET(mainwin_srew));
        widget_show(WIDGET(mainwin_splay));
        widget_show(WIDGET(mainwin_spause));
        widget_show(WIDGET(mainwin_sstop));
        widget_show(WIDGET(mainwin_sfwd));
        widget_show(WIDGET(mainwin_seject));

        widget_show(WIDGET(mainwin_stime_min));
        widget_show(WIDGET(mainwin_stime_sec));

        textbox_set_scroll(mainwin_info, FALSE);
        if (bmp_playback_get_playing()
            && playlist_get_current_length() != -1)
            widget_show(WIDGET(mainwin_sposition));

        mainwin_shade->pb_ny = mainwin_shade->pb_py = 27;
    }
    else {
        dock_shade(dock_window_list, GTK_WINDOW(mainwin), MAINWIN_HEIGHT);

        widget_hide(WIDGET(mainwin_svis));
        svis_clear_data(mainwin_svis);

        widget_hide(WIDGET(mainwin_srew));
        widget_hide(WIDGET(mainwin_splay));
        widget_hide(WIDGET(mainwin_spause));
        widget_hide(WIDGET(mainwin_sstop));
        widget_hide(WIDGET(mainwin_sfwd));
        widget_hide(WIDGET(mainwin_seject));

        widget_hide(WIDGET(mainwin_stime_min));
        widget_hide(WIDGET(mainwin_stime_sec));
        widget_hide(WIDGET(mainwin_sposition));

        textbox_set_scroll(mainwin_info, TRUE);
        mainwin_shade->pb_ny = mainwin_shade->pb_py = 18;
    }

    draw_main_window(TRUE);
}

static void
mainwin_vis_set_active_vis(gint new_vis)
{
    switch (new_vis) {
    case MAINWIN_VIS_ACTIVE_MAINWIN:
//        playlistwin_vis_disable();
        active_vis = mainwin_vis;
        break;
    case MAINWIN_VIS_ACTIVE_PLAYLISTWIN:
//        playlistwin_vis_enable();
        active_vis = playlistwin_vis;
        break;
    }
}

static void
mainwin_vis_set_refresh(RefreshRate rate)
{
    cfg.vis_refresh = rate;
}

static void
mainwin_vis_set_afalloff(FalloffSpeed speed)
{
    cfg.analyzer_falloff = speed;
}

static void
mainwin_vis_set_pfalloff(FalloffSpeed speed)
{
    cfg.peaks_falloff = speed;
}

static void
mainwin_vis_set_analyzer_mode(AnalyzerMode mode)
{
    cfg.analyzer_mode = mode;
}

static void
mainwin_vis_set_analyzer_type(AnalyzerType mode)
{
    cfg.analyzer_type = mode;
}

void
mainwin_vis_set_type(VisType mode)
{
    gchar *path =
        mainwin_vis_menu_entries[MAINWIN_VIS_MENU_VIS_MODE + mode].path;
    GtkWidget *widget = gtk_item_factory_get_widget(mainwin_vis_menu, path);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widget), TRUE);
}

static void
mainwin_vis_set_type_menu_cb(VisType mode)
{
    cfg.vis_type = mode;

    if (mode == VIS_OFF) {
        if (cfg.player_shaded && cfg.player_visible)
            svis_clear(mainwin_svis);
        else
            vis_clear(active_vis);
    }
    if (mode == VIS_ANALYZER) {
        vis_clear_data(active_vis);
        svis_clear_data(mainwin_svis);
    }
}

static void
mainwin_menubtn_cb(void)
{
    gint x, y;
    gtk_window_get_position(GTK_WINDOW(mainwin), &x, &y);
    util_item_factory_popup(mainwin_general_menu,
                            x + 6,
                            y + MAINWIN_SHADED_HEIGHT,
                            1, GDK_CURRENT_TIME);
}

void
mainwin_minimize_cb(void)
{
    if (!mainwin)
        return;

    gtk_window_iconify(GTK_WINDOW(mainwin));
}

static void
mainwin_shade_toggle(void)
{
    mainwin_set_shade(!cfg.player_shaded);
}

void
mainwin_quit_cb(void)
{
//    gtk_widget_hide(equalizerwin);
//    gtk_widget_hide(playlistwin);
    gtk_widget_hide(mainwin);
    gdk_flush();

    if(mainwin_timeout_id > 0)
        g_source_remove(mainwin_timeout_id);

    util_set_cursor(NULL);

//    bmp_config_save();
//    gtk_accel_map_save(bmp_paths[BMP_PATH_ACCEL_FILE]);

//    ctrlsocket_cleanup();

//    playlist_stop_get_info_thread();
    playlist_clear();

    plugin_system_cleanup();
    
    gtk_main_quit();
}

static void
mainwin_destroy(GtkWidget * widget, gpointer data)
{
    mainwin_quit_cb();
}

static void
mainwin_draw_titlebar(gboolean focus)
{
    skin_draw_mainwin_titlebar(bmp_active_skin, mainwin_bg, mainwin_gc,
                               cfg.player_shaded, focus || !cfg.dim_titlebar);
}

void
draw_main_window(gboolean force)
{
    GList *wl;
    Widget *w;
    gboolean redraw;

    if (!cfg.player_visible)
        return;

    widget_list_lock(mainwin_wlist);

    if (force) {
        if (!cfg.player_shaded)
            skin_draw_pixmap(bmp_active_skin, mainwin_bg, mainwin_gc,
                             SKIN_MAIN, 0, 0, 0, 0, MAINWIN_WIDTH,
                             MAINWIN_HEIGHT);
        mainwin_draw_titlebar(gtk_window_has_toplevel_focus
                              (GTK_WINDOW(mainwin)));
    }

    widget_list_draw(mainwin_wlist, &redraw, force);

    if (redraw || force) {
        if (force) {
            gdk_window_clear(mainwin->window);
        }
        else {
            for (wl = mainwin_wlist; wl; wl = g_list_next(wl)) {
                w = WIDGET(wl->data);

                if (!w->redraw || !w->visible)
                    continue;

                gdk_window_clear_area(mainwin->window, w->x, w->y,
                                      w->width, w->height);
                w->redraw = FALSE;
            }
        }

        gdk_flush();
    }

    widget_list_unlock(mainwin_wlist);
}


void
mainwin_set_info_text(void)
{
    gchar *text;

    if (mainwin_info_text_locked)
        return;

    if ((text = input_get_info_text()) != NULL) {
        textbox_set_text(mainwin_info, text);
        g_free(text);
    }
    else if ((text = playlist_get_info_text()) != NULL) {
        textbox_set_text(mainwin_info, text);
        g_free(text);
    }
}

void
mainwin_lock_info_text(const gchar * text)
{
    mainwin_info_text_locked = TRUE;
    textbox_set_text(mainwin_info, text);
}

void
mainwin_release_info_text(void)
{
    mainwin_info_text_locked = FALSE;
    mainwin_set_info_text();
}


static gchar *
make_mainwin_title(const gchar * title)
{
    if (title)
        return g_strdup_printf(_("%s - BMP"), title);
    else
        return g_strdup(_("BMP"));
}

void
mainwin_set_song_title(const gchar * title)
{
    G_LOCK(mainwin_title);
    g_free(mainwin_title_text);
    mainwin_title_text = make_mainwin_title(title);
    G_UNLOCK(mainwin_title);
}

void
mainwin_set_song_info(gint bitrate,
                      gint frequency,
                      gint n_channels)
{
    gchar text[10];
    gchar *title;

    playback_set_sample_params(bitrate, frequency, n_channels);

    if (bitrate != -1) {
        bitrate /= 1000;

        if (bitrate < 1000) {
            /* Show bitrate in 1000s */
            g_snprintf(text, sizeof(text), "%3d", bitrate);
            textbox_set_text(mainwin_rate_text, text);
        }
        else {
            /* Show bitrate in 100,000s */
            bitrate /= 100;
            g_snprintf(text, sizeof(text), "%2dH", bitrate);
            textbox_set_text(mainwin_rate_text, text);
        }
    }
    else
        textbox_set_text(mainwin_rate_text, _("VBR"));

    /* Show sampling frequency in kHz */
    g_snprintf(text, sizeof(text), "%2d", frequency / 1000);
    textbox_set_text(mainwin_freq_text, text);

    monostereo_set_num_channels(mainwin_monostereo, n_channels);

    widget_show(WIDGET(mainwin_minus_num));
    widget_show(WIDGET(mainwin_10min_num));
    widget_show(WIDGET(mainwin_min_num));
    widget_show(WIDGET(mainwin_10sec_num));
    widget_show(WIDGET(mainwin_sec_num));

    if (!bmp_playback_get_paused())
        playstatus_set_status(mainwin_playstatus, STATUS_PLAY);

    if (playlist_get_current_length() != -1) {
        if (cfg.player_shaded)
            widget_show(WIDGET(mainwin_sposition));
        widget_show(WIDGET(mainwin_position));
    }
    else {
        widget_hide(WIDGET(mainwin_position));
        widget_hide(WIDGET(mainwin_sposition));
        mainwin_force_redraw = TRUE;
    }

    title = playlist_get_info_text();
    mainwin_set_song_title(title);
    g_free(title);
}

void
mainwin_clear_song_info(void)
{
    /* clear title */
    G_LOCK(mainwin_title);
    g_free(mainwin_title_text);
    mainwin_title_text = NULL;
    G_UNLOCK(mainwin_title);

    /* clear sampling parameters */
    playback_set_sample_params(0, 0, 0);

    mainwin_position->hs_pressed = FALSE;
    mainwin_sposition->hs_pressed = FALSE;

    /* clear sampling parameter displays */
    textbox_set_text(mainwin_rate_text, "   ");
    textbox_set_text(mainwin_freq_text, "  ");
    monostereo_set_num_channels(mainwin_monostereo, 0);

    playstatus_set_status(mainwin_playstatus, STATUS_STOP);

    /* hide playback time */
    widget_hide(WIDGET(mainwin_minus_num));
    widget_hide(WIDGET(mainwin_10min_num));
    widget_hide(WIDGET(mainwin_min_num));
    widget_hide(WIDGET(mainwin_10sec_num));
    widget_hide(WIDGET(mainwin_sec_num));

    textbox_set_text(mainwin_stime_min, "   ");
    textbox_set_text(mainwin_stime_sec, "  ");

    widget_hide(WIDGET(mainwin_position));
    widget_hide(WIDGET(mainwin_sposition));

    playlistwin_hide_timer();
    draw_main_window(TRUE);

    vis_clear(active_vis);
}

void
mainwin_disable_seekbar(void)
{
    /*
     * We dont call draw_main_window() here so this will not
     * remove them visually.  It will only prevent us from sending
     * any seek calls to the input plugin before the input plugin
     * calls ->set_info().
     */
    widget_hide(WIDGET(mainwin_position));
    widget_hide(WIDGET(mainwin_sposition));
}

static gboolean
mainwin_mouse_button_release(GtkWidget * widget,
                             GdkEventButton * event,
                             gpointer callback_data)
{
    gdk_pointer_ungrab(GDK_CURRENT_TIME);

    /*
     * The gdk_flush() is just for making sure that the pointer really
     * gets ungrabbed before calling any button callbacks
     *
     */

    gdk_flush();

    if (dock_is_moving(GTK_WINDOW(mainwin))) {
        dock_move_release(GTK_WINDOW(mainwin));
    }

    if (mainwin_menurow->mr_doublesize_selected) {
        event->x /= 2;
        event->y /= 2;
    }

    handle_release_cb(mainwin_wlist, widget, event);

    draw_main_window(FALSE);

    return FALSE;
}

static gboolean
mainwin_motion(GtkWidget * widget,
               GdkEventMotion * event,
               gpointer callback_data)
{
    XEvent ev;
    gint i = 0;

    XSync(GDK_DISPLAY(), False);

    while (XCheckTypedEvent(GDK_DISPLAY(), MotionNotify, &ev)) {
        event->x = ev.xmotion.x;
        event->y = ev.xmotion.y;
        i++;
    }

    if (dock_is_moving(GTK_WINDOW(mainwin))) {
        dock_move_motion(GTK_WINDOW(mainwin), event);
    }
    else {
        handle_motion_cb(mainwin_wlist, widget, event);
        draw_main_window(FALSE);
    }
    gdk_flush();

    return FALSE;
}

static gboolean
inside_sensitive_widgets(gint x, gint y)
{
    return (widget_contains(WIDGET(mainwin_menubtn), x, y)
            || widget_contains(WIDGET(mainwin_minimize), x, y)
            || widget_contains(WIDGET(mainwin_shade), x, y)
            || widget_contains(WIDGET(mainwin_close), x, y)
            || widget_contains(WIDGET(mainwin_rew), x, y)
            || widget_contains(WIDGET(mainwin_play), x, y)
            || widget_contains(WIDGET(mainwin_pause), x, y)
            || widget_contains(WIDGET(mainwin_stop), x, y)
            || widget_contains(WIDGET(mainwin_fwd), x, y)
            || widget_contains(WIDGET(mainwin_eject), x, y)
            || widget_contains(WIDGET(mainwin_shuffle), x, y)
            || widget_contains(WIDGET(mainwin_repeat), x, y)
            || widget_contains(WIDGET(mainwin_pl), x, y)
            || widget_contains(WIDGET(mainwin_eq), x, y)
            || widget_contains(WIDGET(mainwin_info), x, y)
            || widget_contains(WIDGET(mainwin_menurow), x, y)
            || widget_contains(WIDGET(mainwin_volume), x, y)
            || widget_contains(WIDGET(mainwin_balance), x, y)
            || (widget_contains(WIDGET(mainwin_position), x, y) &&
                widget_is_visible(WIDGET(mainwin_position)))
            || widget_contains(WIDGET(mainwin_minus_num), x, y)
            || widget_contains(WIDGET(mainwin_10min_num), x, y)
            || widget_contains(WIDGET(mainwin_min_num), x, y)
            || widget_contains(WIDGET(mainwin_10sec_num), x, y)
            || widget_contains(WIDGET(mainwin_sec_num), x, y)
            || widget_contains(WIDGET(mainwin_vis), x, y)
            || widget_contains(WIDGET(mainwin_minimize), x, y)
            || widget_contains(WIDGET(mainwin_shade), x, y)
            || widget_contains(WIDGET(mainwin_close), x, y)
            || widget_contains(WIDGET(mainwin_menubtn), x, y)
            || widget_contains(WIDGET(mainwin_sposition), x, y)
            || widget_contains(WIDGET(mainwin_stime_min), x, y)
            || widget_contains(WIDGET(mainwin_stime_sec), x, y)
            || widget_contains(WIDGET(mainwin_srew), x, y)
            || widget_contains(WIDGET(mainwin_splay), x, y)
            || widget_contains(WIDGET(mainwin_spause), x, y)
            || widget_contains(WIDGET(mainwin_sstop), x, y)
            || widget_contains(WIDGET(mainwin_sfwd), x, y)
            || widget_contains(WIDGET(mainwin_seject), x, y)
            || widget_contains(WIDGET(mainwin_svis), x, y)
            || widget_contains(WIDGET(mainwin_about), x, y));
}

void
mainwin_scrolled(GtkWidget * widget,
                 GdkEventScroll * event,
                 gpointer callback_data)
{
    gint d = cfg.mouse_change;
    if (event->direction == GDK_SCROLL_DOWN)
        d *= -1;
    mainwin_set_volume_diff(d);
}


static gboolean
mainwin_mouse_button_press(GtkWidget * widget,
                           GdkEventButton * event,
                           gpointer callback_data)
{

    gboolean grab = TRUE;

    if (event->button == 1 && event->type == GDK_BUTTON_PRESS &&
        !inside_sensitive_widgets(event->x, event->y) && event->y < 14) {
        if (0 && hint_move_resize_available()) {
            hint_move_resize(mainwin, event->x_root, event->y_root, TRUE);
            grab = FALSE;
        }
        else {
            gtk_window_present(GTK_WINDOW(mainwin));
            dock_move_press(dock_window_list, GTK_WINDOW(mainwin), event,
                            TRUE);
        }
    }
    else if (event->button == 1 && event->type == GDK_2BUTTON_PRESS &&
             event->y < 14 && !inside_sensitive_widgets(event->x, event->y)) {
        mainwin_set_shade(!cfg.player_shaded);
        if (dock_is_moving(GTK_WINDOW(mainwin)))
            dock_move_release(GTK_WINDOW(mainwin));
    }
    else if (event->button == 1 && event->type == GDK_2BUTTON_PRESS &&
             widget_contains(WIDGET(mainwin_info), event->x, event->y)) {
        playlist_fileinfo_current();
    }
    else {
        handle_press_cb(mainwin_wlist, widget, event);
        draw_main_window(FALSE);
    }

    if ((event->button == 1) && event->type != GDK_2BUTTON_PRESS &&
        (widget_contains(WIDGET(mainwin_vis), event->x, event->y) ||
         widget_contains(WIDGET(mainwin_svis), event->x, event->y))) {

        cfg.vis_type++;

        if (cfg.vis_type > VIS_OFF)
            cfg.vis_type = VIS_ANALYZER;

        mainwin_vis_set_type(cfg.vis_type);
    }

    if (event->button == 3) {
        if (widget_contains(WIDGET(mainwin_info), event->x, event->y)) {
            util_item_factory_popup(mainwin_songname_menu,
                                    event->x_root, event->y_root,
                                    3, event->time);
            grab = FALSE;
        }
        else if (widget_contains(WIDGET(mainwin_vis), event->x, event->y) ||
                 widget_contains(WIDGET(mainwin_svis), event->x, event->y)) {
            util_item_factory_popup(mainwin_vis_menu, event->x_root,
                                    event->y_root, 3, event->time);
            grab = FALSE;
        }
        else if ( (event->y > 70) && (event->x < 128) )
        {

            util_item_factory_popup(mainwin_play_menu,
                                    event->x_root,
                                    event->y_root, 3, event->time);
            grab = FALSE;
        } else {
            /*
             * Pop up the main menu a few pixels down.
             * This will avoid that anything is selected
             * if one right-clicks to focus the window
             * without raising it.
             *
             ***MD I think the above is stupid, people don't expect this
             *
             */
            util_item_factory_popup(mainwin_general_menu,
                                    event->x_root,
                                    event->y_root, 3, event->time);
            grab = FALSE;
        }
    }
    if (event->button == 1) {
        if ((event->x > 35 && event->x < 100 &&
             event->y > 25 && event->y < 40) ||
            widget_contains(WIDGET(mainwin_stime_min), event->x, event->y) ||
            widget_contains(WIDGET(mainwin_stime_sec), event->x, event->y)) {

            if (cfg.timer_mode == TIMER_ELAPSED)
                set_timer_mode(TIMER_REMAINING);
            else
                set_timer_mode(TIMER_ELAPSED);
        }

    }

    if (grab)
        gdk_pointer_grab(mainwin->window, FALSE,
                         GDK_BUTTON_MOTION_MASK |
                         GDK_BUTTON_RELEASE_MASK,
                         GDK_WINDOW(GDK_NONE), NULL, GDK_CURRENT_TIME);

    return FALSE;
}

static gboolean
mainwin_focus_in(GtkWidget * window,
                 GdkEventFocus * event,
                 gpointer data)
{
    mainwin_menubtn->pb_allow_draw = TRUE;
    mainwin_minimize->pb_allow_draw = TRUE;
    mainwin_shade->pb_allow_draw = TRUE;
    mainwin_close->pb_allow_draw = TRUE;
    draw_main_window(TRUE);

    return TRUE;
}


static gboolean
mainwin_focus_out(GtkWidget * widget,
                  GdkEventFocus * event,
                  gpointer callback_data)
{
    mainwin_menubtn->pb_allow_draw = FALSE;
    mainwin_minimize->pb_allow_draw = FALSE;
    mainwin_shade->pb_allow_draw = FALSE;
    mainwin_close->pb_allow_draw = FALSE;
    draw_main_window(TRUE);

    return TRUE;
}

static gboolean
mainwin_keypress(GtkWidget * grab_widget,
                 GdkEventKey * event,
                 gpointer data)
{

    switch (event->keyval) {

    case GDK_Up:
    case GDK_KP_Up:
        mainwin_set_volume_diff(2);
        break;
    case GDK_Down:
    case GDK_KP_Down:
        mainwin_set_volume_diff(-2);
        break;
    case GDK_Left:
    case GDK_KP_Left:
        if (playlist_get_current_length() != -1)
            bmp_playback_seek(CLAMP
                              (bmp_playback_get_time() - 5000, 0,
                               playlist_get_current_length()) / 1000);
        break;
    case GDK_Right:
    case GDK_KP_Right:
        if (playlist_get_current_length() != -1)
            bmp_playback_seek(CLAMP
                              (bmp_playback_get_time() + 5000, 0,
                               playlist_get_current_length()) / 1000);
        break;
    case GDK_Escape:
        mainwin_minimize_cb();
        break;
    default:
        return FALSE;
    }

    return TRUE;
}

static void
mainwin_jump_to_time_cb(GtkWidget * widget,
                        GtkWidget * entry)
{
    guint min = 0, sec = 0, params, time;

    params = sscanf(gtk_entry_get_text(GTK_ENTRY(entry)), "%u:%u",
                    &min, &sec);
    if (params == 2)
        time = (min * 60) + sec;
    else if (params == 1)
        time = min;
    else
        return;

    if (playlist_get_current_length() > -1 &&
        time <= (playlist_get_current_length() / 1000)) {
        bmp_playback_seek(time);
        gtk_widget_destroy(mainwin_jtt);
    }
}


void
mainwin_jump_to_time(void)
{
    GtkWidget *vbox, *hbox_new, *hbox_total;
    GtkWidget *time_entry, *label, *bbox, *jump, *cancel;
    guint tindex;
    gchar time_str[10];

    if (!bmp_playback_get_playing()) {
        /* FIXME: pop an error dialog and/or disable menu option to
           indicate JTT can't be launched when no track is being
           played */
        return;
    }

    if (mainwin_jtt) {
        gtk_window_present(GTK_WINDOW(mainwin_jtt));
        return;
    }

    mainwin_jtt = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_type_hint(GTK_WINDOW(mainwin_jtt),
                             GDK_WINDOW_TYPE_HINT_DIALOG);

    gtk_window_set_title(GTK_WINDOW(mainwin_jtt), _("Jump to Time"));
    gtk_window_set_position(GTK_WINDOW(mainwin_jtt), GTK_WIN_POS_CENTER);
    gtk_window_set_transient_for(GTK_WINDOW(mainwin_jtt),
                                 GTK_WINDOW(mainwin));

    g_signal_connect(mainwin_jtt, "destroy",
                     G_CALLBACK(gtk_widget_destroyed), &mainwin_jtt);
    gtk_container_border_width(GTK_CONTAINER(mainwin_jtt), 10);

    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(mainwin_jtt), vbox);

    hbox_new = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_new, TRUE, TRUE, 5);

    time_entry = gtk_entry_new();
    gtk_box_pack_start(GTK_BOX(hbox_new), time_entry, FALSE, FALSE, 5);
    g_signal_connect(time_entry, "activate",
                     G_CALLBACK(mainwin_jump_to_time_cb), time_entry);

    gtk_widget_set_size_request(time_entry, 70, -1);
    label = gtk_label_new(_("minutes:seconds"));
    gtk_box_pack_start(GTK_BOX(hbox_new), label, FALSE, FALSE, 5);

    hbox_total = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_total, TRUE, TRUE, 5);
    gtk_widget_show(hbox_total);

    /* FIXME: Disable display of current track length. It's not
       updated when track changes */
#if 0
    label = gtk_label_new(_("Track length:"));
    gtk_box_pack_start(GTK_BOX(hbox_total), label, FALSE, FALSE, 5);

    len = playlist_get_current_length() / 1000;
    g_snprintf(time_str, sizeof(time_str), "%u:%2.2u", len / 60, len % 60);
    label = gtk_label_new(time_str);

    gtk_box_pack_start(GTK_BOX(hbox_total), label, FALSE, FALSE, 10);
#endif

    bbox = gtk_hbutton_box_new();
    gtk_box_pack_start(GTK_BOX(vbox), bbox, TRUE, TRUE, 0);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
    gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 5);

    cancel = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    GTK_WIDGET_SET_FLAGS(cancel, GTK_CAN_DEFAULT);
    gtk_container_add(GTK_CONTAINER(bbox), cancel);
    g_signal_connect_swapped(cancel, "clicked",
                             G_CALLBACK(gtk_widget_destroy), mainwin_jtt);

    jump = gtk_button_new_from_stock(GTK_STOCK_JUMP_TO);
    GTK_WIDGET_SET_FLAGS(jump, GTK_CAN_DEFAULT);
    gtk_container_add(GTK_CONTAINER(bbox), jump);
    g_signal_connect(jump, "clicked",
                     G_CALLBACK(mainwin_jump_to_time_cb), time_entry);

    tindex = bmp_playback_get_time() / 1000;
    g_snprintf(time_str, sizeof(time_str), "%u:%2.2u", tindex / 60,
               tindex % 60);
    gtk_entry_set_text(GTK_ENTRY(time_entry), time_str);

    gtk_entry_select_region(GTK_ENTRY(time_entry), 0, strlen(time_str));

    gtk_widget_show_all(mainwin_jtt);

    gtk_widget_grab_focus(time_entry);
    gtk_widget_grab_default(jump);
}

static void
change_song(guint pos)
{
    if (bmp_playback_get_playing())
        bmp_playback_stop();

    playlist_set_position(pos);
    bmp_playback_initiate();
}

static void
mainwin_jump_to_file_jump(GtkTreeView * treeview)
{
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gchar *pos_str;
    guint pos;

    model = gtk_tree_view_get_model(treeview);
    selection = gtk_tree_view_get_selection(treeview);

    if (!gtk_tree_selection_get_selected(selection, NULL, &iter))
        return;

    gtk_tree_model_get(model, &iter, 0, &pos_str, -1);
    pos = g_ascii_strtoull(pos_str, NULL, 10) - 1;

    change_song(pos);

    /* FIXME: should only hide window */
    gtk_widget_destroy(mainwin_jtf);
    mainwin_jtf = NULL;
}

static void
mainwin_jump_to_file_jump_cb(GtkTreeView * treeview,
                             gpointer data)
{
    mainwin_jump_to_file_jump(treeview);
}

static void
mainwin_jump_to_file_set_queue_button_label(GtkButton * button,
                                      guint pos)
{
    if (playlist_is_position_queued(pos))
        gtk_button_set_label(button, _("Un_queue"));
    else
        gtk_button_set_label(button, _("_Queue"));
}

static void
mainwin_jump_to_file_queue_cb(GtkButton * button,
                              gpointer data)
{
    GtkTreeView *treeview;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gchar *pos_str;
    guint pos;

    treeview = GTK_TREE_VIEW(data);
    model = gtk_tree_view_get_model(treeview);
    selection = gtk_tree_view_get_selection(treeview);

    if (!gtk_tree_selection_get_selected(selection, NULL, &iter))
        return;

    gtk_tree_model_get(model, &iter, 0, &pos_str, -1);
    pos = g_ascii_strtoull(pos_str, NULL, 10) - 1;

    playlist_queue_position(pos);

    mainwin_jump_to_file_set_queue_button_label(button, pos);
}

static void
mainwin_jump_to_file_selection_changed_cb(GtkTreeSelection *treesel,
                                          gpointer data)
{
    GtkTreeView *treeview;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gchar *pos_str;
    guint pos;

    treeview = gtk_tree_selection_get_tree_view(treesel);
    model = gtk_tree_view_get_model(treeview);
    selection = gtk_tree_view_get_selection(treeview);

    if (!gtk_tree_selection_get_selected(selection, NULL, &iter))
        return;

    gtk_tree_model_get(model, &iter, 0, &pos_str, -1);
    pos = g_ascii_strtoull(pos_str, NULL, 10) - 1;

    mainwin_jump_to_file_set_queue_button_label(GTK_BUTTON(data), pos);
}

static gboolean
mainwin_jump_to_file_keypress_cb(GtkWidget * object,
                                 GdkEventKey * event,
                                 gpointer data)
{
    switch (event->keyval) {
    case GDK_Escape:
        /* FIXME: show only hide window */
        gtk_widget_destroy(mainwin_jtf);
        mainwin_jtf = NULL;
        return TRUE;
    case GDK_Return:
        mainwin_jump_to_file_jump(GTK_TREE_VIEW(data));
        return TRUE;
    default:
        return FALSE;
    };
}

static gboolean
mainwin_jump_to_file_match(const gchar * song, gchar ** keys)
{
    gint i = 0;
    gchar *key;
    gchar *song_lc;

    song_lc = g_ascii_strdown(song, -1);

    while (keys[i]) {
        key = g_ascii_strdown(keys[i], -1);
        if (!g_strrstr(song_lc, key)) {
            g_free(key);
            g_free(song_lc);
            return FALSE;
        }

        g_free(key);
        i++;
    }

    g_free(song_lc);

    return TRUE;
}

/* FIXME: Clear the entry when the list gets updated */
static void
mainwin_update_jtf(GtkWidget * widget, gpointer user_data)
{
    /* FIXME: Is not in sync with playlist due to delayed extinfo
     * reading */
    gint row;
    GList *playlist;
    gchar *desc_buf;
    gchar *row_str;
    GtkTreeIter iter;
    GtkTreeSelection *selection;

    GtkTreeModel *store;

    if (!mainwin_jtf)
        return;

    store = gtk_tree_view_get_model(GTK_TREE_VIEW(user_data));
    gtk_list_store_clear(GTK_LIST_STORE(store));

    row = 1;
    for (playlist = playlist_get(); playlist;
         playlist = g_list_next(playlist)) {
        PlaylistEntry *entry = PLAYLIST_ENTRY(playlist->data);

        if (entry->title)
            desc_buf = entry->title;
        else if (strchr(entry->filename, '/'))
            desc_buf = strrchr(entry->filename, '/') + 1;
        else
            desc_buf = entry->filename;

        row_str = g_strdup_printf("%d", row++);

        gtk_list_store_append(GTK_LIST_STORE(store), &iter);
        gtk_list_store_set(GTK_LIST_STORE(store), &iter,
                           0, row_str, 1, desc_buf, -1);

        g_free(row_str);
    }

    gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(user_data));
    gtk_tree_selection_select_iter(selection, &iter);
}

static void
mainwin_jump_to_file_edit_cb(GtkEntry * entry, gpointer user_data)
{
    GtkTreeView *treeview = GTK_TREE_VIEW(user_data);
    GtkTreeSelection *selection;
    GtkTreeIter iter;

    GtkListStore *store;

    gint song_index = 0;
    gchar **words;
    GList *playlist;

    gboolean match = FALSE;

    /* Chop the key string into ' '-separated key words */
    words = g_strsplit(gtk_entry_get_text(entry), " ", 0);

    /* FIXME: Remove the connected signals before clearing
     * (row-selected will still eventually arrive once) */
    store = GTK_LIST_STORE(gtk_tree_view_get_model(treeview));
    gtk_list_store_clear(store);

    PLAYLIST_LOCK();

    for (playlist = playlist_get(); playlist;
         playlist = g_list_next(playlist)) {

        PlaylistEntry *entry = PLAYLIST_ENTRY(playlist->data);
        const gchar *title, *filename;

        title = entry->title;
        if (!title) {
            filename = entry->filename;

            if (strchr(filename, '/'))
                title = strrchr(filename, '/') + 1;
            else
                title = filename;
        }

        /* Compare the key words to the string - if all the words
           match, add to the ListStore */

        /*
         * FIXME: The search string should be adapted to the
         * current display setting, e.g. if the user has set it to
         * "%p - %t" then build the match string like that too, or
         * even better, search for each of the tags seperatly.
         *
         * In any case the string to match should _never_ contain
         * something the user can't actually see in the playlist.
         */
        if (words[0])
            match = mainwin_jump_to_file_match(title, words);
        else
            match = TRUE;

        if (match) {
            gchar *song_index_str = g_strdup_printf("%d", song_index + 1);
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, song_index_str, 1, title, -1);
            g_free(song_index_str);
        }

        song_index++;
    }

    PLAYLIST_UNLOCK();

    g_strfreev(words);

    if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter)) {
        selection = gtk_tree_view_get_selection(treeview);
        gtk_tree_selection_select_iter(selection, &iter);
    }
}

void
mainwin_jump_to_file(void)
{
    GtkWidget *scrollwin;
    GtkWidget *vbox, *bbox, *sep;
    GtkWidget *jump, *queue, *cancel;
    GtkWidget *rescan, *edit;
    GtkWidget *search_label, *hbox;
    GList *playlist;
    gchar *desc_buf;
    gchar *row_str;
    gint row;

    GtkWidget *treeview;
    GtkListStore *jtf_store;

    GtkTreeIter iter;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    if (mainwin_jtf) {
        gtk_window_present(GTK_WINDOW(mainwin_jtf));
        return;
    }

    mainwin_jtf = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_type_hint(GTK_WINDOW(mainwin_jtf),
                             GDK_WINDOW_TYPE_HINT_DIALOG);

    gtk_window_set_title(GTK_WINDOW(mainwin_jtf), _("Jump to Track"));

    gtk_window_set_position(GTK_WINDOW(mainwin_jtf), GTK_WIN_POS_CENTER);
    g_signal_connect(mainwin_jtf, "destroy",
                     G_CALLBACK(gtk_widget_destroyed), &mainwin_jtf);

    gtk_container_border_width(GTK_CONTAINER(mainwin_jtf), 10);
    gtk_window_set_default_size(GTK_WINDOW(mainwin_jtf), 550, 350);

    vbox = gtk_vbox_new(FALSE, 5);
    gtk_container_add(GTK_CONTAINER(mainwin_jtf), vbox);

    jtf_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_STRING);
    treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(jtf_store));
    g_object_unref(jtf_store);

    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(treeview), TRUE);

    column = gtk_tree_view_column_new();
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(treeview), FALSE);
    gtk_tree_view_column_set_sizing(column, GTK_TREE_VIEW_COLUMN_AUTOSIZE);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_set_attributes(column, renderer, "text", 0, NULL);
    gtk_tree_view_column_set_spacing(column, 4);

    renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(column, renderer, FALSE);
    gtk_tree_view_column_set_attributes(column, renderer, "text", 1, NULL);
    gtk_tree_view_column_set_spacing(column, 4);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    g_signal_connect(treeview, "row-activated",
                     G_CALLBACK(mainwin_jump_to_file_jump), NULL);

    hbox = gtk_hbox_new(FALSE, 3);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 3);

    search_label = gtk_label_new(_("Filter: "));
    gtk_label_set_markup_with_mnemonic(GTK_LABEL(search_label), "_Filter:");
    gtk_box_pack_start(GTK_BOX(hbox), search_label, FALSE, FALSE, 0);

    edit = gtk_entry_new();
    gtk_entry_set_editable(GTK_ENTRY(edit), TRUE);
    gtk_label_set_mnemonic_widget(GTK_LABEL(search_label), edit);
    g_signal_connect(edit, "changed",
                     G_CALLBACK(mainwin_jump_to_file_edit_cb), treeview);

    g_signal_connect(mainwin_jtf, "key_press_event",
                     G_CALLBACK(mainwin_jump_to_file_keypress_cb), treeview);

    gtk_box_pack_start(GTK_BOX(hbox), edit, TRUE, TRUE, 3);

    scrollwin = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scrollwin), treeview);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollwin),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(scrollwin),
                                        GTK_SHADOW_IN);
    gtk_box_pack_start(GTK_BOX(vbox), scrollwin, TRUE, TRUE, 0);

    sep = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), sep, FALSE, FALSE, 0);

    bbox = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(bbox), GTK_BUTTONBOX_END);
    gtk_button_box_set_spacing(GTK_BUTTON_BOX(bbox), 5);
    gtk_box_pack_start(GTK_BOX(vbox), bbox, FALSE, FALSE, 0);

    queue = gtk_button_new_with_mnemonic(_("_Queue"));
    gtk_box_pack_start(GTK_BOX(bbox), queue, FALSE, FALSE, 0);
    GTK_WIDGET_SET_FLAGS(queue, GTK_CAN_DEFAULT);
    g_signal_connect(queue, "clicked", 
                     G_CALLBACK(mainwin_jump_to_file_queue_cb),
                     treeview);
    g_signal_connect(gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview)), "changed",
                     G_CALLBACK(mainwin_jump_to_file_selection_changed_cb),
                     queue);

    rescan = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
    gtk_box_pack_start(GTK_BOX(bbox), rescan, FALSE, FALSE, 0);
    g_signal_connect(rescan, "clicked",
                     G_CALLBACK(mainwin_update_jtf), treeview);
    GTK_WIDGET_SET_FLAGS(rescan, GTK_CAN_DEFAULT);
    gtk_widget_grab_default(rescan);

    jump = gtk_button_new_from_stock(GTK_STOCK_JUMP_TO);
    gtk_box_pack_start(GTK_BOX(bbox), jump, FALSE, FALSE, 0);

    g_signal_connect_swapped(jump, "clicked",
                             G_CALLBACK(mainwin_jump_to_file_jump_cb),
                             treeview);

    GTK_WIDGET_SET_FLAGS(jump, GTK_CAN_DEFAULT);
    gtk_widget_grab_default(jump);

    cancel = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
    gtk_box_pack_start(GTK_BOX(bbox), cancel, FALSE, FALSE, 0);
    g_signal_connect_swapped(cancel, "clicked",
                             G_CALLBACK(gtk_widget_destroy),
                             mainwin_jtf);
    GTK_WIDGET_SET_FLAGS(cancel, GTK_CAN_DEFAULT);

    gtk_list_store_clear(jtf_store);

    row = 1;

    PLAYLIST_LOCK();

    for (playlist = playlist_get(); playlist;
         playlist = g_list_next(playlist)) {

        PlaylistEntry *entry = PLAYLIST_ENTRY(playlist->data);

        if (entry->title)
            desc_buf = entry->title;
        else if (strchr(entry->filename, '/'))
            desc_buf = strrchr(entry->filename, '/') + 1;
        else
            desc_buf = entry->filename;

        row_str = g_strdup_printf("%d", row++);

        gtk_list_store_append(GTK_LIST_STORE(jtf_store), &iter);
        gtk_list_store_set(GTK_LIST_STORE(jtf_store), &iter,
                           0, row_str, 1, desc_buf, -1);

        g_free(row_str);
    }

    PLAYLIST_UNLOCK();

    gtk_widget_show_all(mainwin_jtf);
}

static gboolean
mainwin_configure(GtkWidget * window,
                  GdkEventConfigure * event,
                  gpointer data)
{
    if (!GTK_WIDGET_VISIBLE(window))
        return FALSE;

    if (cfg.show_wm_decorations)
        gdk_window_get_root_origin(window->window,
                                   &cfg.player_x, &cfg.player_y);
    else
        gdk_window_get_deskrelative_origin(window->window,
                                           &cfg.player_x, &cfg.player_y);
    return FALSE;
}

void
mainwin_set_back_pixmap(void)
{
    gdk_window_set_back_pixmap(mainwin->window, mainwin_bg, 0);
    gdk_window_clear(mainwin->window);
}

void
mainwin_drag_data_received(GtkWidget * widget,
                           GdkDragContext * context,
                           gint x,
                           gint y,
                           GtkSelectionData * selection_data,
                           guint info,
                           guint time,
                           gpointer user_data)
{
    ConfigDb *db;
    gchar *path, *decoded;

    if (!selection_data->data) {
        g_warning("DND data string is NULL");
        return;
    }

    path = (gchar *) selection_data->data;

	g_message(path);

    /* FIXME: use a real URL validator/parser */

    if (str_has_prefix_nocase(path, "fonts:///")) {
        path[strlen(path) - 2] = 0; /* Why the hell a CR&LF? */
        path += 8;

        /* plain, since we already stripped the first URI part */
        decoded = xmms_urldecode_plain(path);

        /* Get the old font's size, and add it to the dropped
         * font's name */
        cfg.playlist_font = g_strconcat(decoded + 1,
                                        strrchr(cfg.playlist_font, ' '),
                                        NULL);
        playlist_list_set_font(cfg.playlist_font);
        playlistwin_update_list();
        
        g_free(decoded);
        return;
    }

    if (str_has_prefix_nocase(path, "file:///")) {
        path[strlen(path) - 2] = 0; /* Why the hell a CR&LF? */
        path += 7;
    }
    else if (str_has_prefix_nocase(path, "file:")) {
        path += 5;
    }

    if (file_is_archive(path)) {
        bmp_active_skin_load(path);
        skin_install_skin(path);    /* ...and install the skin */
        skin_view_update(user_data);
        /* Change skin name in the config file */
        db = bmp_cfg_db_open();
        bmp_cfg_db_set_string(db, NULL, "skin", path);
        bmp_cfg_db_close(db);
    }
    else {
        if (input_check_file((gchar *) selection_data->data, FALSE)) {
            playlist_clear();
            playlist_add_url((gchar *) selection_data->data);
            bmp_playback_initiate();
        }
    }
}

static void
dirbrowser_add_dir(const gchar * dir)
{
    g_free(cfg.filesel_path);
    cfg.filesel_path = g_strdup(dir);
    playlist_add_dir(dir);
}

static void
dirbrowser_on_response(GtkFileChooserDialog * dialog,
                       gint result,
                       gpointer data)
{

    gchar *pathname;

//    gtk_widget_hide(GTK_WIDGET(dialog));

    switch (result) {
    case GTK_RESPONSE_ACCEPT:
        pathname = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        dirbrowser_add_dir(pathname);
        g_free(pathname);
        break;

    case GTK_RESPONSE_CLOSE:
        break;
    }

}


static GtkWidget *
dirbrowser_new(const gchar * path)
{
    GtkWidget *dialog;

    dialog = gtk_file_chooser_dialog_new(_("Add Folders"),
                                         GTK_WINDOW(mainwin),
                                         GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
                                         GTK_STOCK_ADD, GTK_RESPONSE_OK,
                                         GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                         NULL);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);

    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog),
                                        path);
    g_signal_connect(dialog, "response",
                     G_CALLBACK(dirbrowser_on_response),
                     NULL);

    return dialog;
}

void
mainwin_run_dirbrowser(void)
{
    static GtkWidget *browser = NULL;

    if (!browser) {
        browser = dirbrowser_new(cfg.filesel_path);

        g_signal_connect(browser, "destroy",
                         G_CALLBACK(gtk_widget_destroyed),
                         &browser);

        gtk_widget_show(GTK_WIDGET(browser));
    }

    gtk_window_present(GTK_WINDOW(browser));
}

static void
on_add_url_add_clicked(GtkWidget * widget,
                       GtkWidget * entry)
{
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry));
    if (text && *text)
        playlist_add_url(text);
}

void
mainwin_show_add_url_window(void)
{
    static GtkWidget *url_window = NULL;

    if (!url_window) {
        url_window =
            util_add_url_dialog_new(_("Add Internet Address"),
                                    G_CALLBACK(on_add_url_add_clicked));
        gtk_window_set_transient_for(GTK_WINDOW(url_window),
                                     GTK_WINDOW(mainwin));
        g_signal_connect(url_window, "destroy",
                         G_CALLBACK(gtk_widget_destroyed),
                         &url_window);
    }

    gtk_window_present(GTK_WINDOW(url_window));
}

static void
check_set(GtkItemFactory * factory, 
          const gchar * path,
          gboolean active)
{
    GtkWidget *item = gtk_item_factory_get_widget(factory, path);
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), active);
}

void
mainwin_eject_pushed(void)
{
    util_run_filebrowser(PLAY_BUTTON);
}

void
mainwin_play_pushed(void)
{
    if (bmp_playback_get_paused()) {
        bmp_playback_pause();
        return;
    }
    
    if (playlist_get_length())
    {
        bmp_playback_initiate();
    }
    else
        mainwin_eject_pushed();
}

void
mainwin_stop_pushed(void)
{
    mainwin_clear_song_info();
    bmp_playback_stop();
}

void
mainwin_shuffle_pushed(gboolean toggled)
{
    check_set(mainwin_play_menu, "/Shuffle", toggled);
}

void
mainwin_repeat_pushed(gboolean toggled)
{
    check_set(mainwin_play_menu, "/Repeat", toggled);
}

void
mainwin_pl_pushed(gboolean toggled)
{
    if (toggled)
        playlistwin_show();
    else
        playlistwin_hide();
}

gint
mainwin_spos_frame_cb(gint pos)
{
    if (mainwin_sposition) {
        if (pos < 6)
            mainwin_sposition->hs_knob_nx = mainwin_sposition->hs_knob_px =
                17;
        else if (pos < 9)
            mainwin_sposition->hs_knob_nx = mainwin_sposition->hs_knob_px =
                20;
        else
            mainwin_sposition->hs_knob_nx = mainwin_sposition->hs_knob_px =
                23;
    }
    return 1;
}

void
mainwin_spos_motion_cb(gint pos)
{
    gint time;
    gchar *time_msg;

    pos--;

    time = ((playlist_get_current_length() / 1000) * pos) / 12;

    if (cfg.timer_mode == TIMER_REMAINING) {
        time = (playlist_get_current_length() / 1000) - time;
        time_msg = g_strdup_printf("-%2.2d", time / 60);
        textbox_set_text(mainwin_stime_min, time_msg);
        g_free(time_msg);
    }
    else {
        time_msg = g_strdup_printf(" %2.2d", time / 60);
        textbox_set_text(mainwin_stime_min, time_msg);
        g_free(time_msg);
    }

    time_msg = g_strdup_printf("%2.2d", time % 60);
    textbox_set_text(mainwin_stime_sec, time_msg);
    g_free(time_msg);
}

void
mainwin_spos_release_cb(gint pos)
{
    bmp_playback_seek(((playlist_get_current_length() / 1000) *
                       (pos - 1)) / 12);
}

void
mainwin_position_motion_cb(gint pos)
{
    gint length, time;
    gchar *seek_msg;

    length = playlist_get_current_length() / 1000;
    time = (length * pos) / 219;
    seek_msg = g_strdup_printf(_("SEEK TO: %d:%-2.2d/%d:%-2.2d (%d%%)"),
                               time / 60, time % 60,
                               length / 60, length % 60,
                               (length != 0) ? (time * 100) / length : 0);
    mainwin_lock_info_text(seek_msg);
    g_free(seek_msg);
}

void
mainwin_position_release_cb(gint pos)
{
    gint length, time;

    length = playlist_get_current_length() / 1000;
    time = (length * pos) / 219;
    bmp_playback_seek(time);
    mainwin_release_info_text();
}

gint
mainwin_volume_frame_cb(gint pos)
{
    return (gint) rint((pos / 52.0) * 28);
}

void
mainwin_adjust_volume_motion(gint v)
{
    gchar *volume_msg;

    setting_volume = TRUE;

    volume_msg = g_strdup_printf(_("VOLUME: %d%%"), v);
    mainwin_lock_info_text(volume_msg);
    g_free(volume_msg);

    if (balance < 0)
        input_set_volume(v, (v * (100 - abs(balance))) / 100);
    else if (balance > 0)
        input_set_volume((v * (100 - abs(balance))) / 100, v);
    else
        input_set_volume(v, v);
}

void
mainwin_adjust_volume_release(void)
{
    mainwin_release_info_text();
    setting_volume = FALSE;
    read_volume(VOLUME_ADJUSTED);
}

void
mainwin_adjust_balance_motion(gint b)
{
    gchar *balance_msg;
    gint v, pvl, pvr;

    setting_volume = TRUE;
    balance = b;
    input_get_volume(&pvl, &pvr);
    v = MAX(pvl, pvr);
    if (b < 0) {
        balance_msg = g_strdup_printf(_("BALANCE: %d%% LEFT"), -b);
        input_set_volume(v, (gint) rint(((100 + b) / 100.0) * v));
    }
    else if (b == 0) {
        balance_msg = g_strdup_printf(_("BALANCE: CENTER"));
        input_set_volume(v, v);
    }
    else {                      /* b > 0 */
        balance_msg = g_strdup_printf(_("BALANCE: %d%% RIGHT"), b);
        input_set_volume((gint) rint(((100 - b) / 100.0) * v), v);
    }
    mainwin_lock_info_text(balance_msg);
    g_free(balance_msg);
}

void
mainwin_adjust_balance_release(void)
{
    mainwin_release_info_text();
    setting_volume = FALSE;
    read_volume(VOLUME_ADJUSTED);
}

void
mainwin_set_volume_slider(gint percent)
{
    hslider_set_position(mainwin_volume, (gint) rint((percent * 51) / 100.0));
}

void
mainwin_set_balance_slider(gint percent)
{
    hslider_set_position(mainwin_balance,
                         (gint) rint(((percent * 12) / 100.0) + 12));
}

void
mainwin_volume_motion_cb(gint pos)
{
    gint vol = (pos * 100) / 51;
    mainwin_adjust_volume_motion(vol);
    equalizerwin_set_volume_slider(vol);
}

void
mainwin_volume_release_cb(gint pos)
{
    mainwin_adjust_volume_release();
}

gint
mainwin_balance_frame_cb(gint pos)
{
    return ((abs(pos - 12) * 28) / 13);
}

void
mainwin_balance_motion_cb(gint pos)
{
    gint bal = ((pos - 12) * 100) / 12;
    mainwin_adjust_balance_motion(bal);
    equalizerwin_set_balance_slider(bal);
}

void
mainwin_balance_release_cb(gint pos)
{
    mainwin_adjust_volume_release();
}

void
mainwin_set_volume_diff(gint diff)
{
    gint vl, vr, vol;

    input_get_volume(&vl, &vr);
    vol = MAX(vl, vr);
    vol = CLAMP(vol + diff, 0, 100);

    mainwin_adjust_volume_motion(vol);
    setting_volume = FALSE;
    mainwin_set_volume_slider(vol);
//    equalizerwin_set_volume_slider(vol);
    read_volume(VOLUME_SET);
}

void
mainwin_set_balance_diff(gint diff)
{
    gint b;
    b = CLAMP(balance + diff, -100, 100);
    mainwin_adjust_balance_motion(b);
    setting_volume = FALSE;
    mainwin_set_balance_slider(b);
    equalizerwin_set_balance_slider(b);
    read_volume(VOLUME_SET);
}

void
mainwin_show(gboolean show)
{
    if (show)
        mainwin_real_show();
    else
        mainwin_real_hide();
}

void
mainwin_real_show(void)
{
    cfg.player_visible = TRUE;

    if (cfg.player_shaded)
        vis_clear_data(active_vis);

    mainwin_vis_set_active_vis(MAINWIN_VIS_ACTIVE_MAINWIN);
    mainwin_set_shape_mask();
    if (cfg.show_wm_decorations) {
        if (!pposition_broken && cfg.player_x != -1
            && cfg.save_window_position)
            gtk_window_move(GTK_WINDOW(mainwin), cfg.player_x, cfg.player_y);

        gtk_widget_show(mainwin);

        if (pposition_broken && cfg.player_x != -1
            && cfg.save_window_position)
            gtk_window_move(GTK_WINDOW(mainwin), cfg.player_x, cfg.player_y);

        return;
    }
    gtk_widget_show_all(mainwin);

    if (!nullmask)
        return;

    g_object_unref(nullmask);
    nullmask = NULL;

    gdk_window_set_hints(mainwin->window, 0, 0,
                         PLAYER_WIDTH, PLAYER_HEIGHT,
                         PLAYER_WIDTH, PLAYER_HEIGHT,
                         GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE);
    gtk_window_resize(GTK_WINDOW(mainwin), PLAYER_WIDTH, PLAYER_HEIGHT);

    if (cfg.player_x != -1 && cfg.player_y != -1)
        gtk_window_move(GTK_WINDOW(mainwin), cfg.player_x, cfg.player_y);
    draw_main_window(TRUE);

    gtk_window_present(GTK_WINDOW(mainwin));
}

void
mainwin_real_hide(void)
{
    GdkGC *gc;
    GdkColor pattern;

    if (cfg.player_shaded) {
        svis_clear_data(mainwin_svis);
        vis_clear_data(playlistwin_vis);
    }

    if (!cfg.show_wm_decorations) {
        nullmask = gdk_pixmap_new(mainwin->window, 20, 20, 1);
        gc = gdk_gc_new(nullmask);
        pattern.pixel = 0;
        gdk_gc_set_foreground(gc, &pattern);
        gdk_draw_rectangle(nullmask, gc, TRUE, 0, 0, 20, 20);
        gdk_gc_destroy(gc);
        gtk_widget_shape_combine_mask(mainwin, nullmask, 0, 0);

        gdk_window_set_hints(mainwin->window, 0, 0, 0, 0, 0, 0,
                             GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE);
        gdk_window_resize(mainwin->window, 0, 0);
    }

    gtk_widget_hide(mainwin);

    mainwin_vis_set_active_vis(MAINWIN_VIS_ACTIVE_PLAYLISTWIN);
    cfg.player_visible = FALSE;
}

static void
mainwin_songname_menu_callback(gpointer data,
                               guint action,
                               GtkWidget * item)
{
    GtkCheckMenuItem *check;

    switch (action) {
    case MAINWIN_SONGNAME_FILEINFO:
        playlist_fileinfo_current();
        break;
    case MAINWIN_SONGNAME_JTF:
        mainwin_jump_to_file();
        break;
    case MAINWIN_SONGNAME_JTT:
        mainwin_jump_to_time();
        break;
    case MAINWIN_SONGNAME_SCROLL:
        check = GTK_CHECK_MENU_ITEM(item);
        mainwin_set_title_scroll(gtk_check_menu_item_get_active(check));
        break;
    }
}

static void
mainwin_play_menu_callback(gpointer data,
                           guint action,
                           GtkWidget * item)
{
    GtkCheckMenuItem *check;

    switch (action) {
    case MAINWIN_OPT_SHUFFLE:
        check = GTK_CHECK_MENU_ITEM(item);
        cfg.shuffle = gtk_check_menu_item_get_active(check);
        playlist_set_shuffle(cfg.shuffle);
        tbutton_set_toggled(mainwin_shuffle, cfg.shuffle);
        break;
    case MAINWIN_OPT_REPEAT:
        check = GTK_CHECK_MENU_ITEM(item);
        cfg.repeat = gtk_check_menu_item_get_active(check);
        tbutton_set_toggled(mainwin_repeat, cfg.repeat);
        break;
    case MAINWIN_OPT_NPA:
        check = GTK_CHECK_MENU_ITEM(item);
        cfg.no_playlist_advance = gtk_check_menu_item_get_active(check);
        break;
    }
}

                               
static void
mainwin_view_menu_callback(gpointer data,
                           guint action,
                           GtkWidget * item)
{
    switch (action) {
    case MAINWIN_OPT_TELAPSED:
        set_timer_mode_menu_cb(TIMER_ELAPSED);
        break;
    case MAINWIN_OPT_TREMAINING:
        set_timer_mode_menu_cb(TIMER_REMAINING);
        break;
    case MAINWIN_OPT_ALWAYS:
        mainwin_menurow->mr_always_selected = GTK_CHECK_MENU_ITEM(item)->active;
        cfg.always_on_top = mainwin_menurow->mr_always_selected;
        widget_draw(WIDGET(mainwin_menurow));
        hint_set_always(cfg.always_on_top);
        break;
    case MAINWIN_OPT_STICKY:
        cfg.sticky = GTK_CHECK_MENU_ITEM(item)->active;
        hint_set_sticky(cfg.sticky);
        break;
    case MAINWIN_OPT_WS:
        mainwin_set_shade_menu_cb(GTK_CHECK_MENU_ITEM(item)->active);
        break;
    case MAINWIN_OPT_PWS:
        playlistwin_set_shade(GTK_CHECK_MENU_ITEM(item)->active);
        break;
    case MAINWIN_OPT_EQWS:
        equalizerwin_set_shade_menu_cb(GTK_CHECK_MENU_ITEM(item)->active);
        break;
    }
}

void
mainwin_vis_menu_callback(gpointer data,
                          guint action,
                          GtkWidget * item)
{
    switch (action) {
    case MAINWIN_VIS_ANALYZER:
    case MAINWIN_VIS_SCOPE:
    case MAINWIN_VIS_OFF:
        mainwin_vis_set_type_menu_cb(action - MAINWIN_VIS_ANALYZER);
        break;
    case MAINWIN_VIS_ANALYZER_NORMAL:
    case MAINWIN_VIS_ANALYZER_FIRE:
    case MAINWIN_VIS_ANALYZER_VLINES:
        mainwin_vis_set_analyzer_mode(action - MAINWIN_VIS_ANALYZER_NORMAL);
        break;
    case MAINWIN_VIS_ANALYZER_LINES:
    case MAINWIN_VIS_ANALYZER_BARS:
        mainwin_vis_set_analyzer_type(action - MAINWIN_VIS_ANALYZER_LINES);
        break;
    case MAINWIN_VIS_ANALYZER_PEAKS:
        cfg.analyzer_peaks = GTK_CHECK_MENU_ITEM(item)->active;
        break;
    case MAINWIN_VIS_SCOPE_DOT:
    case MAINWIN_VIS_SCOPE_LINE:
    case MAINWIN_VIS_SCOPE_SOLID:
        cfg.scope_mode = action - MAINWIN_VIS_SCOPE_DOT;
        break;
    case MAINWIN_VIS_VU_NORMAL:
    case MAINWIN_VIS_VU_SMOOTH:
        cfg.vu_mode = action - MAINWIN_VIS_VU_NORMAL;
        break;
    case MAINWIN_VIS_REFRESH_FULL:
    case MAINWIN_VIS_REFRESH_HALF:
    case MAINWIN_VIS_REFRESH_QUARTER:
    case MAINWIN_VIS_REFRESH_EIGHTH:
        mainwin_vis_set_refresh(action - MAINWIN_VIS_REFRESH_FULL);
        break;
    case MAINWIN_VIS_AFALLOFF_SLOWEST:
    case MAINWIN_VIS_AFALLOFF_SLOW:
    case MAINWIN_VIS_AFALLOFF_MEDIUM:
    case MAINWIN_VIS_AFALLOFF_FAST:
    case MAINWIN_VIS_AFALLOFF_FASTEST:
        mainwin_vis_set_afalloff(action - MAINWIN_VIS_AFALLOFF_SLOWEST);
        break;
    case MAINWIN_VIS_PFALLOFF_SLOWEST:
    case MAINWIN_VIS_PFALLOFF_SLOW:
    case MAINWIN_VIS_PFALLOFF_MEDIUM:
    case MAINWIN_VIS_PFALLOFF_FAST:
    case MAINWIN_VIS_PFALLOFF_FASTEST:
        mainwin_vis_set_pfalloff(action - MAINWIN_VIS_PFALLOFF_SLOWEST);
        break;
    }
}

void
mainwin_general_menu_callback(gpointer data,
                              guint action,
                              GtkWidget * item)
{
    switch (action) {
    case MAINWIN_GENERAL_PREFS:
        show_prefs_window();
        break;
    case MAINWIN_GENERAL_ABOUT:
        show_about_window();
        break;
    case MAINWIN_GENERAL_PLAYFILE:
        util_run_filebrowser(NO_PLAY_BUTTON);
        break;
    case MAINWIN_GENERAL_PLAYDIRECTORY:
        mainwin_run_dirbrowser();
        break;
    case MAINWIN_GENERAL_PLAYCD:
        play_medium();
        break;
    case MAINWIN_GENERAL_ADDCD:
        add_medium();
        break;
    case MAINWIN_GENERAL_PLAYLOCATION:
        mainwin_show_add_url_window();
        break;
    case MAINWIN_GENERAL_FILEINFO:
        playlist_fileinfo_current();
        break;
    case MAINWIN_GENERAL_FOCUSPLWIN:
        gtk_window_present(GTK_WINDOW(playlistwin));
        break;
    case MAINWIN_GENERAL_SHOWPLWIN:
        if (GTK_CHECK_MENU_ITEM(item)->active)
            playlistwin_show();
        else
            playlistwin_hide();
        break;
    case MAINWIN_GENERAL_SHOWEQWIN:
        if (GTK_CHECK_MENU_ITEM(item)->active)
            equalizerwin_real_show();
        else
            equalizerwin_real_hide();
        break;
    case MAINWIN_GENERAL_PREV:
        playlist_prev();
        break;
    case MAINWIN_GENERAL_PLAY:
        mainwin_play_pushed();
        break;
    case MAINWIN_GENERAL_PAUSE:
        bmp_playback_pause();
        break;
    case MAINWIN_GENERAL_STOP:
        mainwin_stop_pushed();
        break;
    case MAINWIN_GENERAL_NEXT:
        playlist_next();
        break;
    case MAINWIN_GENERAL_BACK5SEC:
        if (bmp_playback_get_playing()
            && playlist_get_current_length() != -1)
            bmp_playback_seek_relative(-5);
        break;
    case MAINWIN_GENERAL_FWD5SEC:
        if (bmp_playback_get_playing()
            && playlist_get_current_length() != -1)
            bmp_playback_seek_relative(5);
        break;
    case MAINWIN_GENERAL_START:
        playlist_set_position(0);
        break;
    case MAINWIN_GENERAL_JTT:
        mainwin_jump_to_time();
        break;
    case MAINWIN_GENERAL_JTF:
        mainwin_jump_to_file();
        break;
    case MAINWIN_GENERAL_EXIT:
        mainwin_quit_cb();
        break;
    }
}

static void
mainwin_mr_change(MenuRowItem i)
{
    switch (i) {
    case MENUROW_NONE:
        mainwin_set_info_text();
        break;
    case MENUROW_OPTIONS:
        mainwin_lock_info_text(_("OPTIONS MENU"));
        break;
    case MENUROW_ALWAYS:
        if (!hint_always_on_top_available()) {
            if (mainwin_menurow->mr_always_selected)
                mainwin_lock_info_text(_("DISABLE ALWAYS ON TOP (N/A)"));
            else
                mainwin_lock_info_text(_("ENABLE ALWAYS ON TOP (N/A)"));
        }
        else if (mainwin_menurow->mr_doublesize_selected)
            mainwin_lock_info_text(_("DISABLE ALWAYS ON TOP"));
        else
            mainwin_lock_info_text(_("ENABLE ALWAYS ON TOP"));
        break;
    case MENUROW_FILEINFOBOX:
        mainwin_lock_info_text(_("FILE INFO BOX"));
        break;
    case MENUROW_DOUBLESIZE:
        mainwin_lock_info_text(_("** DOUBLESIZE HAS BEEN REMOVED **"));
        break;
    case MENUROW_VISUALIZATION:
        mainwin_lock_info_text(_("VISUALIZATION MENU"));
        break;
    }
}

static void
mainwin_mr_release(MenuRowItem i)
{
    GdkModifierType modmask;
    GtkWidget *widget;
    gint x, y;

    switch (i) {
    case MENUROW_OPTIONS:
        gdk_window_get_pointer(NULL, &x, &y, &modmask);
        util_item_factory_popup(mainwin_view_menu, x, y, 1,
                                GDK_CURRENT_TIME);
        break;
    case MENUROW_ALWAYS:
        widget =
            gtk_item_factory_get_widget(mainwin_view_menu,
                                        "/Always On Top");
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(widget),
                                       mainwin_menurow->mr_always_selected);
        break;
    case MENUROW_FILEINFOBOX:
        playlist_fileinfo_current();
        break;
    case MENUROW_DOUBLESIZE:
        /* double size removed, do nothing */
        break;
    case MENUROW_VISUALIZATION:
        gdk_window_get_pointer(NULL, &x, &y, &modmask);
        util_item_factory_popup(mainwin_vis_menu, x, y, 1, GDK_CURRENT_TIME);
        break;
    case MENUROW_NONE:
        break;
    }
    mainwin_release_info_text();
}

static void
run_no_audiocd_dialog(void)
{
    const gchar *markup =
        N_("<b><big>No playable CD found.</big></b>\n\n"
           "No CD inserted, or inserted CD is not an audio CD.\n");

    GtkWidget *dialog =
        gtk_message_dialog_new_with_markup(GTK_WINDOW(mainwin),
                                           GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_MESSAGE_ERROR,
                                           GTK_BUTTONS_OK,
                                           _(markup));
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

static void
run_no_output_device_dialog(void)
{
    const gchar *markup =
        N_("<b><big>Couldn't open audio.</big></b>\n\n"
           "Please check that:\n"
           "1. You have the correct output plugin selected.\n"
           "2. No other programs is blocking the soundcard.\n"
           "3. Your soundcard is configured properly.\n");

    GtkWidget *dialog =
        gtk_message_dialog_new_with_markup(GTK_WINDOW(mainwin),
                                           GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_MESSAGE_ERROR,
                                           GTK_BUTTONS_OK,
                                           _(markup));
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}


void
add_medium(void)
{
    GList *list, *node;
    gchar *filename;
    gchar *path;
    ConfigDb *db;

    db = bmp_cfg_db_open();

    bmp_cfg_db_get_string(db, "CDDA", "directory", &path);
    bmp_cfg_db_close(db);

    if (!(list = input_scan_dir(path))) {
        run_no_audiocd_dialog();
        return;
    }

    for (node = list; node; node = g_list_next(node)) {
        filename = g_build_filename(path, node->data, NULL);
        playlist_add(filename);
        g_free(filename);
        g_free(node->data);
    }

    g_free(path);
    g_list_free(list);

}

void
play_medium(void)
{
    GList *list, *node;
    gchar *filename;
    gchar *path;
    ConfigDb *db;

    db = bmp_cfg_db_open();
    bmp_cfg_db_get_string(db, "CDDA", "directory", &path);
    bmp_cfg_db_close(db);

    if (!(list = input_scan_dir(path))) {
        run_no_audiocd_dialog();
        return;
    }

    playlist_clear();

    for (node = list; node; node = g_list_next(node)) {
        filename = g_build_filename(path, node->data, NULL);
        playlist_add(filename);
        g_free(filename);
        g_free(node->data);
    }

    g_free(path);
    g_list_free(list);

    playlist_set_position(0);
    bmp_playback_initiate();
}

void
read_volume(gint when)
{
    static gint pvl = 0, pvr = 0;
    static gint times = VOLSET_DISP_TIMES;
    static gboolean changing = FALSE;

    gint vl, vr, b, v;

    input_get_volume(&vl, &vr);

    switch (when) {
    case VOLSET_STARTUP:
        vl = CLAMP(vl, 0, 100);
        vr = CLAMP(vr, 0, 100);
        pvl = vl;
        pvr = vr;
        v = MAX(vl, vr);
        if (vl > vr)
            b = (gint) rint(((gdouble) vr / vl) * 100) - 100;
        else if (vl < vr)
            b = 100 - (gint) rint(((gdouble) vl / vr) * 100);
        else
            b = 0;

        balance = b;
        mainwin_set_volume_slider(v);
    //    equalizerwin_set_volume_slider(v);
        mainwin_set_balance_slider(b);
    //    equalizerwin_set_balance_slider(b);
        return;

    case VOLSET_UPDATE:
        if (vl == -1 || vr == -1)
            return;

        if (setting_volume) {
            pvl = vl;
            pvr = vr;
            return;
        }

        if (pvr == vr && pvl == vl && changing) {
            if (times < VOLSET_DISP_TIMES)
                times++;
            else {
                mainwin_release_info_text();
                changing = FALSE;
            }
        }
        else if (pvr != vr || pvl != vl) {
            gchar *tmp;

            v = MAX(vl, vr);
            if (vl > vr)
                b = (gint) rint(((gdouble) vr / vl) * 100) - 100;
            else if (vl < vr)
                b = 100 - (gint) rint(((gdouble) vl / vr) * 100);
            else
                b = 0;

            if (MAX(vl, vr) != MAX(pvl, pvr))
                tmp = g_strdup_printf(_("VOLUME: %d%%"), v);
            else {
                if (vl > vr) {
                    tmp = g_strdup_printf(_("BALANCE: %d%% LEFT"), -b);
                }
                else if (vr == vl)
                    tmp = g_strdup_printf(_("BALANCE: CENTER"));
                else {          /* (vl < vr) */
                    tmp = g_strdup_printf(_("BALANCE: %d%% RIGHT"), b);
                }
            }
            mainwin_lock_info_text(tmp);
            g_free(tmp);

            pvr = vr;
            pvl = vl;
            times = 0;
            changing = TRUE;
            mainwin_set_volume_slider(v);
       //     equalizerwin_set_volume_slider(v);

            /* Don't change the balance slider if the volume has been
             * set to zero.  The balance can be anything, and our best
             * guess is what is was before. */
            if (v > 0) {
                balance = b;
                mainwin_set_balance_slider(b);
       //         equalizerwin_set_balance_slider(b);
            }
        }
        break;

    case VOLUME_ADJUSTED:
        pvl = vl;
        pvr = vr;
        break;

    case VOLUME_SET:
        times = 0;
        changing = TRUE;
        pvl = vl;
        pvr = vr;
        break;
    }
}


/* TODO: HAL! */
gboolean
can_play_cd(void)
{
    GList *ilist;

    for (ilist = get_input_list(); ilist; ilist = g_list_next(ilist)) {
        InputPlugin *ip = INPUT_PLUGIN(ilist->data);

        if (!g_ascii_strcasecmp(g_basename(ip->filename),
                                PLUGIN_FILENAME("cdaudio"))) {
            return TRUE;
        }
    }

    return FALSE;
}


static void
set_timer_mode(TimerMode mode)
{
    if (mode == TIMER_ELAPSED)
        check_set(mainwin_view_menu, "/Time Elapsed", TRUE);
    else
        check_set(mainwin_view_menu, "/Time Remaining", TRUE);
}

static void
set_timer_mode_menu_cb(TimerMode mode)
{
    cfg.timer_mode = mode;
}


void
mainwin_setup_menus(void)
{
    set_timer_mode(cfg.timer_mode);

    /* View menu */

    check_set(mainwin_view_menu, "/Always On Top", cfg.always_on_top);
    check_set(mainwin_view_menu, "/Put on All Workspaces", cfg.sticky);
    check_set(mainwin_view_menu, "/Roll up Player", cfg.player_shaded);
    check_set(mainwin_view_menu, "/Roll up Playlist Editor", cfg.playlist_shaded);
    check_set(mainwin_view_menu, "/Roll up Equalizer", cfg.equalizer_shaded);

    /* Songname menu */

    check_set(mainwin_songname_menu, "/Autoscroll Songname", cfg.autoscroll);

    /* Playback menu */

    check_set(mainwin_play_menu, "/Repeat", cfg.repeat);
    check_set(mainwin_play_menu, "/Shuffle", cfg.shuffle);

    /* Visualization menu */

    check_set(mainwin_vis_menu,
              mainwin_vis_menu_entries[MAINWIN_VIS_MENU_VIS_MODE +
                                       cfg.vis_type].path, TRUE);
    check_set(mainwin_vis_menu,
              mainwin_vis_menu_entries[MAINWIN_VIS_MENU_ANALYZER_MODE +
                                       cfg.analyzer_mode].path, TRUE);
    check_set(mainwin_vis_menu,
              mainwin_vis_menu_entries[MAINWIN_VIS_MENU_ANALYZER_TYPE +
                                       cfg.analyzer_type].path, TRUE);
    check_set(mainwin_vis_menu,
              mainwin_vis_menu_entries[MAINWIN_VIS_MENU_ANALYZER_PEAKS].
              path, cfg.analyzer_peaks);
    check_set(mainwin_vis_menu,
              mainwin_vis_menu_entries[MAINWIN_VIS_MENU_SCOPE_MODE +
                                       cfg.scope_mode].path, TRUE);
    check_set(mainwin_vis_menu,
              mainwin_vis_menu_entries[MAINWIN_VIS_MENU_WSHADEVU_MODE +
                                       cfg.vu_mode].path, TRUE);
    check_set(mainwin_vis_menu,
              mainwin_vis_menu_entries[MAINWIN_VIS_MENU_REFRESH_RATE +
                                       cfg.vis_refresh].path, TRUE);
    check_set(mainwin_vis_menu,
              mainwin_vis_menu_entries[MAINWIN_VIS_MENU_AFALLOFF +
                                       cfg.analyzer_falloff].path, TRUE);
    check_set(mainwin_vis_menu,
              mainwin_vis_menu_entries[MAINWIN_VIS_MENU_PFALLOFF +
                                       cfg.peaks_falloff].path, TRUE);
}

static void
mainwin_create_widgets(void)
{
    mainwin_menubtn =
        create_pbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 6, 3, 9, 9,
                       0, 0, 0, 9, mainwin_menubtn_cb, SKIN_TITLEBAR);
    mainwin_menubtn->pb_allow_draw = FALSE;
    mainwin_minimize =
        create_pbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 244, 3, 9,
                       9, 9, 0, 9, 9, mainwin_minimize_cb, SKIN_TITLEBAR);
    mainwin_minimize->pb_allow_draw = FALSE;
    mainwin_shade =
        create_pbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 254, 3, 9,
                       9, 0, cfg.player_shaded ? 27 : 18, 9,
                       cfg.player_shaded ? 27 : 18, mainwin_shade_toggle,
                       SKIN_TITLEBAR);
    mainwin_shade->pb_allow_draw = FALSE;
    mainwin_close =
        create_pbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 264, 3, 9,
                       9, 18, 0, 18, 9, mainwin_quit_cb, SKIN_TITLEBAR);
    mainwin_close->pb_allow_draw = FALSE;

    mainwin_rew =
        create_pbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 16, 88, 23,
                       18, 0, 0, 0, 18, playlist_prev, SKIN_CBUTTONS);
    mainwin_play =
        create_pbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 39, 88, 23,
                       18, 23, 0, 23, 18, mainwin_play_pushed, SKIN_CBUTTONS);
    mainwin_pause =
        create_pbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 62, 88, 23,
                       18, 46, 0, 46, 18, bmp_playback_pause, SKIN_CBUTTONS);
    mainwin_stop =
        create_pbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 85, 88, 23,
                       18, 69, 0, 69, 18, mainwin_stop_pushed, SKIN_CBUTTONS);
    mainwin_fwd =
        create_pbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 108, 88, 22,
                       18, 92, 0, 92, 18, playlist_next, SKIN_CBUTTONS);
    mainwin_eject =
        create_pbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 136, 89, 22,
                       16, 114, 0, 114, 16, mainwin_eject_pushed,
                       SKIN_CBUTTONS);

    mainwin_srew =
        create_sbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 169, 4, 8,
                       7, playlist_prev);
    mainwin_splay =
        create_sbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 177, 4, 10,
                       7, mainwin_play_pushed);
    mainwin_spause =
        create_sbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 187, 4, 10,
                       7, bmp_playback_pause);
    mainwin_sstop =
        create_sbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 197, 4, 9,
                       7, mainwin_stop_pushed);
    mainwin_sfwd =
        create_sbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 206, 4, 8,
                       7, playlist_next);
    mainwin_seject =
        create_sbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 216, 4, 9,
                       7, mainwin_eject_pushed);

    mainwin_shuffle =
        create_tbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 164, 89, 46,
                       15, 28, 0, 28, 15, 28, 30, 28, 45,
                       mainwin_shuffle_pushed, SKIN_SHUFREP);

    mainwin_repeat =
        create_tbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 210, 89, 28,
                       15, 0, 0, 0, 15, 0, 30, 0, 45,
                       mainwin_repeat_pushed, SKIN_SHUFREP);

    mainwin_eq =
        create_tbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 219, 58, 23,
                       12, 0, 61, 46, 61, 0, 73, 46, 73, equalizerwin_show,
                       SKIN_SHUFREP);
    tbutton_set_toggled(mainwin_eq, cfg.equalizer_visible);
    mainwin_pl =
        create_tbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 242, 58, 23,
                       12, 23, 61, 69, 61, 23, 73, 69, 73,
                       mainwin_pl_pushed, SKIN_SHUFREP);
    tbutton_set_toggled(mainwin_pl, cfg.playlist_visible);

    mainwin_info =
        create_textbox(&mainwin_wlist, mainwin_bg, mainwin_gc, 112, 27,
                       153, 1, SKIN_TEXT);
    textbox_set_scroll(mainwin_info, cfg.autoscroll);
    textbox_set_xfont(mainwin_info, TRUE, cfg.mainwin_font);
    mainwin_rate_text =
        create_textbox(&mainwin_wlist, mainwin_bg, mainwin_gc, 111, 43, 15,
                       0, SKIN_TEXT);
    mainwin_freq_text =
        create_textbox(&mainwin_wlist, mainwin_bg, mainwin_gc, 156, 43, 10,
                       0, SKIN_TEXT);

    mainwin_menurow =
        create_menurow(&mainwin_wlist, mainwin_bg, mainwin_gc, 10, 22, 304,
                       0, 304, 44, mainwin_mr_change, mainwin_mr_release,
                       SKIN_TITLEBAR);
    mainwin_menurow->mr_doublesize_selected = FALSE;
    mainwin_menurow->mr_always_selected = cfg.always_on_top;

    mainwin_volume =
        create_hslider(&mainwin_wlist, mainwin_bg, mainwin_gc, 107, 57, 68,
                       13, 15, 422, 0, 422, 14, 11, 15, 0, 0, 51,
                       mainwin_volume_frame_cb, mainwin_volume_motion_cb,
                       mainwin_volume_release_cb, SKIN_VOLUME);
    mainwin_balance =
        create_hslider(&mainwin_wlist, mainwin_bg, mainwin_gc, 177, 57, 38,
                       13, 15, 422, 0, 422, 14, 11, 15, 9, 0, 24,
                       mainwin_balance_frame_cb, mainwin_balance_motion_cb,
                       mainwin_balance_release_cb, SKIN_BALANCE);

    mainwin_monostereo =
        create_monostereo(&mainwin_wlist, mainwin_bg, mainwin_gc, 212, 41,
                          SKIN_MONOSTEREO);

    mainwin_playstatus =
        create_playstatus(&mainwin_wlist, mainwin_bg, mainwin_gc, 24, 28);

    mainwin_minus_num =
        create_number(&mainwin_wlist, mainwin_bg, mainwin_gc, 36, 26,
                      SKIN_NUMBERS);
    widget_hide(WIDGET(mainwin_minus_num));
    mainwin_10min_num =
        create_number(&mainwin_wlist, mainwin_bg, mainwin_gc, 48, 26,
                      SKIN_NUMBERS);
    widget_hide(WIDGET(mainwin_10min_num));

    mainwin_min_num =
        create_number(&mainwin_wlist, mainwin_bg, mainwin_gc, 60, 26,
                      SKIN_NUMBERS);
    widget_hide(WIDGET(mainwin_min_num));

    mainwin_10sec_num =
        create_number(&mainwin_wlist, mainwin_bg, mainwin_gc, 78, 26,
                      SKIN_NUMBERS);
    widget_hide(WIDGET(mainwin_10sec_num));

    mainwin_sec_num =
        create_number(&mainwin_wlist, mainwin_bg, mainwin_gc, 90, 26,
                      SKIN_NUMBERS);
    widget_hide(WIDGET(mainwin_sec_num));

    mainwin_about =
        create_sbutton(&mainwin_wlist, mainwin_bg, mainwin_gc, 247, 83, 20,
                       25, show_about_window);

    mainwin_vis =
        create_vis(&mainwin_wlist, mainwin_bg, mainwin->window, mainwin_gc,
                   24, 43, 76);
    mainwin_svis = create_svis(&mainwin_wlist, mainwin_bg, mainwin_gc, 79, 5);
    active_vis = mainwin_vis;

    mainwin_position =
        create_hslider(&mainwin_wlist, mainwin_bg, mainwin_gc, 16, 72, 248,
                       10, 248, 0, 278, 0, 29, 10, 10, 0, 0, 219, NULL,
                       mainwin_position_motion_cb,
                       mainwin_position_release_cb, SKIN_POSBAR);
    widget_hide(WIDGET(mainwin_position));

    mainwin_sposition =
        create_hslider(&mainwin_wlist, mainwin_bg, mainwin_gc, 226, 4, 17,
                       7, 17, 36, 17, 36, 3, 7, 36, 0, 1, 13,
                       mainwin_spos_frame_cb, mainwin_spos_motion_cb,
                       mainwin_spos_release_cb, SKIN_TITLEBAR);
    widget_hide(WIDGET(mainwin_sposition));

    mainwin_stime_min =
        create_textbox(&mainwin_wlist, mainwin_bg, mainwin_gc, 130, 4, 15,
                       FALSE, SKIN_TEXT);
    mainwin_stime_sec =
        create_textbox(&mainwin_wlist, mainwin_bg, mainwin_gc, 147, 4, 10,
                       FALSE, SKIN_TEXT);

    if (!cfg.player_shaded) {
        widget_hide(WIDGET(mainwin_svis));
        widget_hide(WIDGET(mainwin_srew));
        widget_hide(WIDGET(mainwin_splay));
        widget_hide(WIDGET(mainwin_spause));
        widget_hide(WIDGET(mainwin_sstop));
        widget_hide(WIDGET(mainwin_sfwd));
        widget_hide(WIDGET(mainwin_seject));
        widget_hide(WIDGET(mainwin_stime_min));
        widget_hide(WIDGET(mainwin_stime_sec));
    }

}

static void
mainwin_create_window(void)
{
    gint width, height;

    mainwin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(mainwin), _("BMP"));
    gtk_window_set_role(GTK_WINDOW(mainwin), "player");
    gtk_window_set_resizable(GTK_WINDOW(mainwin), FALSE);

    width = MAINWIN_WIDTH;
    height = cfg.player_shaded ? MAINWIN_SHADED_HEIGHT : MAINWIN_HEIGHT;

    gtk_widget_set_size_request(mainwin, width, height);
    gtk_widget_set_app_paintable(mainwin, TRUE);

    dock_window_list = dock_window_set_decorated(dock_window_list,
                                                 GTK_WINDOW(mainwin),
                                                 cfg.show_wm_decorations);

    if (cfg.player_x != -1 && cfg.save_window_position)
        gtk_window_move(GTK_WINDOW(mainwin), cfg.player_x, cfg.player_y);

    gtk_widget_add_events(mainwin,
                          GDK_FOCUS_CHANGE_MASK | GDK_BUTTON_MOTION_MASK |
                          GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK |
                          GDK_SCROLL_MASK | GDK_KEY_PRESS_MASK |
                          GDK_VISIBILITY_NOTIFY_MASK);
    gtk_widget_realize(mainwin);

    util_set_cursor(mainwin);

    g_signal_connect(mainwin, "destroy", G_CALLBACK(mainwin_destroy), NULL);
    g_signal_connect(mainwin, "button_press_event",
                     G_CALLBACK(mainwin_mouse_button_press), NULL);
    g_signal_connect(mainwin, "scroll_event",
                     G_CALLBACK(mainwin_scrolled), NULL);
    g_signal_connect(mainwin, "button_release_event",
                     G_CALLBACK(mainwin_mouse_button_release), NULL);
    g_signal_connect(mainwin, "motion_notify_event",
                     G_CALLBACK(mainwin_motion), NULL);
    g_signal_connect_after(mainwin, "focus_in_event",
                           G_CALLBACK(mainwin_focus_in), NULL);
    g_signal_connect_after(mainwin, "focus_out_event",
                           G_CALLBACK(mainwin_focus_out), NULL);
    g_signal_connect(mainwin, "configure_event",
                     G_CALLBACK(mainwin_configure), NULL);
    g_signal_connect(mainwin, "style_set",
                     G_CALLBACK(mainwin_set_back_pixmap), NULL);

    bmp_drag_dest_set(mainwin);

    g_signal_connect(mainwin, "key_press_event",
                     G_CALLBACK(mainwin_keypress), NULL);
}

static void
mainwin_create_menus(void)
{
    mainwin_general_menu = create_menu(mainwin_general_menu_entries,
                                       mainwin_general_menu_entries_num,
                                       mainwin_accel);

    mainwin_play_menu = create_menu(mainwin_playback_menu_entries,
                                    mainwin_playback_menu_entries_num,
                                    mainwin_accel);

    mainwin_view_menu = create_menu(mainwin_view_menu_entries,
                                    mainwin_view_menu_entries_num,
                                    mainwin_accel);

    mainwin_songname_menu = create_menu(mainwin_songname_menu_entries,
                                        mainwin_songname_menu_entries_num,
                                        mainwin_accel);

    mainwin_add_menu = create_menu(mainwin_add_menu_entries,
                                   mainwin_add_menu_entries_num,
                                   mainwin_accel);

    mainwin_vis_menu = create_menu(mainwin_vis_menu_entries,
                                   mainwin_vis_menu_entries_num,
                                   mainwin_accel);

    make_submenu(mainwin_general_menu, "/View", mainwin_view_menu);

    gtk_window_add_accel_group(GTK_WINDOW(mainwin), mainwin_accel);
}

void
mainwin_create(void)
{
    mainwin_create_window();

    mainwin_accel = gtk_accel_group_new();
    mainwin_create_menus();

    mainwin_gc = gdk_gc_new(mainwin->window);
    mainwin_bg = gdk_pixmap_new(mainwin->window,
                                MAINWIN_WIDTH, MAINWIN_HEIGHT, -1);
    mainwin_set_back_pixmap();
    mainwin_create_widgets();

    vis_set_window(mainwin_vis, mainwin->window);
}

void
mainwin_attach_idle_func(void)
{
    mainwin_timeout_id = g_timeout_add(MAINWIN_UPDATE_INTERVAL,
                                       mainwin_idle_func, NULL);
}

static gboolean
idle_func_change_song(gboolean waiting)
{
    static GTimer *pause_timer = NULL;

    if (!pause_timer)
        pause_timer = g_timer_new();

    if (cfg.pause_between_songs) {
        gint timeleft;

        if (!waiting) {
            g_timer_start(pause_timer);
            waiting = TRUE;
        }

        timeleft = cfg.pause_between_songs_time -
            (gint) g_timer_elapsed(pause_timer, NULL);

//        number_set_number(mainwin_10min_num, timeleft / 600);
//        number_set_number(mainwin_min_num, (timeleft / 60) % 10);
//        number_set_number(mainwin_10sec_num, (timeleft / 10) % 6);
//        number_set_number(mainwin_sec_num, timeleft % 10);
/*
        if (!mainwin_sposition->hs_pressed) {
            gchar time_str[5];

            g_snprintf(time_str, sizeof(time_str), "%2.2d", timeleft / 60);
            textbox_set_text(mainwin_stime_min, time_str);

            g_snprintf(time_str, sizeof(time_str), "%2.2d", timeleft % 60);
            textbox_set_text(mainwin_stime_sec, time_str);
        }
*/
//        playlistwin_set_time(timeleft * 1000, 0, TIMER_ELAPSED);
    }

    if (!cfg.pause_between_songs ||
        g_timer_elapsed(pause_timer, NULL) >= cfg.pause_between_songs_time) {

        GDK_THREADS_ENTER();
        playlist_eof_reached();
        GDK_THREADS_LEAVE();

        waiting = FALSE;
    }

    return waiting;
}

static void
idle_func_update_song_info(gint time)
{
    gint length, t;
    gchar stime_prefix;

    length = playlist_get_current_length();
//    playlistwin_set_time(time, length, cfg.timer_mode);
//    input_update_vis(time);

    if (cfg.timer_mode == TIMER_REMAINING) {
        if (length != -1) {
//            number_set_number(mainwin_minus_num, 11);
            t = length - time;
//            stime_prefix = '-';
        }
        else {
//            number_set_number(mainwin_minus_num, 10);
            t = time;
//            stime_prefix = ' ';
        }
    }
    else {
//        number_set_number(mainwin_minus_num, 10);
        t = time;
//        stime_prefix = ' ';
    }
    t /= 1000;

    /* Show the time in the format HH:MM when we have more than 100
     * minutes. */
    if (t >= 100 * 60)
        t /= 60;
//    number_set_number(mainwin_10min_num, t / 600);
//    number_set_number(mainwin_min_num, (t / 60) % 10);
//    number_set_number(mainwin_10sec_num, (t / 10) % 6);
//    number_set_number(mainwin_sec_num, t % 10);
/*
    if (!mainwin_sposition->hs_pressed) {
        gchar *time_str;

        time_str = g_strdup_printf("%c%2.2d", stime_prefix, t / 60);
        textbox_set_text(mainwin_stime_min, time_str);
        g_free(time_str);

        time_str = g_strdup_printf("%2.2d", t % 60);
        textbox_set_text(mainwin_stime_sec, time_str);
        g_free(time_str);
    }
*/
    time /= 1000;
    length /= 1000;
    if (length > 0) {
        if (time > length) {
//            hslider_set_position(mainwin_position, 219);
//            hslider_set_position(mainwin_sposition, 13);
        }
        else {
//            hslider_set_position(mainwin_position, (time * 219) / length);
//            hslider_set_position(mainwin_sposition,
//                                 ((time * 12) / length) + 1);
        }
    }
    else {
//        hslider_set_position(mainwin_position, 0);
//        hslider_set_position(mainwin_sposition, 1);
    }
}


static gboolean
mainwin_idle_func(gpointer data)
{
    static gboolean waiting = FALSE;
    static gint count = 0;

    gint time;

    if (bmp_playback_get_playing()) {
//        GDK_THREADS_ENTER();
//        vis_playback_start();
//        GDK_THREADS_LEAVE();

        time = bmp_playback_get_time();

	g_print("\n time = %s\n", time);

        switch (time) {
        case -1:
            /* no song playing */
            waiting = idle_func_change_song(waiting);
            break;

        case -2:
            /* no usable output device */
            GDK_THREADS_ENTER();
            run_no_output_device_dialog();
            mainwin_stop_pushed();
            GDK_THREADS_LEAVE();
            waiting = FALSE;
            break;

        default:
            /* song playing, all's well */
            idle_func_update_song_info(time);
            waiting = FALSE;
        }
    }
    else {
//        GDK_THREADS_ENTER();
//        vis_playback_stop();
//        GDK_THREADS_LEAVE();
    }

    GDK_THREADS_ENTER();

//    ctrlsocket_check();

//    draw_main_window(mainwin_force_redraw);

    if (!count) {
        read_volume(VOLSET_UPDATE);
        count = 10;
    }
    else
        count--;

//    mainwin_force_redraw = FALSE;
//    draw_playlist_window(FALSE);
//    draw_equalizer_window(FALSE);

//    if (mainwin_title_text) {
//       G_LOCK(mainwin_title);
//       gtk_window_set_title(GTK_WINDOW(mainwin), mainwin_title_text);
//        g_free(mainwin_title_text);
//        mainwin_title_text = NULL;
//        G_UNLOCK(mainwin_title);

//        mainwin_set_info_text();
//        playlistwin_update_list();
//    }

    GDK_THREADS_LEAVE();

    return TRUE;
}

static gchar* images_dir = NULL;
static gchar* image_path = NULL;

static gboolean playorpause = FALSE;
GtkButton *play_pause_button = NULL;
GtkWidget* shuffle_button = NULL;
GtkWidget* repeat_button = NULL;
GtkWidget *image = NULL;
GtkWidget *vol_image = NULL;
GtkWidget* title_label = NULL;
GtkWidget* artist_label = NULL;
GtkWidget* track_number_label = NULL;
GtkWidget* total_number_label = NULL;
GtkWidget* elapse_time_label = NULL;
GtkWidget* total_time_label = NULL;
GtkWidget* volume_label = NULL;
GtkWidget* vol_img_alignment = NULL;
GtkWidget* time_hscale = NULL;
gint timeout_id;
static gint timeout_time = 0;

GtkWidget* alignment_eq_pos1 = NULL;
GtkWidget* alignment_eq_pos2 = NULL;
GtkWidget* alignment_eq_pos3 = NULL;
GtkWidget* alignment_eq_pos4 = NULL;
GtkWidget* alignment_eq_pos5 = NULL;
GtkWidget* alignment_eq_pos6 = NULL;
GtkWidget* alignment_eq_pos7 = NULL;
GtkWidget* alignment_eq_pos8 = NULL;
GtkWidget* alignment_eq_pos9 = NULL;
GtkWidget* alignment_eq_pos10 = NULL;
GtkWidget* alignment_eq_pos11 = NULL;
GtkWidget* alignment_eq_pos12 = NULL;

GtkWidget* image_eq_pos1 = NULL;
GtkWidget* image_eq_pos2 = NULL;
GtkWidget* image_eq_pos3 = NULL;
GtkWidget* image_eq_pos4 = NULL;
GtkWidget* image_eq_pos5 = NULL;
GtkWidget* image_eq_pos6 = NULL;
GtkWidget* image_eq_pos7 = NULL;
GtkWidget* image_eq_pos8 = NULL;
GtkWidget* image_eq_pos9 = NULL;
GtkWidget* image_eq_pos10 = NULL;
GtkWidget* image_eq_pos11 = NULL;
GtkWidget* image_eq_pos12 = NULL;

void
openmoko_player_quit(GtkWidget *widget, gpointer data)
{
    g_free(images_dir);
    mainwin_quit_cb();
}

void 
openmoko_read_volume_from_start()
{
    gint vol_left, vol_right, vol;
    ConfigDb* db;
    db = bmp_cfg_db_open();
    bmp_cfg_db_get_int(db, "ALSA", "volume_left", &vol_left);
    bmp_cfg_db_get_int(db, "ALSA", "volume_right", &vol_right);
    vol_left = CLAMP(vol_left, 0, 100);
    vol_right = CLAMP(vol_right, 0, 100);
    bmp_cfg_db_close(db);
    vol = MAX(vol_left, vol_right);
    gchar* vol_str = g_strdup_printf("%d", vol);
    gtk_label_set_text(GTK_LABEL(volume_label), vol_str);
    vol_str = g_strdup_printf("%s/ind-music-volume-%02d.png", images_dir,vol/10);
    gtk_container_remove(GTK_CONTAINER(vol_img_alignment), vol_image);
    vol_image = gtk_image_new_from_file(vol_str);
    gtk_container_add(GTK_CONTAINER(vol_img_alignment), vol_image);
    g_free(vol_str);
    
}

void
openmoko_change_vol_img(gint vol)
{
    gchar* vol_str;
    vol_str = g_strdup_printf("%s/ind-music-volume-%02d.png", images_dir, vol/10);
    gtk_container_remove(GTK_CONTAINER(vol_img_alignment), vol_image);
    vol_image = gtk_image_new_from_file(vol_str);
    gtk_container_add(GTK_CONTAINER(vol_img_alignment), vol_image);
    gtk_widget_show(vol_image);
    g_free(vol_str);
}

void 
openmoko_update_vis_data(gint pos, gint h)
{
    if(h < 1 || h > 12)
        return;

    if(h < 0)
	return;
    
    gint eq = 0;
    gchar* img_path = NULL;
    
    switch(h)
    {
    case 0:
    case 1:
    case 2:
	eq = 1;
	break;
    case 3:
    case 4:
	eq = 2;
	break;
    case 5:
    case 6:
	eq = 3;
	break;
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
	eq = h - 3;
	break;
    default:
	eq = 12;
	break;
    }
	
    img_path = g_strdup_printf("%s/ind-music-eq-%02d.png", images_dir, eq);
    
    switch(pos)
    {
    case 1:
	//gtk_container_remove(GTK_CONTAINER(alignment_eq_pos1), image_eq_pos1);
	gtk_image_set_from_file(GTK_IMAGE(image_eq_pos1), img_path);
	//image_eq_pos1 = gtk_image_new_from_file(img_path);
	//gtk_container_add(GTK_CONTAINER(alignment_eq_pos1), image_eq_pos1);
	gtk_widget_show(image_eq_pos1);
	break;
    case 2:
	//gtk_container_remove(GTK_CONTAINER(alignment_eq_pos2), image_eq_pos2);
	gtk_image_set_from_file(GTK_IMAGE(image_eq_pos2), img_path);
	//image_eq_pos2 = gtk_image_new_from_file(img_path);
	//gtk_container_add(GTK_CONTAINER(alignment_eq_pos2), image_eq_pos2);
	gtk_widget_show(image_eq_pos2);
	break;
    case 3:
	//gtk_container_remove(GTK_CONTAINER(alignment_eq_pos3), image_eq_pos3);
	gtk_image_set_from_file(GTK_IMAGE(image_eq_pos3), img_path);
	//image_eq_pos3 = gtk_image_new_from_file(img_path);
	//gtk_container_add(GTK_CONTAINER(alignment_eq_pos3), image_eq_pos3);
	gtk_widget_show(image_eq_pos3);
	break;
    case 4:
	//gtk_container_remove(GTK_CONTAINER(alignment_eq_pos4), image_eq_pos4);
	gtk_image_set_from_file(GTK_IMAGE(image_eq_pos4), img_path);
	//image_eq_pos4 = gtk_image_new_from_file(img_path);
	//gtk_container_add(GTK_CONTAINER(alignment_eq_pos4), image_eq_pos4);
	gtk_widget_show(image_eq_pos4);
	break;
    case 5:
	//gtk_container_remove(GTK_CONTAINER(alignment_eq_pos5), image_eq_pos5);
	gtk_image_set_from_file(GTK_IMAGE(image_eq_pos5), img_path);
	//image_eq_pos5 = gtk_image_new_from_file(img_path);
	//gtk_container_add(GTK_CONTAINER(alignment_eq_pos5), image_eq_pos5);
	gtk_widget_show(image_eq_pos5);
	break;
    case 6:
	//gtk_container_remove(GTK_CONTAINER(alignment_eq_pos6), image_eq_pos6);
	gtk_image_set_from_file(GTK_IMAGE(image_eq_pos6), img_path);
	//image_eq_pos6 = gtk_image_new_from_file(img_path);
	//gtk_container_add(GTK_CONTAINER(alignment_eq_pos6), image_eq_pos6);
	gtk_widget_show(image_eq_pos6);
	break;
    case 7:
	//gtk_container_remove(GTK_CONTAINER(alignment_eq_pos7), image_eq_pos7);
	gtk_image_set_from_file(GTK_IMAGE(image_eq_pos7), img_path);
	//image_eq_pos7 = gtk_image_new_from_file(img_path);
	//gtk_container_add(GTK_CONTAINER(alignment_eq_pos7), image_eq_pos7);
	gtk_widget_show(image_eq_pos7);
	break;
    case 8:
	//gtk_container_remove(GTK_CONTAINER(alignment_eq_pos8), image_eq_pos8);
	gtk_image_set_from_file(GTK_IMAGE(image_eq_pos8), img_path);
	//image_eq_pos8 = gtk_image_new_from_file(img_path);
	//gtk_container_add(GTK_CONTAINER(alignment_eq_pos8), image_eq_pos8);
	gtk_widget_show(image_eq_pos8);
	break;
    case 9:
	//gtk_container_remove(GTK_CONTAINER(alignment_eq_pos9), image_eq_pos9);
	gtk_image_set_from_file(GTK_IMAGE(image_eq_pos9), img_path);
	//image_eq_pos9 = gtk_image_new_from_file(img_path);
	//gtk_container_add(GTK_CONTAINER(alignment_eq_pos9), image_eq_pos9);
	gtk_widget_show(image_eq_pos9);
	break;
    case 10:
	//gtk_container_remove(GTK_CONTAINER(alignment_eq_pos10), image_eq_pos10);
	gtk_image_set_from_file(GTK_IMAGE(image_eq_pos10), img_path);
	//image_eq_pos10 = gtk_image_new_from_file(img_path);
	//gtk_container_add(GTK_CONTAINER(alignment_eq_pos10), image_eq_pos10);
	gtk_widget_show(image_eq_pos10);
	break;
    case 11:
	//gtk_container_remove(GTK_CONTAINER(alignment_eq_pos11), image_eq_pos11);
	gtk_image_set_from_file(GTK_IMAGE(image_eq_pos11), img_path);
	//image_eq_pos11 = gtk_image_new_from_file(img_path);
	//gtk_container_add(GTK_CONTAINER(alignment_eq_pos11), image_eq_pos11);
	gtk_widget_show(image_eq_pos11);
	break;
    case 12:
	//gtk_container_remove(GTK_CONTAINER(alignment_eq_pos12), image_eq_pos12);
	gtk_image_set_from_file(GTK_IMAGE(image_eq_pos12), img_path);
	//image_eq_pos12 = gtk_image_new_from_file(img_path);
	//gtk_container_add(GTK_CONTAINER(alignment_eq_pos12), image_eq_pos12);
	gtk_widget_show(image_eq_pos12);
	break;
    default:
	break;
    }   

    g_free(img_path);
}

void
openmoko_set_tag_info()
{
    gint pos;
    gchar* filename;
    pos = playlist_get_position();
    filename = playlist_get_filename(pos);
    
    if(!filename)
    {
	openmoko_set_title(NULL);
	openmoko_set_artist(NULL);
	return;
    }
    
    char* surfix = strrchr(filename, '.');
    if(!strcmp(surfix+1, "mp3"))
    {
        ID3Tag *id3tag;
        ID3Frame *frame;
        ID3Field *text_field;

        id3tag = ID3Tag_New();
        ID3Tag_Link(id3tag, filename);
        if(ID3Tag_HasTagType(id3tag, ID3TT_ID3V2))
        {
            ID3Tag_Clear(id3tag);
	    ID3Tag_LinkWithFlags(id3tag, filename, ID3TT_ID3V2);
        }
    
        frame = ID3Tag_FindFrameWithID(id3tag, ID3FID_TITLE);
        if(frame)
        {
            gchar info[4096];
	    text_field = ID3Frame_GetField(frame, ID3FN_TEXT);
	    ID3Field_GetASCII(text_field, info, 4096);
	    openmoko_set_title(info);
        }
        else
            openmoko_set_title(NULL);
    
        frame = ID3Tag_FindFrameWithID(id3tag, ID3FID_LEADARTIST);
        if(frame)
        {
            gchar info[4096];
	    text_field = ID3Frame_GetField(frame, ID3FN_TEXT);
	    ID3Field_GetASCII(text_field, info, 4096);
	    openmoko_set_artist(info);
        }
        else
            openmoko_set_artist(NULL);

	return;
    }
    else if(!strcmp(surfix+1, "ogg"))
    {
	return;
    }
    else if(!strcmp(surfix+1, "mp4") || !strcmp(surfix+1, "m4a") || !strcmp(surfix+1, "aac"))
    {
    /*    
        MP4FileHandle mp4file;
	mp4file = MP4Modify(filename, 0, 0);
	if(mp4file == MP4_INVALID_FILE_HANDLE)
	{
	    g_print("Can't open %s...aborting\n", filename);
            openmoko_set_title(NULL);
	    openmoko_set_artist(NULL);
	    return;
	}
	
	gchar* title;
	gchar* artist;
	MP4GetMetadataName(mp4file, &title);
	MP4GetMetadataArtist(mp4file, &artist);
	if(title)
	    openmoko_set_title(title);
	else
            openmoko_set_title(NULL);
	if(artist)
	    openmoko_set_artist(artist);
	else
	    openmoko_set_artist(NULL);
	
	g_free(title);
	g_free(artist);
	MP4Close(mp4file);
    */	
	return;
    }
    else
    {
 	g_print("current file is %s file\n", surfix+1);
	openmoko_set_title(NULL);
	openmoko_set_artist(NULL);
	return;	
    }		   
}

void 
openmoko_update_ogg_title(const gchar* title)
{
    if(title)
        openmoko_set_title(title);
    else
	openmoko_set_title(NULL);
}

void 
openmoko_update_ogg_artist(const gchar* artist)
{
    if(artist)
        openmoko_set_artist(artist);
    else
	openmoko_set_artist(NULL);
}

void
openmoko_set_title(gchar *title)
{
    if(!title)
    {
        gtk_label_set_text(GTK_LABEL(title_label), "Unknown Title");
	return;
    }
    gtk_label_set_text(GTK_LABEL(title_label), title);
}

void
openmoko_set_artist(gchar* artist)
{
    if(!artist)
    {
        gtk_label_set_text(GTK_LABEL(artist_label), "Unknown Artist");
        return;
    }
    gtk_label_set_text(GTK_LABEL(artist_label), artist);
}

void
openmoko_set_track_number()
{
    gchar* track_number;
    track_number = g_strdup_printf("%d", playlist_get_position() + 1);
    gtk_label_set_text(GTK_LABEL(track_number_label), track_number);
    g_free(track_number);
}
	
void
openmoko_set_total_number()
{
    gchar* total_number;
    total_number = g_strdup_printf("%d", playlist_get_length());
    gtk_label_set_text(GTK_LABEL(total_number_label), total_number);
    g_free(total_number);
}

void
openmoko_set_elapse_time(gint elapse_time)
{
    gint pos, time;
    pos = playlist_get_position();
    time = playlist_get_songtime(pos);
    if(elapse_time > time)
	return;
    gchar* elapse_str;
    elapse_str = g_strdup_printf("%d:%-2.2d", elapse_time/60000, (elapse_time/1000)%60);
    gtk_label_set_text(GTK_LABEL(elapse_time_label), elapse_str);
    g_free(elapse_str);
}
	
void
openmoko_set_total_time()
{
    gint pos, time;
    gchar* total_time;
    pos = playlist_get_position();
    time = playlist_get_songtime(pos);
    total_time = g_strdup_printf("%d:%-2.2d", time/60000, (time/1000)%60);
    gtk_label_set_text(GTK_LABEL(total_time_label), total_time);
    g_free(total_time);
}

void 
openmoko_set_info()
{
    openmoko_set_track_number();
    openmoko_set_total_time();
    openmoko_set_tag_info();
}

gboolean
openmoko_update_elapse_time(gpointer data)
{
    timeout_time = bmp_playback_get_time();
    
    if(timeout_time == -1)
    {
	g_usleep(500);
	idle_func_change_song(FALSE);
	openmoko_set_info();
	return TRUE;
    }
   
    input_update_vis(timeout_time);
    
    openmoko_set_elapse_time(timeout_time);

    gint length;
    length = playlist_get_current_length();
    timeout_time /= 1000;
    length /= 1000;
    if(length > 0)
    {
	if(timeout_time > length)
	{
	    gtk_range_set_value(GTK_RANGE(time_hscale), 338.0);       
	}
	else
	{
	    gtk_range_set_value(GTK_RANGE(time_hscale), (gdouble)((timeout_time * 338) / length));
	}
    }
    else
    {
	gtk_range_set_value(GTK_RANGE(time_hscale), 0.0);       
    }
 
    return TRUE;
}

void 
openmoko_play_pause_action()
{
    if(playlist_get_length() == 0)
	return;
	
    if(!playorpause)
    {
        mainwin_play_pushed();
        image_path = g_build_path("/", images_dir, "ico-pause.png", NULL);
        image = gtk_image_new_from_file(image_path);
	g_free(image_path);
        moko_pixmap_button_set_finger_toolbox_btn_center_image(play_pause_button, image);
	playorpause = TRUE;
        timeout_id = g_timeout_add(100, openmoko_update_elapse_time, NULL);
	openmoko_set_info();
    }
    else
    {
        bmp_playback_pause();
        image_path = g_build_path("/", images_dir, "ico-play.png", NULL);
        image = gtk_image_new_from_file(image_path);
	g_free(image_path);
        moko_pixmap_button_set_finger_toolbox_btn_center_image(play_pause_button, image);
	playorpause = FALSE;
	g_source_remove(timeout_id);
    }
}

void
openmoko_play_pause_button_pushed(GtkWidget *widget, gpointer data)
{
    openmoko_play_pause_action();
}

void 
openmoko_playlist_prev_action()
{
    playlist_prev();
    openmoko_set_info();
}

void 
openmoko_playlist_prev(GtkWidget *widget, gpointer data)
{
    openmoko_playlist_prev_action();
}

void 
openmoko_playlist_next_action()
{
    playlist_next();
    openmoko_set_info();
}

void 
openmoko_playlist_next(GtkWidget *widget, gpointer data)
{
    openmoko_playlist_next_action();
}

void 
openmoko_increase_volume()
{
    gint vl, vr, orignal_vol,vol;
    input_get_volume(&vl, &vr);
    orignal_vol = MAX(vl, vr);
    if(vl < 100 && vr < 100)
    {
	vl++;
	vr++;
    }
    else
	return;
    input_set_volume(vl, vr);
    vol = MAX(vl, vr);
    gchar* vol_str = g_strdup_printf("%d", vol);
    gtk_label_set_text(GTK_LABEL(volume_label), vol_str);
    if((orignal_vol / 10) != (vol / 10))
        openmoko_change_vol_img(vol);
    g_free(vol_str);
}

void
openmoko_wheel_press_left_up_cb(GtkWidget *widget, gpointer data)
{
    openmoko_increase_volume();
}

void
openmoko_decrease_volume()
{
    gint vl, vr, orignal_vol, vol;
    input_get_volume(&vl, &vr);
    orignal_vol = MAX(vl, vr);
    if(vl > 0 && vr > 0)
    {
	vl--;
	vr--;
    }
    else
	return;
    input_set_volume(vl, vr);
    vol = MAX(vl, vr);
    gchar* vol_str = g_strdup_printf("%d", vol);
    gtk_label_set_text(GTK_LABEL(volume_label), vol_str);
    if((orignal_vol / 10) != (vol / 10))
        openmoko_change_vol_img(vol);
    g_free(vol_str);
}

void
openmoko_wheel_press_right_down_cb(GtkWidget *widget, gpointer data)
{
    openmoko_decrease_volume();
}

void
openmoko_quit_musicplayer()
{
    if(playorpause)
        openmoko_play_pause_action();
	
    gint vl, vr;
    ConfigDb* db;
    db = bmp_cfg_db_open();
    input_get_volume(&vl, &vr);
    bmp_cfg_db_set_int(db, "ALSA", "volume_left", vl);
    bmp_cfg_db_set_int(db, "ALSA", "volume_right", vr);
    bmp_cfg_db_set_bool(db, "beep", "shuffle", cfg.shuffle);
    bmp_cfg_db_set_bool(db, "beep", "repeat", cfg.repeat);
    bmp_cfg_db_close(db);
    mainwin_quit_cb();

    g_free(images_dir);
  
    gtk_main_quit();
}

void
openmoko_main_quit(GtkWidget* widget, gpointer data)
{
    openmoko_quit_musicplayer();
}

void 
openmoko_set_shuffle_state()
{
    gboolean shuffle_state;
    ConfigDb* db;
    db = bmp_cfg_db_open();
    bmp_cfg_db_get_bool(db, "beep", "shuffle", &shuffle_state);
    bmp_cfg_db_close(db);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(shuffle_button), shuffle_state);
}

void 
openmoko_shuffle_button_callback(GtkWidget* widget, gpointer data)
{
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
    {
	cfg.shuffle = TRUE;	    
	playlist_set_shuffle(TRUE);
    }
    else
    {
	cfg.shuffle= FALSE;
	playlist_set_shuffle(FALSE);
    }
}

void 
openmoko_set_repeat_state()
{
    gboolean repeat_state;
    ConfigDb* db;
    db = bmp_cfg_db_open();
    bmp_cfg_db_get_bool(db, "beep", "repeat", &repeat_state);
    bmp_cfg_db_close(db);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(repeat_button), repeat_state);
}

void 
openmoko_repeat_button_callback(GtkWidget* widget, gpointer data)
{
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
    {
	cfg.repeat = TRUE;	    
    }
    else
    {
	cfg.repeat= FALSE;
    }
}

void
openmoko_playlist_button_callback(GtkWidget* widget, gpointer data)
{
    g_print("show play list\n");
}

gboolean
openmoko_press_on_slider_cb(GtkRange* range,
		            GtkScrollType scroll,
			    gdouble value,
			    gpointer data)
{
    g_source_remove(timeout_id);
    gint length, time, pos;
    length = playlist_get_current_length()/1000;
    pos = (gint)gtk_range_get_value(GTK_RANGE(time_hscale));
    time = (length * pos ) / 338;
    bmp_playback_seek(time);
    timeout_id = g_timeout_add(100, openmoko_update_elapse_time, NULL);
    return FALSE;
}

void 
btn_set_center_image(GtkButton* button, GtkImage* image)
{
    GtkWidget* btn_vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(button), btn_vbox);
    GtkWidget* alignment  = gtk_alignment_new(0.43, 0.43, 0, 0);
    gtk_box_pack_start(GTK_BOX(btn_vbox), alignment, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(alignment), GTK_WIDGET(image)); 
}

static DBusHandlerResult
signal_filter(DBusConnection *connection, DBusMessage *message, void *user_data)
{
    GMainLoop *loop = user_data;
    if(dbus_message_is_signal(message, DBUS_INTERFACE_LOCAL,
			    "Disconnected"))
    {
        g_main_loop_quit(loop);
	return DBUS_HANDLER_RESULT_HANDLED;
    }
    else if(dbus_message_is_signal(message, "com.burtonini.dbus.Signal", "Ping"))
    {
	DBusError error;
	char *s;
	dbus_error_init(&error);
	if(dbus_message_get_args(message, &error, DBUS_TYPE_STRING, &s,DBUS_TYPE_INVALID))
	{
	    g_print("Ping received: %s\n", s);
	    if(!g_ascii_strcasecmp(s, "quit"))
	    {
	        openmoko_quit_musicplayer();
		exit(0);
	    }
	    else if(!g_ascii_strcasecmp(s, "pause"))
	    {
		openmoko_play_pause_action();
	    }
	    else if(!g_ascii_strcasecmp(s, "play"))
	    {
		openmoko_play_pause_action();
	    }
	    else if(!g_ascii_strcasecmp(s, "prev"))
	    {
		openmoko_playlist_prev_action();
	    }
	    else if(!g_ascii_strcasecmp(s, "next"))
	    {
		openmoko_playlist_next_action();
	    }
	    else if(!g_ascii_strcasecmp(s, "v+"))
	    {
		openmoko_increase_volume();
	    }
	    else if(!g_ascii_strcasecmp(s, "v-"))
	    {
		openmoko_decrease_volume();
	    }
	    dbus_free(s);
	}
	else
	{
	    g_print("Ping received, but error getting message: %s\n", error.message);
	    dbus_error_free(&error);
	}
	return DBUS_HANDLER_RESULT_HANDLED;
    }
    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

void init_image_dir()
{
    gchar* share_string = g_strrstr(DATA_DIR, "share");
    int len = strlen(DATA_DIR);  
    int len1= strlen(share_string);
    gchar* prefix = g_strndup(DATA_DIR, len-len1 +5);
    images_dir = g_build_filename(prefix, "/images", NULL);
    g_free(prefix);
}

void 
openmoko_mainwin_create()
{
    MokoFingerToolBox *tools = NULL;
    GtkButton *prev_button = NULL;
    GtkButton *next_button = NULL;
    GtkWidget* playlist_button = NULL;
    MokoFixed *fixed = NULL;
    GtkWidget *alignment = NULL;
    GtkWidget *mainvbox = NULL;
    GtkWidget* background_vbox = NULL;
    GtkWidget* up_hbox = NULL;
    GtkWidget* middle_hbox = NULL;
    GtkWidget* down_hbox = NULL;
    
    GdkColor color;
    PangoFontDescription *font_desc;
   
    init_image_dir();
    
    MokoApplication *app = MOKO_APPLICATION(moko_application_get_instance());

    MokoFingerWindow *window = MOKO_FINGER_WINDOW(moko_finger_window_new());
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(openmoko_main_quit), NULL);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

    fixed = MOKO_FIXED(moko_fixed_new());
    moko_finger_window_set_contents(window, GTK_WIDGET(fixed));
   
/*******************************************/
    background_vbox = gtk_vbox_new(FALSE, 0);
    
    GtkWidget* app_name = gtk_label_new("Music Player");
    gtk_misc_set_alignment(GTK_MISC(app_name), 0.5, 0.5);
    gtk_widget_set_size_request(GTK_WIDGET(app_name), 480, 30);
    GtkWidget* app_name_eventbox = gtk_event_box_new();
    gtk_widget_set_name(GTK_WIDGET(app_name_eventbox), "gtkeventbox-black");
    gtk_container_add(GTK_CONTAINER(app_name_eventbox), app_name);
    gtk_box_pack_start(GTK_BOX(background_vbox), app_name_eventbox, TRUE, TRUE, 0);
    
    alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 35, 0, 15, 0);
    gtk_box_pack_start(GTK_BOX(background_vbox), alignment, TRUE, TRUE, 0);
    image_path = g_build_path("/", images_dir, "bg-music-player.png", NULL);
    image = gtk_image_new_from_file(image_path);
    g_free(image_path);
    gtk_container_add(GTK_CONTAINER(alignment), image);
    
    moko_fixed_set_cargo(MOKO_FIXED(fixed), background_vbox);
/*******************************************/
    
    alignment = gtk_alignment_new(0.5, 0.25, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 65, 0, 20, 0); 
    moko_fixed_set_cargo(MOKO_FIXED(fixed), alignment);
    mainvbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(alignment), mainvbox);
    
/*******************************************/
    //set title label
    alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 30, 0, 50, 30);
    gtk_box_pack_start(GTK_BOX(mainvbox), alignment, TRUE, TRUE, 0);
    title_label = gtk_label_new("");  
    font_desc = pango_font_description_from_string("Times 22");
    gtk_widget_modify_font(title_label, font_desc);
    pango_font_description_free(font_desc);
    gtk_label_set_width_chars(GTK_LABEL(title_label), 23);
    gtk_misc_set_alignment(GTK_MISC(title_label), 0, 0.5);
    gtk_label_set_ellipsize(GTK_LABEL(title_label), PANGO_ELLIPSIZE_END);
    gdk_color_parse("black", &color);
    gtk_widget_modify_fg(GTK_WIDGET(title_label), GTK_STATE_NORMAL, &color);
    gtk_container_add(GTK_CONTAINER(alignment), title_label);
/*******************************************/

/*******************************************/
    //set artist label
    alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 5, 0, 50, 30);
    gtk_box_pack_start(GTK_BOX(mainvbox), alignment, TRUE, TRUE, 0);
    artist_label = gtk_label_new("");  
    font_desc = pango_font_description_from_string("Times 12");
    gtk_widget_modify_font(artist_label, font_desc);
    pango_font_description_free(font_desc);
    gtk_misc_set_alignment(GTK_MISC(artist_label), 0, 0.5);
    gtk_label_set_width_chars(GTK_LABEL(artist_label), 40);
    gtk_label_set_ellipsize(GTK_LABEL(artist_label), PANGO_ELLIPSIZE_END);
    gdk_color_parse("black", &color);
    gtk_widget_modify_fg(GTK_WIDGET(artist_label), GTK_STATE_NORMAL, &color);
    openmoko_set_tag_info();
    gtk_container_add(GTK_CONTAINER(alignment), artist_label);
/*******************************************/
    
/*******************************************/
    //set up_hbox to show track number and time 
    alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 20, 0, 50, 30);
    gtk_box_pack_start(GTK_BOX(mainvbox), alignment, TRUE, TRUE, 0);
    up_hbox = gtk_hbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(alignment), up_hbox);
    
    //set song image
    alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 2, 0, 10, 0);
    gtk_box_pack_start(GTK_BOX(up_hbox), alignment, TRUE, TRUE, 0);
    image_path = g_build_path("/", images_dir, "icon-musicplayer-song.png", NULL);
    image = gtk_image_new_from_file(image_path);
    g_free(image_path);
    gtk_container_add(GTK_CONTAINER(alignment), image);
   
    //set track number
    alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 0, 0, 10, 0);
    gtk_box_pack_start(GTK_BOX(up_hbox), alignment, TRUE, TRUE, 0);
    track_number_label = gtk_label_new("");
    openmoko_set_track_number();
    gdk_color_parse("black", &color);
    gtk_widget_modify_fg(GTK_WIDGET(track_number_label), GTK_STATE_NORMAL, &color);
    font_desc = pango_font_description_from_string("Times 13");
    gtk_widget_modify_font(track_number_label, font_desc);
    pango_font_description_free(font_desc);
    gtk_container_add(GTK_CONTAINER(alignment), track_number_label); 
   
    //set separator
    alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 0, 0, 6, 0);
    gtk_box_pack_start(GTK_BOX(up_hbox), alignment, TRUE, TRUE, 0);
    GtkWidget* separate_label = gtk_label_new("/");
    gdk_color_parse("black", &color);
    gtk_widget_modify_fg(GTK_WIDGET(separate_label), GTK_STATE_NORMAL, &color);
    font_desc = pango_font_description_from_string("Times 13");
    gtk_widget_modify_font(separate_label, font_desc);
    pango_font_description_free(font_desc);
    gtk_container_add(GTK_CONTAINER(alignment), separate_label); 
   
    //set total number label
    alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 0, 0, 6, 0);
    gtk_box_pack_start(GTK_BOX(up_hbox), alignment, TRUE, TRUE, 0);
    total_number_label = gtk_label_new("");
    openmoko_set_total_number();
    gdk_color_parse("black", &color);
    gtk_widget_modify_fg(GTK_WIDGET(total_number_label), GTK_STATE_NORMAL, &color);
    font_desc = pango_font_description_from_string("Times 13");
    gtk_widget_modify_font(total_number_label, font_desc);
    pango_font_description_free(font_desc);
    gtk_container_add(GTK_CONTAINER(alignment), total_number_label); 
   
    //set time image
    alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 2, 0, 120, 0);
    gtk_box_pack_start(GTK_BOX(up_hbox), alignment, TRUE, TRUE, 0);
    image_path = g_build_path("/", images_dir, "icon-musicplayer-time.png", NULL);
    image = gtk_image_new_from_file(image_path);
    g_free(image_path);
    gtk_container_add(GTK_CONTAINER(alignment), image);
    
    //set elapse time
    alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 0, 0, 10, 0);
    gtk_box_pack_start(GTK_BOX(up_hbox), alignment, TRUE, TRUE, 0);
    elapse_time_label = gtk_label_new("0:00");
    gdk_color_parse("black", &color);
    gtk_widget_modify_fg(GTK_WIDGET(elapse_time_label), GTK_STATE_NORMAL, &color);
    font_desc = pango_font_description_from_string("Times 13");
    gtk_widget_modify_font(elapse_time_label, font_desc);
    pango_font_description_free(font_desc);
    gtk_container_add(GTK_CONTAINER(alignment), elapse_time_label); 
   
    //set separator
    alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 0, 0, 6, 0);
    gtk_box_pack_start(GTK_BOX(up_hbox), alignment, TRUE, TRUE, 0);
    separate_label = gtk_label_new("/");
    gdk_color_parse("black", &color);
    gtk_widget_modify_fg(GTK_WIDGET(separate_label), GTK_STATE_NORMAL, &color);
    font_desc = pango_font_description_from_string("Times 13");
    gtk_widget_modify_font(separate_label, font_desc);
    pango_font_description_free(font_desc);
    gtk_container_add(GTK_CONTAINER(alignment), separate_label); 
   
    //set total number label
    alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 0, 0, 6, 0);
    gtk_box_pack_start(GTK_BOX(up_hbox), alignment, TRUE, TRUE, 0);
    total_time_label = gtk_label_new("");
    openmoko_set_total_time();
    gdk_color_parse("black", &color);
    gtk_widget_modify_fg(GTK_WIDGET(total_time_label), GTK_STATE_NORMAL, &color);
    font_desc = pango_font_description_from_string("Times 13");
    gtk_widget_modify_font(total_time_label, font_desc);
    pango_font_description_free(font_desc);
    gtk_container_add(GTK_CONTAINER(alignment), total_time_label); 
/*******************************************/
   
/*******************************************/
    //set time hscale 
    alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 5, 0, 48, 0);
    gtk_box_pack_start(GTK_BOX(mainvbox), alignment, TRUE, TRUE, 0);
    time_hscale = gtk_hscale_new_with_range(0.0, 338.0, 1.0);
    gtk_scale_set_draw_value(GTK_SCALE(time_hscale), FALSE);
    GTK_WIDGET_UNSET_FLAGS(GTK_WIDGET(time_hscale), GTK_CAN_FOCUS);
    gtk_widget_set_size_request(GTK_WIDGET(time_hscale), 338, 35);
    gtk_range_set_value(GTK_RANGE(time_hscale), 0.0);
    g_signal_connect(G_OBJECT(time_hscale), "change_value", 
		    G_CALLBACK(openmoko_press_on_slider_cb), NULL);
    gtk_container_add(GTK_CONTAINER(alignment), time_hscale);
/*******************************************/

/*******************************************/
    //set middle_hbox
    alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 42, 0, 70, 0);
    gtk_box_pack_start(GTK_BOX(mainvbox), alignment, TRUE, TRUE, 0);
    middle_hbox = gtk_hbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(alignment), middle_hbox);
  
    //set vis
    alignment_eq_pos1 = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment_eq_pos1), 0, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(middle_hbox), alignment_eq_pos1, TRUE, TRUE, 0);
    image_path = g_build_path("/", images_dir, "ind-music-eq-01.png", NULL);
    image_eq_pos1 = gtk_image_new_from_file(image_path); 
    gtk_container_add(GTK_CONTAINER(alignment_eq_pos1), image_eq_pos1);
   
    alignment_eq_pos2 = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment_eq_pos2), 0, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(middle_hbox), alignment_eq_pos2, TRUE, TRUE, 0);
    image_eq_pos2 = gtk_image_new_from_file(image_path);
    gtk_container_add(GTK_CONTAINER(alignment_eq_pos2), image_eq_pos2);
    
    alignment_eq_pos3 = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment_eq_pos3), 0, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(middle_hbox), alignment_eq_pos3, TRUE, TRUE, 0);
    image_eq_pos3 = gtk_image_new_from_file(image_path);
    gtk_container_add(GTK_CONTAINER(alignment_eq_pos3), image_eq_pos3);
    
    alignment_eq_pos4 = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment_eq_pos4), 0, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(middle_hbox), alignment_eq_pos4, TRUE, TRUE, 0);
    image_eq_pos4 = gtk_image_new_from_file(image_path);
    gtk_container_add(GTK_CONTAINER(alignment_eq_pos4), image_eq_pos4);
    
    alignment_eq_pos5 = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment_eq_pos5), 0, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(middle_hbox), alignment_eq_pos5, TRUE, TRUE, 0);
    image_eq_pos5 = gtk_image_new_from_file(image_path); 
    gtk_container_add(GTK_CONTAINER(alignment_eq_pos5), image_eq_pos5);
    
    alignment_eq_pos6 = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment_eq_pos6), 0, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(middle_hbox), alignment_eq_pos6, TRUE, TRUE, 0);
    image_eq_pos6 = gtk_image_new_from_file(image_path);
    gtk_container_add(GTK_CONTAINER(alignment_eq_pos6), image_eq_pos6);
    
    alignment_eq_pos7 = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment_eq_pos7), 0, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(middle_hbox), alignment_eq_pos7, TRUE, TRUE, 0);
    image_eq_pos7 = gtk_image_new_from_file(image_path);
    gtk_container_add(GTK_CONTAINER(alignment_eq_pos7), image_eq_pos7);
    
    alignment_eq_pos8 = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment_eq_pos8), 0, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(middle_hbox), alignment_eq_pos8, TRUE, TRUE, 0);
    image_eq_pos8 = gtk_image_new_from_file(image_path);
    gtk_container_add(GTK_CONTAINER(alignment_eq_pos8), image_eq_pos8);
    
    alignment_eq_pos9 = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment_eq_pos9), 0, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(middle_hbox), alignment_eq_pos9, TRUE, TRUE, 0);
    image_eq_pos9 = gtk_image_new_from_file(image_path);
    gtk_container_add(GTK_CONTAINER(alignment_eq_pos9), image_eq_pos9);
    
    alignment_eq_pos10 = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment_eq_pos10), 0, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(middle_hbox), alignment_eq_pos10, TRUE, TRUE, 0);
    image_eq_pos10 = gtk_image_new_from_file(image_path);
    gtk_container_add(GTK_CONTAINER(alignment_eq_pos10), image_eq_pos10);
    
    alignment_eq_pos11 = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment_eq_pos11), 0, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(middle_hbox), alignment_eq_pos11, TRUE, TRUE, 0);
    image_eq_pos11 = gtk_image_new_from_file(image_path);
    gtk_container_add(GTK_CONTAINER(alignment_eq_pos11), image_eq_pos11);
    
    alignment_eq_pos12 = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment_eq_pos12), 0, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(middle_hbox), alignment_eq_pos12, TRUE, TRUE, 0);
    image_eq_pos12 = gtk_image_new_from_file(image_path);
    g_free(image_path);
    gtk_container_add(GTK_CONTAINER(alignment_eq_pos12), image_eq_pos12);
    
    //set middle right vbox
    GtkWidget* middle_right_vbox = gtk_vbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(middle_hbox), middle_right_vbox, TRUE, TRUE, 0); 
    
     //set volume hbox
    GtkWidget* volume_box = gtk_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(middle_right_vbox), volume_box, TRUE, TRUE, 0); 
    
    //set volume image
    vol_img_alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(vol_img_alignment), 0, 0, 30, 0);
    gtk_box_pack_start(GTK_BOX(volume_box), vol_img_alignment, TRUE, TRUE, 0);
    image_path = g_build_path("/", images_dir, "ind-music-volume-00.png", NULL);
    vol_image = gtk_image_new_from_file(image_path);
    g_free(image_path);
    gtk_container_add(GTK_CONTAINER(vol_img_alignment), vol_image);
    
    //set volume label
    alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 2, 0, 6, 0);
    gtk_box_pack_start(GTK_BOX(volume_box), alignment, TRUE, TRUE, 0);
    volume_label = gtk_label_new("");
    gdk_color_parse("darkorange", &color);
    gtk_widget_modify_fg(GTK_WIDGET(volume_label), GTK_STATE_NORMAL, &color);
    font_desc = pango_font_description_from_string("Times Bold 13");
    gtk_widget_modify_font(volume_label, font_desc);
    pango_font_description_free(font_desc);
    openmoko_read_volume_from_start();
    gtk_container_add(GTK_CONTAINER(alignment), volume_label); 
   
    //set balance image
    alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 10, 0, 30, 0);
    gtk_box_pack_start(GTK_BOX(middle_right_vbox), alignment, TRUE, TRUE, 0);
    image_path = g_build_path("/", images_dir, "ind-music-pan-0.png", NULL);
    image = gtk_image_new_from_file(image_path);
    g_free(image_path);
    gtk_container_add(GTK_CONTAINER(alignment), image);
/*******************************************/
    
/*******************************************/
    //set down hbox
    alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 30, 0, 0, 0);
    gtk_box_pack_start(GTK_BOX(mainvbox), alignment, TRUE, TRUE, 0);
    down_hbox = gtk_hbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(alignment), down_hbox);
    
    //set shuffle toggle button
    alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 0, 0, 120, 0);
    gtk_box_pack_start(GTK_BOX(down_hbox), alignment, TRUE, TRUE, 0);
    shuffle_button = gtk_toggle_button_new();
    gtk_widget_set_size_request(GTK_WIDGET(shuffle_button), 63, 62);
    gtk_widget_set_name(GTK_WIDGET(shuffle_button), "mokofingerbutton-white");
    openmoko_set_shuffle_state();
    g_signal_connect(G_OBJECT(shuffle_button), "clicked",
		    G_CALLBACK(openmoko_shuffle_button_callback), NULL);
    GTK_WIDGET_UNSET_FLAGS(GTK_WIDGET(shuffle_button), GTK_CAN_FOCUS);    
    gtk_container_add(GTK_CONTAINER(alignment), shuffle_button);
    image_path = g_build_path("/", images_dir, "ico-shuffle.png", NULL); 
    image = gtk_image_new_from_file(image_path);
    g_free(image_path);
    btn_set_center_image(GTK_BUTTON(shuffle_button), GTK_IMAGE(image));
    
    //set repeat toggle button
    alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 0, 0, 10, 0);
    gtk_box_pack_start(GTK_BOX(down_hbox), alignment, TRUE, TRUE, 0);
    repeat_button = gtk_toggle_button_new();
    gtk_widget_set_size_request(GTK_WIDGET(repeat_button), 63, 62);
    gtk_widget_set_name(GTK_WIDGET(repeat_button), "mokofingerbutton-white");
    openmoko_set_repeat_state();
    g_signal_connect(G_OBJECT(repeat_button), "clicked", 
		    G_CALLBACK(openmoko_repeat_button_callback), NULL);
    GTK_WIDGET_UNSET_FLAGS(GTK_WIDGET(repeat_button), GTK_CAN_FOCUS);    
    gtk_container_add(GTK_CONTAINER(alignment), repeat_button);
    image_path = g_build_path("/", images_dir, "ico-repeat.png", NULL);
    image = gtk_image_new_from_file(image_path);
    g_free(image_path);
    btn_set_center_image(GTK_BUTTON(repeat_button), GTK_IMAGE(image));

    //set playlist button
    alignment = gtk_alignment_new(0, 0, 0, 0);
    gtk_alignment_set_padding(GTK_ALIGNMENT(alignment), 0, 0, 10, 0);
    gtk_box_pack_start(GTK_BOX(down_hbox), alignment, TRUE, TRUE, 0);
    playlist_button = gtk_button_new();
    gtk_widget_set_size_request(GTK_WIDGET(playlist_button), 63, 62);
    gtk_widget_set_name(GTK_WIDGET(playlist_button), "mokofingerbutton-white");
    GTK_WIDGET_UNSET_FLAGS(GTK_WIDGET(playlist_button), GTK_CAN_FOCUS);    
    gtk_container_add(GTK_CONTAINER(alignment), playlist_button);
    g_signal_connect(G_OBJECT(playlist_button), "clicked", 
		    G_CALLBACK(openmoko_playlist_button_callback), NULL);
    image_path = g_build_path("/", images_dir, "ico-list.png", NULL);
    image = gtk_image_new_from_file(image_path);
    g_free(image_path);
    btn_set_center_image(GTK_BUTTON(playlist_button), GTK_IMAGE(image));
/*******************************************/
    
    gtk_widget_show_all(GTK_WIDGET(window));
    
    gtk_widget_show(GTK_WIDGET(moko_finger_window_get_wheel(window)));
    
    g_signal_connect(G_OBJECT(moko_finger_window_get_wheel(window)),
		    "press_left_up",
		    G_CALLBACK(openmoko_wheel_press_left_up_cb),
		    NULL);
    g_signal_connect(G_OBJECT(moko_finger_window_get_wheel(window)),
		    "long_press_left_up",
		    G_CALLBACK(openmoko_wheel_press_left_up_cb),
		    NULL);
    g_signal_connect(G_OBJECT(moko_finger_window_get_wheel(window)),
		    "press_right_down",
		    G_CALLBACK(openmoko_wheel_press_right_down_cb),
		    NULL);
    g_signal_connect(G_OBJECT(moko_finger_window_get_wheel(window)),
		    "long_press_right_down",
		    G_CALLBACK(openmoko_wheel_press_right_down_cb),
		    NULL);
    g_signal_connect(G_OBJECT(moko_finger_window_get_wheel(window)),
		    "press_bottom",
		    G_CALLBACK(openmoko_main_quit),
		    NULL);

    if(!tools)
    {
        tools = moko_finger_window_get_toolbox(window);
       
	prev_button = moko_finger_tool_box_add_button_without_label(tools);
        image_path = g_build_path("/", images_dir, "ico-previoustrack.png", NULL);
        image = gtk_image_new_from_file(image_path);
	g_free(image_path);
	moko_pixmap_button_set_finger_toolbox_btn_center_image(prev_button, image);
        g_signal_connect(G_OBJECT(prev_button), "clicked",
			G_CALLBACK(openmoko_playlist_prev), NULL);
	
	play_pause_button = moko_finger_tool_box_add_button_without_label(tools);
        image_path = g_build_path("/", images_dir, "ico-play.png", NULL);
        image = gtk_image_new_from_file(image_path);
	g_free(image_path);
	moko_pixmap_button_set_finger_toolbox_btn_center_image(play_pause_button, image);
	g_signal_connect(G_OBJECT(play_pause_button), "clicked",
			G_CALLBACK(openmoko_play_pause_button_pushed), NULL);
	
	next_button = moko_finger_tool_box_add_button_without_label(tools);
        image_path = g_build_path("/", images_dir, "ico-nexttrack.png", NULL);
        image = gtk_image_new_from_file(image_path);
	g_free(image_path);
	moko_pixmap_button_set_finger_toolbox_btn_center_image(next_button, image);
	g_signal_connect(G_OBJECT(next_button), "clicked",
			G_CALLBACK(openmoko_playlist_next), NULL);

	gtk_widget_show(GTK_WIDGET(tools));
    }
    
    //added by lijiang
    DBusConnection *bus;
    DBusError error;

    dbus_error_init(&error);
    bus = dbus_bus_get(DBUS_BUS_SESSION, &error);
    if(!bus)
    {
        g_print("Failed to connect to the D-Bus daemon: %s", error.message);
	dbus_error_free(&error);
	return;
    }
    dbus_connection_setup_with_g_main(bus, NULL);
    dbus_bus_add_match(bus, "type='signal',interface='com.burtonini.dbus.Signal'", &error);
    dbus_connection_add_filter(bus, signal_filter, mainwin, NULL);
    //added end
}
