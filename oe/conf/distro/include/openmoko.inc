#
# Header
#
DISTRO_NAME = "OpenMoKo"
DISTRO_VERSION = ".dev-snapshot-${SRCDATE}"
#DISTRO_TYPE = "release"
DISTRO_TYPE = "debug"

#
# Naming schemes
#
PARALLEL_INSTALL_MODULES = "1"
INHERIT += "debian multimachine"

#
# Packaging and output format
#
INHERIT += "package_ipk"
IMAGE_FSTYPES = "tar.gz ext2.gz jffs2"
IMAGE_FSTYPES_x86 = "tar.gz"
IMAGE_ROOTFS_SIZE_ext2 = "131072"

#
# Binutils & Compiler
#
PREFERRED_PROVIDERS += " virtual/${TARGET_PREFIX}gcc-initial:gcc-cross-initial"
PREFERRED_PROVIDERS += " virtual/${TARGET_PREFIX}gcc:gcc-cross"
PREFERRED_PROVIDERS += " virtual/${TARGET_PREFIX}g++:gcc-cross"

# EABI stuff
PREFERRED_PROVIDER_virtual/${TARGET_PREFIX}-libc-for-gcc = "glibc-intermediate"
PREFERRED_PROVIDER_virtual/arm-angstrom-linux-gnueabi-libc-for-gcc = "glibc-intermediate"
PREFERRED_PROVIDER_virtual/arm-linux-libc-for-gcc = "glibc-intermediate"

#
# Target OS & FPU system
# 
TARGET_OS = "linux"
TARGET_FPU_arm = "soft"
TARGET_FPU_armeb = "soft"
PREFERRED_PROVIDER_virtual/libiconv = "glibc"
PREFERRED_PROVIDER_virtual/libintl = "glibc"

#
# Features
#
DISTRO_FEATURES = "alsa bluetooth ext2 ipsec nfs ppp smbfs usbgadget"

#
# Bootstrap & Init
#
require conf/distro/include/sane-srcdates.inc

#
# Framebuffer & X
#
PREFERRED_PROVIDER_virtual/libx11 ?= "libx11"
PREFERRED_PROVIDER_virtual/ecore ?= "ecore-x11"
PREFERRED_PROVIDER_virtual/evas ?= "evas-x11"
PREFERRED_PROVIDER_virtual/esmart ?= "esmart-x11"
PREFERRED_PROVIDER_virtual/libsdl = "libsdl-x11"
PREFERRED_PROVIDER_virtual/xserver ?= "xserver-kdrive"
PREFERRED_PROVIDER_tslib ?= "tslib"
PREFERRED_PROVIDER_xserver ?= "xserver-kdrive"
require conf/distro/include/preferred-xorg-versions-X11R7.1.inc

#
# Opie
#
QTE_VERSION = "2.3.10"
PALMTOP_USE_MULTITHREADED_QTE = "yes"
require conf/distro/include/preferred-opie-versions.inc

#
# Enlightenment
#
require conf/distro/include/preferred-e-versions.inc

#
# Versions for OpenMoko
#
require conf/distro/include/preferred-openmoko-versions.inc

