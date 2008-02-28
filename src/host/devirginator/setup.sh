#!/bin/sh -e
#
# setup.sh - Set up the devirginator
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


# USB ID of DFU target
USB_ID_GTA01=-d1457:5119
USB_ID_GTA02=-d1d50:5119

DEFAULT_CONFIG=config

mkdir -p tmp


# --- Functions ---------------------------------------------------------------


download()
{
    base="`basename \"$2\"`"

    # if it is a local file, just use it
    if [ -r "$2" ]; then
	if $tarball; then
	    cp "$2" "tmp/$base"
	    eval $1=\"tmp/$base\"
	    add_file "tmp/$base"
	fi
	return
    fi

    # don't look for updates if we can use cached files
    if [ -r "tmp/$base" ] && $local; then
	eval $1=\"tmp/$base\"
	add_file "tmp/$base"
	return
    fi

    index=tmp/index-${SNAPSHOT}.html
    rm -f $index
    wget -O $index "`dirname \"$2\"`/"
    n="`basename \"$2\" | sed 's/*/[-a-zA-Z0-9_.+]*/g'`"
    sed '/^.*[^-a-zA-Z0-9_.+]\('"$n"'\)[^-a-zA-Z0-9_.+].*$/s//\1/p;d' \
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
    eval $1=\"tmp/$base\"
    cd ..
    add_file "tmp/$base"
}


probe()
{
    if ! "$@" >/dev/null 2>&1; then
	echo "WARNING: cannot execute $1" 1>&2
    fi
}


add_file()
{
    [ -f "$1" ] || { echo "$1: not found" 2>&1; exit 1; }
    tmp_files="$1 $tmp_files"
}


cppify()
{
    sed '/^#[a-z]/{n;};s/#.*$//' | 
      cpp -D$U_PLATFORM -D$U_PLATFORM$U_BOARD $defines
}


# --- Configuration defaults --------------------------------------------------


# System environment
OPENOCD_HOST=localhost
OPENOCD_PORT=4444
DFU_UTIL=dfu-util

# Target platform
PLATFORM=gta01
BOARD=bv4

# Locations
SPLASH=http://wiki.openmoko.org/images/c/c2/System_boot.png


# --- Command line parsing ----------------------------------------------------


usage()
{
cat <<EOF 1>&2
usage: $0 [-c config_file] [-l] [-t] [-Dvar[=value]] [variable=value ...]

  -c config_file  use the specified file (default: $DEFAULT_CONFIG)
  -l              use locally cached files (in tmp/), if present
  -t              make a tarball of all the downloaded and generated files
  -D var[=value]  pass a cpp-style -D option to cpp and envedit.pl
EOF
    exit 1
}


tarball=false
local=false
config=$DEFAULT_CONFIG
defines=

while [ ! -z "$*" ]; do
    case "$1" in
	-c)	shift
		[ ! -z "$1" ] || usage
		config=$1;;
	-t)	tarball=true;;
	-l)	local=true;;
	-D)	shift
		[ ! -z "$1" ] || usage
		defines="$defines -D $1"
		;;
	*=*)	eval "$1";;
	*)	usage;;
    esac
    shift
done


# --- Read config file --------------------------------------------------------


. $config


# --- Post configuration ------------------------------------------------------


if [ -z "$RELEASE_DIR" ]; then
    RELEASE_DIR=http://buildhost.openmoko.org/releases/
    RELEASE_DIR=${RELEASE_DIR}/${PLATFORM}-${SNAPSHOT}/tmp/deploy/images
fi

U_PLATFORM="`echo \"$PLATFORM\" | LANG=C tr a-z A-Z`"
U_BOARD="`echo \"$BOARD\" | LANG=C tr a-z A-Z`"

# lowlevel_foo.bin

if [ -z "$LOWLEVEL" ]; then
    LOWLEVEL=${RELEASE_DIR}/lowlevel_foo-${PLATFORM}${BOARD}-'*'.bin
fi
download LOWLEVEL "$LOWLEVEL"

# u-boot.bin

if [ -z "$UBOOT" ]; then
    UBOOT=${RELEASE_DIR}/u-boot-${PLATFORM}${BOARD}-'*'.bin
fi
download UBOOT "$UBOOT"

# uImage.bin

if [ -z "$UIMAGE" ]; then
    UIMAGE=${RELEASE_DIR}/uImage-2.6-'*'-fic-${PLATFORM}-'*'.bin
fi
download UIMAGE "$UIMAGE"

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
download ROOTFS "$ROOTFS"

# Splash screen

if [ ! -f "$SPLASH" ]; then
    download SPLASH "$SPLASH"
fi

# dfu-util
#
# Search priority:
# - direct file
# - command in local PATH
# - URL
#
if [ ! -r "$DFU_UTIL" ]; then
    if found="`which \"$DFU_UTIL\" 2>/dev/null`"; then
	DFU_UTIL="$found"
    fi
