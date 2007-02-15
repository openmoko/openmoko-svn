#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>

static GList *get_dirs(void);
static void preview_clicked(GtkWidget *button, gpointer data);
static void update_newfont (void);
static void apply_clicked(GtkWidget *button, gpointer data);
static void usage(void);
static void backup_gtkrc(gchar *path_to_gtkrc);
static void ok_clicked(gchar *rc);
static void preview_ok_clicked(gchar *rc);
#ifdef SWITCH_GTK2
static GtkTreeModel *create_model (void);
void clist_insert (GtkTreeView *clist);
#endif
static void dock(void);
#ifndef SWITCH_GTK2
static int rightclick (GtkWidget *w, GdkEventButton *event, gpointer data);
static void clist_insert(GtkWidget *clist);
#endif
static void preview(gchar *rc_file);
static void preview_window(gchar *rc_file);
static void send_refresh_signal(void);
static short is_themedir (gchar *path, gchar **rc_file);
static short is_installed_theme (gchar *path, gchar **rc_file);
static short install_tarball (gchar *path, gchar **rc_file);
static int switcheroo (gchar *actual);
static void install_clicked (GtkWidget *w, gpointer data);
static void install_ok_clicked (GtkWidget *w, gpointer data);
static void search_for_theme_or_die_trying (gchar *actual, gchar **rc_file);
static void set_font (GtkWidget *w, GtkWidget *dialog);
static void font_browse_clicked (GtkWidget *w, gpointer data);
static short fgrep_gtkrc_for (gchar *needle);
static GList *compare_glists (GList *t1, GList *t2, GCompareFunc cmpfunc);
void quit_preview();
void quit();
void hide_stuff();
void show_stuff();
void on_eventbox_click();
