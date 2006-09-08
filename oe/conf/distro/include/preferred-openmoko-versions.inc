#
# Specify which versions of packages to build
#

#work around some breakages
#SRCDATE_minimo=20050401

#set some preferred providers:
PREFERRED_PROVIDER_dbus-glib = "dbus-glib"
PREFERRED_PROVIDER_gconf = "gconf-dbus"
PREFERRED_PROVIDER_gnome-vfs = "gnome-vfs-dbus"
PREFERRED_PROVIDER_virtual/xserver = "xserver-kdrive"
PREFERRED_PROVIDER_bluez-utils = "bluez-utils-dbus"

PREFERRED_VERSION_atk = "1.9.0"
PREFERRED_VERSION_pango = "1.8.2"
PREFERRED_VERSION_glib-2.0 = "2.6.4"
PREFERRED_VERSION_gtk+ = "2.6.10" 
PREFERRED_VERSION_gtk-engines = "2.6.5"
