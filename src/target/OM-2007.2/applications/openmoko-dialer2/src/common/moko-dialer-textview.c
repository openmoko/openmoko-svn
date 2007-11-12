/*  moko-dialer-textview.c
 *
 *  Authored by:
 *    Tony Guan<tonyguan@fic-sh.com.cn>
 *    OpenedHand Ltd. <info@openedhand.com>
 *
 *  Copyright (C) 2006 FIC Shanghai Lab
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date) [$Author: Tony Guan $]
 
 */

#include <string.h>

#include "moko-dialer-textview.h"
#include "error.h"

G_DEFINE_TYPE (MokoDialerTextview, moko_dialer_textview, GTK_TYPE_TEXT_VIEW)
     enum
     {
       CLICKED_SIGNAL,
       HOLD_SIGNAL,
       LAST_SIGNAL
     };

static void
moko_dialer_textview_class_init (MokoDialerTextviewClass * class)
{

  GtkWidgetClass *widget_class;

  widget_class = GTK_WIDGET_CLASS (class);


  gtk_widget_class_install_style_property (widget_class,
      g_param_spec_int (
        "small_font",
        "Small Font",
        "Smallest font size for the display",
        0,
        128,
        10,
        G_PARAM_READABLE | G_PARAM_WRITABLE));
  gtk_widget_class_install_style_property (widget_class,
      g_param_spec_int (
        "medium_font",
        "Medium Font",
        "Medium font size for the display",
        0,
        128,
        15,
        G_PARAM_READABLE | G_PARAM_WRITABLE));
  gtk_widget_class_install_style_property (widget_class,
      g_param_spec_int (
        "large_font",
        "Large Font",
        "Largest font size for the display",
        0,
        128,
        20,
        G_PARAM_READABLE | G_PARAM_WRITABLE));



}


static void
moko_dialer_textview_realize (MokoDialerTextview *moko_dialer_textview, gpointer user_data)
{
  /* Get the initial size of the textview and make sure it does not become
   * smaller. This will prevent the widget resizing if the font size
   * is reduced later on
   */
  GtkRequisition r;
  gtk_widget_size_request (GTK_WIDGET (moko_dialer_textview), &r);
  gtk_widget_set_size_request (GTK_WIDGET (moko_dialer_textview), r.width, r.height);

}

static void
moko_dialer_textview_init (MokoDialerTextview * moko_dialer_textview)
{

  GtkTextView *textview = 0;
  GtkTextBuffer *buffer;
  gint large;

  textview = &moko_dialer_textview->textview;
  buffer = gtk_text_view_get_buffer (textview);
  moko_dialer_textview->font_desc_textview = NULL;
  moko_dialer_textview->tag_for_inputed = NULL;
//  moko_dialer_textview->tag_for_cursor = NULL;
//  moko_dialer_textview->tag_for_autofilled = NULL;

  GTK_WIDGET_UNSET_FLAGS (textview, GTK_CAN_FOCUS);
  gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), FALSE);
  gtk_text_view_set_accepts_tab (GTK_TEXT_VIEW (textview), FALSE);
  gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview), GTK_WRAP_CHAR);
  gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textview), FALSE);
  gtk_text_view_set_left_margin (GTK_TEXT_VIEW (textview), 1);
  gtk_text_view_set_right_margin (GTK_TEXT_VIEW (textview), 1);



  PangoFontDescription *font_desc_textview = NULL;
  font_desc_textview = pango_font_description_new ();

  /* get font sizes */
  gtk_widget_style_get (GTK_WIDGET (moko_dialer_textview), "large_font", &large, NULL);

  /* set the default font for the textview. */
  pango_font_description_set_size (font_desc_textview, large * PANGO_SCALE);
  gtk_widget_modify_font (GTK_WIDGET (moko_dialer_textview), font_desc_textview);

  if (font_desc_textview)
  {
    /* save it to the structure for later usage. */
    moko_dialer_textview->font_desc_textview = font_desc_textview;
  }

  moko_dialer_textview->sensed = FALSE;


  g_signal_connect (moko_dialer_textview, "realize", G_CALLBACK (moko_dialer_textview_realize), NULL);
}


