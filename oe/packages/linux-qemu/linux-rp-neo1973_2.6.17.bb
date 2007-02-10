MACHINE = "qemuarm"

require linux-rp_${PV}.bb

FILESDIR = "${@os.path.dirname(bb.data.getVar('FILE',d,1))}/linux-rp-${PV}"

SRC_URI += "file://versatile-change-fb-orientation.patch;patch=1"
