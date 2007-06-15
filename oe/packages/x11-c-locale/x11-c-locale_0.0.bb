SRC_URI = "file://locale"

do_install() {
	install -d ${D}${libdir}/X11
	cp -a ${WORKDIR}/locale ${D}${libdir}/X11/
}

FILES_${PN} = "${libdir}"

pkg_postinst_${PN}() {
#!/bin/sh

if [ "x$D" != "x" ]; then
  exit 1
fi

fc-cache -r
}

