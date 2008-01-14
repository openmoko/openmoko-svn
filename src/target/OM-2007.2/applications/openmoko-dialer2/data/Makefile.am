# A shell check command that I find convenient:
# ls -1 *.png | xargs --replace=pngimg sh -c '(grep -qw "pngimg" Makefile.am  || echo "Missing pngimg")'

dist_pkgdata_DATA =unknown.png\
                dialed.png\
                missed.png\
                received.png\
                speaker.png\
                email.png\
                phone.png\
                dtmf.png\
                talking.png\
                sms.png\
                all.png\
                history.png\
                hangup.png\
                failure.png\
                incoming_0.png\
                incoming_1.png\
                incoming_2.png\
                incoming_3.png\
                outgoing_0.png\
                outgoing_1.png\
                outgoing_2.png\
                outgoing_3.png\
                talking_0.png\
                talking_1.png\
                talking_2.png\
                talking_3.png\
                exit.png\
                connecting.png\
                incall.png \
                moko-call-answer.png \
                moko-call-hangup.png \
                moko-call-hold.png \
                moko-call-ignore.png

#
# desktop integration: .desktop file
#
desktopdir = $(datadir)/applications
dist_desktop_DATA = openmoko-dialer.desktop

#
# desktop integration: application icon
#
appicondir = $(datadir)/pixmaps
dist_appicon_DATA = Dialer.png

#
# Dbus service file
#
servicedir = $(datadir)/dbus-1/services
service_in_files = org.openmoko.Dialer.service.in
service_DATA = $(service_in_files:.service.in=.service)

# Expand the bindir in the service file
$(service_DATA) : $(service_in_files) Makefile
	sed -e "s|\@bindir\@|$(bindir)|g" $< > $@
