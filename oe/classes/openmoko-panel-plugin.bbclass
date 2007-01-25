inherit openmoko

SECTION = "openmoko/panel-plugin"
DEPENDS += "matchbox-panel-2"

FILES_${PN} = "${libdir}/matchbox-panel/lib*.so* ${datadir}"