GtkWidget *
moko_dialer_textview_new ()
{
  MokoDialerTextview *dp;

  dp = (MokoDialerTextview *) g_object_new (MOKO_TYPE_DIALER_TEXTVIEW, NULL);
  return GTK_WIDGET (dp);

}

/**
 * @brief moko_dialer_textview_set_color(MokoDialerTextview *moko_dialer_textview)
 *
 * set the text left to the cursor to black, and right to red.
 *
 * @param moko_dialer_textview the display area
 * @param len the char length,if it exceeds 13,then automatically decrease the size.
 * @return  void
 * @retval void
 */

void
moko_dialer_textview_set_color (MokoDialerTextview * moko_dialer_textview)
{

  GtkTextBuffer *buffer;
  GtkTextIter start, cursoriter;
  GtkTextIter end;
  gint small = 10, medium = 10, large = 10;

  PangoLayout *pl;
  gchar *text;
  int pl_w, pl_h, textview_w, textview_h;
  GdkWindow *textview_window;
  gboolean centre_v = TRUE;

  /* Obtaining the buffer associated with the widget. */
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (moko_dialer_textview));

  gtk_text_buffer_get_iter_at_mark (buffer,
                                    &cursoriter,
                                    gtk_text_buffer_get_insert (buffer));

  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_buffer_get_end_iter (buffer, &end);

  text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);

  /* no need to continue if we don't have any text */
  if (!text)
    return;
  if (text && (strlen (text) < 1))
    return;

  /* get font sizes */
  gtk_widget_style_get (GTK_WIDGET (moko_dialer_textview), "small_font", &small, NULL);
  gtk_widget_style_get (GTK_WIDGET (moko_dialer_textview), "medium_font", &medium, NULL);
  gtk_widget_style_get (GTK_WIDGET (moko_dialer_textview), "large_font", &large, NULL);

  /* create a pango layout to try different text sizes on */
  pl = pango_layout_new (gtk_widget_get_pango_context (GTK_WIDGET (moko_dialer_textview)));
  pango_layout_set_text (pl, text, -1);

  /* get the textview width */
  textview_window = gtk_text_view_get_window (GTK_TEXT_VIEW (moko_dialer_textview), GTK_TEXT_WINDOW_WIDGET);
  gdk_drawable_get_size (textview_window, &textview_w, &textview_h);

  /* try large size */
  pango_font_description_set_size (moko_dialer_textview->font_desc_textview, large * PANGO_SCALE);
  pango_layout_set_font_description (pl, moko_dialer_textview->font_desc_textview);
  pango_layout_get_pixel_size (pl, &pl_w, &pl_h);

  if (pl_w >= textview_w)
  {
    /* try medium size */
    pango_font_description_set_size (moko_dialer_textview->font_desc_textview, medium * PANGO_SCALE);
    pango_layout_set_font_description (pl, moko_dialer_textview->font_desc_textview);
    pango_layout_get_pixel_size (pl, &pl_w, &pl_h);

    /* set size to small if medium does not fit */
    if (pl_w >= textview_w)
    {
      pango_font_description_set_size (moko_dialer_textview->font_desc_textview, small * PANGO_SCALE);
      centre_v = FALSE;
    }
  }

  /* we only want to centre the text vertically for large and medium fonts */
  if (centre_v)
  {
    int padding = 0;
    padding = MAX(0, (textview_h - pl_h) / 2);
    gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (moko_dialer_textview), padding);
    gtk_text_view_set_pixels_below_lines (GTK_TEXT_VIEW (moko_dialer_textview), padding);
  }
  else
  {
    gtk_text_view_set_pixels_above_lines (GTK_TEXT_VIEW (moko_dialer_textview), 0);
    gtk_text_view_set_pixels_below_lines (GTK_TEXT_VIEW (moko_dialer_textview), 0);
  }

  gtk_widget_modify_font (GTK_WIDGET (moko_dialer_textview),
                          moko_dialer_textview->font_desc_textview);

  gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW (moko_dialer_textview),
                                      gtk_text_buffer_get_insert (buffer));

  g_free (text);
  g_object_unref (pl);
}



 /**
 * @brief moko_dialer_textview_insert  
 *
 * This function should be called upon the keypad is clicked in dialer window.
 *
 * @param button the button hit.
 * @param number the number to be added to the display.
 * @return  int
 * @retval 
 */

