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
#define __MOKO_JOURNAL_H__

#include <glib.h>
#include <glib-object.h>
#include "moko-time.h"

G_BEGIN_DECLS

#define MOKO_TYPE_JOURNAL             (moko_journal_get_type())
#define MOKO_JOURNAL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), MOKO_TYPE_JOURNAL, MokoJournal))
#define MOKO_JOURNAL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), MOKO_TYPE_JOURNAL, MokoJournalClass))
#define MOKO_IS_JOURNAL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MOKO_TYPE_JOURNAL))
#define MOKO_IS_JOURNAL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), MOKO_TYPE_JOURNAL))
#define MOKO_JOURNAL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), MOKO_TYPE_JOURNAL, MokoJournalClass))

#define MOKO_TYPE_LOCATION            (moko_location_get_type())
#define MOKO_TYPE_GSM_LOCATION        (moko_gsm_location_get_type())
#define MOKO_TYPE_JOURNAL_ENTRY       (moko_journal_entry_get_type())

/*
 * ***********************************************************
 * this API abstracts the process of adding
 * journal entries into the default system
 * journal. Journal entries contain information
 * about communation history like calls, sms, or,
 * emails issued or received.
 ***********************************************************/

typedef struct _MokoJournal             MokoJournal;
typedef struct _MokoJournalClass        MokoJournalClass;

typedef struct _MokoJournalEntry        MokoJournalEntry;

typedef struct _MokoLocation            MokoLocation;
typedef struct _MokoGSMLocation         MokoGSMLocation;

struct _MokoJournal
{
  GObject parent;
};

struct _MokoJournalClass
{
  GObjectClass parent_class;

  void (*entry_added)   (MokoJournal            *journal,
                         const MokoJournalEntry *entry);
  void (*entry_removed) (MokoJournal            *journal,
                         const MokoJournalEntry *entry);
} ;

/**
 * MokoJournalEntryType:
 * @UNDEF_ENTRY: used internally
 * @EMAIL_JOURNAL_ENTRY: E-mail
 * @SMS_JOURNAL_ENTRY: SMS calls
 * @VOICE_JOURNAL_ENTRY: voice calls
 * @FAX_JOURNAL_ENTRY: fax calls
 * @DATA_JOURNAL_ENTRY: data calls (like modules)
 * @NB_OF_ENTRY_TYPES: terminator of entry types
 *
 * Reresents the primary type of a journal entry.
 */
typedef enum {
  UNDEF_ENTRY,
  EMAIL_JOURNAL_ENTRY,
  SMS_JOURNAL_ENTRY,
  VOICE_JOURNAL_ENTRY,
  FAX_JOURNAL_ENTRY,
  DATA_JOURNAL_ENTRY,

  NB_OF_ENTRY_TYPES
} MokoJournalEntryType;

typedef enum {
  DIRECTION_IN,
  DIRECTION_OUT
} MessageDirection;

/**
 * MokoLocation:
 * @longitude: longitude of a journal entry
 * @latitude: latitude of a journal entry
 *
 * Geographical location of a journal entry
 */
struct _MokoLocation
{
  gfloat longitude;
  gfloat latitude;
};

/**
 * MokoGSMLocation:
 * @lac: local area code
 * @id: cell id
 *
 * GSM location of a journal entry
 */
struct _MokoGSMLocation
{
  gushort lac;
  gushort cid;
};

MokoJournal *moko_journal_open_default        (void);
void         moko_journal_close               (MokoJournal       *journal);
gboolean     moko_journal_add_entry           (MokoJournal       *journal,
                                               MokoJournalEntry  *entry);
gint         moko_journal_get_nb_entries      (MokoJournal       *journal) ;
gboolean     moko_journal_get_entry_at        (MokoJournal       *journal,
                                               guint              index_,
                                               MokoJournalEntry **entry) ;
gboolean     moko_journal_remove_entry_at     (MokoJournal       *journal,
                                               guint              index_);
gboolean     moko_journal_remove_entry_by_uid (MokoJournal       *journal,
                                               const gchar       *uid);
gboolean     moko_journal_write_to_storage    (MokoJournal       *journal);
gboolean     moko_journal_load_from_storage   (MokoJournal       *journal);

/* journal entries management */
MokoJournalEntry *    moko_journal_entry_new                     (MokoJournalEntryType  type);
MokoJournalEntryType  moko_journal_entry_get_entry_type          (MokoJournalEntry     *entry);
void                  moko_journal_entry_set_entry_type          (MokoJournalEntry     *entry,
                                                                  MokoJournalEntryType  type);
