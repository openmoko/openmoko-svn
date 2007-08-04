/*
 *  OpenMoko Media Player
 *   http://openmoko.org/
 *
 *  Copyright (C) 2007 by the OpenMoko team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/**
 * @file playback.h
 * Playback engine interface
 */

#ifndef _PLAYBACK_H
#define _PLAYBACK_H

#include <gst/gst.h>

#define OMP_EVENT_PLAYBACK_EOS "playback_end_of_stream"
#define OMP_EVENT_PLAYBACK_STATUS_CHANGED "playback_status_change"
#define OMP_EVENT_PLAYBACK_POSITION_CHANGED "playback_position_change"

// Player states masking the gstreamer states so we can be more abstract
#define OMP_PLAYBACK_STATE_PAUSED GST_STATE_PAUSED
#define OMP_PLAYBACK_STATE_PLAYING GST_STATE_PLAYING

// The UI will be updated at this interval when a track is playing (in ms)
#define PLAYBACK_UI_UPDATE_INTERVAL 1000

void omp_playback_init();
void omp_playback_free();

gboolean omp_playback_load_track_from_uri(gchar *uri);

void omp_playback_play();
void omp_playback_pause();
gint omp_playback_get_state();
gulong omp_playback_get_track_position();
void omp_playback_set_track_position(glong position);
gulong omp_playback_get_track_length();

static gboolean omp_gst_message_eos(GstBus *bus, GstMessage *message, gpointer data);
static gboolean omp_gst_message_error(GstBus *bus, GstMessage *message, gpointer data);
static gboolean omp_gst_message_warning(GstBus *bus, GstMessage *message, gpointer data);

#endif
