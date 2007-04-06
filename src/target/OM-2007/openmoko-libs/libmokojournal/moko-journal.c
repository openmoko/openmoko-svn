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
#include <libecal/e-cal.h>
#include "moko-journal.h"

struct _MokoJournal
{
  ECal *ecal ;
};

struct _MokoJEmailInfo
{
  gboolean was_sent ;
};

struct _MokoJEntry
{
  MokoJEntryType type;
  gchar *contact_uid ;
  gchar *summary ;
  MokoTime *dtstart ;
  MokoTime *dtend ;
  union
  {
    MokoJEmailInfo *email_info ;
  } extra_info ;
};

/*<private funcs>*/
static MokoJournal*
moko_journal_alloc ()
{
  MokoJournal *result ;
  result = g_new0 (MokoJournal, 1) ;
  return result ;
}

static void
moko_journal_free (MokoJournal *a_journal)
{
  g_return_if_fail (a_journal) ;
  if (a_journal->ecal)
  {
    g_object_unref (G_OBJECT (a_journal->ecal)) ;
    a_journal->ecal = NULL ;
  }
  g_free (a_journal) ;
}

MokoJEmailInfo*
moko_j_email_info_new ()
{
  return g_new0 (MokoJEmailInfo, 1) ;
}

void
moko_j_email_info_free (MokoJEmailInfo *a_info)
{
  g_return_if_fail (a_info) ;
  g_free (a_info) ;
}

static MokoJEntry*
moko_j_entry_alloc ()
{
  MokoJEntry *result ;

  result = g_new0 (MokoJEntry, 1) ;
  return result ;
}

static void
moko_j_entry_free_real (MokoJEntry *a_entry)
{
  g_return_if_fail (a_entry) ;

  if (a_entry->contact_uid)
  {
    g_free (a_entry->contact_uid) ;
    a_entry->contact_uid = NULL ;
  }
  if (a_entry->summary)
  {
    g_free (a_entry->summary) ;
    a_entry->summary = NULL ;
  }
  if (a_entry->dtstart)
  {
    moko_time_free (a_entry->dtstart) ;
    a_entry->dtstart = NULL ;
  }
  if (a_entry->dtend)
  {
    moko_time_free (a_entry->dtend) ;
    a_entry->dtend = NULL ;
  }

  switch (a_entry->type)
  {
    case EMAIL_JOURNAL_ENTRY:
      if (a_entry->extra_info.email_info)
      {
        moko_j_email_info_free (a_entry->extra_info.email_info) ;
        a_entry->extra_info.email_info = NULL ;
      }
      break ;
    case SMS_JOURNAL_ENTRY:
      break ;
    case MMS_JOURNAL_ENTRY:
      break ;
    case CALL_JOURNAL_ENTRY:
         break ;
    default:
         g_warning ("unknown journal entry type. This is a leak!\n") ;
         break ;
  }
}

/*</private funcs>*/


/*<public funcs>*/

/**
 * moko_journal_open_default:
 *
 * Opens the default journal.
 *
 * Return value: a pointer to the journal object
 */
MokoJournal*
moko_journal_open_default ()
{
  ECal *ecal = NULL;
  gchar *uri = NULL ;
  MokoJournal *result=NULL, *journal=NULL ;
  GError *error = NULL ;

  uri = g_build_filename ("file://", g_get_home_dir (),
                          ".moko", "journal", NULL);

  ecal = e_cal_new_from_uri (uri, E_CAL_SOURCE_TYPE_JOURNAL) ;
  if (!ecal)
  {
    g_warning ("failed to create ecal with uri: %s\n", uri) ;
    goto out ;
  }

  if (!e_cal_open (ecal, FALSE, &error))
  {
    g_warning ("could not open the journal at uri %s\n", uri) ;
    goto out ;
  }
  if (error)
  {
    g_warning ("got error: %s\n", error->message) ;
    goto out ;
  }
  journal = moko_journal_alloc () ;
  journal->ecal = ecal ;
  ecal = NULL ;

  result = journal ;
  journal = NULL ;

out:
  g_free (uri) ;

  if (ecal)
    g_object_unref (G_OBJECT (ecal)) ;

  if (journal)
    moko_journal_free (journal) ;

  if (error)
    g_error_free (error) ;

  return result ;
}

