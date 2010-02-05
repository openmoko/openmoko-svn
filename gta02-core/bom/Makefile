UPLOAD=werner@sita.openmoko.org:public_html/gta02-core/
COPY=rsync -e ssh --progress

KITS=20
EQU=fic/fic.equ fic/missing.equ digi-key.equ
INV=fic/fic.inv gta02-core.inv dummy.inv fic/missing.inv digi-key.inv
CHR=darfon.chr ralec.chr avx.chr acx.chr misc.chr

.PHONY:		all spotless upload show-missing


all:		gta02-core.ord

gta02-core.ord:	part2order parser.pl \
		  gta02-core.par $(INV)
		perl part2order $(KITS) \
		  $(INV) gta02-core.par >$@ || \
		  { rm -rf $@; exit 1; }
	
gta02-core.par:	bom2part parser.pl match.pl \
		  $(EQU) $(INV) $(CHR) ../gta02-core.lst gta02-core.sub
		perl bom2part $(EQU) $(INV) $(CHR) \
		  ../gta02-core.lst gta02-core.sub \
		    >$@ || { rm -rf $@; exit 1; }

darfon.chr:	gen2chr parser.pl match.pl fic/fic.equ darfon.gen
		perl gen2chr DARFON fic/fic.equ darfon.gen >$@ || \
		  { rm -f $@; exit 1; }

ralec.chr:	gen2chr parser.pl match.pl fic/fic.equ ralec.gen
		perl gen2chr RALEC fic/fic.equ ralec.gen >$@ || \
		  { rm -f $@; exit 1; }

fic/fic.equ:
		$(MAKE) -C fic fic.equ

fic/fic.inv:
		$(MAKE) -C fic fic.inv

workflow.pdf:	workflow.fig
		fig2dev -L pdf $< >$@ || { rm -f $@; exit 1; }

upload:		workflow.pdf
		$(COPY) workflow.pdf $(UPLOAD)/bom-workflow.pdf

spotless:
		$(MAKE) -C fic spotless
		rm -f gta02-core.par gta02-core.ord darfon.chr ralec.chr

show-missing:
		(echo '#ORD'; grep MISSING gta02-core.ord ; ) | ./prettyord -
