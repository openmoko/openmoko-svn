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


mkdir -p tmp


# --- Functions ---------------------------------------------------------------


download()
{
    index=tmp/index-${SNAPSHOT}.html
    if [ ! -f $index ]; then
	wget -O tmp/index "`dirname \"$2\"`/"
	mv tmp/index $index
    fi
    n="`basename \"$2\" | sed 's/*/[-a-zA-Z0-9_.]*/g'`"
    sed '/^.*[^-a-zA-Z0-9_.]\('"$n"'\)[^-a-zA-Z0-9_.].*$/s//\1/p;d' \
      <$index >tmp/files
    case `grep -c '' tmp/files`  in
	0)	echo "not found: $2" 1>&2
		exit 1;;
	1)	n="`dirname \"$2\"`/`cat tmp/files`";;
	*)	echo "too many files: $2" 2>&1
		exit 1;;
    esac
    rm -f $index tmp/files
    base="`basename \"$n\"`"
    cd tmp
    wget -N "$n" || { rm -f $base; exit 1; }
    eval $1=\"$PWD/$base\"
    cd ..
}


probe()
{
    if ! "$@" >/dev/null 2>&1; then
	echo "WARNING: cannot execute $1" 1>&2
    fi
}


# --- Configuration defaults --------------------------------------------------


# System environment
OPENOCD_HOST=localhost
OPENOCD_PORT=4444
DFU_UTIL=dfu-util

# Target platform
PLATFORM=gta01
BOARD=bv3

# Locations
SPLASH=http://wiki.openmoko.org/images/c/c2/System_boot.png

for n in "$@"; do
    eval "$n"
done


# --- Read config file --------------------------------------------------------


. config


# --- Check executables -------------------------------------------------------


probe "$DFU_UTIL" -l
probe telnet </dev/null


# --- Post configuration ------------------------------------------------------


if [ -z "$RELEASE_DIR" ]; then
    RELEASE_DIR=http://buildhost.openmoko.org/releases/
    RELEASE_DIR=${RELEASE_DIR}/${PLATFORM}-${SNAPSHOT}/tmp/deploy/images
fi

# lowlevel_foo.bin

if [ -z "$LOWLEVEL" ]; then
    LOWLEVEL=${RELEASE_DIR}/lowlevel_foo-${PLATFORM}${BOARD}-'*'.bin
fi
if [ ! -f "$LOWLEVEL" ]; then
    download LOWLEVEL "$LOWLEVEL"
fi

# u-boot.bin

if [ -z "$UBOOT" ]; then
    UBOOT=${RELEASE_DIR}/u-boot-${PLATFORM}${BOARD}-'*'.bin
fi
if [ ! -f "$UBOOT" ]; then
    download UBOOT "$UBOOT"
fi

# uImage.bin

if [ -z "$UIMAGE" ]; then
    UIMAGE=${RELEASE_DIR}/uImage-2.6-'*'-fic-${PLATFORM}-'*'.bin
fi
if [ ! -r "$UIMAGE" ]; then
    download UIMAGE "$UIMAGE"
fi

## # modules.tar.gz
## 
## if [ -z "$MODULES" ]; then
##     MODULES=${RELEASE_DIR}/modules-2.6-'*'-fic-${PLATFORM}.tgz
## fi
## if [ ! -f "$MODULES" ]; then
##     download MODULES "$MODULES"
## fi
## 
## # rootfs.tar.gz
## 
## if [ -z "$ROOTFS" ]; then
##     ROOTFS=${RELEASE_DIR}/openmoko-devel-image-fic-${PLATFORM}-'*'.rootfs.tar.gz
## fi
## if [ ! -f "$ROOTFS" ]; then
##     download ROOTFS "$ROOTFS"
## fi

# rootfs.jffs2

if [ -z "$ROOTFS" ]; then
    ROOTFS=${RELEASE_DIR}/openmoko-devel-image-fic-${PLATFORM}-'*'.rootfs.jffs2