gint
moko_dialer_textview_insert (MokoDialerTextview * moko_dialer_textview,
                             const gchar * number)
{

  gint len = 0;

  GtkTextBuffer *buffer;
  GtkTextIter start;
  GtkTextIter end;
  GtkTextIter selectioniter, insertiter;
  GtkTextMark *selectmark, *insertmark;

  /* Obtaining the buffer associated with the widget. */
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (moko_dialer_textview));

  selectmark = gtk_text_buffer_get_selection_bound (buffer);
  insertmark = gtk_text_buffer_get_insert (buffer);

  /* get current cursor iterator */
  gtk_text_buffer_get_iter_at_mark (buffer, &insertiter, insertmark);
  gtk_text_buffer_get_iter_at_mark (buffer, &selectioniter, selectmark);

  /* to see whether there is a selection range. */
  if (gtk_text_iter_get_offset (&insertiter) !=
      gtk_text_iter_get_offset (&selectioniter))
  {
    /* first delete the range */
    gtk_text_buffer_delete (buffer, &selectioniter, &insertiter);
    insertmark = gtk_text_buffer_get_insert (buffer);
    gtk_text_buffer_get_iter_at_mark (buffer, &insertiter, insertmark);
  }

  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_buffer_get_end_iter (buffer, &end);


  len = gtk_text_buffer_get_char_count (buffer);
  gtk_text_buffer_insert (buffer, &end, number, -1);
  len = len + g_utf8_strlen (number, -1);

  /* reget the cursor iter. */
  insertmark = gtk_text_buffer_get_insert (buffer);
  /* get current cursor iterator */
  gtk_text_buffer_get_iter_at_mark (buffer, &insertiter, insertmark);
  /* get the inputed string lengh. */
  len = gtk_text_iter_get_offset (&insertiter);

  moko_dialer_textview_set_color (moko_dialer_textview);
  return len;
}


//get the input section of the textview 
//if ALL=true, get whole text
//else only get the inputed digits.
gchar *
moko_dialer_textview_get_input (MokoDialerTextview * moko_dialer_textview,
                                gboolean all_text)
{
  GtkTextBuffer *buffer;
  GtkTextIter start;
  GtkTextIter end;
  GtkTextIter insertiter;
  GtkTextMark *insertmark;

  /* Obtaining the buffer associated with the widget. */
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (moko_dialer_textview));

  // get current cursor iterator
  insertmark = gtk_text_buffer_get_insert (buffer);
  gtk_text_buffer_get_iter_at_mark (buffer, &insertiter, insertmark);

  //get start & end iterator
  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_buffer_get_end_iter (buffer, &end);

  /* FIXME: Should this check all_text too? */
  if (gtk_text_iter_get_offset (&insertiter) ==
      gtk_text_iter_get_offset (&start))
  {
    return NULL;
  }

  if (all_text)
    /* Get the entire buffer text. */
    return gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  else
    return gtk_text_buffer_get_text (buffer, &start, &insertiter, FALSE);

}

//delete all the input 
int
moko_dialer_textview_empty (MokoDialerTextview * moko_dialer_textview)
{
  GtkTextBuffer *buffer;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (moko_dialer_textview));

  gtk_text_buffer_set_text (buffer, "", -1);
  return 1;
}

///delete the selection or one character.
int
moko_dialer_textview_delete (MokoDialerTextview * moko_dialer_textview)
{
  GtkTextBuffer *buffer;
  GtkTextIter selectioniter, insertiter;
  GtkTextMark *selectmark, *insertmark;
  gint len;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (moko_dialer_textview));

  selectmark = gtk_text_buffer_get_selection_bound (buffer);
  insertmark = gtk_text_buffer_get_insert (buffer);
