#
# Makefile - Setup and handling of prerequisites
#
# Copyright (C) 2006-2007 by OpenMoko, Inc.
# Written by Werner Almesberger <werner@openmoko.org>
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


DIR=devirginator
SCRIPTS=crc32.pl envedit.pl openocdcmd.pl scriptify.pl devirginate
TARBALL=devirginate-$(TARBALL_VERSION).tar.gz

.PHONY:		tarball

devirginate:	config environment.in openocd.in u-boot.in \
		  setup.sh smiley.gz \
		  crc32.pl envedit.pl openocdcmd.pl scriptify.pl
		mkdir -p tmp
		./setup.sh

smiley.ppm:	smiley.fig
		fig2dev -L ppm -m 2 $< >$@ || { rm -f $@; exit 1; }

smiley.png:	smiley.ppm
		pnmpad -white -width 480 -height 640 $< | \
		  ppmchange '#ffffff' '#00ff00' | \
		  pnmtopng >$@ || { rm -f $@; exit 1; }

smiley.gz:	smiley.png
		../splash/splashimg.pl $< | gzip -9 >$@

config:
		@echo 'Please provide a "config" file.' 1>&2
		@echo 'See "config.example" for details.' 1>&2
		@exit 1

tarball:	$(TARBALL)

$(TARBALL):
		@[ ! -z "$(TARBALL_VERSION)" ] || \
		  { echo "Please set TARBALL_VERSION"; exit 1; }
		cd .. && ln -sf $(DIR) $(DIR)-$(TARBALL_VERSION) && \
		  tar cfz $(DIR)/$(TARBALL) \
		    $(SCRIPTS:%=$(DIR)-$(TARBALL_VERSION)/%) \
		    $(DIR)-$(TARBALL_VERSION)/tmp/ || \
		  { rm -f $(DIR)-$(TARBALL_VERSION); exit 1; }
		rm -f $(DIR)-$(TARBALL_VERSION)
