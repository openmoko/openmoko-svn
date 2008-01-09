
desktopdir = $(datadir)/applications
desktop_DATA = openmoko-messages.desktop

#
# Dbus service file
#
servicedir = $(datadir)/dbus-1/services
service_in_files = org.openmoko.OpenmokoMessages2.service.in
service_DATA = $(service_in_files:.service.in=.service)

# Expand the bindir in the service file
$(service_DATA) : $(service_in_files) Makefile
	sed -e "s|\@bindir\@|$(bindir)|g" $< > $@

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = $(desktop_DATA)

