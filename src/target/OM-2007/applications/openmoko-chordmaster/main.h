/*
 *  ChordMaster -- A Chord Application for the OpenMoko Framework
 *
 *  Authored By Michael 'Mickey' Lauer <mlauer@vanille-media.de>
 *
 *  Copyright (C) 2006 Vanille-Media
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

#include <mokoui/moko-application.h>
#include <mokoui/moko-paned-window.h>
#include <mokoui/moko-toolbox.h>

#include <gtk/gtkliststore.h>

typedef struct _ChordMasterData {
    MokoApplication* app;
    MokoPanedWindow* window;
    GtkMenu* menu;
    MokoToolBox* toolbox;
    ChordsDB* chordsdb;
    FretboardWidget* fretboard;
    GtkListStore* liststore;
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
