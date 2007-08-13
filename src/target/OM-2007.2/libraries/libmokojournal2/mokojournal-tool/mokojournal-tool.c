/* vi: set sw=2: */
/*
 * Copyright (C) 2007 by OpenMoko, Inc.
 * Written by OpenedHand Ltd <info@openedhand.com>
 * All Rights Reserved
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
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>
#include <glib-object.h>

#include "moko-journal.h"

static GMainLoop *mainloop = NULL;

/* Forwards */
static gboolean _list (MokoJournal *journal);
static void     _add (MokoJournal *journal);

/* Command line params */
static gchar *action = NULL;
static gchar *type   = NULL;
static gchar *number = NULL;
static gboolean missed = FALSE;
static gchar *direction = NULL;

static GOptionEntry entries[] =
{
  {
    "action",
    'a', 0,
    G_OPTION_ARG_STRING,
    &action,
    "The action to take [list|size]",
    NULL
  },
  {
    "type",
    't', 0,
    G_OPTION_ARG_STRING,
    &type,
    "The type of entry [all|voice]",
    NULL
  },
  {
    "number",
    'n', 0,
    G_OPTION_ARG_STRING,
    &number,
    "The number to add or search for",
    NULL
  },
  {
    "missed",
    'm', 0,
    G_OPTION_ARG_NONE,
    &missed,
    "Whether the call was missed",
    NULL
  },
  {
    "direction",
    'd', 0,
    G_OPTION_ARG_STRING,
    &direction,
    "The direction of the call",
    NULL
  }


};


int
main (int argc, gchar *argv[])
{
  MokoJournal *journal = NULL;
  GOptionContext *context;

  g_type_init ();

  /* Load options */
  context = g_option_context_new (" - mokojournal-tool Options");
  g_option_context_add_main_entries (context, entries, NULL);
  g_option_context_parse (context, &argc, &argv, NULL);

  /* Do the right thing */
  if (!action)
  {
    g_print ("You must specify an action\n");
    return EXIT_FAILURE;
  }

  /* Load the journal */
  journal = moko_journal_open_default ();
  g_return_val_if_fail (MOKO_IS_JOURNAL (journal), EXIT_FAILURE);

  /* Load all journal entries from the journal on storage */
  if (!moko_journal_load_from_storage (journal))
  {
    g_message ("Failed to load journal from storage\n") ;
    return EXIT_FAILURE;
  }

  if (strcmp (action, "list") == 0)
  {
    g_timeout_add (5000, (GSourceFunc)_list, (gpointer)journal);
  }
  else if (strcmp (action, "size") == 0)
  {
    g_timeout_add (5000, (GSourceFunc)_list, (gpointer)journal);
  }
  else
  {
    g_print ("%s is not a valid action\n", action);
    return EXIT_FAILURE;
  }

  mainloop =  g_main_loop_new (g_main_context_default (), FALSE); 
  g_main_loop_run (mainloop);

  moko_journal_close (journal) ;
  
  return 0;
}

static void
_add (MokoJournal *journal)
{
  if (strcmp (type, "voice") == 0)
  {
    MokoJournalEntry *entry = moko_journal_entry_new (VOICE_JOURNAL_ENTRY);
    
    if (strcmp (direction, "in") == 0)
      moko_journal_entry_set_direction (entry, DIRECTION_IN);
    else
      moko_journal_entry_set_direction (entry, DIRECTION_OUT);

    if (missed)
      moko_journal_entry_set_was_missed (entry, TRUE);

    moko_journal_voice_info_set_distant_number (entry, number);

    moko_journal_add_entry (journal, entry);

    moko_journal_write_to_storage (journal);
  }

  g_main_loop_quit (mainloop);
}

static gboolean
_list (MokoJournal *journal)
{
  gint len, i, t;

  if (!type || strcmp (type, "all") == 0)
    t = 0;
  else if (strcmp (type, "voice") == 0)
    t = VOICE_JOURNAL_ENTRY;
  else
    t = VOICE_JOURNAL_ENTRY;
  
  len = moko_journal_get_nb_entries (journal);
  g_print ("Journal size = %d\n", len);

  if (strcmp (action, "size"))
    g_main_loop_quit (mainloop);

  for (i = 0; i < len; i++)
  {
    MokoJournalEntry *entry = NULL;

    moko_journal_get_entry_at (journal, i, &entry);
    
    if (!entry)
      continue;

    if (t == VOICE_JOURNAL_ENTRY)
    {
      if (moko_journal_entry_has_voice_info (entry))
      {
        g_print ("Voice Entry\t%20s\t%s\n", 
                 moko_journal_voice_info_get_distant_number (entry),
                moko_journal_voice_info_get_was_missed (entry) ? "missed":"");
      }
    }

  }
  g_main_loop_quit (mainloop);
  return FALSE;
}

