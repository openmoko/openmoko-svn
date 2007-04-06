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
#include <string.h>
#include <libecal/e-cal.h>
#include <libecal/e-cal-component.h>
#include "moko-journal.h"
#include "moko-time-priv.h"

static const int JOURNAL_MAX_SIZE = 100 ;

struct _MokoJournal
{
  ECal *ecal ;
  MokoJEntry **entries ;
  int next_entry ;
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

struct _MokoJEntryInfo
{
  MokoJEntryType type;
  const gchar *type_as_string ;
};
typedef struct _MokoJEntryInfo MokoJEntryInfo ;

static const MokoJEntryInfo entries_info[] =
{
  {EMAIL_JOURNAL_ENTRY, "EMAILENTRY"},
  {SMS_JOURNAL_ENTRY, "SMSENTRY"},
  {MMS_JOURNAL_ENTRY, "MMSENTRY"},
  {CALL_JOURNAL_ENTRY, "CALLENTRY"},
  {0}
} ;

static const gchar*
entry_type_to_string (MokoJEntryType a_type)
{
  MokoJEntryInfo *cur ;

  for (cur = (MokoJEntryInfo*)entries_info ; cur ; ++cur)
  {
    if (cur->type == a_type)
      return cur->type_as_string ;
  }
  return NULL ;
}

/*
static MokoJEntryType
entry_type_from_string (const gchar* a_str)
{
  MokoJEntryInfo *cur ;

  for (cur = (MokoJEntryInfo*)entries_info ; cur ; ++cur)
  {
    if (!strcmp (cur->type_as_string, a_str))
    {
      return cur->type ;
    }
  }
  return UNDEF_ENTRY ;
}
*/

/*<private funcs>*/
static MokoJournal*
moko_journal_alloc ()
{
  MokoJournal *result ;
  result = g_new0 (MokoJournal, 1) ;
  result->entries = g_new0 (MokoJEntry*, JOURNAL_MAX_SIZE) ;
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
  if (a_journal->entries)
  {
    int i ;
    for (i=0 ; i < JOURNAL_MAX_SIZE ; ++i)
    {
      if (a_journal->entries[i])
      {
        moko_j_entry_free (a_journal->entries[i]) ;
        a_journal->entries[i] = NULL ;
      }
    }
    g_free (a_journal->entries) ;
    a_journal->entries = NULL ;
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

static gboolean
moko_j_entry_type_is_valid (MokoJEntryType a_type)
{
  if (a_type > 0 && a_type < NB_OF_ENTRY_TYPES)
    return TRUE ;
  return FALSE ;
}

static gboolean
moko_j_entry_to_icalcomponent (MokoJEntry *a_entry,
                               icalcomponent **a_comp)
{
  icalcomponent *comp = NULL ;
  icalproperty *prop = NULL ;
  gboolean result = FALSE ;

  g_return_val_if_fail (a_entry, FALSE) ;
  g_return_val_if_fail (a_comp, FALSE) ;
  g_return_val_if_fail (moko_j_entry_type_is_valid (a_entry->type),
                        FALSE) ;

  comp = icalcomponent_new (ICAL_VJOURNAL_COMPONENT) ;

  /*add contact prop*/
  prop = icalproperty_new_contact (moko_j_entry_get_contact_uid (a_entry)) ;
  icalcomponent_add_property (comp, prop) ;

  /*add summary prop*/
  prop = icalproperty_new_summary (moko_j_entry_get_summary (a_entry)) ;
  icalcomponent_add_property (comp, prop) ;

  /*add dtstart*/
  const MokoTime *date = moko_j_entry_get_dtstart (a_entry) ;
  prop = NULL ;
  if (!date)
    goto out ;

  prop = icalproperty_new_dtstart (date->t) ;
  icalcomponent_add_property (comp, prop) ;

  /*add entry type*/
  prop = icalproperty_new_x
                  (entry_type_to_string (moko_j_entry_get_type (a_entry))) ;
  icalproperty_set_x_name (prop, "X_OPENMOKO_ENTRY_TYPE") ;
  icalcomponent_add_property (comp, prop) ;

  switch (moko_j_entry_get_type (a_entry))
  {
    case UNDEF_ENTRY:
      g_warning ("entry is of undefined type\n") ;
      return FALSE ;
    case EMAIL_JOURNAL_ENTRY:
      {
        MokoJEmailInfo *info=NULL ;
        if (!moko_j_entry_get_email_info (a_entry, &info) || !info)
          goto out ;
        if (moko_j_email_info_get_was_sent (info))
          prop = icalproperty_new_x ("YES") ;
        else
          prop = icalproperty_new_x ("NO") ;
        icalproperty_set_x_name (prop, "X_OPENMOKO_EMAIL_WAS_SENT") ;
        icalcomponent_add_property (comp, prop) ;
      }
      break ;
    case SMS_JOURNAL_ENTRY:
      break ;
    case MMS_JOURNAL_ENTRY:
      break ;
    case CALL_JOURNAL_ENTRY:
      break ;
    default:
      break ;
  }

  *a_comp = comp ;
  prop = NULL ;
  comp = NULL ;
  result = TRUE ;
out:
  if (prop)
    icalproperty_free (prop) ;

  if (comp)
    icalcomponent_free (comp) ;

  return result ;
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
 * moko_journal_add_entry:
 * @journal: the current instance of journal
 * @entry: the new entry to add to the journal. The journal is responsible
 * of deallocating the memory of the entry object.
 *
 * Add a journal entry to the journal
 *
 * Return value: TRUE if the entry got successfully added to the journal,
 * FALSE otherwise
 */
gboolean
moko_journal_add_entry (MokoJournal *a_journal, MokoJEntry *a_entry)
{
  g_return_val_if_fail (a_journal, FALSE) ;
  g_return_val_if_fail (a_entry, FALSE) ;
  g_return_val_if_fail (a_journal->next_entry < JOURNAL_MAX_SIZE, FALSE) ;

  if (a_journal->entries[a_journal->next_entry])
  {
    moko_j_entry_free (a_journal->entries[a_journal->next_entry]) ;
  }

  a_journal->entries[a_journal->next_entry] = a_entry ;
  a_journal->next_entry++ ;
  if (a_journal->next_entry >= JOURNAL_MAX_SIZE)
  {
    a_journal->next_entry = 0 ;
  }

  return TRUE ;
}

gboolean
moko_journal_write_to_storage (MokoJournal *a_journal)
{
  MokoJEntry *cur_entry=NULL ;
  int i=0 ;
  GList *ecal_comps=NULL, *cur_elem=NULL ;
  ECalComponent *ecal_comp=NULL ;
  icalcomponent *ical_comp=NULL ;
  GError *error=NULL ;
  gchar *uid=NULL ;
  gboolean result = TRUE;

  g_return_val_if_fail (a_journal, FALSE) ;
  g_return_val_if_fail (a_journal->ecal, FALSE) ;

  for (i=0; i<JOURNAL_MAX_SIZE; ++i)
  {
    cur_entry = a_journal->entries[i] ;
    if (!cur_entry)
      break ;
    if (!moko_j_entry_to_icalcomponent (cur_entry, &ical_comp))
    {
      if (ical_comp)
      {
        icalcomponent_free (ical_comp) ;
        ical_comp = NULL ;
      }
      continue ;
    }
    ecal_comp = e_cal_component_new () ;
    e_cal_component_set_icalcomponent (ecal_comp, ical_comp) ;
    ecal_comps = g_list_prepend (ecal_comps, ecal_comp) ;
    ecal_comp = NULL ;
    ical_comp = NULL ;
  }

  for (cur_elem = ecal_comps ; cur_elem ; cur_elem = cur_elem->next)
  {
    if (!cur_elem->data)
    {
      g_warning ("Got an empty ECalComponent\n") ;
      result = FALSE ;
      continue ;
    }
    ical_comp = e_cal_component_get_icalcomponent (cur_elem->data) ;
    if (!ical_comp)
    {
      g_warning ("Got an empty icalcomponent") ;
      result = FALSE ;
      continue ;
    }
    if (!e_cal_create_object (a_journal->ecal, ical_comp, &uid, &error))
    {
      g_warning ("Could not write the entry to the journal") ;
      result = FALSE ;
    }
    else
    {
      e_cal_component_commit_sequence (cur_elem->data) ;
    }
    if (error)
    {
      g_warning ("got error: %s\n", error->message) ;
      g_error_free (error) ;
      error = NULL ;
    }
    if (uid)
    {
      g_free (uid) ;
      uid = NULL ;
    }
  }
  return result ;
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
moko_j_entry_new (MokoJEntryType a_type)
{
  MokoJEntry *result ;
  result = moko_j_entry_alloc () ;
  result->type = a_type ;
  return result ;
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

  if (!a_entry->dtstart)
    a_entry->dtstart = moko_time_new_today () ;

  return a_entry->dtstart ;
}

/**
 * moko_j_entry_set_dtstart:
 * @entry: the current instance of journal entry
 * @dtstart: the new starting date associated to the journal entry.
 */
void
moko_j_entry_set_dtstart (MokoJEntry *a_entry, MokoTime* a_dtstart)
{
  g_return_if_fail (a_entry) ;

  if (a_entry->dtstart)
  {
    moko_time_free (a_entry->dtstart) ;
    a_entry->dtstart = NULL ;
  }

  if (a_dtstart)
    a_entry->dtstart = a_dtstart ;
}

/**
 * moko_j_entry_get_email_info:
 * @entry: the current instance of journal entry
 * @info: extra information attached to the email info, or NULL.
 * Client code must *NOT* of deallocate the returned info.
 * It is the duty of the MokoJEntry code to deallocate it when
 * necessary
 *
 * Return value: TRUE if the call succeeded, FALSE otherwise.
 */
gboolean
moko_j_entry_get_email_info (MokoJEntry *a_entry,
                             MokoJEmailInfo **a_info)
{
  g_return_val_if_fail (a_entry->type == EMAIL_JOURNAL_ENTRY, FALSE) ;

  if (!a_entry->extra_info.email_info)
  {
    a_entry->extra_info.email_info = moko_j_email_info_new () ;
  }
  g_return_val_if_fail (a_entry->extra_info.email_info, FALSE) ;

  *a_info = a_entry->extra_info.email_info ;
  return TRUE ;
}

/**
 * moko_j_email_info_get_was_sent:
 * @info: the current instance of email info
 *
 * Get a boolean property stating if the email was sent or received.
 *
 * Return value: TRUE if the email was sent, false if it was received
 */
gboolean
moko_j_email_info_get_was_sent (MokoJEmailInfo *a_info)
{
  g_return_val_if_fail (a_info, FALSE) ;
  return a_info->was_sent ;
}

/**
 * moko_j_email_info_set_was_sent:
 * @info: the current instance of email info
 * @was_sent: TRUE if the email was sent, FALSE if it was received
 *
 * Set a boolean property stating if the email was sent or received
 */
void
moko_j_email_info_set_was_sent (MokoJEmailInfo *a_info, gboolean a_was_sent)
{
  g_return_if_fail (a_info) ;
  a_info->was_sent = a_was_sent ;
}

/*</public funcs>*/
