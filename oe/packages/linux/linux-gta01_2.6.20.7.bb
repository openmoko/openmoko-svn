require linux-gta01.inc

SRC_URI += "svn://svn.openmoko.org/branches/src/target/kernel/2.6.20.x;module=patches;proto=http"

MOKOR = "moko9"
PR = "${MOKOR}-r0"

VANILLA_VERSION = "2.6.20.7"

