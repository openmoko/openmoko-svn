/*  chordsdb.h
 *
 *  Authored by Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006 First International Computer Inc.
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
 *  Current Version: $Rev$ ($Date: 2006/10/05 17:38:14 $) [$Author: mickey $]
 */

#ifndef __CHORDSDB_H_
#define __CHORDSDB_H_

#include <glib-object.h>

G_BEGIN_DECLS

#define TYPE_CHORDSDB chordsdb_get_type()
#define CHORDSDB(obj)     (G_TYPE_CHECK_INSTANCE_CAST ((obj),     TYPE_CHORDSDB, ChordsDB))
#define CHORDSDB_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),     TYPE_CHORDSDB, ChordsDBClass))
#define IS_CHORDSDB(obj)     (G_TYPE_CHECK_INSTANCE_TYPE ((obj),     TYPE_CHORDSDB))
#define IS_CHORDSDB_CLASS(klass)     (G_TYPE_CHECK_CLASS_TYPE ((klass),     TYPE_CHORDSDB))
#define CHORDSDB_GET_CLASS(obj)     (G_TYPE_INSTANCE_GET_CLASS ((obj),     TYPE_CHORDSDB, ChordsDBClass))

typedef struct {
    gchar* name;
    gchar* frets;
} Chord;

typedef struct {
    GObject parent;
} ChordsDB;

typedef struct {
    GObjectClass parent_class;
    GSList* categories;
    GSList* chords;
} ChordsDBClass;

GType chordsdb_get_type (void);
ChordsDB* chordsdb_new (void);
GSList* chordsdb_get_categories(ChordsDB* self);
GSList* chordsdb_get_chords(ChordsDB* self );

G_END_DECLS

#endif // __CHORDSDB_H_

