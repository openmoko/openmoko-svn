#------------------------------------------------------
# OpenMoko Image Recipe
#------------------------------------------------------

export IMAGE_BASENAME = "${PN}"
export IMAGE_LINGUAS = ""

export IPKG_INSTALL = "\
  ${MACHINE_TASK_PROVIDER} \
  task-openmoko-linux \
  task-openmoko-net \
  task-openmoko-ui \
  task-openmoko-base \
  task-openmoko-phone \
  task-openmoko-finger \
  task-openmoko-pim \
"
DEPENDS = "\
  ${MACHINE_TASK_PROVIDER} \
  task-openmoko \
"

RDEPENDS = "${IPKG_INSTALL}"

inherit image_ipk

LICENSE = MIT
