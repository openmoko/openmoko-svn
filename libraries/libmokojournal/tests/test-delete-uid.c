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
#include <glib-object.h>
#include "moko-journal.h"

int
main ()
{
    MokoJournal *journal=NULL ;
    MokoJournalEntry *entry=NULL ;
    int result=-1 ;

    g_type_init () ;

    /*open the journal*/
    journal = moko_journal_open_default () ;
    g_return_val_if_fail (journal, -1) ;

    if (!moko_journal_load_from_storage (journal))
    {
        g_warning ("failed to load journal from storage\n") ;
        goto out ;
    }
    g_message ("initial number of journal entries: %d\n",
               moko_journal_get_nb_entries (journal)) ;

    /*remove all entries from journal starting from the oldest one*/
    while (moko_journal_get_nb_entries (journal) > 0)
    {
        entry = NULL ;
        /*get the oldest entry*/
        if (!moko_journal_get_entry_at (journal, 0, &entry) || !entry)
        {
            g_message ("failed to get entry at index 0\n") ;
            goto out ;
        }
        /*make sure it has an UID*/
        if (!moko_journal_entry_get_uid (entry))
        {
            g_message ("error: came accross an entry without UID\n") ;
            goto out ;
        }
        /*remove the entry from the journal, using its UID*/
        if (!moko_journal_remove_entry_by_uid
                                        (journal,
                                         moko_journal_entry_get_uid (entry)))
        {
            g_message ("failed to remove entry of UID '%s' from journal\n",
                       moko_journal_entry_get_uid (entry)) ;
            goto out ;
        }
    }
    /*write the modifications we did, back to persistent storage*/
    if (!moko_journal_write_to_storage (journal))
    {
        g_message ("failed to write to storage\n") ;
        goto out ;
    }

    if (moko_journal_get_nb_entries (journal) != 0)
    {
        g_message ("test failed, we still have %d entries left\n",
                   moko_journal_get_nb_entries (journal)) ;
        goto out ;
    }

    /*if we reached this place, the test is prolly successful*/
    g_message ("test succeeded") ;
    result = 0 ;

out:
    if (journal)
        moko_journal_close (journal) ;

    return result ;
}
