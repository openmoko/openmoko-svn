.PHONY:		all spotless

all:		gta02-core.ord

gta02-core.ord:	part2order gta02-core.par fic/fic.inv gta02-core.inv
		perl part2order fic/fic.inv gta02-core.inv gta02-core.par
# \ >$@ || { rm -rf $@; exit 1; }
	
gta02-core.par:	bom2part fic/fic.equ ../gta02-core.lst gta02-core.inv
		perl bom2part fic/fic.equ ../gta02-core.lst  gta02-core.inv \
		    >$@ || { rm -rf $@; exit 1; }

fic/fic.equ:
		$(MAKE) -C fic/fic.equ

fic/fic.inv:
		$(MAKE) -C fic/fic.inv

spotless:
		$(MAKE) -C fic spotless
		rm -f gta02-core.par gta02-core.ord
