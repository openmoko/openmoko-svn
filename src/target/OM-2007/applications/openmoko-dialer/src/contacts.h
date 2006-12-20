/***************************************************************************
 *            contacts.h
 *
 *  Wed Aug 16 19:44:25 2006
 *  Copyright  2006  Tony Guan
 *  Email tonyguan@fic-sh.com.cn
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
 
#ifndef _CONTACTS_H
#define _CONTACTS_H

#include "moko-dialer-declares.h"
#include "error.h"
#ifdef __cplusplus
extern "C"
{
#endif



	
/**
 * @brief phone number entry for the contact  structure in open dialer.
 */
typedef struct dialer_contact_entry {
  int ID;///<the unique ID for an contact entry
  char *desc; ///<the description of this entry,such as homenumber
  char *content; ///<the content of this entry, such as 62495726
  struct dialer_contact_entry* next;
}DIALER_CONTACT_ENTRY;


/**
 * @brief contact  structure for open dialer.
 */
typedef struct dialer_contact {
  int ID;///<the unique ID for an contact entry
  char *name;       	///<person name
  char *picpath;  ///<the picture file path for the person
  DIALER_CONTACT_ENTRY * entry;                    ///<first number entry for the person
  
  struct dialer_contact* next;         ///<pointer to next contact
}DIALER_CONTACT;


/**
 * @brief the structure for intelligent search results.
 */	
typedef struct dialer_ready_contact
{
	DIALER_CONTACT_ENTRY* p_entry;
	DIALER_CONTACT* p_contact;
}DIALER_READY_CONTACT;

/**
 * @brief contacts list head structure.
 */
typedef struct dialer_contacts_list_head {
  int length;                   ///<the number of contacts
  DIALER_CONTACT *contacts;        ///<package list head pointer
}DIALER_CONTACTS_LIST_HEAD;


int contact_init_contact_list(DIALER_CONTACTS_LIST_HEAD * head);
int contact_release_contact_entry(DIALER_CONTACT_ENTRY* contactentry);
int contact_release_contact(DIALER_CONTACT *contact);
int contact_release_contact_list(DIALER_CONTACTS_LIST_HEAD * head);
int contact_init_from_cmd(DIALER_CONTACTS_LIST_HEAD * head);
int contact_get_info_from_number(DIALER_CONTACT* contacts,char* name,char* picpath,const char* number);
int contact_init_contact_data(DIALER_CONTACTS_LIST_HEAD   *p_contactlist);
int contact_print_contact_list(DIALER_CONTACTS_LIST_HEAD * head);
int contact_string_has_sensentive (char * content, char *string);
#ifdef __cplusplus
}
#endif

#endif /* _CONTACTS_H */
