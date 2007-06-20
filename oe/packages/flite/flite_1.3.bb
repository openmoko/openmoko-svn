require flite.inc

SRC_URI += "http://homepage.hispeed.ch/loehrer/downloads/flite-1.3-alsa_support-1.2.diff.bz2;patch=1;pnum=0"

PRÂ = "r1"

# FIXME: The flite application is still being statically linked against libflite
