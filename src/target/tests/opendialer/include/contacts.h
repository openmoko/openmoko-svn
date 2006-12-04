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
  struct dialer_contact* next;
}DIALER_CONTACT_ENTRY;
/**
 * @brief contact  structure for open dialer.
 */

typedef struct dialer_contact {
  int ID;///<the unique ID for an contact entry
  char *name;                       ///<person name
  DIALER_CONTACT_ENTRY * entry;                    ///<first number entry for the person
  
  struct dialer_contact* next;         ///<pointer to next contact
}DIALER_CONTACT;


/**
 * @brief contacts list head structure.
 */
typedef struct dialer_contacts_list_head {
  int length;                   ///<the number of contacts
  DIALER_CONTACT *contacts;        ///<package list head pointer
}DIALER_CONTACTS_LIST_HEAD;











#ifdef __cplusplus
}
#endif

#endif /* _CONTACTS_H */
