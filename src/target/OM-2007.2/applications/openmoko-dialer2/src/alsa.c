/*  ALSA.C -USED TO CONTROL VOLUME
 *  Copyright (C) 2007 Li Jiang
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
 */

#include "alsa.h"
#include <ctype.h>
#include <glib.h>

/* Set/Get volume */
static snd_mixer_elem_t *pcm_element = NULL;
static snd_mixer_t *mixer = NULL;

static gboolean mixer_start = TRUE;

static guint mixer_timeout;

/*
 * mixer stuff
 */
static void
parse_mixer_name (char *str, char **name, int *index)
{
  char *end;

  while (isspace (*str))
    str++;

  if ((end = strchr (str, ',')) != NULL)
  {
    *name = g_strndup (str, end - str);
    end++;
    *index = atoi (end);
  }
  else
  {
    *name = g_strdup (str);
    *index = 0;
  }
}

int
alsa_get_mixer (snd_mixer_t ** mixer, int card)
{
  char *dev;
  int err;

  g_debug ("alsa_get_mixer");

  dev = g_strdup_printf ("hw:%i", card);

  if ((err = snd_mixer_open (mixer, 0)) < 0)
  {
    g_warning ("alsa_get_mixer(): Failed to open empty mixer: %s",
               snd_strerror (-err));
    mixer = NULL;
    return -1;
  }
  if ((err = snd_mixer_attach (*mixer, dev)) < 0)
  {
    g_warning ("alsa_get_mixer(): Attaching to mixer %s failed: %s",
               dev, snd_strerror (-err));
    return -1;
  }
  if ((err = snd_mixer_selem_register (*mixer, NULL, NULL)) < 0)
  {
    g_warning ("alsa_get_mixer(): Failed to register mixer: %s",
               snd_strerror (-err));
    return -1;
  }
  if ((err = snd_mixer_load (*mixer)) < 0)
  {
    g_warning ("alsa_get_mixer(): Failed to load mixer: %s",
               snd_strerror (-err));
    return -1;
  }

  g_free (dev);

  return (*mixer != NULL);
}


static snd_mixer_elem_t *
alsa_get_mixer_elem (snd_mixer_t * mixer, char *name, int index)
{
  snd_mixer_selem_id_t *selem_id;
  snd_mixer_elem_t *elem;
  snd_mixer_selem_id_alloca (&selem_id);

  if (index != -1)
    snd_mixer_selem_id_set_index (selem_id, index);
  if (name != NULL)
    snd_mixer_selem_id_set_name (selem_id, name);

  elem = snd_mixer_find_selem (mixer, selem_id);

  return elem;
}

static int
alsa_setup_mixer (void)
{
  char *name;
  long int a, b;
  long alsa_min_vol, alsa_max_vol;
  int err, index;

  g_debug ("alsa_setup_mixer");

  if ((err = alsa_get_mixer (&mixer, 0)) < 0)
    return err;

  parse_mixer_name ("Master", &name, &index);

  pcm_element = alsa_get_mixer_elem (mixer, name, index);

  g_free (name);

  if (!pcm_element)
  {
    g_warning ("alsa_setup_mixer(): Failed to find mixer element: Master");
    return -1;
  }

  /*
   * Work around a bug in alsa-lib up to 1.0.0rc2 where the
   * new range don't take effect until the volume is changed.
   * This hack should be removed once we depend on Alsa 1.0.0.
   */
  snd_mixer_selem_get_playback_volume (pcm_element,
                                       SND_MIXER_SCHN_FRONT_LEFT, &a);
  snd_mixer_selem_get_playback_volume (pcm_element,
                                       SND_MIXER_SCHN_FRONT_RIGHT, &b);

  snd_mixer_selem_get_playback_volume_range (pcm_element,
                                             &alsa_min_vol, &alsa_max_vol);
  snd_mixer_selem_set_playback_volume_range (pcm_element, 0, 100);

  if (alsa_max_vol == 0)
  {
    pcm_element = NULL;
    return -1;
  }

  g_debug ("alsa_setup_mixer: end");

  return 0;
}

static int
alsa_mixer_timeout (void *data)
{
  if (mixer)
  {
    snd_mixer_close (mixer);
    mixer = NULL;
    pcm_element = NULL;
  }
  mixer_timeout = 0;
  mixer_start = TRUE;

  g_message ("alsa mixer timed out");
  return FALSE;
}

void
alsa_get_volume (int *l, int *r)
{
  long ll = *l, lr = *r;

  if (mixer_start)
  {
    alsa_setup_mixer ();
    mixer_start = FALSE;
  }

  if (!pcm_element)
    return;

  snd_mixer_handle_events (mixer);

  snd_mixer_selem_get_playback_volume (pcm_element,
                                       SND_MIXER_SCHN_FRONT_LEFT, &ll);
  snd_mixer_selem_get_playback_volume (pcm_element,
                                       SND_MIXER_SCHN_FRONT_RIGHT, &lr);
  *l = ll;
  *r = lr;

  if (mixer_timeout)
    g_source_remove (mixer_timeout);
  mixer_timeout = g_timeout_add (5000, alsa_mixer_timeout, NULL);
}


void
alsa_set_volume (int l, int r)
{
  if (!pcm_element)
    return;

  snd_mixer_selem_set_playback_volume (pcm_element,
                                       SND_MIXER_SCHN_FRONT_LEFT, l);
  snd_mixer_selem_set_playback_volume (pcm_element,
                                       SND_MIXER_SCHN_FRONT_RIGHT, r);
}
