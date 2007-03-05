DEPLOY_DIR_SRC ?= "${DEPLOY_DIR}/source"
EXCLUDE_FROM ?= ".pc autom4te.cache"

# used as part of a path. make sure it's set
DISTRO ?= "openembedded"
DISTRO_VERSION ?= "${SRCDATE}"

def get_src_tree(d):
	import bb
	import os, os.path

	workdir = bb.data.getVar('WORKDIR', d, 1)
	if not workdir:
		bb.error("WORKDIR not defined, unable to find source tree.")
		return

	s = bb.data.getVar('S', d, 0)
	if not s:
		bb.error("S not defined, unable to find source tree.")
		return
	try:
		s_tree_raw = s.split('/')[1]
	except IndexError:
		s_tree_raw = "/"
	s_tree = bb.data.expand(s_tree_raw, d)

	src_tree_path = os.path.join(workdir, s_tree)
	try:
		os.listdir(src_tree_path)
	except OSError:
		bb.note("Expected to find source tree in '%s' which doesn't exist." % src_tree_path)
		os.mkdir(src_tree_path)
	bb.debug("Assuming source tree is '%s'" % src_tree_path)

	return s_tree

sourcepkg_do_archive_patched_sourcetree() {

	mkdir -p ${DEPLOY_DIR_SRC}
	cd ${WORKDIR}
	src_tree=${@get_src_tree(d)}
	echo $src_tree
	if [ "${src_tree}" != "/" ]; then
		oenote "Creating ${DEPLOY_DIR_SRC}/${P}-${PR}-${DISTRO}-${DISTRO_VERSION}.tar.gz"
		tar cvzf ${DEPLOY_DIR_SRC}/${P}-${PR}-${DISTRO}-${DISTRO_VERSION}.tar.gz --ignore-failed-read $src_tree
	else
		echo "Not creating ${DEPLOY_DIR_SRC}/${P}-${PR}-${DISTRO}-${DISTRO_VERSION}.tar.gz because src_tree=/" >> ${HOME}/openembedded-source-missing.log
	fi
}

EXPORT_FUNCTIONS do_archive_patched_sourcetree

addtask archive_patched_sourcetree after do_patch before do_configure

