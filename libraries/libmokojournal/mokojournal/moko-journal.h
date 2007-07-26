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
#ifndef __MOKO_JOURNAL_H__
#define  __MOKO_JOURNAL_H__

#include <glib.h>
#include <glib-object.h>
#include "moko-time.h"

G_BEGIN_DECLS

#define MOKO_TYPE_JOURNAL             (moko_journal_get_type())
#define MOKO_JOURNAL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_JOURNAL, MokoJournal))
#define MOKO_JOURNAL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_JOURNAL, MokoJournalClass))
#define MOKO_IS_JOURNAL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_JOURNAL))
#define MOKO_IS_JOURNAL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_JOURNAL))

#define MOKO_TYPE_LOCATION            (moko_location_get_type())
#define MOKO_TYPE_GSM_LOCATION        (moko_gsm_location_get_type())
#define MOKO_TYPE_JOURNAL_ENTRY       (moko_journal_entry_get_type())

/************************************************************
 * this API abstracts the process of adding
 * journal entries into the default system
 * journal. Journal entries contain information
 * about communation history like calls, sms, or,
 * emails issued or received.
 ***********************************************************/

typedef struct _MokoJournal MokoJournal ;
typedef struct _MokoJournalClass MokoJournalClass ;
typedef struct _MokoJournalEntry MokoJournalEntry ;

struct _MokoJournal
{
  GObject parent;
} ;

struct _MokoJournalClass
{
  GObjectClass parent_class;

  void (*entry_added)   (MokoJournal *journal, const MokoJournalEntry *entry) ;
  void (*entry_removed) (MokoJournal *journal, const MokoJournalEntry *entry) ;
} ;

/**
 * this represents the primary type of
 * a journal entry.
 */
enum MokoJournalEntryType {
  UNDEF_ENTRY=0,
  EMAIL_JOURNAL_ENTRY,
  /*sms calls*/
  SMS_JOURNAL_ENTRY,
  /*voice calls*/
  VOICE_JOURNAL_ENTRY,
  /*fax call*/
  FAX_JOURNAL_ENTRY,
  /*data calls (like modems)*/
  DATA_JOURNAL_ENTRY,
  NB_OF_ENTRY_TYPES /*must always be the last*/
} ;

enum MessageDirection {
  DIRECTION_IN=0,
  DIRECTION_OUT
};

typedef struct
{
  gfloat longitude ;
  gfloat latitude ;
} MokoLocation ;

typedef struct
{
  /*local area code*/
  gushort lac ;
  /*cell id*/
  gushort cid ;
} MokoGSMLocation ;

/* The 'entry added' callback function */


/*<journal management>*/
/**
 * moko_journal_open_default:
 *
 * Opens the default journal.
 *
 * Return value: a pointer to the journal object
 */
MokoJournal* moko_journal_open_default () ;

/**
 * moko_journal_close:
 * @journal: the journal to close
 *
 * Close the journal previously opened with moko_journal_open_default().
 * This function deallocates the memory of the Journal object.
 */
void moko_journal_close (MokoJournal *journal) ;

/**
 * moko_journal_add_entry:
 * @journal: the current instance of journal
 * @entry: the new entry to add to the journal. Must be non NULL.
 * The journal is responsible
 * of deallocating the memory of the entry object.
 *
 * Add a journal entry to the journal
 *
 * Return value: TRUE if the entry got successfully added to the journal,
 * FALSE otherwise
 */
gboolean moko_journal_add_entry (MokoJournal *journal, MokoJournalEntry *entry) ;

/**
 * moko_journal_get_nb_entries:
 * @journal: the current instance of journal
 *
 * Return value: the number of entries in the journal or a negative value
 * in case of error.
 */
int moko_journal_get_nb_entries (MokoJournal *journal) ;

/**
 * moko_journal_get_entry_at:
 * @journal: the current instance of journal
 * @index: the index to get the journal entry from
 * @entry: out parameter. the resulting journal entry
 *
 * Get the journal entry at a given index.
 *
 * Return value: TRUE in case of success, FALSE otherwise.
 */
gboolean moko_journal_get_entry_at (MokoJournal *journal,
                                    guint index,
                                    MokoJournalEntry **entry) ;

