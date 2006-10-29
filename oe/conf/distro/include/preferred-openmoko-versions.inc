#
# Specify which versions of packages to build
#

#work around some breakages
#SRCDATE_minimo=20050401

PREFERRED_PROVIDER_dbus-glib = "dbus-glib"
PREFERRED_PROVIDER_gconf = "gconf-dbus"
PREFERRED_PROVIDER_gnome-vfs = "gnome-vfs-dbus"
PREFERRED_PROVIDER_virtual/xserver = "xserver-kdrive"
PREFERRED_PROVIDER_bluez-utils = "bluez-utils-dbus"

# dbus / Gtk+

PREFERRED_VERSION_dbus = "0.92"
PREFERRED_VERSION_dbus-glib = "0.71"
PREFERRED_VERSION_atk = "1.9.0"
PREFERRED_VERSION_pango = "1.8.2"
PREFERRED_VERSION_glib-2.0 = "2.6.4"
PREFERRED_VERSION_gtk+ = "2.6.10" 
PREFERRED_VERSION_gtk-engines = "2.6.5"

# Gnome

# Development
PREFERRED_VERSION_qemu-native = "0.8.2+cvs${SRCDATE}"

# O-Hand
PREFERRED_VERSION_dates = "0.1"
PREFERRED_VERSION_contacts = "0.1"

# OpenMoko

SRCDATE_gsm = "now"
SRCDATE_openmoko-session = "now"
SRCDATE_openmoko-common = "now"
SRCDATE_openmoko-theme-standard = "now"

