/*
 *  test-notes.c: small test application to add notes to the system
 *
 *  Authored by OpenedHand Ltd <info@openedhand.com>
 *
 *  Copyright (C) 2008 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 */

#include <libjana/jana-note.h>
#include <libjana-ecal/jana-ecal.h>
#include <glib/gprintf.h>

int
main (int argc, char **argv)
{
  JanaNote *note;
  JanaStore *store;
  gchar *number = "0123456789";
  gchar *body = "Test Message";
  
  g_type_init ();

  if (argc >= 2)
    number = argv[1];

  if (argc >= 3)
    body = argv[2];

  store = jana_ecal_store_new (JANA_COMPONENT_NOTE);
  jana_store_open (store);
  
  note = jana_ecal_note_new ();
  jana_note_set_author (note, number);
  jana_note_set_body (note, body);

  if (argc >= 4)
  {
    jana_utils_component_insert_category (JANA_COMPONENT (note), "Sent", -1);
    jana_note_set_recipient (JANA_NOTE (note), argv[3]);
  }
  
  jana_store_add_component (store, JANA_COMPONENT (note));
  
  g_printf ("Add a new note:\n\tAuthor: %s\n\t  Body: %s\n",
	    jana_note_get_author (note),
	    jana_note_get_body (note));
  
  return 0;
}