//get current cursor iterator
  gtk_text_buffer_get_iter_at_mark (buffer, &insertiter, insertmark);
  gtk_text_buffer_get_iter_at_mark (buffer, &selectioniter, selectmark);
  // to see whether there is a selection range.
  if (gtk_text_iter_get_offset (&insertiter) !=
      gtk_text_iter_get_offset (&selectioniter))
  {
    // yes, first delete the range.
    gtk_text_buffer_delete (buffer, &selectioniter, &insertiter);
  }
  else
  {
    /* no selection, then just perform backspace. */
    GtkTextIter enditer;

    gtk_text_buffer_get_end_iter (buffer, &enditer);
    gtk_text_buffer_backspace (buffer, &enditer, TRUE, TRUE);
  }

//now we get the inputed string length. 
  insertmark = gtk_text_buffer_get_insert (buffer);
  // get current cursor iterator
  gtk_text_buffer_get_iter_at_mark (buffer, &insertiter, insertmark);
  len = gtk_text_iter_get_offset (&insertiter);

  /* update colours */
  moko_dialer_textview_set_color (moko_dialer_textview);

  return 1;

}

//autofill the string to the inputed digits string on the textview

int
moko_dialer_textview_fill_it (MokoDialerTextview * moko_dialer_textview,
                              gchar * string)
{
  GtkTextBuffer *buffer;
  GtkTextIter start;
  GtkTextIter end;
  GtkTextIter insertiter;
  GtkTextMark *insertmark;
  gint offset;
  gint offsetend;
  gint offsetstart;

//DBG_ENTER();
//DBG_MESSAGE("Sensative string:%s",string);

/* Obtaining the buffer associated with the widget. */
  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (moko_dialer_textview));

//get current cursor iterator
  insertmark = gtk_text_buffer_get_insert (buffer);
  gtk_text_buffer_get_iter_at_mark (buffer, &insertiter, insertmark);
//get start & end iterator
  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_buffer_get_end_iter (buffer, &end);
  offsetend = gtk_text_iter_get_offset (&end);
  offset = gtk_text_iter_get_offset (&insertiter);
//if startpos=endpos, that means we didn't input anything
//so we just insert the text.

  offsetstart = gtk_text_iter_get_offset (&start);
  if (offsetend == offsetstart)
  {

    gtk_text_buffer_set_text (buffer, string, -1);

    gtk_text_buffer_get_start_iter (buffer, &start);
    gtk_text_buffer_place_cursor (buffer, &start);
    moko_dialer_textview_set_color (moko_dialer_textview);
    moko_dialer_textview->sensed = TRUE;
    // gtk_widget_grab_focus(text_view);
    return 1;

  }

/* Get the entire buffer text. */

//codestring = gtk_text_buffer_get_text (buffer, &start, &insertiter, FALSE);

  gtk_text_buffer_delete (buffer, &insertiter, &end);


//reget current cursor iterator
  insertmark = gtk_text_buffer_get_insert (buffer);
  gtk_text_buffer_get_iter_at_mark (buffer, &insertiter, insertmark);

//here we have to call the get sensentivestring to get "139" or something.
//gtk_text_buffer_insert_with_tags_by_name(buffer,&insertiter,"139",3,tag_name);
  if (string != 0)
  {
    gint len;
    len = g_utf8_strlen (string, -1);
    if (len > 0)
    {
      gtk_text_buffer_insert (buffer, &insertiter, string, len);

      //reget current cursor iterator
      insertmark = gtk_text_buffer_get_insert (buffer);
      gtk_text_buffer_get_iter_at_mark (buffer, &insertiter, insertmark);
      gtk_text_iter_set_offset (&insertiter, offset);
      // set the private data of sensed.
    }
  }

//setback the cursor position
  gtk_text_buffer_place_cursor (buffer, &insertiter);

  moko_dialer_textview_set_color (moko_dialer_textview);

//gtk_widget_grab_focus(text_view);
//g_free (codestring );

//DBG_LEAVE();
  return 1;
}

gint
moko_dialer_textview_confirm_it (MokoDialerTextview * moko_dialer_textview,
                                 const gchar * string)
{

  GtkTextBuffer *buffer;
  GtkTextIter end;

  buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (moko_dialer_textview));


  gtk_text_buffer_set_text (buffer, string, -1);


  gtk_text_buffer_get_end_iter (buffer, &end);
//set the cursor to the end of the buffer
  gtk_text_buffer_place_cursor (buffer, &end);

  moko_dialer_textview_set_color (moko_dialer_textview);
//

  return 1;

}