/**
 * moko_journal_remove_entry_at:
 * @journal: the current instance of journal
 * @index: the index to remove the entry from
 *
 * Remove a journal entry from index #index
 *
 * Return value: TRUE in case of success, FALSE otherwise
 */
gboolean moko_journal_remove_entry_at (MokoJournal *journal,
                                       guint index) ;

/**
 * moko_journal_remove_entry_by_uid:
 * @journal: the current instance of journal
 * @uid: the uid of the journal entry to remove
 *
 * Remove the journal entry that has a given UID.
 *
 * Return value: TRUE in case of success, FALSE otherwise
 */
gboolean moko_journal_remove_entry_by_uid (MokoJournal *journal,
                                           const gchar* uid) ;

/**
 * moko_journal_write_to_storage:
 * @journal: the journal to save to storage
 *
 * Saves the journal to persistent storage (e.g disk) using the
 * appropriate backend. The backend currently used is evolution data server
 *
 * Return value: TRUE in case of success, FALSE otherwise
 */
gboolean moko_journal_write_to_storage (MokoJournal *journal) ;

/**
 * moko_journal_load_from_storage:
 * @journal: the journal to load entries into
 *
 * Read the journal entries stored in the persistent storage (filesystem)
 * and load then into the current instance of MokoJournal.
 *
 * Return value: TRUE in case of success, FALSE otherwise
 */
gboolean moko_journal_load_from_storage (MokoJournal *journal) ;

/*<journal entries querying>*/
/*</journal entries querying>*/

/*</journal management>*/


/*<journal entries management>*/

/**
 * moko_journal_entry_new:
 * @type: the type of journal entry
 *
 * Create a Journal entry with no properties set.
 * Use the JEntry accessors to get/set properties.
 *
 * Return value: the newly created journal entry object
 */
MokoJournalEntry* moko_journal_entry_new (enum MokoJournalEntryType type) ;

/**
 * moko_journal_entry_get_entry_type:
 * @entry: the current journal entry
 *
 * get the primary type of the journal entry
 *
 * Return value: the type of the journal entry
 */
enum MokoJournalEntryType moko_journal_entry_get_entry_type (MokoJournalEntry *entry);

/**
 * moko_journal_entry_set_type:
 * @entry: the current instance of journal entry
 * @type: the new type
 *
 * Set the type of the journal entry
 */
void moko_journal_entry_set_type (MokoJournalEntry *entry,
                                  enum MokoJournalEntryType type) ;

/**
 * moko_journal_entry_get_uid:
 * @entry: the current instance of journal entry
 *
 * Gets the UID of the current entry. This UID is non NULL if and
 * only if the entry has been persistet at least once.
 *
 * Return value: the UID in case the entry has been persisted at least once,
 * NULL otherwise. The client code must *NOT* free the returned string.
 */
const gchar* moko_journal_entry_get_uid (MokoJournalEntry *entry) ;

/**
 * moko_journal_entry_get_contact_uid:
 * @entry: the current instance of journal entry
 *
 * get the contact uid
 *
 * Return value: the UID of the contact. It can be NULL. Client code
 * must not deallocate or attempt to alter it.
 */
const gchar* moko_journal_entry_get_contact_uid (MokoJournalEntry *entry) ;

/**
 * moko_journal_entry_set_contact_uid:
 * @entry: the current instance of journal entry
 * @uid: the uid to set. This string is copied so the client code
 * must free it.
 *
 * Associate a new contact UID to the journal entry uid.
 * The UID is copied by this function so the caller is reponsible of
 * taking care of the uid string lifecycle.
 */
void  moko_journal_entry_set_contact_uid (MokoJournalEntry *entry,
                                          const gchar *uid) ;

/**
 * moko_journal_entry_get_summary:
 * @entry: the current instance of journal entry
 *
 * get the summary of the journal entry
 *
 * Return value: the summary of the journal entry. It can be NULL.
 * Client code must not deallocate or alter it.
 */
const gchar* moko_journal_entry_get_summary (MokoJournalEntry *entry) ;

