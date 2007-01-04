/***************************************************************************
 *            history.c
 *
 *  Wed Oct 25 18:25:13 2006
 *  Copyright  2006  User
 *  Email
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
 
#include "history.h" 
#include "error.h" 
#include <stdlib.h>
/**
 * @brief initialze the contact list by calling the external APIs.
 *
 * This function should be called once at the initial process.
 *
 * @param head DIALER_CONTACTS_LIST_HEAD head pointer
 * @return the number of the contacts. 
 * @retval
 */
int history_read_list(HISTORY_LIST_HEAD* historyhead)
{
	return 0;
}


/**
 * @brief read the history list using internal data,just for debug use
 * 
 * 
 *
 * @param head HISTORY_LIST_HEAD*,head pointer
 * @return the number of the history items 
 * @retval 0 failed 
 * @retval other success
 */
int history_read_list_cmd(HISTORY_LIST_HEAD* historyhead)
{
	
historyhead->length=0;
historyhead->first=0;
historyhead->last=0;
	history_add_entry(historyhead,MISSED,"","1391721112",
	"","17:58","12/20",100);

	history_add_entry(historyhead,INCOMING,"tony","13917309523",
	"./tony.png","13:58","12/20",10);
history_add_entry(historyhead,OUTGOING,"sally","13361900551",
	"./sally.png","10:58","12/20",106);
history_add_entry(historyhead,MISSED,"chaowei","1391110923",
	"./chaowei.png","11:58","12/20",120);
	history_add_entry(historyhead,OUTGOING,"","1395721111",
	"","17:58","12/20",100);

	history_add_entry(historyhead,INCOMING,"steven","1391721111",
	"./steven.png","17:58","12/20",100);
history_add_entry(historyhead,INCOMING,"ken","1381720923",
	"./ken.png","18:58","12/20",200);
	history_add_entry(historyhead,MISSED,"","1391721113",
	"","17:58","12/20",100);
	history_add_entry(historyhead,MISSED,"","1394721111",
	"","17:58","12/20",100);
	history_add_entry(historyhead,MISSED,"","1396721111",
	"","17:58","12/20",100);
return historyhead->length;

}
/* 
typedef struct historyentry
{
  int ID;///<the unique ID for an contact entry 
  HISTORY_TYPE type;
  char *name;       	///<person name
  char *picpath;  ///<the picture file path for the person
  char *time;
  int durationsec;
  struct historyentry* next;         ///<pointer to next entry
  struct historyentry* prev;         ///<pointer to next entry
}HISTORY_ENTRY;
*/

/**
 * @brief delete the supplied entry from the list 
 * 
 * 
 *
 * @param head HISTORY_LIST_HEAD*,head pointer
 * @param entry HISTORY_ENTRY*, the history entry to be deleted
 * @return 
 * @retval 0 failed
 * @retval 1 success
 */
int history_delete_entry(HISTORY_LIST_HEAD* historyhead,HISTORY_ENTRY* entry)
{
	DBG_ENTER();
	if(entry==0)return 0;
	if(entry->number==0)return 0;
	if(entry->prev==0&&entry->next==0)return 0;
		
	DBG_MESSAGE("deleting %s",entry->number);
	
	if(entry->prev)
	{
		entry->prev->next=entry->next;
	}
	else
	{//entry is the first one.
		historyhead->first=entry->next;
	}
	
	if(entry->next)
	{
		entry->next->prev=entry->prev;
	}
	else
	{
		historyhead->last=entry->prev;
	}

	history_release_entry(entry);

	historyhead->length--;
	return historyhead->length;
}



/**
 * @brief create a history entry, and add this entry to the list.
 * 
 * 
 *
 * @param name  const char*, the name of the counterpart
 * @param number const char*, the number of the counterpart
 * @param picpath const char*, the picture path of the counterpart
 * @param time char*, the time of that connection
 * @param date char*, the date of that connection
 * @param durationsec int, the duaration of that connection in seconds
 * @return 
 * @retval the newly created entry pointer
 */
