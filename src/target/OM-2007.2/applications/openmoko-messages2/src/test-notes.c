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
  
  g_type_init ();
  
  store = jana_ecal_store_new (JANA_COMPONENT_NOTE);
  jana_store_open (store);
  
  note = jana_ecal_note_new ();
  jana_note_set_author (note, "0123456789");
  jana_note_set_body (note, "Test Message");
  
  jana_store_add_component (store, JANA_COMPONENT (note));
  
  g_printf ("Add a new note:\n\tAuthor: %s\n\t  Body: %s\n",
	    jana_note_get_author (note),
	    jana_note_get_body (note));
  
  return 0;
}
