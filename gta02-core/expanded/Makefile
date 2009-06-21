.PHONY:	gen generate upload sch

gen generate:
	cd ../components && ./mkloe
	../scripts/expand-pintype ../components/gta02-core.lib \
	  ../components/gta02-core-expanded.lib
	eeschema --plot `pwd`/gta02-core-expanded.sch
	../scripts/all-sheets-ps gta02-core-expanded
	gzip -f -9 gta02-core-expanded-all.ps

upload:	generate
	scp gta02-core-expanded-all.ps.gz \
	  werner@sita.openmoko.org:public_html/gta02-core/

sch:
	eeschema `pwd`/gta02-core-expanded.sch
