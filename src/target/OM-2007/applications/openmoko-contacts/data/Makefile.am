uidir = $(pkgdatadir)

desktopdir = $(datadir)/applications
desktop_in_files = contacts.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

icondir = $(datadir)/pixmaps/
icon_DATA = openmoko-contacts.png

schemadir = $(GCONF_SCHEMA_FILE_DIR)
schema_DATA = contacts.schemas

if HAVE_GCONF
install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) \
		$(GCONFTOOL) --makefile-install-rule $(schema_DATA) || \
		(echo ;\
		echo "*****************************************************"; \
		echo "Installation of schemas failed, install them manually"; \
		echo "*****************************************************";)
		@true
endif

CLEANFILES = $(desktop_DATA)

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = $(desktop_in_files) $(icon_DATA) $(schema_DATA)
