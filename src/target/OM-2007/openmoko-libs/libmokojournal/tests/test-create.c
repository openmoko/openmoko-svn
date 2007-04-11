#include <unistd.h>
#include <gtk/gtk.h>
#include "moko-journal.h"

int
main (int argc, char **argv)
{
    MokoJournal *journal=NULL ;
    MokoJEntry *entry=NULL ;
    MokoJEmailInfo *email_info=NULL ;
    int result = 1 ;

    g_type_init () ;

    journal = moko_journal_open_default () ;
    g_return_val_if_fail (journal, 1) ;

    if (!moko_journal_load_from_storage (journal))
    {
        g_message ("failed to load journal from storage\n") ;
        return FALSE ;
    }
    while (g_main_context_pending (g_main_context_default ()))
    {
        g_main_context_iteration (g_main_context_default (), FALSE) ;
    }
    g_message ("loaded journal from storage okay\n") ;
    g_message ("number journal entries: %d\n",
                moko_journal_get_nb_entries (journal)) ;

    entry = moko_j_entry_new (EMAIL_JOURNAL_ENTRY) ;
    if (!entry)
    {
        g_warning ("failed to create journal entry\n") ;
        goto out ;
    }

    moko_j_entry_set_contact_uid (entry, "foobarbazuid") ;
    moko_j_entry_set_summary (entry, "back from fostel") ;
    moko_j_entry_set_dtstart (entry, moko_time_new_today ()) ;
    if (!moko_j_entry_get_email_info (entry, &email_info) || !email_info)
    {
        g_warning ("failed to get email extra info from journal entry\n") ;
        goto out ;
    }
    moko_j_email_info_set_was_sent (email_info, TRUE) ;

    if (!moko_journal_add_entry (journal, entry))
    {
        g_warning ("could not add entry to journal\n") ;
        goto out ;
    }
    entry = NULL ;
    if (!moko_journal_write_to_storage (journal))
    {
        g_warning ("Could not write journal to storage") ;
        goto out ;
    }

    sleep (2) ;
    while (g_main_context_pending (g_main_context_default ()))
    {
        g_main_context_iteration (g_main_context_default (), FALSE) ;
    }
    g_message ("number journal entries after one got added: %d\n",
                moko_journal_get_nb_entries (journal)) ;

    result = 0;
    g_print ("test succeeded\n") ;

out:
    if (journal)
        moko_journal_close (journal) ;
    if (entry)
        moko_j_entry_free (entry) ;

    return result ;
}