/**
 * moko_journal_entry_set_summary:
 * @entry: the current instance of journal entry
 * @summary: the new summary of the journal entry. It is copied
 * so client code is reponsible of its lifecyle.
 *
 * Set the summary of the journal entry
 */
void moko_journal_entry_set_summary (MokoJournalEntry *entry,
                                     const gchar* summary) ;

/**
 * moko_journal_entry_get_start_location:
 * @entry: the current instance of journal entry
 * @location: the requested location
 *
 * Get the location at which the message got received or sent.
 *
 * Returns: TRUE upon sucessful completion, FALSE otherwise.
 */
gboolean moko_journal_entry_get_start_location (MokoJournalEntry *entry,
                                                MokoLocation *location) ;

/**
 * moko_journal_entry_set_location:
 * @entry: the current intance of journal entry
 * @location: the new location
 *
 * Set a new location to the journal entry
 * Location represents the longitude/latitude at which a call or message
 * occured.
 *
 * Returns: TRUE upon successful completion, FALSE otherwise.
 */
gboolean moko_journal_entry_set_start_location (MokoJournalEntry *entry,
                                                MokoLocation *location) ;

/**
 * moko_journal_entry_get_direction:
 * @entry: the current instance of journal entry
 * @direction: either DIRECTION_IN for a received message or DIRECTION_OUT
 * for a sent message.
 *
 * get the direction of the message
 *
 * Returns: TRUE in case of success, FALSE otherwise.
 */
gboolean moko_journal_entry_get_direction (MokoJournalEntry *entry,
                                           enum MessageDirection *direction) ;

/**
 * moko_journal_entry_set_direction:
 * @entry: the current instance of journal entry
 * @direction: the new message direction to set
 *
 * set message direction
 *
 */
void moko_journal_entry_set_direction (MokoJournalEntry *entry,
                                       enum MessageDirection direction) ;

/**
 * moko_journal_entry_get_dtdstart:
 * @entry: the current instance of journal entry
 *
 * get the starting date associated to the journal entry
 *
 * Returns: an icaltimetype representing the starting date expected.
 * It can be NULL. Client code must not deallocate it.
 */
const MokoTime* moko_journal_entry_get_dtstart (MokoJournalEntry *entry) ;

/**
 * moko_journal_entry_set_dtstart:
 * @entry: the current instance of journal entry
 * @dtstart: the new starting date associated to the journal entry.
 */
void moko_journal_entry_set_dtstart (MokoJournalEntry *entry, MokoTime* dtstart);

/**
 * moko_journal_entry_get_source:
 * @entry: the current instance of journal entry
 *
 * Returns: the source property. It is an arbitrary string representing
 * the application that was the source of the entry (like mokodialer)
 */
const gchar* moko_journal_entry_get_source (MokoJournalEntry *entry) ;

/**
 * moko_journal_entry_set_source:
 * @entry: the current instance of journal entry
 * @source: the new source to set
 *
 * Set the source property. It is an arbitrary string representing
 * the application that was the source of the entry (like mokodialer)
 */
void moko_journal_entry_set_source (MokoJournalEntry *entry,
                                    const gchar *source) ;
/**
 * moko_journal_entry_set_gsm_location:
 * @entry: the current instance of journal entry
 * @location: the gsm location
 *
 * Returns: TRUE upon completion, FALSE otherwise
 */
gboolean moko_journal_entry_info_set_gsm_location (MokoJournalEntry *entry,
                                                   MokoGSMLocation *location) ;

/**
 * moko_journal_entry_get_gsm_location:
 * @entry: the current instance of journal entry
 *
 * Returns TRUE upon completion, FALSE otherwise
 */
gboolean moko_journal_entry_get_gsm_location (MokoJournalEntry *entry,
                                              MokoGSMLocation *location);

/**
 * moko_journal_entry_set_wifi_ap_mac_address:
 * @entry: the current instance of journal entry
 *
 * the mac address of the wifi access point.
 * It is must be a 48 bits long string of bytes.
 *
 * Returns: TRUE in case of success, FALSE otherwise.
 */
gboolean moko_journal_entry_set_wifi_ap_mac_address (MokoJournalEntry *entry,
                                                     const guchar *address) ;

