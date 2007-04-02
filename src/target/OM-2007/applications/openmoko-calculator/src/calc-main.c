/*
 *  Calculator -- OpenMoko simple Calculator
 *
 *  Authored by Rodolphe Ortalo <rodolphe.ortalo@free.fr>
 *
 *  Copyright (C) 2007 Rodolphe Ortalo
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#include "calc-main.h"

#include <stdio.h>
#include <math.h>

#include <libmokoui/moko-application.h>
#include <libmokoui/moko-finger-tool-box.h>
#include <libmokoui/moko-finger-window.h>
#include <libmokoui/moko-finger-wheel.h>

#include <gtk/gtkalignment.h>
#include <gtk/gtkbutton.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkmain.h>
#include <gtk/gtkmenu.h>
#include <gtk/gtkvbox.h>
#include <gtk/gtk.h>

#include <glib.h>
#include <glib/gi18n.h>

#define BOOL short unsigned int

/* state of calculator */
typedef struct {
  double last_operand;
  double current_operand;
  double (*func)(const double,const double);
  BOOL decimal_point;
  unsigned short int n_digits;
  unsigned short int n_fractional;
} calc_state;

enum operation {
  notimplemented,
  backspace, clearall, clear,
  zero, one, two, three, four, five, six, seven, eight, nine, ten,
  add, sub, mult, div, equal, minus, point } ;

/*
 * functions for operations
 */
static double noop_func (const double a, const double b) { return a; }
static double add_func  (const double a, const double b) { return a+b; }
static double sub_func  (const double a, const double b) { return a-b; }
static double mult_func (const double a, const double b) { return a*b; }
static double div_func  (const double a, const double b) { return a/b; }

/*
 * Internal variables
 */
static calc_state the_state = { .last_operand = 0.0,
				.current_operand = 0.0,
				.func = &add_func,
				.decimal_point = FALSE,
				.n_digits = 0,
				.n_fractional = 0, };
static GtkWidget* displayed_label;

#define CALC_ROWS 5
#define CALC_COLS 4
static const gchar *label[CALC_ROWS][CALC_COLS] = {
    {N_("Clear All"), N_("Clear"), "<big><b>/</b></big>", "<big><b>*</b></big>" },
    {"<big><b>7</b></big>", "<big><b>8</b></big>", "<big><b>9</b></big>", "<big><b>+</b></big>"},
    {"<big><b>4</b></big>", "<big><b>5</b></big>", "<big><b>6</b></big>", "<big><b>-</b></big>"},
    {"<big><b>1</b></big>", "<big><b>2</b></big>", "<big><b>3</b></big>", "<big><b>=</b></big>"},
    {"<big><b>0</b></big>", N_("<big><b>.</b></big>"), "<big><b>+</b></big>/<big><b>-</b></big>", "none"},
  };
static enum operation ops[CALC_ROWS][CALC_COLS] = {
    { clearall, clear, div, mult},
    { seven, eight, nine, add},
    { four, five, six, sub},
    { one, two, three, equal},
    { zero, point, minus, notimplemented},
  };
#if 1
gchar *wnames[CALC_ROWS][CALC_COLS] = {
  {"mokofingerbutton-orange", "mokofingerbutton-orange", "mokofingerbutton-dialer", "mokofingerbutton-dialer"},
  {"mokofingerbutton-dialer", "mokofingerbutton-dialer", "mokofingerbutton-dialer", "mokofingerbutton-dialer"},
  {"mokofingerbutton-dialer", "mokofingerbutton-dialer", "mokofingerbutton-dialer", "mokofingerbutton-dialer"},
  {"mokofingerbutton-dialer", "mokofingerbutton-dialer", "mokofingerbutton-dialer", "mokofingerbutton-big"},
  {"mokofingerbutton-dialer", "mokofingerbutton-dialer", "mokofingerbutton-dialer", "none"},
};
#else
/* Alternative styling - more flashy */
gchar *wnames[CALC_ROWS][CALC_COLS] = {
  {"mokofingerbutton-black", "mokofingerbutton-black", "mokofingerbutton-orange", "mokofingerbutton-orange"},
  {"mokofingerbutton-orange", "mokofingerbutton-orange", "mokofingerbutton-orange", "mokofingerbutton-orange"},
  {"mokofingerbutton-orange", "mokofingerbutton-orange", "mokofingerbutton-orange", "mokofingerbutton-orange"},
  {"mokofingerbutton-orange", "mokofingerbutton-orange", "mokofingerbutton-orange", "mokofingerbutton-black"},
  {"mokofingerbutton-orange", "mokofingerbutton-orange", "mokofingerbutton-orange", "none"},
};
#endif

/*
 * Signal function and static helpers
 */
