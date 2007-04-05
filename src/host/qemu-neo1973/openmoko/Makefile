# Makefile - builds a Neo1973 flash image
#
# Copyright (C) 2007 OpenMoko, Inc.
# Written by Andrzej Zaborowski <andrew@openedhand.com>
# All Rights Reserved
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
SPLASH=splashimg.pl
SPLASHIMG=System_boot.png
FLASHIMG=openmoko-flash.image
SHELL=bash

splash.gz: splash
	gzip -c < $^ > $@

splash: $(SPLASH) $(SPLASHIMG)
	[ -x $(SPLASH) ]
	[ -s $(SPLASHIMG) ]

	./$(SPLASH) $(SPLASHIMG) > $@

$(SPLASHIMG):
	wget http://wiki.openmoko.org/images/c/c2/$(SPLASHIMG)

$(SPLASH):
	wget http://svn.openmoko.org/trunk/src/host/splash/$(SPLASH)
	chmod 0755 $(SPLASH)

$(FLASHIMG):
	# Making an empty/erased flash image.  Need a correct echo behavior.
	$(echo) -en \\0377\\0377\\0377\\0377\\0377\\0377\\0377\\0377 > .8b
	cat .8b .8b > .16b # OOB is 16 bytes
	cat .16b .16b .16b .16b .16b .16b .16b .16b > .512b
	cat .16b .16b .16b .16b .16b .16b .16b .16b >> .512b
	cat .16b .16b .16b .16b .16b .16b .16b .16b >> .512b
	cat .16b .16b .16b .16b .16b .16b .16b .16b >> .512b
	cat .512b .16b > .sec # A sector is 512 bytes of data + OOB
	cat .sec .sec .sec .sec .sec .sec .sec .sec > .8sec
	cat .8sec .8sec .8sec .8sec .8sec .8sec .8sec .8sec > .64sec
	cat .64sec .64sec .64sec .64sec .64sec .64sec .64sec .64sec > .512sec
	cat .512sec .512sec .512sec .512sec > .2ksec
	cat .2ksec .2ksec .2ksec .2ksec .2ksec .2ksec .2ksec .2ksec > .16ksec
	# Neo NAND is 128k sectors big
	cat .16ksec .16ksec .16ksec .16ksec .16ksec .16ksec .16ksec .16ksec > $@
	rm -rf .8b .16b .512b .sec .8sec .64sec .512sec .2ksec .16ksec

clean:
	rm -rf $(SPLASH) $(SPLASHIMG) $(FLASHIMG) splash splash.gz
