#!/bin/sh -e
#
# setup.sh - Set up the devirginator
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

OPENOCD_HOST=localhost
OPENOCD_PORT=4444
UBOOTDIR=${OMDIR:-/home/moko}/u-boot
LOWLEVEL=$UBOOTDIR/board/neo1973/lowlevel_foo.bin
UBOOT=$UBOOTDIR/u-boot.bin

. config

cat <<EOF >tmp/script.ocd
reset halt
wait_halt
load_binary $LOWLEVEL 0
bp 0x33f80000 4 hw
resume
wait_halt
rbp 0x33f80000
load_binary $UBOOT 0x32000000
load_binary $PWD/tmp/preboot_override 0x32100000
load_binary $PWD/tmp/u-boot.out 0x32200000
load_binary $PWD/tmp/smiley.gz 0x32300000
mww 0x32000040 0x32100000
resume 0x32000000
exit
EOF

sed 's/#.*//;/^ *$/d' <<EOF | tr '\n' ';' | tr '!' '\000' >tmp/preboot_override
autoscr 0x32200000!
EOF

perl ./scriptify.pl u-boot.in >tmp/u-boot.out

export OMDIR
make smiley.gz
mv smiley.gz tmp/
rm -f smiley.png smiley.ppm

cat <<EOF >devirginate
#!/bin/sh
# MACHINE-GENERATED. DO NOT EDIT !
echo ===== STARTING ===========================================================
{
    echo script $PWD/tmp/script.ocd
    sleep 120
} | telnet $OPENOCD_HOST $OPENOCD_PORT
echo ===== CONTINUING IN THE BACKGROUND =======================================
EOF
chmod +x devirginate

cat <<EOF
-------------------------------------------------------------------------------

Your devirginator is now ready.

To set up a device,

- connect it to power and JTAG
- switch it on
- run ./devirginate
- wait until the smiley appears (takes about one minute)

-------------------------------------------------------------------------------
EOF
