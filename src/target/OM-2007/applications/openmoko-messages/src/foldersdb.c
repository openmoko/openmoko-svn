/*
 *  foldersdb.c
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


#include "foldersdb.h"

#include <stdio.h>

G_DEFINE_TYPE (FoldersDB, foldersdb, G_TYPE_OBJECT)

#define FOLDERSDB_PRIVATE(o)     (G_TYPE_INSTANCE_GET_PRIVATE ((o), TYPE_FOLDERSDB, FoldersDBPrivate))

/*typedef struct _FoldersDBPrivate
{
} FoldersDBPrivate;*/

static void
foldersdb_dispose (GObject *object)
{
  if (G_OBJECT_CLASS (foldersdb_parent_class)->dispose)
    G_OBJECT_CLASS (foldersdb_parent_class)->dispose (object);
}

static void
foldersdb_finalize (GObject *object)
{
  G_OBJECT_CLASS (foldersdb_parent_class)->finalize (object);
}

static void
foldersdb_class_init (FoldersDBClass *klass)
{
  g_debug( "foldersdb_class_init" );
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  /* register private data */
  /*g_type_class_add_private (klass, sizeof (FoldersDBPrivate));*/

  /* hook virtual methods */
  /* ... */

  /* install properties */
  /* ... */

  object_class->dispose = foldersdb_dispose;
  object_class->finalize = foldersdb_finalize;

  FILE* file = fopen( PKGDATADIR "/folderlist", "rw" );
  g_assert( file ); //FIXME error handling, if folder file is not present
  char line[256];
  char *elem;
  char *tok = "\n";

  while( fgets(line, sizeof(line), file) )
    {
      elem = g_strdup(strtok(line, tok));
      klass->folders = g_slist_append(klass->folders, elem);
    }

  fclose( file );
}

static void
foldersdb_init (FoldersDB *self)
{}

FoldersDB*
foldersdb_new (void)
{
  return g_object_new (TYPE_FOLDERSDB, NULL);
}

GSList* foldersdb_get_folders(FoldersDB* self)
{
  FoldersDBClass* klass = FOLDERSDB_GET_CLASS(self);
  return klass->folders;
}

void foldersdb_update ( GSList* folderlist )
{
  /*FILE* file = g_fopen( PKGDATADIR "/folderlist", "w" );*/
  FILE* file = fopen( PKGDATADIR "/folderlist", "w" );
  g_assert( file );
  GSList* c;
  char *elem;

  for( c =folderlist; c; c=g_slist_next(c))
    {
      if(g_slist_next(c) != NULL)
        elem = g_strdup_printf( "%s\n", (char*)c->data);
      else
        elem = g_strdup(c->data);
      fputs(elem, file);
    }
  fclose( file );
}


