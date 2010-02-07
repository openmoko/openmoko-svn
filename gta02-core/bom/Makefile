UPLOAD=werner@sita.openmoko.org:public_html/gta02-core/
COPY=rsync -e ssh --progress

KITS=20
EQU=fic/fic.equ fic/missing.equ digi-key.equ
INV=fic/fic.inv gta02-core.inv dummy.inv fic/missing.inv digi-key.inv
CHR=darfon.chr ralec.chr avx.chr acx.chr misc.chr
SCH=audio bt cpu-power cpu gps io lcm memory modem pmu sd-sim usb wlan

.PHONY:		all spotless upload show-dup show-missing show-dk
# mark them as phony since we have incomplete dependencies in this Makefile
.PHONY:		fic/fic.equ fic/fic.inv fic/missing.equ

all:		gta02-core.ord

gta02-core.ord:	part2order parser.pl \
		  gta02-core.par $(INV) $(EQU)
		perl part2order $(KITS) \
		  $(INV) $(EQU) gta02-core.par >$@ || \
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

fic/fic.dsc:
		$(MAKE) -C fic fic.dsc

fic/missing.equ:
		$(MAKE) -C fic missing.equ

workflow.pdf:	workflow.fig
		fig2dev -L pdf $< >$@ || { rm -f $@; exit 1; }

xpdf:		workflow.pdf
		xpdf workflow.pdf

upload:		workflow.pdf
		$(COPY) workflow.pdf $(UPLOAD)/bom-workflow.pdf

spotless:
		$(MAKE) -C fic spotless
		rm -f gta02-core.par gta02-core.ord darfon.chr ralec.chr
		rm -f gta02-core-bom.pro gta02-core-bom.sch $(SCH:%=%-bom.sch)

show-dup:	pardup.pl gta02-core.par
		perl ./pardup.pl gta02-core.par

show-missing:	$(EQU) fic/fic.dsc
		(echo '#ORD'; grep MISSING gta02-core.ord ; ) | \
		  ./prettyord - $(EQU) fic/fic.dsc | \
		  sed 's/^...  \(.\{,78\}\).*/\1/'

show-dk:
		(echo '#ORD'; grep DIGI-KEY gta02-core.ord ; ) | \
		  ./prettyord - digi-key.dsc | \
		  sed 's/^...  //'

ann:		annotate ../pmu.sch fic/fic.dsc gta02-core.par $(EQU)
		perl ./annotate ../pmu.sch fic/fic.dsc gta02-core.par $(EQU) \
		  >pmu-bom.sch || { rm -f pmu-bom.sch; exit 1; }

$(SCH:%=%-bom.sch): \
		annotate $(SCH:%=../%.sch) fic/fic.dsc gta02-core.par $(EQU)
		for n in $(SCH); do \
		    perl ./annotate ../$$n.sch fic/fic.dsc gta02-core.par \
		      $(EQU) >$$n-bom.sch || { rm -f $$n-bom.sch; exit 1; }; \
		done

gta02-core-bom.pro:	../gta02-core.pro
		sed 's|\./|../|' <../gta02-core.pro >$@ || \
		  { rm -f $@; exit 1; }

gta02-core-bom.sch:	../gta02-core.sch
		sed 's/\.sch"/-bom.sch"/' <$< >$@ || \
		  { rm -f $@; exit 1; }

sch:		gta02-core-bom.pro gta02-core-bom.sch #$(SCH:%=%-bom.sch)
		eeschema `pwd`/gta02-core-bom.sch