#define MAX_DISPLAY_CHARS 10
#define MAX_DISPLAY_MARKUP (MAX_DISPLAY_CHARS+80)
static gchar dispstring[MAX_DISPLAY_MARKUP+1];

static void update_display(const double v)
{
  if (the_state.n_digits == 0) {
    /* We are certainly displaying a computation result */
    /* Manually build the display string ala %g */
    double value = v;
    int expof10;

    expof10 = (int) log10((value>=0.)?(value):(-value));
    value *= pow(10,-expof10);
    if (expof10 >= MAX_DISPLAY_CHARS)
      snprintf(dispstring,MAX_DISPLAY_MARKUP,"<span font_desc=\"48\" >%.*g <small>e</small><sup><big>%d</big></sup></span>",(MAX_DISPLAY_CHARS-2),value,expof10);
    else
      snprintf(dispstring,MAX_DISPLAY_MARKUP,"<span font_desc=\"48\" >%.*g</span>",MAX_DISPLAY_CHARS,v);
  } else {
    /* We display entered value including trailing 0s */
    if (the_state.decimal_point == TRUE)
      snprintf(dispstring,MAX_DISPLAY_MARKUP,"<span font_desc=\"48\" >%#.*f</span>",the_state.n_fractional,v);
    else
      snprintf(dispstring,MAX_DISPLAY_MARKUP,"<span font_desc=\"48\" >%.*f</span>",the_state.n_fractional,v);
  }
#if 1
  calc_debug("cur = %.12lf  last = %.12lf", the_state.current_operand, the_state.last_operand);
  calc_debug("n_digits = %i  n_fractional = %i  dec_point = %i", the_state.n_digits, the_state.n_fractional, the_state.decimal_point);
  calc_debug(dispstring);
#endif
	  
  gtk_label_set_markup(GTK_LABEL(displayed_label), dispstring);
}

static void update_state_for_digit(const int v)
{
  if (the_state.func == &noop_func) {
    /* renew a fresh calc */
    the_state.last_operand = 0.0;
    the_state.func = &add_func;
  }
  if (the_state.n_digits < MAX_DISPLAY_CHARS) {
    the_state.n_digits++;
    if (the_state.decimal_point == FALSE) {
      the_state.current_operand *= 10.0;
      the_state.current_operand += v
	* (the_state.current_operand ? (the_state.current_operand / fabs(the_state.current_operand)) : 1.0);
    } else {
      the_state.n_fractional++;
      the_state.current_operand += (v * pow(0.1,the_state.n_fractional))
	* (the_state.current_operand ? (the_state.current_operand / fabs(the_state.current_operand)) : 1.0);
    }
  }
  update_display(the_state.current_operand);
}
static void update_state_for_operation(double (*nextfunc)(const double,const double))
{
  the_state.last_operand = (*(the_state.func))(the_state.last_operand,the_state.current_operand);
  the_state.current_operand = 0.0;
  the_state.func = nextfunc;
  the_state.decimal_point = FALSE;
  the_state.n_digits = 0;
  the_state.n_fractional = 0;
  update_display(the_state.last_operand);
}

void calc_button_pressed( GtkButton* button, enum operation *k)
{
  calc_debug( "openmoko-calculator: button pressed" );
  switch (*k) {
  case zero:
    update_state_for_digit(0);
    break;
  case one:
    update_state_for_digit(1);
    break;
  case two:
    update_state_for_digit(2);
    break;
  case three:
    update_state_for_digit(3);
    break;
  case four:
    update_state_for_digit(4);
    break;
  case five:
    update_state_for_digit(5);
    break;
  case six:
    update_state_for_digit(6);
    break;
  case seven:
    update_state_for_digit(7);
    break;
  case eight:
    update_state_for_digit(8);
    break;
  case nine:
    update_state_for_digit(9);
    break;
  case minus:
    the_state.current_operand *= -1;
    update_display(the_state.current_operand);
    break;
  case point:
    the_state.decimal_point = TRUE;
    update_display(the_state.current_operand);
    break;
  case clearall:
    the_state.last_operand = 0.0;
    the_state.func = &add_func;
  case clear:
    the_state.current_operand = 0.0;
    the_state.decimal_point = FALSE;
    the_state.n_digits = 0;
    the_state.n_fractional = 0;
    update_display(the_state.current_operand);
    break;
  case add:
    update_state_for_operation(&add_func);
    break;
  case sub:
    update_state_for_operation(&sub_func);
    break;
  case mult:
    update_state_for_operation(&mult_func);
    break;
  case div:
    update_state_for_operation(&div_func);
    break;
#if 0
    /* Not implemented to win one button slot and display a bigger = button */
  case backspace:
    break;
#endif
  case equal:
    update_state_for_operation(&noop_func);
    break;
  case notimplemented:
    g_debug("openmoko-calculator: operation not-implemented");
    break;
  default:
    g_debug("openmoko-calculator: unknown operation (%i)!", *k);
    break;
  }
}

