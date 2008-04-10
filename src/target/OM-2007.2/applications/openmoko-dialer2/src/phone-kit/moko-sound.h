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

#ifndef _HAVE_MOKO_SOUND_H
#define _HAVE_MOKO_SOUND_H

enum { SOUND_PROFILE_GSM_HANDSET,
SOUND_PROFILE_GSM_HEADSET,
SOUND_PROFILE_GSM_SPEAKER_OUT,
SOUND_PROFILE_STEREO_OUT,
SOUND_PROFILE_HEADSET};

void moko_sound_profile_set(int profile);
void moko_sound_profile_save(int profile);

#endif /* _HAVE_MOKO_SOUND_H */
