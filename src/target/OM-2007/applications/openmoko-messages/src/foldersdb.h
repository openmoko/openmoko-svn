/*
 *  foldersdb.h
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
 * GNU Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#ifndef __FOLDERsDB_H_
#define __FOLDERsDB_H_

#include <glib-object.h>
#include <stdio.h>
#include <string.h>

G_BEGIN_DECLS

#define TYPE_FOLDERSDB foldersdb_get_type()
#define FOLDERSDB(obj)     (G_TYPE_CHECK_INSTANCE_CAST ((obj),     TYPE_FOLDERSDB, FoldersDB))
#define FOLDERSDB_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),     TYPE_FOLDERSDB, FoldersDBClass))
#define IS_FOLDERSDB(obj)     (G_TYPE_CHECK_INSTANCE_TYPE ((obj),     TYPE_FOLDERSDB))
#define IS_FOLDERSDB_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass),     TYPE_FOLDERSDB))
#define FOLDERSDB_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj),     TYPE_FOLDERSDB, FoldersDBClass))

typedef struct
  {
    GObject parent;
  }
FoldersDB;

typedef struct
  {
    GObjectClass parent_class;
    GSList* folders;
  }
FoldersDBClass;

GType foldersdb_get_type (void);
FoldersDB* foldersdb_new (void);
GSList* foldersdb_get_folders(FoldersDB* self);
void foldersdb_update ( GSList* folderlist );

G_END_DECLS


#endif // __FOLDERsDB_H_

