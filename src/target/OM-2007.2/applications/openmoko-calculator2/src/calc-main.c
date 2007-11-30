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
#include <string.h>

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
  {N_("CE"), N_("C"), "\303\267" /* divide */, "\303\227" /* multiply */ },
    {"7", "8", "9", "+"},
    {"4", "5", "6", "\342\210\222" /* minus */},
    {"1", "2", "3", "="},
    {"0", N_("."), "\302\261" /* plus/minus */, "none"},
  };
static enum operation ops[CALC_ROWS][CALC_COLS] = {
    { clearall, clear, div, mult},
    { seven, eight, nine, add},
    { four, five, six, sub},
    { one, two, three, equal},
    { zero, point, minus, notimplemented},
  };
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
    if (!isfinite(v)) {
      /* first gets rid of very special cases... */
      if (isnan(v)) {
	static int toomany = 0;
	if ((the_state.func != &noop_func) || (++toomany % 7))
	  gtk_label_set_markup(GTK_LABEL(displayed_label),
			       _("Error"));
	else
	  /* ok, let's have some fun too... */
	  gtk_label_set_markup(GTK_LABEL(displayed_label),
			       _("Error\n"
				 "<span foreground=\"darkgrey\" size=\"smaller\">covert_channel/ack</span>\n"
				 "<span foreground=\"orange\" style=\"italic\">Beam request transmitted...</span>"));
      }
      else if (isinf(v)>0)
      {
        /* positive infinity */
        gtk_label_set_markup(GTK_LABEL(displayed_label),"&#x221E;");
      }
      else
      {
        /* only negative infinity remains... */
        gtk_label_set_markup(GTK_LABEL(displayed_label),"-&#x221E;");
      }
      return; /* short-cut out */
    } else {
      /* Manually build the display string ala %g */
      double value = v;
      int expof10;

      expof10 = (int) log10(fabs(value));
      value *= pow(10,-expof10);
      if (expof10 >= MAX_DISPLAY_CHARS)
	snprintf(dispstring,MAX_DISPLAY_MARKUP,"%.*g <small>e</small><sup><big>%d</big></sup>",(MAX_DISPLAY_CHARS-2),value,expof10);
      else
	snprintf(dispstring,MAX_DISPLAY_MARKUP,"%.*g",MAX_DISPLAY_CHARS,v);
    }
  } else {
    /* We display entered value including trailing 0s */
    if (the_state.decimal_point == TRUE)
      snprintf(dispstring,MAX_DISPLAY_MARKUP,"%#.*f",the_state.n_fractional,v);
    else
      snprintf(dispstring,MAX_DISPLAY_MARKUP,"%.*f",the_state.n_fractional,v);
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
    if (the_state.current_operand != 0.0) {
      the_state.current_operand *= -1;
      update_display(the_state.current_operand);
    } else if (the_state.last_operand != 0.0) {
      the_state.last_operand *= -1;
      update_display(the_state.last_operand);
    }
    /* else: no-op */
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
static GtkWidget*
calc_panel_init (void)
{

  GtkWidget *table;
  int i, j;

  table = gtk_table_new (CALC_ROWS, CALC_COLS, TRUE);
  gtk_widget_set_name (table, "calculator-table" );

  for (j = 0; j < CALC_COLS; j++)
    for (i = 0; i < CALC_ROWS; i++)
    {
      GtkWidget* button = gtk_button_new_with_label( gettext(label[i][j]) );
      //GtkWidget* blabel = gtk_label_new(NULL);
      //gtk_label_set_markup(GTK_LABEL(blabel),gettext(label[i][j]));
      //gtk_container_add(GTK_CONTAINER(button),blabel);

      g_signal_connect (G_OBJECT(button), "clicked", G_CALLBACK(calc_button_pressed),&(ops[i][j]));

        if ((j == (CALC_COLS-1)) && (i == (CALC_ROWS-2))) {
        /* Last button spans two cells vertically */
        gtk_table_attach_defaults (GTK_TABLE (table), button,
                        j, j + 1, i, i + 2);
        i++;
            } else {
        gtk_table_attach_defaults (GTK_TABLE (table), button,
                        j, j + 1, i, i + 1);
            }
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
    GtkWidget* vbox;
    GtkWidget* table;

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

    /* application */
    g_set_application_name( _("Calculator") );

    /* main window */
    GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    /* connect close event */
    g_signal_connect( G_OBJECT(window), "delete_event", G_CALLBACK(gtk_main_quit), NULL );

    /* contents */
    vbox = gtk_vbox_new(FALSE, 10);

    displayed_label = gtk_label_new("123");
    gtk_widget_set_name (displayed_label, "calculator-display");
    gtk_misc_set_alignment(GTK_MISC(displayed_label), 1, 0);
    update_display(0.0);

    GtkWidget *display_frame;
    display_frame = gtk_frame_new (NULL);
    gtk_container_add (GTK_CONTAINER (display_frame), displayed_label);

    GtkWidget *display_eventbox = gtk_event_box_new ();
    gtk_widget_set_name (display_eventbox, "calculator-display-background");
    gtk_container_add (GTK_CONTAINER (display_eventbox), display_frame);
    gtk_box_pack_start( GTK_BOX(vbox), display_eventbox, FALSE, FALSE, 0);

    table = calc_panel_init();
    gtk_box_pack_start( GTK_BOX(vbox), table, TRUE, TRUE, 0);

    GtkWidget *main_frame = gtk_frame_new (NULL);
    gtk_frame_set_shadow_type (GTK_FRAME (main_frame), GTK_SHADOW_NONE);
    gtk_widget_set_name (main_frame, "calculator-frame");
    gtk_container_add (GTK_CONTAINER (main_frame), vbox);
    gtk_container_add (GTK_CONTAINER (window), main_frame);

    /* show everything and run main loop */
    gtk_widget_show_all( GTK_WIDGET(window) );
    calc_debug( "calculator entering main loop" );
    gtk_main();
    calc_debug( "calculator left main loop" );

    return 0;
}
