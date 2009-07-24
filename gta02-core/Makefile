.PHONY:		all gen generate gv sch update upload xpdf bom-by-ref

all:
		@echo "make what ? target: gen generate sch upload"
		@exit 1

gen generate:
		eeschema --plot `pwd`/gta02-core.sch
		# For A3, remove the --shrink and prepend
		# GS_OPTIONS=-sPAPERSIZE=a3
		scripts/all-sheets --shrink gta02-core
		gzip -f -9 gta02-core-all.ps
		scripts/all-sheets --shrink --pdf gta02-core

update:
		svn update && cd components && ./mkloe

gv:
		gv gta02-core-all.ps.gz

xpdf:
		xpdf gta02-core-all.pdf

upload:		generate
		scp gta02-core-all.ps.gz gta02-core-all.pdf \
		  werner@sita.openmoko.org:public_html/gta02-core/

sch:
		eeschema `pwd`/gta02-core.sch

bom-by-ref:
		sed '/^#Cmp ( order = Reference )/,/#End Cmp/p;d' \
		  gta02-core.lst | \
		  sed '/^#/d;/ NC /d;s/^| //;/^TP/d'
