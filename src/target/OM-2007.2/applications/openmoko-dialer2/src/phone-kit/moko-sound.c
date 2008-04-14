/*
 *  moko-sound; Crude hack to switch audio profiles until we have a proper solution
 *
 *  by Daniel Willmann <daniel@totalueberwachung.de>
 *
 *  Copyright (C) 2006-2007 OpenMoko Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser Public License as published by
 *  the Free Software Foundation; version 2 of the license.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser Public License for more details.
 *
 *  Current Version: $Rev$ ($Date$) [$Author$]
 */

#include "moko-dialer.h"

#include <stdio.h>
#include <stdlib.h>

char *sound_profile_filenames[] = {
  "/usr/share/openmoko/scenarios/gsmhandset.state",
  "/usr/share/openmoko/scenarios/gsmheadset.state",
  "/usr/share/openmoko/scenarios/gsmspeakerout.state",
  "/usr/share/openmoko/scenarios/stereoout.state",
  "/usr/share/openmoko/scenarios/headset.state"
};

void moko_sound_profile_set(int profile) {
  char command[100];
  snprintf(command, 100, "alsactl -f %s restore", sound_profile_filenames[profile]);
  system(command);
}

void moko_sound_profile_save(int profile) {
  char command[100];
  snprintf(command, 100, "alsactl -f %s store", sound_profile_filenames[profile]);
  system(command);
}

