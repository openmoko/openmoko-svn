/*
 *  messages.h
 *
 *  Authored By Alex Tang <alex@fic-sh.com.cn>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Public License as published by
 *  the Free Software Foundation; version 2.1 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */


#ifndef _MESSAGE_H_
#define _MESSAGE_H_

typedef struct message
  {
    gchar* name;
    gchar* subject;
    gchar* folder;
    gchar* content;
    gint   status;
  }
message;

enum {
  UNREAD,
  READ,
  REPLIED,
  FORWARD,
  NUM_STATES,
};

const static gint states[] =
  {
    UNREAD, READ, UNREAD, UNREAD, FORWARD,
    UNREAD, UNREAD, REPLIED, READ, UNREAD
  };

const static gchar *names[] =
  { "John B.", "Jane Z.", "Carl O.", "Owen P.", "Jeremy F.",
    "Michael M.", "Ute D.", "Akira T.", "Thomas F.", "Matthew J."
  };

const static gchar *subjects[] =
  { "Hello Alex", "We need sms support", "I need u", "Help harald", "The gui is really cool", "Can't u see", "2:00 pm", "Bugzillia page", "Hi there", "Target support"
  };

const static gchar *folders[] =
  { "Inbox", "Outbox", "Sent", "Inbox", "Inbox",
    "Inbox", "Inbox", "Inbox", "Inbox", "Inbox"
  };

const static gchar *contents[] =
  {"Hello Alex", "We need sms support", "I need u", "Help harald", "The gui is really cool", "Can't u see", "2:00 pm", "Bugzillia page", "Hi there", "Target support"
  };

#endif

