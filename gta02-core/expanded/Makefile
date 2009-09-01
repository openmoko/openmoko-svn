.PHONY:	gen generate gv sch upload xpdf

UPLOAD=werner@sita.openmoko.org:public_html/gta02-core/
COPY=rsync -e ssh --progress

gen generate:
	cd ../components && ./mkloe
	../scripts/expand-pintype ../components/gta02-core.lib \
	  ../components/gta02-core-expanded.lib
	eeschema --plot `pwd`/gta02-core-expanded.sch
	../scripts/all-sheets gta02-core-expanded
	gzip -f -9 gta02-core-expanded-all.ps
	../scripts/all-sheets --pdf gta02-core-expanded

gv:
	gv gta02-core-expanded-all.ps.gz

xpdf:
	xpdf gta02-core-expanded-all.pdf

upload:	generate
	$(COPY) gta02-core-expanded-all.ps.gz gta02-core-expanded-all.pdf \
	  $(UPLOAD)

sch:
	eeschema `pwd`/gta02-core-expanded.sch
