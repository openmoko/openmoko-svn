/*
 *  moko-headset; set and get the headset status 
 *
 *  by Sean Chiang <sean_chiang@openmoko.com>
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

#include "moko-headset.h"

static int headset_status = HEADSET_STATUS_OUT;

void moko_headset_status_set(int status) {
	headset_status = status;
}

int moko_headset_status_get(void) {
	return headset_status;
}
