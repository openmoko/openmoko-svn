#ifndef _MESSAGE_H_
#define _MESSAGE_H_

typedef struct message{
	gchar* name;
	gchar* subject;
	gchar* folder;
	gchar* content;
	gint	 status;
}message;

#endif
