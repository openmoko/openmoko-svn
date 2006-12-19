DESCRIPTION = "OpenMoko: Tasks for the OpenMoko Linux Distribution"
SECTION = "openmoko/base"
ALLOW_EMPTY = "1"
PACKAGE_ARCH = "all"
LICENSE = "MIT"
PROVIDES = "task-openmoko-everything"
DEPENDS = "dropbear"
PR = "r17"

PACKAGES = "\
  task-openmoko-linux \
  task-openmoko-net \
  task-openmoko-phone \
  task-openmoko-ui \
  task-openmoko-base \
  task-openmoko-finger \
  task-openmoko-pim \
  \
  task-openmoko-demo \
  task-openmoko-examples \
  task-openmoko-devel \
  task-openmoko-native-sdk \
"

RDEPENDS_task-openmoko-everything := "${PACKAGES}"

#
# task-openmoko-core
#
DESCRIPTION_task-openmoko-linux = "OpenMoko: Linux Core Services"
RDEPENDS_task-openmoko-linux = "\
  task-base \
  base-files \
  base-passwd \
  busybox \
  dropbear \
  fuser \
  initscripts \
  netbase \
  sysfsutils \
  setserial \
  sysvinit \
  sysvinit-pidof \
  tinylogin \
  modutils-initscripts \
  module-init-tools-depmod \
  udev \
#  update-alternatives \
"

#
# task-openmoko-phone
#
DESCRIPTION_task-openmoko-phone = "OpenMoko: GSM Phone Services"
RDEPENDS_task-openmoko-phone = "\
  libgsmd-daemon \
  libgsmd-tools \
"

#
# task-openmoko-net
#
DESCRIPTION_task-openmoko-net = "OpenMoko: Linux Advanced Networking"
RDEPENDS_task-openmoko-net = "\
  bluez-utils \
"

#
# task-openmoko-ui
#
DESCRIPTION_task-openmoko-ui = "OpenMoko: The X11/Gtk+2 based native User Interface"
RDEPENDS_task-openmoko-ui = "\
  gdk-pixbuf-loader-png \
  gdk-pixbuf-loader-gif \
  gdk-pixbuf-loader-xpm \
  gdk-pixbuf-loader-jpeg \
  pango-module-basic-x \
  pango-module-basic-fc \
  gtk+ \
  matchbox-common \
  matchbox-wm \
  matchbox-panel \
  xserver-kdrive-fbdev \
  xserver-kdrive-common \
  xserver-nodm-init \
  ttf-bitstream-vera \
  xauth \
  xhost \
  xset \
  xrandr \
  openmoko-common \
  openmoko-session \
  openmoko-theme-standard \
#  psplash \
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
# task-openmoko-demo
#
DESCRIPTION_task-openmoko-demo = "OpenMoko: Demo Applications"
RDEPENDS_task-openmoko-demo = "\
  matchbox-desktop \
  matchbox-keyboard \
  matchbox-stroke \
  matchbox-config-gtk \
  matchbox-panel-manager \
  matchbox-panel-hacks \
  matchbox-themes-extra \
  matchbox-themes-gtk \
  matchbox-applet-inputmanager \
  matchbox-applet-startup-monitor \
  xcursor-transparent-theme \
  settings-daemon \
  gtk-clearlooks-engine \
  gtk-theme-clearlooks \
  contacts \
  dates \
  web \
  rxvt-unicode \
  gnome-vfs-plugin-dbus \
  gnome-vfs-plugin-file \
  gnome-vfs-plugin-http \
"

#
# task-openmoko-examples
#
DESCRIPTION_task-openmoko-examples = "OpenMoko: Example Applications"
RDEPENDS_task-openmoko-examples = "\
  openmoko-stylus-demo-simple \
  openmoko-stylus-demo \
  openmoko-finger-demo \
  openmoko-chordmaster"

#
# task-openmoko-devel
#
DESCRIPTION_task-openmoko-devel = "OpenMoko: Debugging Tools"
RDEPENDS_task-openmoko-devel = "\
  alsa-utils-amixer \
  alsa-utils-aplay \
  strace \
#  ltrace \
  gdb \
  gdbserver \
  tcpdump \
  tslib-calibrate \
  tslib-tests \
  fstests \
  lsof \
  lrzsz \
  udev-utils \
  usbutils \
  uucp \
  cu \
  sensors-i2cdetect sensors-i2cdump sensors-i2cset \
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
  \
  xoo \
"
