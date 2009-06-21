.PHONY:		all gen generate sch upload

all:
		@echo "make what ? target: gen generate sch upload"
		@exit 1

gen generate:
		eeschema --plot `pwd`/gta02-core.sch
		GS_OPTIONS=-sPAPERSIZE=a3 scripts/all-sheets-ps gta02-core
		gzip -f -9 gta02-core-all.ps

upload:		generate
		scp gta02-core-all.ps.gz \
		  werner@sita.openmoko.org:public_html/gta02-core/

sch:
		eeschema `pwd`/gta02-core.sch

