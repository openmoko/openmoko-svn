DESCRIPTION = "OpenMoko: Tasks for the OpenMoko Linux Distribution"
MAINTAINER = "Michael 'Mickey' Lauer <mickey@Vanille.de>"
SECTION = "openmoko/base"
ALLOW_EMPTY = "1"
PACKAGE_ARCH = "all"
LICENSE = "MIT"
PROVIDES = "task-openmoko-everything"
DEPENDS = "dropbear"
PR = "r11"

PACKAGES = "\
  task-openmoko-core \
  task-openmoko-net \
  task-openmoko-ui \
  task-openmoko-base \
  task-openmoko-finger \
  task-openmoko-pim \
  \
  task-openmoko-devel \
  task-openmoko-native-sdk \
"

RDEPENDS_task-openmoko-everything := "${PACKAGES}"

#
# task-openmoko-core
#
DESCRIPTION_task-openmoko-core = "OpenMoko: Linux Core Services"
RDEPENDS_task-openmoko-core = "\
  task-base \
  base-files \
  base-passwd \
  busybox \
  initscripts \
  sysvinit \
  tinylogin \
  dropbear \
"

#
# task-openmoko-net
#
DESCRIPTION_task-openmoko-net = "OpenMoko: Linux Networking"
RDEPENDS_task-openmoko-net = "\
  bluez-utils \
"

#
# task-openmoko-ui
#
DESCRIPTION_task-openmoko-ui = "OpenMoko: The X11/Gtk+2 based native User Interface"
RDEPENDS_task-openmoko-ui = "\
  gtk+ \
  matchbox-common \
  matchbox-wm \
  matchbox-themes-gtk \
  matchbox-desktop \
  matchbox-panel-manager \
  matchbox-panel \
  xserver-kdrive-common \
  xserver-kdrive-fbdev \
  xserver-nodm-init \
  openmoko-common \
  openmoko-session \
"

#
# task-openmoko-base
#
DESCRIPTION_task-openmoko-base = "OpenMoko: Main-Menu Launcher, Phone Application, and Panel"
RDEPENDS_task-openmoko-base = "\
"

#
# task-openmoko-finger
#
DESCRIPTION_task-openmoko-finger = "OpenMoko: Finger UI Applications"
RDEPENDS_task-openmoko-finger = "\
"

#
# task-openmoko-pim
#
DESCRIPTION_task-openmoko-pim = "OpenMoko: PIM Applications"
RDEPENDS_task-openmoko-pim = "\
  eds-dbus \
"

#
# task-openmoko-devel
#
DESCRIPTION_task-openmoko-devel = "OpenMoko: Debugging Tools"
RDEPENDS_task-openmoko-devel = "\
  strace \
#  ltrace \
  gdb \
  gdbserver \
  tcpdump \
  tslib-calibrate \
  tslib-tests \
  lsof \
  lrzsz \
  udev-utils \
  usbutils \
  uucp \
"

#
# task-openmoko-sdk-native
#
DESCRIPTION_task-openmoko-native-sdk = "OpenMoko: Native SDK"
RDEPENDS_task-openmoko-native-sdk = "\
  binutils \
  binutils-symlinks \
  gcc \
  gcc-symlinks \
  cpp \
  cpp-symlinks \
  libc6-dev \
  libgcc-dev \
  libgcc-s-dev \
  glibc-utils \
  ldd \
  g++ \
  g++-symlinks \
  libstdc++-dev \
  \
  make \
  flex \
  flex-dev \
  bison \
  gawk \
  grep \
  sed \
  automake \
  autoconf \
  patch \
  patchutils \
  diffstat \
  diffutils \
  libtool \
  pkgconfig \
"