/**
 * moko_journal_close:
 * @journal: the journal to close
 *
 * Close the journal previously opened with moko_journal_open_default().
 * This function deallocates the memory of the Journal object.
 */
void
moko_journal_close (MokoJournal *a_journal)
{
  g_return_if_fail (a_journal) ;

  moko_journal_free (a_journal) ;
}

/**
 * moko_j_entry_new:
 *
 * Create a Journal entry with no properties set.
 * Use the JEntry accessors to get/set properties.
 *
 * Return value: the newly created journal entry object
 */
MokoJEntry*
moko_j_entry_new ()
{
  return moko_j_entry_alloc () ;
}

/**
 * moko_j_entry_free:
 * @entry: the entry to free
 *
 * Deallocate the memory of the journal entry object
 */
void
moko_j_entry_free (MokoJEntry *a_entry)
{
  g_return_if_fail (a_entry) ;

  moko_j_entry_free_real (a_entry) ;
}

/**
 * moko_j_entry_get_type:
 * @entry: the current journal entry
 *
 * get the primary type of the journal entry
 *
 * Return value: the type of the journal entry
 */
MokoJEntryType
moko_j_entry_get_type (MokoJEntry *a_entry)
{
  g_return_val_if_fail (a_entry, UNDEF_ENTRY) ;

  return a_entry->type ;
}


/**
 * moko_j_entry_set_type:
 * @entry: the current instance of journal entry
 * @type: the new type
 *
 * Set the type of the journal entry
 */
void
moko_j_entry_set_type (MokoJEntry *a_entry, MokoJEntryType a_type)
{
  g_return_if_fail (a_entry) ;
  g_return_if_fail (a_type != UNDEF_ENTRY) ;

  a_entry->type = a_type ;
}

/**
 * moko_j_entry_get_contact_uid:
 * @entry: the current instance of journal entry
 *
 * get the contact uid
 *
 * Return value: the UID of the contact. It can be NULL. Client code
 * must not deallocate or attempt to alter it.
 */
const gchar*
moko_j_entry_get_contact_uid (MokoJEntry *a_entry)
{
  g_return_val_if_fail (a_entry, NULL) ;

  return a_entry->contact_uid ;
}

void
moko_j_entry_set_contact_uid (MokoJEntry *a_entry, const gchar *a_uid)
{
  g_return_if_fail (a_entry) ;

  if (a_entry->contact_uid)
  {
    g_free (a_entry->contact_uid) ;
    a_entry->contact_uid = NULL ;
  }

  if (a_uid)
  {
    a_entry->contact_uid = g_strdup (a_uid) ;
  }
}

/**
 * moko_j_entry_get_summary:
 * @entry: the current instance of journal entry
 *
 * get the summary of the journal entry
 *
 * Return value: the summary of the journal entry. It can be NULL.
 * Client code must not deallocate or alter it.
 */
const gchar*
moko_j_entry_get_summary (MokoJEntry *a_entry)
{
  g_return_val_if_fail (a_entry, NULL) ;

  return a_entry->summary ;
}

/**
 * moko_j_entry_set_summary:
 * @entry: the current instance of journal entry
 * @summary: the new summary of the journal entry. It is copied
 * so client code is reponsible of its lifecyle.
 *
 * Set the summary of the journal entry
 */
void
moko_j_entry_set_summary (MokoJEntry *a_entry, const gchar* a_summary)
{
  g_return_if_fail (a_entry) ;

  if (a_entry->summary)
  {
    g_free (a_entry->summary) ;
    a_entry->summary = NULL ;
  }
  if (a_summary)
  {
    a_entry->summary = g_strdup (a_summary) ;
  }
}

/**
 * moko_j_entry_get_dtdstart:
 * @entry: the current instance of journal entry
 *
 * get the starting date associated to the journal entry
 *
 * Return value: an icaltimetype representing the starting date expected.
 * It can be NULL. Client code must not deallocate it.
 */
const MokoTime*
moko_j_entry_get_dtstart (MokoJEntry *a_entry)
{
  g_return_val_if_fail (a_entry, NULL) ;

  return a_entry->dtstart ;
}

/*<public funcs>*/