HISTORY_ENTRY * history_add_entry(HISTORY_LIST_HEAD* historyhead, HISTORY_TYPE type,
const char *name,const char *number,const char *picpath,  char *time,char *date,int durationsec)
{

	DBG_ENTER();
	HISTORY_ENTRY * pentry=(HISTORY_ENTRY *)calloc(1,sizeof(HISTORY_ENTRY ));

//	DBG_MESSAGE("pentry add:0X%x",pentry);
	
	if(name&&strlen(name)>0)
	{	pentry->name=(char*)calloc(1,strlen(name)+1);
		strcpy(pentry->name,name);
		pentry->hasname=1;
	}
	else
		{
		pentry->name=0;
		pentry->hasname=0;
		}

	if(number&&strlen(number)>0)
	{
		pentry->number=(char*)calloc(1,strlen(number)+1);
		strcpy(pentry->number,number);
	}
	else
		{
		//release memory, and return;
		history_release_entry(pentry);
		return 0;
		}

	//DBG_MESSAGE("History add:0X%x,%s,%s,%s,%s,%s,%d",historyhead,name,number,picpath,time,date,durationsec);
	
	if(picpath&&strlen(picpath)>0)
	{
		pentry->picpath=(char*)calloc(1,strlen(picpath)+1);
		strcpy(pentry->picpath,picpath);
	}
		if(time&&strlen(time)>0)
	{
		pentry->time=(char*)calloc(1,strlen(time)+1);
		strcpy(pentry->time,time);
	}
		pentry->durationsec=durationsec;
	
	pentry->type=type;
	historyhead->length++;
	pentry->ID=historyhead->length;
	
	if(historyhead->first)
	{	//DBG_TRACE();
		//DBG_MESSAGE("first=0x%x",historyhead->first);
		//DBG_MESSAGE("first=%s",historyhead->first->number);
		historyhead->first->prev=pentry;
		//DBG_TRACE();
		pentry->next=historyhead->first;
		//DBG_TRACE();
		historyhead->first=pentry;
		//DBG_TRACE();
	}
	else
	{//DBG_TRACE();
		historyhead->first=pentry;
		historyhead->last=pentry;
	}
//	DBG_LEAVE();
	return pentry;
}

int history_release_entry(HISTORY_ENTRY * pentry)
{
if(!pentry)return 1;
if(pentry->name)
{
free(pentry->name);
pentry->name=0;
}
if(pentry->number)
{
free(pentry->number);
pentry->number=0;
}
if(pentry->picpath)
{
free(pentry->picpath);
pentry->picpath=0;
}
if(pentry->time)
{
free(pentry->time);
pentry->time=0;
}
if(pentry->date)
{
free(pentry->date);
pentry->date=0;
}

pentry->prev=0;
pentry->next=0;
free(pentry);
pentry=0;
return 1;

}

/**
 * @brief release the momory by the list and it's entry
 * 
 * 
 *
 * @param head HISTORY_LIST_HEAD*,head pointer
 * @return 
 * @retval 0 failed 
 * @retval 1 success
 */
int history_release_history_list(HISTORY_LIST_HEAD* historyhead)
{
	HISTORY_ENTRY * pentry;
	HISTORY_ENTRY * next;
	pentry=historyhead->first;
	while(pentry)
		{
		next=pentry->next;
		history_release_entry(pentry);
		pentry=next;
		}
	historyhead->first=0;
	historyhead->last=0;
	historyhead->length=0;
	return 1;
}

int history_init_history_data(HISTORY_LIST_HEAD* historyhead)
{

 DBG_ENTER();
  int res = history_read_list(historyhead);

  if(res == 0)
  {
    res = history_read_list_cmd (historyhead);
  }
  DBG_MESSAGE("History:%d",historyhead->length);
  DBG_LEAVE();
  return res;
}

