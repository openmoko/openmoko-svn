/***************************************************************************
 *            contacts.c
 *
 *  Wed Aug 16 23:45:09 2006
 *  Copyright  2006  tony
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
#include "contacts.h" 

/**
 * @brief init_contact_list
 *
 * This function should be called once at the initial process.
 *
 * @param head The main window widget pointer
 * @return  code
 * @retval 
 */

int init_contact_list(DIALER_CONTACTS_LIST_HEAD * head)
{
	
	return 0;
}
int release_contact_entry(DIALER_CONTACT_ENTRY* contactentry)
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

int release_contact(DIALER_CONTACT *contact)
{
	DIALER_CONTACT_ENTRY * entry=0;
	DIALER_CONTACT_ENTRY * nextentry=0;
	if(contact==0)return 1;
	entry=contact->entry;
	//free every entry
	while(entry)
	{
		nextentry=entry->next;
		release_contact_entry(entry);
		entry=nextentry;
	}
	contact->entry=0;
	
	//free name
	if(contact->name)
	{
	free(contact->name);
	contact->name=0;
	}
	
	//free contact itself
	contact->entry=0;
	free(contact);
	return 1;
}

int release_contact_list(DIALER_CONTACTS_LIST_HEAD * head)
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
		release_contact(contact);
		contact=nextcontact;
	}
	head->length=0;
	head->contacts=0;
	return 1;
	
	
}

int init_contact_list_cmd(DIALER_CONTACTS_LIST_HEAD * head)
{
	DIALER_CONTACT_ENTRY* entry;
	DIALER_CONTACT_ENTRY* nextentry;
	DIALER_CONTACT* contact;
	DIALER_CONTACT* nextcontact;

	nextentry=(DIALER_CONTACT_ENTRY*)calloc(1,sizeof(DIALER_CONTACT_ENTRY));
	nextentry->desc=(char *)calloc(1,30);
	strcpy(nextentry->desc,"cell");
	nextentry->content=(char *)calloc(1,30);
	strcpy(nextentry->content,"13917309523");
	
	entry=nextentry;
	
	nextentry=(DIALER_CONTACT_ENTRY*)calloc(1,sizeof(DIALER_CONTACT_ENTRY));
	nextentry->desc=(char *)calloc(1,30);
	strcpy(nextentry->desc,"work phone");
	nextentry->content=(char *)calloc(1,30);
	strcpy(nextentry->content,"62495726");
	entry->next=nextentry;

	
	

	nextcontact=(DIALER_CONTACT* )calloc(1,sizeof(DIALER_CONTACT));
	nextcontact->ID=0;
	nextcontact->name=(char *)calloc(1,30);
	strcpy(nextcontact->name,"Tony Guan");
	nextcontact->entry=entry;
	head->contacts=nextcontact;

	contact=nextcontact;
	head->length=1;

	//the second one;
	nextentry=(DIALER_CONTACT_ENTRY*)calloc(1,sizeof(DIALER_CONTACT_ENTRY));
	nextentry->desc=(char *)calloc(1,30);
	strcpy(nextentry->desc,"homephone");
	nextentry->content=(char *)calloc(1,30);
	strcpy(nextentry->content,"13917309365");

	nextcontact=(DIALER_CONTACT* )calloc(1,sizeof(DIALER_CONTACT));
	nextcontact->ID=0;
	nextcontact->name=(char *)calloc(1,30);
	strcpy(nextcontact->name,"Sally Xu");

	nextcontact->entry=nextentry;
	contact->next=nextcontact;
	head->length=	head->length+1;
	contact=nextcontact;

	//the 3rd one;
	nextentry=(DIALER_CONTACT_ENTRY*)calloc(1,sizeof(DIALER_CONTACT_ENTRY));
	nextentry->desc=(char *)calloc(1,30);
	strcpy(nextentry->desc,"companyphone");
	nextentry->content=(char *)calloc(1,30);
	strcpy(nextentry->content,"65173800");

	nextcontact=(DIALER_CONTACT* )calloc(1,sizeof(DIALER_CONTACT));
	nextcontact->ID=0;
	nextcontact->name=(char *)calloc(1,30);
	strcpy(nextcontact->name,"chaowei song");

	nextcontact->entry=nextentry;
	contact->next=nextcontact;
	head->length=	head->length+1;
	contact=nextcontact;
	
	
	
	//the 4th one;
	nextentry=(DIALER_CONTACT_ENTRY*)calloc(1,sizeof(DIALER_CONTACT_ENTRY));
	nextentry->desc=(char *)calloc(1,30);
	strcpy(nextentry->desc,"company phone");
	nextentry->content=(char *)calloc(1,30);
	strcpy(nextentry->content,"63173800");

	nextcontact=(DIALER_CONTACT* )calloc(1,sizeof(DIALER_CONTACT));
	nextcontact->ID=0;
	nextcontact->name=(char *)calloc(1,30);
	strcpy(nextcontact->name,"ken zhao");

	nextcontact->entry=nextentry;
	contact->next=nextcontact;
	head->length=	head->length+1;
	contact=nextcontact;
	
	
	//the 5th one;
	nextentry=(DIALER_CONTACT_ENTRY*)calloc(1,sizeof(DIALER_CONTACT_ENTRY));
	nextentry->desc=(char *)calloc(1,30);
	strcpy(nextentry->desc,"cell phone");
	nextentry->content=(char *)calloc(1,30);
	strcpy(nextentry->content,"12065899000");

	nextcontact=(DIALER_CONTACT* )calloc(1,sizeof(DIALER_CONTACT));
	nextcontact->ID=0;
	nextcontact->name=(char *)calloc(1,30);
	strcpy(nextcontact->name,"steven chen");

	nextcontact->entry=nextentry;
	contact->next=nextcontact;
	head->length=	head->length+1;
	contact=nextcontact;
	
	return 1;
	
}