fi
download DFU_UTIL "$DFU_UTIL"
chmod +x "$DFU_UTIL"

# openocd

if [ ! -z "$OPENOCD" ]; then
    if [ ! -r "$OPENOCD" ]; then
	if found="`which \"$OPENOCD\" 2>/dev/null`"; then
	    OPENOCD="$found"
	fi
    fi
    download OPENOCD "$OPENOCD"
    chmod +x "$OPENOCD"
fi

if [ ! -z "$OPENOCD_CFG" ]; then
    download OPENOCD_CFG "$OPENOCD_CFG"
fi

# Set default for TARBALL_VERSION

if [ -z "$TARBALL_VERSION" ]; then
    TARBALL_VERSION="$SNAPSHOT"
fi


# --- Check executables -------------------------------------------------------


probe "$DFU_UTIL" -l
probe telnet </dev/null


# --- Stage 1: OpenOCD script -------------------------------------------------


cppify <openocd.in | {
    while read l; do
       eval "echo $l"
    done
} >tmp/script.ocd
add_file tmp/script.ocd


# --- Stage 1: First u-boot command(s) ----------------------------------------


sed 's/#.*//;/^ *$/d' <<EOF | tr '\n' ';' | tr '!' '\000' \
  >tmp/preboot_override.scrub
setenv scrub true; autoscr 0x32200000!
EOF
add_file tmp/preboot_override.scrub

sed 's/#.*//;/^ *$/d' <<EOF | tr '\n' ';' | tr '!' '\000' \
  >tmp/preboot_override.noscrub
autoscr 0x32200000!
EOF
add_file tmp/preboot_override.noscrub


# --- Stage 1: u-boot script --------------------------------------------------


cppify <u-boot.in | perl ./scriptify.pl >tmp/u-boot.out
add_file tmp/u-boot.out


# --- Stage 1: smiley splash screen -------------------------------------------


export OMDIR
make smiley.gz
mv smiley.gz tmp/
rm -f smiley.png smiley.ppm
add_file tmp/smiley.gz


# --- Stage 2: the default environment ----------------------------------------


cp environment.in tmp/environment
add_file tmp/environment


# --- Stage 2: official splash screen -----------------------------------------


../splash/splashimg.pl "$SPLASH" | gzip -9 >tmp/splash.gz
add_file tmp/splash.gz


# --- Stage 3: the NOR --------------------------------------------------------


if [ "$PLATFORM" != gta01 ]; then
    ./mknor -o tmp/nor.bin "$UBOOT"
    add_file tmp/nor.bin
fi


# --- "devirginate" shell script ----------------------------------------------


if [ "$PLATFORM" = gta01 ]; then
    env_size_opt=
    usb_id=$USB_ID_GTA01
else
    env_size_opt="-s 0x40000"
    usb_id=$USB_ID_GTA02
fi

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
      "script tmp/script.ocd" exit
fi

if \$stage2; then
    echo === STAGE 2: DFU upload
    sleep 5
    $DFU_UTIL $usb_id -a kernel -D $UIMAGE
    $DFU_UTIL $usb_id -a rootfs -D $ROOTFS
    $DFU_UTIL $usb_id -a splash -D tmp/splash.gz
    rm -f tmp/env.old tmp/env.new
    $DFU_UTIL $usb_id -a u-boot_env -U tmp/env.old
    ./openocdcmd.pl $OPENOCD_HOST $OPENOCD_PORT \
      "reset halt" wait_halt resume exit
    sleep 5
    ./envedit.pl $env_size_opt -i tmp/env.old -o tmp/env.new \
       -D $U_PLATFORM -D $U_PLATFORM$U_BOARD $defines -f tmp/environment
    $DFU_UTIL $usb_id -a u-boot_env -D tmp/env.new
    ./openocdcmd.pl $OPENOCD_HOST $OPENOCD_PORT "reset run" exit
fi

echo ===== DONE ===============================================================
EOF
chmod +x devirginate


# --- Done --------------------------------------------------------------------


if $tarball; then
    make tarball TARBALL_VERSION="$TARBALL_VERSION" TMP_FILES="$tmp_files"
cat <<EOF
-------------------------------------------------------------------------------

Your devirginator is now ready.

To install it on a new machine,

- copy the file devirginate-${TARBALL_VERSION}.tar.gz to the new machine
- tar xfz devirginate-${TARBALL_VERSION}.tar.gz
- cd devirginate-${TARBALL_VERSION}

To set up a device,

- connect it to power and JTAG
- switch it on
- run ./devirginate
- follow the progress, as described in README

-------------------------------------------------------------------------------
EOF
else
cat <<EOF
-------------------------------------------------------------------------------

Your devirginator is now ready.

To set up a device,

- connect it to power and JTAG
- switch it on
- run ./devirginate
- follow the process, as described in README

-------------------------------------------------------------------------------
EOF
fi