G_CONST_RETURN gchar *moko_journal_entry_get_uid                 (MokoJournalEntry     *entry);
G_CONST_RETURN gchar *moko_journal_entry_get_contact_uid         (MokoJournalEntry     *entry);
void                  moko_journal_entry_set_contact_uid         (MokoJournalEntry     *entry,
                                                                  const gchar          *uid);
G_CONST_RETURN gchar *moko_journal_entry_get_summary             (MokoJournalEntry     *entry);
void                  moko_journal_entry_set_summary             (MokoJournalEntry     *entry,
                                                                  const gchar          *summary);
gboolean              moko_journal_entry_get_start_location      (MokoJournalEntry     *entry,
                                                                  MokoLocation         *location);
gboolean              moko_journal_entry_set_start_location      (MokoJournalEntry     *entry,
                                                                  MokoLocation         *location);
gboolean              moko_journal_entry_get_direction           (MokoJournalEntry     *entry,
                                                                  MessageDirection     *direction);
void                  moko_journal_entry_set_direction           (MokoJournalEntry     *entry,
                                                                  MessageDirection      direction);
const MokoTime *      moko_journal_entry_get_dtstart             (MokoJournalEntry     *entry);
void                  moko_journal_entry_set_dtstart             (MokoJournalEntry     *entry,
                                                                  MokoTime             *dtstart);
const MokoTime *      moko_journal_entry_get_dtend               (MokoJournalEntry     *entry);
void                  moko_journal_entry_set_dtend               (MokoJournalEntry     *entry,
                                                                  MokoTime             *dtend);
G_CONST_RETURN gchar *moko_journal_entry_get_source              (MokoJournalEntry     *entry);
void                  moko_journal_entry_set_source              (MokoJournalEntry     *entry,
                                                                  const gchar          *source);
gboolean              moko_journal_entry_set_gsm_location        (MokoJournalEntry     *entry,
                                                                  MokoGSMLocation      *location);
gboolean              moko_journal_entry_get_gsm_location        (MokoJournalEntry     *entry,
                                                                  MokoGSMLocation      *location);
gboolean              moko_journal_entry_set_wifi_ap_mac_address (MokoJournalEntry     *entry,
                                                                  const guchar         *address);
const guchar *        moko_journal_entry_get_wifi_ap_mac_address (MokoJournalEntry     *entry);

/* voice call info */
gboolean              moko_journal_entry_has_voice_info          (MokoJournalEntry *entry);
void                  moko_journal_voice_info_set_distant_number (MokoJournalEntry *info,
                                                                  const gchar      *number);
G_CONST_RETURN gchar *moko_journal_voice_info_get_distant_number (MokoJournalEntry *info);
void                  moko_journal_voice_info_set_local_number   (MokoJournalEntry *info,
                                                                  const gchar      *number);
G_CONST_RETURN gchar *moko_journal_voice_info_get_local_number   (MokoJournalEntry *info);
void                  moko_journal_voice_info_set_was_missed     (MokoJournalEntry *info,
                                                                  gboolean          missing);
gboolean              moko_journal_voice_info_get_was_missed     (MokoJournalEntry *info);

/* fax call info */
gboolean moko_journal_entry_has_fax_info  (MokoJournalEntry *entry);
gboolean moko_journal_entry_has_data_info (MokoJournalEntry *entry);

/* sms info */
gboolean moko_journal_entry_has_sms_info (MokoJournalEntry *entry);

/* email info */
gboolean moko_journal_entry_has_email_info (MokoJournalEntry *entry);

/* helpers for bindings */
GType moko_journal_get_type (void) G_GNUC_CONST;
GType moko_location_get_type (void) G_GNUC_CONST;
GType moko_gsm_location_get_type (void) G_GNUC_CONST;
GType moko_journal_entry_get_type (void) G_GNUC_CONST;

MokoLocation* moko_location_copy (const MokoLocation *location);
void          moko_location_free (MokoLocation       *location);

MokoGSMLocation* moko_gsm_location_copy (const MokoGSMLocation *location);
void             moko_gsm_location_free (MokoGSMLocation       *location);

MokoJournalEntry* moko_journal_entry_ref   (MokoJournalEntry *entry);
void              moko_journal_entry_unref (MokoJournalEntry *entry);

G_END_DECLS

#endif /*__MOKO_JOURNAL_H__*/