#if 0
int
main (int argc, gchar **argv)
{
    MokoJournal *journal=NULL ;
    MokoJournalEntry *entry=NULL ;
    MokoLocation loc ;
   MokoGSMLocation gsm_loc ;
    int result = 1 ;
    gchar *contact_uid;

    if (argc == 2)
      contact_uid = argv[1];
    else
      contact_uid = "foobarbazuid";

    g_type_init () ;

    /*open the journal*/
    journal = moko_journal_open_default () ;
    g_return_val_if_fail (MOKO_IS_JOURNAL (journal),1) ;

    /*load all journal entries from the journal on storage*/
    if (!moko_journal_load_from_storage (journal))
    {
        g_message ("failed to load journal from storage\n") ;
        goto out ;
    }
    g_message ("loaded journal from storage okay\n") ;
    g_message ("number journal entries: %d\n",
                moko_journal_get_nb_entries (journal)) ;

    /*create a journal entry of type 'email journal entry'*/
    entry = moko_journal_entry_new (EMAIL_JOURNAL_ENTRY) ;
    if (!entry)
    {
        g_warning ("failed to create journal entry\n") ;
        goto out ;
    }

    /*****************************
     * <fill the entry with data>
     *****************************/
    moko_journal_entry_set_contact_uid (entry, contact_uid) ;
    moko_journal_entry_set_summary (entry, "back from fostel") ;
    moko_journal_entry_set_dtstart (entry, moko_time_new_today ()) ;
    moko_journal_entry_set_direction (entry, DIRECTION_OUT) ;
    loc.latitude = 37.386013;
    loc.longitude =  10.082932;
    moko_journal_entry_set_start_location (entry, &loc) ;
    gsm_loc.lac = 68 ;
    gsm_loc.cid = 100 ;
    moko_journal_entry_set_gsm_location (entry, &gsm_loc) ;
    if (!moko_journal_entry_has_email_info (entry))
    {
        g_warning ("failed to get email extra info from journal entry\n") ;
        goto out ;
    }
    /*****************************
     * </fill the entry with data>
     *****************************/

    /*add the entry we created to the journal*/
    if (!moko_journal_add_entry (journal, entry))
    {
        g_warning ("could not add entry to journal\n") ;
        goto out ;
    }
    /*
     * the entry is now owned by the journal, make sure we won't ever
     * free it ourselves (by accident)
     */
    entry = NULL ;

    /*create a journal entry of type 'voice call journal entry'*/
    entry = moko_journal_entry_new (VOICE_JOURNAL_ENTRY) ;
    if (!entry)
    {
        g_warning ("failed to create journal entry\n") ;
        goto out ;
    }

    /*****************************
     * <fill the entry with data>
     *****************************/
    moko_journal_entry_set_contact_uid (entry, contact_uid) ;
    moko_journal_entry_set_summary (entry, "this was a call") ;
    moko_journal_entry_set_dtstart (entry, moko_time_new_today ()) ;
    moko_journal_entry_set_direction (entry, DIRECTION_OUT) ;
    loc.latitude = 43.386013;
    loc.longitude =  15.082932;
    moko_journal_entry_set_start_location (entry, &loc) ;
    gsm_loc.lac = 67 ;
    gsm_loc.cid = 200 ;
    moko_journal_entry_set_gsm_location (entry, &gsm_loc) ;
    if (!moko_journal_entry_has_voice_info (entry))
    {
        g_warning ("failed to get voice extra info from journal entry\n") ;
        goto out ;
    }
    /*****************************
     * </fill the entry with data>
     *****************************/

    /*add the entry we created to the journal*/
    if (!moko_journal_add_entry (journal, entry))
    {
        g_warning ("could not add entry to journal\n") ;
        goto out ;
    }
    /*
     * the entry is now owned by the journal, make sure we won't ever
     * free it ourselves (by accident)
     */
    entry = NULL ;

    /*sync the journal to persistent storage*/
    if (!moko_journal_write_to_storage (journal))
    {
        g_warning ("Could not write journal to storage") ;
        goto out ;
    }

    /*
     * sleep a bit to wait for possible notifications, in case
     * another process has added new journal entries as well
     * this is not mandatory, but is there for the sake of testing.
     */
    sleep (2) ;

    /*
     * notifications of new journal entries being added to the journal
     * is done via dbus, using the glib event loop to dispatch the
     * notifications. So let's give the event loop a chance to
     * let us notified of new entries that could have been added.
     * Note that when using gtk+, you usually don't have to do this,
     * as gtk+ does it for you magically.
     */
    while (g_main_context_pending (g_main_context_default ()))
    {
        g_main_context_iteration (g_main_context_default (), FALSE) ;
    }
    g_message ("number journal entries after two got added: %d\n",
                moko_journal_get_nb_entries (journal)) ;

    /*if we reached this point, the test has probably succeeded*/
    result = 0;
    g_print ("test succeeded\n") ;

out:
    if (journal)
        moko_journal_close (journal) ;
    if (entry)
        moko_journal_entry_unref (entry) ;

    return result ;
}

#endif
