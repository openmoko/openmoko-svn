/***************************************************************************
 *            history.h
 *
 *  Wed Oct 25 18:15:38 2006
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
 
#ifndef _HISTORY_H
#define _HISTORY_H

#ifdef __cplusplus



extern "C"

{
#endif

	

/**
 * @brief enum of history type
 */
	
typedef enum _historytype{
	INCOMING=0, ///<incoming calls
	OUTGOING,///<outgoing calls
	MISSED,///<missed calls 
	ALL ///<all the types including the above
}HISTORY_TYPE;

/**
 * @brief history entry item structure
 */
typedef struct historyentry
{
  int ID;///<the unique ID for an contact entry 
  HISTORY_TYPE type;
  char *name;       	///<person name
  char *number;       	///<person number
  char *picpath;  ///<the picture file path for the person
  char *time;	///< start time of that talk 
  char *date;///<start date  of that talk
  int durationsec;///<seconds of the duaration 
  struct historyentry* next;         ///<pointer to next entry
  struct historyentry* prev;         ///<pointer to next entry
  int hasname;
}HISTORY_ENTRY;


/**
 * @brief contacts list head structure.
 */
typedef struct history_list_head {
  int length;                   ///<the number of history
  HISTORY_ENTRY *first;        ///<list head pointer
  HISTORY_ENTRY *last;        ///<list head pointer
}HISTORY_LIST_HEAD;
/**
 * @brief read the history list using the external APIs,currently only return 0
 * 
 * 
 *
 * @param head HISTORY_LIST_HEAD*,head pointer
 * @retval 0 failed 
 * @retval other success
 */
int history_read_list(HISTORY_LIST_HEAD* historyhead);

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
int history_read_list_cmd(HISTORY_LIST_HEAD* historyhead);

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
int history_release_history_list(HISTORY_LIST_HEAD* historyhead);

/**
 * @brief create a history entry, and add this entry to the list.
 * 
 * 
 *
 * @param name  const char*, the name of the counterpart
 * @param number const char*, the number of the counterpart
 * @param picpath const char*, the picture path of the counterpart
 * @param time char*, the time of that connection,may include date&time, and may be seperated into 2 fields in the future.
 * @param durationsec int, the duaration of that connection in seconds
 * @return 
 * @retval the newly created entry pointer
 */
HISTORY_ENTRY * history_add_entry(HISTORY_LIST_HEAD* historyhead, HISTORY_TYPE type,const char *name,const char *number,const char *picpath,  char *time,char *date,int durationsec);
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
int history_delete_entry(HISTORY_LIST_HEAD* historyhead,HISTORY_ENTRY* entry);

int history_init_history_data(HISTORY_LIST_HEAD* historyhead);
#ifdef __cplusplus
}
#endif

#endif /* _HISTORY_H */