/**
 * moko_journal_entry_get_wifi_ap_mac_address:
 * @entry: the current instance of journal entry
 *
 * Returns: the mac address of the wifi access point.
 * It is a 48 bits long string of bytes. The calling code must
 * *NOT* delete this pointer. This function can also return NULL if
 * no wifi access point mac address has been set in the entry.
 */
const guchar *moko_journal_entry_get_wifi_ap_mac_address
                                                  (MokoJournalEntry *entry) ;
/*<voice call info>*/

/**
 * moko_journal_entry_has_voice_info:
 * @entry: the current instance of journal entry
 *
 * Returns the specific property set associated to instance of MokoJournalEntry
 * of type VOICE_JOURNAL_ENTRY.
 *
 * Returns: TRUE if the type is of VOICE_JOURNAL_ENTRY, FALSE otherwise.
 */
gboolean moko_journal_entry_has_voice_info (MokoJournalEntry *entry);


/**
 * moko_journal_voice_info_set_distant_number:
 * @info: the current
 * @info: the extra property set attached to the voice call
 */
void moko_journal_voice_info_set_distant_number (MokoJournalEntry *info,
                                                gchar *number) ;

const gchar* moko_journal_voice_info_get_distant_number
                                                (MokoJournalEntry *info) ;

void moko_journal_voice_info_set_local_number (MokoJournalEntry *info,
                                                const gchar *number) ;

const gchar* moko_journal_voice_info_get_local_number
                                                (MokoJournalEntry *info) ;

void moko_journal_voice_info_set_was_missed (MokoJournalEntry *info,
                                             gboolean a_flag) ;

gboolean moko_journal_voice_info_get_was_missed (MokoJournalEntry *info) ;

/*</voice call info>*/

/*<fax call info>*/

/**
 * moko_journal_entry_has_fax_info:
 * @entry: the current instance of journal entry
 * @info: the fax info properties set
 *
 * get the extra properties set associated to journal entries of
 * type FAX_JOURNAL_ENTRY
 *
 * Returns: TRUE i, FALSE otherwise.
 */
gboolean moko_journal_entry_has_fax_info (MokoJournalEntry *entry) ;

/**
 * moko_journal_entry_has_data_info:
 * @entry: the current instance of journal entry
 * @info: the resulting properties set
 *
 * Get the extra properties set associated to journal entries of type
 * DATA_JOURNAL_ENTRY
 *
 * Returns: TRUE in case of success, FALSE otherwise.
 */
gboolean moko_journal_entry_has_data_info (MokoJournalEntry *entry) ;
/*</fax call info>*/

/*<sms info>*/
/**
 * moko_journal_entry_get_sms_info:
 * @entry: the current instance of journal entry
 * @info: the resulting properties set
 *
 * Get the extra properties set associated to journal entries of type
 * SMS_JOURNAL_ENTRY
 */
gboolean moko_journal_entry_has_sms_info (MokoJournalEntry *entry) ;
/*</sms info>*/

/*<email info>*/

/**
 * moko_journal_entry_get_email_info:
 * @entry: the current instance of journal entry
 * @info: extra information attached to the email info, or NULL.
 * Client code must *NOT* of deallocate the returned info.
 * It is the duty of the MokoJournalEntry code to deallocate it when
 * necessary
 *
 * Return value: TRUE if the call succeeded, FALSE otherwise.
 */
gboolean moko_journal_entry_has_email_info (MokoJournalEntry *entry) ;

/*</email info>*/


/*</journal entries management>*/

/*<helpers for bindings>*/
GType moko_journal_get_type (void) ;
GType moko_location_get_type (void) ;
GType moko_gsm_location_get_type (void) ;
GType moko_journal_entry_get_type (void) ;

MokoLocation* moko_location_copy (const MokoLocation*) ;
void          moko_location_free (MokoLocation*) ;

MokoGSMLocation* moko_gsm_location_copy (const MokoGSMLocation*);
void             moko_gsm_location_free (MokoGSMLocation*);

MokoJournalEntry* moko_journal_entry_ref (MokoJournalEntry*);
void              moko_journal_entry_unref (MokoJournalEntry*);

/*</helpers for bindings>*/

G_END_DECLS

#endif /*__MOKO_JOURNAL_H__*/
