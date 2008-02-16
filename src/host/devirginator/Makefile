#
# Makefile - Setup and handling of prerequisites
#
# Copyright (C) 2006-2008 by OpenMoko, Inc.
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
TARDIR=devirginate-$(TARBALL_VERSION)
TARBALL=$(TARDIR).tar.gz

.PHONY:		tarball clean

#devirginate:	config environment.in openocd.in u-boot.in \
#		  setup.sh smiley.gz \
#		  crc32.pl envedit.pl openocdcmd.pl scriptify.pl
#		mkdir -p tmp
#		./setup.sh

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

$(TARBALL):	$(SCRIPTS) $(TMP_FILES)
		@[ ! -z "$(TARBALL_VERSION)" ] || \
		  { echo "Please set TARBALL_VERSION"; exit 1; }
		@[ ! -z "$(TMP_FILES)" ] || \
		  { echo "Please set TMP_FILES"; exit 1; }
		cd .. && ln -sf $(DIR) $(TARDIR) && \
		  tar cfz $(DIR)/$(TARBALL) \
		    $(SCRIPTS:%=$(TARDIR)/%) \
		    $(TMP_FILES:%=$(TARDIR)/%) || \
		  { rm -f $(TARDIR); exit 1; }
		rm -f $(TARDIR)

clean:
		rm -f smiley.ppm smiley.png smiley.gz
