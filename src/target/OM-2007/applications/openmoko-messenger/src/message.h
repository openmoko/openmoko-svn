#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <gtk/gtk.h>

/*! \file message.h
    \brief A Documented file.
    
    Details.
*/

/*!
  \def ST_UNREAD
  \brief Message is unread.
*/

/*!
  \def ST_READ
  \brief Message is read.
*/

/*!
  \def ST_REPLIED
  \brief Message is replied.
*/

/*!
  \def ST_FORWARD
  \brief Message is forwarded.
*/
#define ST_UNREAD  0
#define ST_READ    1
#define ST_REPLIED 2
#define ST_FORWARD 3

/*! \var typedef int STATUS
    \brief A type definition for STATUS .
    
    Details.
*/
typedef int STATUS;

/*! \struct MESSAGE
    \brief Struct definition for MESSAGE .
    
    Details.
*/
typedef struct {
  int id;             /**< message id */
  char from[30];      /**< message from address string */ 
  char content[500];  /**< message content string */ 
  char time[50];      /**< message received time string */ 
  STATUS status;      /**< message current status */
  char folder[50];    /**< message stored folder string */ 
} MESSAGE;

/*! \fn int delete_msg(int msgid)
    \brief delete message hanlder
    \param msgid message id
*/

/*! \fn void add_msg(char *from, char *content, char *time, char *folder)
    \brief add message handler
    \param from    message from string
    \param content message content string
    \param time    message time
    \param folder  message folder
*/

/*! \fn void set_msg_status(int id, STATUS st)
    \brief set message status handler
    \param id      message id
    \param st  message status
*/

int delete_msg(int msgid);
int message_empty();
void add_msg(char *from, char *content, char *time, char *folder);
void set_msg_status(int id, STATUS st);
void set_msg_folder(int id, char *fname);
int get_folder_message_number(char *fname);
