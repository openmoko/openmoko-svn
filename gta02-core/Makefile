.PHONY:		all gen generate gv sch brd update upload xpdf bom-by-ref
.PHONY:		dsv dsv-rebuild

UPLOAD=werner@sita.openmoko.org:public_html/gta02-core/
COPY=rsync -e ssh --progress

all:
		@echo "make what ? target: gen update sch gv xpdf brd dsv"
		# we have more targets, but "normal users" don't need them
		@exit 1

gen generate:
		eeschema --plot `pwd`/gta02-core.sch
		# For A3, remove the --shrink and prepend
		# GS_OPTIONS=-sPAPERSIZE=a3
		scripts/all-sheets --shrink gta02-core
		gzip -f -9 gta02-core-all.ps
		scripts/all-sheets --shrink --pdf gta02-core
		$(MAKE) -C expanded gen
		$(MAKE) -C modules gen
		$(MAKE) -C bom null gen

update:
		svn update
		make -C components loe
		make -C modules loe
		[ ! -d "`scripts/mkdsv -d`" ] || make dsv

gv:
		gv gta02-core-all.ps.gz

xpdf:
		xpdf gta02-core-all.pdf

upload:		generate
		$(COPY) gta02-core-all.ps.gz gta02-core-all.pdf $(UPLOAD)
		$(MAKE) -C expanded upload
		$(MAKE) -C modules upload
		$(MAKE) -C bom upload

sch:
		eeschema `pwd`/gta02-core.sch

brd:
		pcbnew `pwd`/gta02-core.brd

bom-by-ref:
		@sed '/^#Cmp ( order = Reference )/,/#End Cmp/p;d' \
		  gta02-core.lst | \
		  sed '/^#/d;/ NC /d;s/^| //;/^TP/d'

dsv:
		$(MAKE) -C scripts dsv

dsv-rebuild:
		$(MAKE) -C scripts dsv-rebuild
