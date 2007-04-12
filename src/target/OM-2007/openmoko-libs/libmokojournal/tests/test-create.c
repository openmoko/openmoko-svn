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
#include <glib-object.h>
#include "moko-journal.h"

int
main ()
{
    MokoJournal *journal=NULL ;
    MokoJEntry *entry=NULL ;
    MokoJEmailInfo *email_info=NULL ;
    int result = 1 ;

    g_type_init () ;

    /*open the journal*/
    journal = moko_journal_open_default () ;
    g_return_val_if_fail (journal, 1) ;

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
    entry = moko_j_entry_new (EMAIL_JOURNAL_ENTRY) ;
    if (!entry)
    {
        g_warning ("failed to create journal entry\n") ;
        goto out ;
    }

    /*****************************
     * <fill the entry with data>
     *****************************/
    moko_j_entry_set_contact_uid (entry, "foobarbazuid") ;
    moko_j_entry_set_summary (entry, "back from fostel") ;
    moko_j_entry_set_dtstart (entry, moko_time_new_today ()) ;
    if (!moko_j_entry_get_email_info (entry, &email_info) || !email_info)
    {
        g_warning ("failed to get email extra info from journal entry\n") ;
        goto out ;
    }
    moko_j_email_info_set_was_sent (email_info, TRUE) ;
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
    g_message ("number journal entries after one got added: %d\n",
                moko_journal_get_nb_entries (journal)) ;

    /*if we reached this point, the test has probably succeeded*/
    result = 0;
    g_print ("test succeeded\n") ;

out:
    if (journal)
        moko_journal_close (journal) ;
    if (entry)
        moko_j_entry_free (entry) ;

    return result ;
}

