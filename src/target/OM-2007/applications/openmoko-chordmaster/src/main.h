/*
 *  ChordMaster -- A Chord Application for the OpenMoko Framework
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
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#ifndef __MAIN__H_
#define __MAIN__H_

#include "chordsdb.h"
#include "fretboard-widget.h"

#include <libmokoui/moko-application.h>
#include <libmokoui/moko-paned-window.h>
#include <libmokoui/moko-tool-box.h>
#include <libmokoui/moko-tree-view.h>
#include <gtk/gtkliststore.h>

typedef struct _ChordMasterData {
    MokoApplication* app;
    MokoPanedWindow* window;
    GtkMenu* menu;
    MokoToolBox* toolbox;
    ChordsDB* chordsdb;
    FretboardWidget* fretboard;
    GtkListStore* liststore;
    MokoTreeView* view;
} ChordMasterData;

enum {
    COLUMN_NAME,
    COLUMN_FRETS,
    NUM_COLS,
};

void setup_ui( ChordMasterData* );
void populate_navigation_area( ChordMasterData* d );
void populate_details_area( ChordMasterData* d );

#endif
