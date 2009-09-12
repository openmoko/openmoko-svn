.PHONY:	all dsv dsv-rebuild

all:
	@echo "make what ?" 1>&2
	@exit 1

dsv:
	../scripts/mkdsv ../components/INFO

dsv-rebuild:
	rm `../scripts/mkdsv -d`/bin/dsv-*
	../scripts/mkdsv ../components/INFO
