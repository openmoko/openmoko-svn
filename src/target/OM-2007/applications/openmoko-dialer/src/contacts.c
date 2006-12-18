/*  contacts.c
 *
 *  Authored by Tony Guan<tonyguan@fic-sh.com.cn>
 *
 *  Copyright (C) 2006 FIC Shanghai Lab
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date) [$Author: Tony Guan $]
 */
#include "contacts.h" 

/**
 * @brief initialze the contact list, this will be called from outside, contactlist
*/
int contact_init_contact_data(DIALER_CONTACTS_LIST_HEAD   *p_contactlist)
{ 
	

  DBG_ENTER();

  int res = contact_init_contact_list (p_contactlist);

  if(res == -1)
  {
    res = contact_init_from_cmd (p_contactlist);
  }
  DBG_MESSAGE("CONTACTS:%d,list@0x%x,first@0x%x",p_contactlist->length,p_contactlist,p_contactlist->contacts);
  
  DBG_LEAVE();
  
  return res;
}


/**
 * @brief initialze the contact list by calling the external APIs.
 *
 * This function should be called once at the initial process.
 *
 * @param head DIALER_CONTACTS_LIST_HEAD head pointer
 * @return the number of the contacts. 
 * @retval
 */




int contact_init_contact_list(DIALER_CONTACTS_LIST_HEAD * head)
{
	
	return -1;
}

/**
 * @brief release all the allocated memory of a contact entry.
 * 
 * 
 *
 * @param head DIALER_CONTACTS_LIST_HEAD head pointer
 * @return the number of the contacts. 
 * @retval 1
 */
int contact_release_contact_entry(DIALER_CONTACT_ENTRY* contactentry)
{
	if(contactentry==0) return 1;
		if(contactentry->desc)
		{
		free(contactentry->desc);
		contactentry->desc=0;	
		}
		if(contactentry->content)
		{
			free(contactentry->content);
			contactentry->content=0;
		}
		contactentry->next=0;
		free(contactentry);
	
		return 1;
}

/**
 * @brief release all the allocated memory of a contact and the entry related
 * 
 * 
 *
 * @param contact DIALER_CONTACT *.
 * @return 
 * @retval 1
 */
int contact_release_contact(DIALER_CONTACT *contact)
{
	DIALER_CONTACT_ENTRY * entry=0;
	DIALER_CONTACT_ENTRY * nextentry=0;
	if(contact==0)return 1;
	entry=contact->entry;
	//free every entry
	while(entry)
	{
		nextentry=entry->next;
		contact_release_contact_entry(entry);
		entry=nextentry;
	}
	contact->entry=0;
	
	//free name
	if(contact->name)
	{
	free(contact->name);
	contact->name=0;
	}
	//free picpath
	if(contact->picpath)
	{
		free(contact->picpath);
		contact->picpath=0;
	}
	//free contact itself
	contact->entry=0;
	free(contact);
	return 1;
}

/**
 * @brief release all the allocated memory of a DIALER_CONTACTS_LIST_HEAD
 * 
 * 
 *
 * @param head DIALER_CONTACTS_LIST_HEAD head pointer
 * @return 
 * @retval 1
 */
int contact_release_contact_list(DIALER_CONTACTS_LIST_HEAD * head)
{
//	g_printf("releasing %s\n",contact->name);
	DIALER_CONTACT* contact=0;
	DIALER_CONTACT* nextcontact=0;
	if(head==0)return 1;
	contact=head->contacts;
	if(contact==0)
		return 1;
	
	while(contact)
	{
	//	g_printf("releasing %s\n",contact->name);
		nextcontact=contact->next;
		contact_release_contact(contact);
		contact=nextcontact;
	}
	head->length=0;
	head->contacts=0;
	return 1;
	
	
}

/**
 * @brief add a contact item to the contact list.
 * 
 * prepend a contact item to the contact list.
 *
 * @param head DIALER_CONTACTS_LIST_HEAD*,the head pointer
 * @param contact DIALER_CONTACT*, the contact itme to be added.
 * @return 
 * @retval 1-success, 0-failed.
 */


int contact_add_contact_to_list(DIALER_CONTACTS_LIST_HEAD * head,DIALER_CONTACT*   contact)
{
if(head==0)return 0;
if(contact==0)return 0;

if(head->contacts==0)
{
	//we are the first
	head->length=1;
	head->contacts=contact;
	contact->next=0;
	contact->ID=0;
}else
{
	contact->ID=head->length;
	contact->next=head->contacts;
	head->contacts=contact;
	head->length++;
	
}

	return contact->ID;
	
	
}

/**
 * @brief allocate memory for base information of an contact
 * 
 * allocate memory for base information of an contact,copy the name and picpath to the memory.
 *
 * @param name, char*, the name of the contact.
 * @param picpath, char*, the picture path of the contact.
 * @return the pointer to created contact item.
 * @retval 
 */


DIALER_CONTACT*  contact_new_contact(char* name, char* picpath)
{
	DIALER_CONTACT* nextcontact;

	nextcontact=(DIALER_CONTACT* )calloc(1,sizeof(DIALER_CONTACT));
	nextcontact->ID=0;
	nextcontact->name=(char *)calloc(1,30);
	strcpy(nextcontact->name,name);
	
	nextcontact->picpath=(char *)calloc(1,128);
	strcpy(nextcontact->picpath,picpath);
	
	return nextcontact;
}

