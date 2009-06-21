.PHONY:	gen generate gv sch upload xpdf

gen generate:
	cd ../components && ./mkloe
	../scripts/expand-pintype ../components/gta02-core.lib \
	  ../components/gta02-core-expanded.lib
	eeschema --plot `pwd`/gta02-core-expanded.sch
	../scripts/all-sheets gta02-core-expanded
	gzip -f -9 gta02-core-expanded-all.ps
	../scripts/all-sheets --pdf gta02-core-expanded

gv:
	gv gta02-core-all.ps.gz

xpdf:
	xpdf gta02-core-all.pdf

upload:	generate
	scp gta02-core-expanded-all.ps.gz gta02-core-expanded-all.pdf \
	  werner@sita.openmoko.org:public_html/gta02-core/

sch:
	eeschema `pwd`/gta02-core-expanded.sch