fi
if [ ! -r "$ROOTFS" ]; then
    download ROOTFS "$ROOTFS"
fi


# Splash screen

if [ ! -f "$SPLASH" ]; then
    download SPLASH "$SPLASH"
fi


# --- Stage 1: OpenOCD script -------------------------------------------------


{
    while read l; do
       eval "echo $l"
    done
} <openocd.in >tmp/script.ocd


# --- Stage 1: First u-boot command(s) ----------------------------------------


sed 's/#.*//;/^ *$/d' <<EOF | tr '\n' ';' | tr '!' '\000' \
  >tmp/preboot_override.scrub
setenv scrub true; autoscr 0x32200000!
EOF


sed 's/#.*//;/^ *$/d' <<EOF | tr '\n' ';' | tr '!' '\000' \
  >tmp/preboot_override.noscrub
autoscr 0x32200000!
EOF


# --- Stage 1: u-boot script --------------------------------------------------


perl ./scriptify.pl u-boot.in >tmp/u-boot.out


# --- Stage 1: smiley splash screen -------------------------------------------

export OMDIR
make smiley.gz
mv smiley.gz tmp/
rm -f smiley.png smiley.ppm


# --- Stage 2: the default environment ----------------------------------------


cp environment.in tmp/environment


# --- Stage 2: official splash screen -----------------------------------------


../splash/splashimg.pl "$SPLASH" | gzip -9 >tmp/splash.gz


# --- "devirginate" shell script ----------------------------------------------


cat <<EOF >devirginate
#!/bin/sh -e
# MACHINE-GENERATED. DO NOT EDIT !
echo ===== STARTING ===========================================================

stage0=false
stage1=false
stage2=false
stage3=false
all=true

usage()
{
    echo "usage: \$0 [[-0] -1] [-2] [-3]" 1>&2
    exit 1
}

while [ ! -z "\$1" ]; do
    case "\$1" in
	-0)	stage0=true
		all=false;;
	-1)	stage1=true
		all=false;;
	-2)	stage2=true
		all=false;;
	-3)	stage3=true
		all=false;;
	*)	usage;;
    esac
    shift
done

if \$all; then
    stage1=true
    stage2=true
    stage3=true
fi

if \$stage0 && ! \$stage1; then
    usage
fi

if \$stage1; then
    echo === STAGE 1: JTAG upload
    if \$stage0; then
	ln -sf preboot_override.scrub tmp/preboot_override
    else
	ln -sf preboot_override.noscrub tmp/preboot_override
    fi
    ./openocdcmd.pl $OPENOCD_HOST $OPENOCD_PORT \
      "script $PWD/tmp/script.ocd" exit
fi

if \$stage2; then
    echo === STAGE 2: DFU upload
    sleep 5
    $DFU_UTIL -a 3 -D $UIMAGE
    $DFU_UTIL -a 5 -D $ROOTFS
    $DFU_UTIL -a 4 -D tmp/splash.gz
    $DFU_UTIL -a 2 -U tmp/env.old
    ./openocdcmd.pl $OPENOCD_HOST $OPENOCD_PORT \
      "bp 0x33f80000 4 hw" reset wait_halt "rbp 0x33f80000" resume exit
    sleep 5
    ./envedit.pl -i tmp/env.old -o tmp/env.new -f tmp/environment
    $DFU_UTIL -a 2 -D tmp/env.new
    ./openocdcmd.pl $OPENOCD_HOST $OPENOCD_PORT "reset run" exit
fi

echo ===== DONE ===============================================================
EOF
chmod +x devirginate


# --- Done --------------------------------------------------------------------


cat <<EOF
-------------------------------------------------------------------------------

Your devirginator is now ready.

To set up a device,

- connect it to power and JTAG
- switch it on
- run ./devirginate
- wait until the smiley appears (takes about 1-2 minutes)

-------------------------------------------------------------------------------
EOF