/**
 * @brief add an entry to an existing contact structure.
 * 
 * 
 *
 * @param contact, DIALER_CONTACT*, the existing contact structure pointer.
 * @param desc, char*, the description field of the entry.
 * @param content, char*, the content field of the entry.
 * @return DIALER_CONTACT_ENTRY* , the newly created contact information entry.
 * @retval 
 */


DIALER_CONTACT_ENTRY* contact_add_entry(DIALER_CONTACT* contact, char* desc,char* content)
{
	
	DIALER_CONTACT_ENTRY* nextentry;
	
	if(contact==0)return 0;
	if(desc==0)return 0;
	if(content==0)return 0;
	nextentry=(DIALER_CONTACT_ENTRY*)calloc(1,sizeof(DIALER_CONTACT_ENTRY));
	nextentry->desc=(char *)calloc(1,30);
	strcpy(nextentry->desc,desc);
	nextentry->content=(char *)calloc(1,30);
	strcpy(nextentry->content,content);
	
	
	nextentry->next=contact->entry;
	contact->entry=nextentry;
	
	return nextentry;
}
/**
 * @brief initialize the contact list from the inner information,only for use tempararily
 * in the case that contact API returns NULL, we will call this function for debug.
 * 
 * 
 *
 * @param head, DIALER_CONTACTS_LIST_HEAD * 
 * @return the list length.
 * @retval 
 */

int contact_init_from_cmd(DIALER_CONTACTS_LIST_HEAD * head)
{
	DIALER_CONTACT* contact;
	if(head==0)return 0;
	contact=contact_new_contact("Tony Guan","./tony.png");
	contact_add_entry(contact,"cell","13917209523");
	contact_add_entry(contact,"work","02162495726");
	contact_add_contact_to_list(head,contact);
	
	contact=contact_new_contact("Sally Xu","./sally.png");
	contact_add_entry(contact,"cell","13361900551");
	contact_add_entry(contact,"work","02165538452");
	contact_add_contact_to_list(head,contact);
	
	contact=contact_new_contact("Chaowei Song","./chaowei.png");
	contact_add_entry(contact,"work1","02162495727");
	contact_add_entry(contact,"work4","02162495730");
	contact_add_entry(contact,"work5","02162495731");
	contact_add_contact_to_list(head,contact);
	
	contact=contact_new_contact("Ken Zhao","./ken.png");
	contact_add_entry(contact,"work2","02162495728");
	contact_add_contact_to_list(head,contact);
	
	contact=contact_new_contact("Steven Chen","./steven.png");
	contact_add_entry(contact,"work3","02162495729");
	contact_add_contact_to_list(head,contact);
	
	contact=contact_new_contact("10086","");
	contact_add_entry(contact,"work","10086");
	contact_add_contact_to_list(head,contact);

	return head->length;	
	
	
}

/**
 * @brief get the name and picture path from the contact list, indexed by the number provided.
 * in the case that contact API returns NULL, we will call this function for debug.
 * 
 * 
 *
 * @param contacts, the first contact pointer of the list.
 * @param name, char*, caller provided memory pointer.
 * @param picpath, char*, caller provided memory pointer.
 * @return 0-failed, 1-found the contact.
 * @retval 
 */

int contact_get_info_from_number(DIALER_CONTACT* contacts,char* name,char* picpath,const char* number)
{


	strcpy(name,"");
	strcpy(picpath,MOKO_DIALER_DEFAULT_PERSON_IMAGE_PATH);
	if(number==0)return 0;
	if(strlen(number)==0)return 0;
	
//  DIALER_CONTACT* contacts=g_contactlist.contacts;

DIALER_CONTACT_ENTRY* entry;
 
   while(contacts!= 0)
   {
     entry=contacts->entry;

	 while(entry)
	 {
	
	 //judge if the entry includes the string
		 
	 if(strcmp(entry->content,number)==0)
	 {	
	 strcpy(picpath,contacts->picpath);
     strcpy(name,contacts->name);
	// DBG_MESSAGE("Yeah, we know the owner is %s.",name);
	 
	 return 1;
	 }
     entry=entry->next;
	 }
    
	 
	 contacts= contacts->next;
	
  }
  //DBG_MESSAGE("Can not find the number.");
  return 0;
}


/**
 * @brief judge if the content includes the sensative string 
 *
 * @param content  the content
 * @param string   the sensatvie string
 * 
 * @return 0-no 1-yes
 */

int contact_string_has_sensentive (char * content, char *string)
{
	int i;
	g_printf("hassensentive:%s,%s\n",content,string);
	if(content==0) 
	return 0;
	
	if(string==0) 
	return 1;
	
	if(strlen(string)==0)
		return 1;
	if(strlen(string)>strlen(content))
		return 0;
	for(i=0;string[i];i++)
	{
		if(content[i]==0)
			return 1;
		if(string[i]==content[i])
			continue;
		return 0;
		
	}
	return 1;
}
int contact_print_contact_list(DIALER_CONTACTS_LIST_HEAD * head)
{
DIALER_CONTACT* contacts;
DIALER_CONTACT_ENTRY* entry;
 contacts=head->contacts;

DBG_MESSAGE("\n\nThere are %d contacts here:",head->length);

   while(contacts!= 0)
   {
   DBG_MESSAGE("CONTACT: name:%s",contacts->name);
     entry=contacts->entry;

	 while(entry)
	 {
	
	 DBG_MESSAGE("--%s:%s",entry->desc,entry->content);
        entry=entry->next;
	 }

      contacts= contacts->next;
  }
  return 0;

}