/*
 * Convenience static function
 */
static GtkTable* calc_panel_init (void)
{

  GtkTable *table;
  int i, j;

  table = GTK_TABLE(gtk_table_new (CALC_ROWS, CALC_COLS, TRUE));
  gtk_table_set_row_spacings(table, 10);
  gtk_table_set_col_spacings(table, 10);

  for (j = 0; j < CALC_COLS; j++)
    for (i = 0; i < CALC_ROWS; i++)
    {
      GtkButton* button = GTK_BUTTON(gtk_button_new());
      GtkWidget* blabel = gtk_label_new(NULL);
      gtk_label_set_markup(GTK_LABEL(blabel),gettext(label[i][j]));
      gtk_container_add(GTK_CONTAINER(button),blabel);

      g_signal_connect (G_OBJECT(button), "clicked", G_CALLBACK(calc_button_pressed),&(ops[i][j]));
      /* TODO: Check if this changes the aspect */
      gtk_widget_set_name(GTK_WIDGET(button), wnames[i][j]);

      if ((j == (CALC_COLS-1)) && (i == (CALC_ROWS-2))) {
	/* Last button spans two cells vertically */
	gtk_table_attach_defaults (table, GTK_WIDGET(button),
				   j, j + 1, i, i + 2);
	i++;
      } else {
	gtk_table_attach_defaults (table, GTK_WIDGET(button),
				   j, j + 1, i, i + 1);	
      }
      gtk_widget_set_size_request (GTK_WIDGET(button), 20, 20);
    }
  return table;
}

/*
 * Command line options definition
 */
static GOptionEntry entries[] = 
{
  /* No options right now except the default ones from GTK */
  { NULL }
};


/*
 * Main
 */
int main( int argc, char** argv )
{
    GtkHBox* hbox;
    GtkVBox* vbox;
    GtkTable* table;

    calc_debug( "openmoko-calculator starting up" );
    /* Initialize GTK+ */
    gtk_init( &argc, &argv );
    if (argc != 1)
      {
	/* Add init code. */
	GError *error = NULL;
	GOptionContext *context = g_option_context_new ("");
	
	g_option_context_add_main_entries (context, entries, NULL);
	g_option_context_add_group (context, gtk_get_option_group (TRUE));
	g_option_context_parse (context, &argc, &argv, &error);
	
	g_option_context_free (context);
      }
	
    setlocale (LC_ALL, "");
    bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
    textdomain (GETTEXT_PACKAGE);

    /* application object */
    /* MokoApplication* app = MOKO_APPLICATION(moko_application_get_instance()); */
    g_set_application_name( _("Calculator") );

    /* main window */
    MokoFingerWindow* window = MOKO_FINGER_WINDOW(moko_finger_window_new());

    /* application menu */
    GtkMenu* appmenu = GTK_MENU(gtk_menu_new());
    GtkMenuItem* closeitem = GTK_MENU_ITEM(gtk_menu_item_new_with_label( _("Close") ));
    g_signal_connect( G_OBJECT(closeitem), "activate", G_CALLBACK(gtk_main_quit), NULL );
    gtk_menu_shell_append( GTK_MENU_SHELL(appmenu), GTK_WIDGET(closeitem) );
    moko_finger_window_set_application_menu( window, appmenu );

    /* connect close event */
    g_signal_connect( G_OBJECT(window), "delete_event", G_CALLBACK(gtk_main_quit), NULL );

    /* contents */
    hbox = GTK_HBOX(gtk_hbox_new(TRUE, 10)); /* The hbox is just to get some border space...! */
    vbox = GTK_VBOX(gtk_vbox_new(FALSE, 10));

    displayed_label = gtk_label_new("123");
    gtk_misc_set_alignment(GTK_MISC(displayed_label), 1, 0);
    update_display(0.0);
    GtkWidget *eventbox1 = gtk_event_box_new ();
    gtk_widget_set_name (eventbox1, "gtkeventbox-black");
    gtk_container_add (GTK_CONTAINER (eventbox1), displayed_label);
    gtk_box_pack_start( GTK_BOX(vbox), eventbox1, FALSE, FALSE, 10);

    table = calc_panel_init();
    gtk_box_pack_start( GTK_BOX(vbox), GTK_WIDGET(table), TRUE, TRUE, 10);

    gtk_box_pack_start( GTK_BOX(hbox), GTK_WIDGET(vbox), TRUE, TRUE, 10);
    moko_finger_window_set_contents(window, GTK_WIDGET(hbox));
    
    /* show everything and run main loop */
    gtk_widget_show_all( GTK_WIDGET(window) );
    calc_debug( "calculator entering main loop" );
    gtk_main();
    calc_debug( "calculator left main loop" );

    return 0;
}
