UPLOAD=werner@sita.openmoko.org:public_html/gta02-core/
COPY=rsync -e ssh --progress

BOOM=../../eda/boom/boom

ANNOTATE=ord	# make ANNOTATE=par ... for the parts list

KITS=20
EQU=gta02-core.equ fic/fic.equ fic/missing.equ \
    dk/digi-key.equ mouser/mouser.equ farnell/farnell.equ arrow/arrow.equ
INV=fic/fic.inv gta02-core.inv dummy.inv fic/missing.inv \
    dk/digi-key.inv mouser/mouser.inv farnell/farnell.inv arrow/arrow.inv
CHR=darfon.chr ralec.chr avx.chr acx.chr misc.chr stackpole.chr
SCH=audio bt cpu-power cpu gps io lcm memory modem pmu sd-sim usb wlan

.PHONY:		all optimist pessimist null spotless
.PHONY:		show-dup show-missing
.PHONY:		show-dk show-fic show-mouser show-farnell show-arrow
.PHONY:		ann sch gen generate gv xpdf upload

all:		gta02-core.ord

optimist:
		$(MAKE) spotless
		$(MAKE) -C fic INTERPRETATION=--optimistic
		$(MAKE) all

pessimist:
		$(MAKE) spotless
		$(MAKE) -C fic
		$(MAKE) all

null:
		$(MAKE) spotless
		$(MAKE) -C fic null
		$(MAKE) all

gta02-core.ord:	gta02-core.par $(INV) $(EQU)
		$(BOOM) part2order $(KITS) \
		  $(INV) $(EQU) gta02-core.par >$@ || \
		  { rm -rf $@; exit 1; }
	
gta02-core.par:	$(EQU) $(INV) $(CHR) ../gta02-core.lst gta02-core.sub
		$(BOOM) bom2part $(EQU) $(INV) $(CHR) \
		  ../gta02-core.lst gta02-core.sub \
		    >$@ || { rm -rf $@; exit 1; }

darfon.chr:	fic/fic.equ darfon.gen
		$(BOOM) gen2chr DARFON fic/fic.equ darfon.gen >$@ || \
		  { rm -f $@; exit 1; }

ralec.chr:	fic/fic.equ ralec.gen
		$(BOOM) gen2chr RALEC fic/fic.equ ralec.gen >$@ || \
		  { rm -f $@; exit 1; }

stackpole.chr:	dk/digi-key.equ stackpole.gen
		$(BOOM) gen2chr STACKPOLE dk/digi-key.equ stackpole.gen >$@ || \
		  { rm -f $@; exit 1; }

fic/fic.equ:
		$(MAKE) -C fic fic.equ

fic/fic.inv:
		$(MAKE) -C fic fic.inv

fic/fic.dsc:
		$(MAKE) -C fic fic.dsc

dk/digi-key.inv:
		$(MAKE) -C dk digi-key.inv

dk/digi-key.dsc:
		$(MAKE) -C dk digi-key.dsc

fic/missing.equ:
		$(MAKE) -C fic missing.equ

spotless:
		$(MAKE) -C fic spotless
		$(MAKE) -C dk spotless
		rm -f gta02-core.par gta02-core.ord
		rm -f darfon.chr ralec.chr stackpole.chr
		rm -f gta02-core-bom.pro gta02-core-bom.sch
		rm -f gta02-core-bom-all.ps.gz gta02-core-bom-all.pdf
		rm -f $(SCH:%=%-bom.sch) gta02-core-bom-*.ps

show-dup:	gta02-core.par
		$(BOOM) ./pardup.pl gta02-core.par

show-missing:	$(EQU) fic/fic.dsc
		(echo '#ORD'; grep '^MISSING ' gta02-core.ord ; ) | \
		  $(BOOM) prettyord - $(EQU) fic/fic.dsc | \
		  sed 's/^...  \(.\{,78\}\).*/\1/'


# --- show priced orders by distributor ---------------------------------------


SHOW_PRETTY = (echo '\#ORD'; grep '^$(1) ' gta02-core.ord ; ) | \
                  $(BOOM) prettyord - $(2) | \
                  sed 's/^...  //' | \
                  awk '{ s += $$(NF); if ($$(NF)+0) n++; print; } \
		    END { print "$(3)", s, "items:", n }'

show-dk:	gta02-core.ord dk/digi-key.dsc
		$(call SHOW_PRETTY,DIGI-KEY,dk/digi-key.dsc,USD)

show-mouser:	gta02-core.ord mouser/mouser.dsc
		$(call SHOW_PRETTY,MOUSER,mouser/mouser.dsc,USD)

show-farnell:	gta02-core.ord farnell/farnell.dsc
		$(call SHOW_PRETTY,FARNELL,farnell/farnell.dsc,UKP)

show-arrow:	gta02-core.ord arrow/arrow.dsc
		$(call SHOW_PRETTY,ARROW,arrow/arrow.dsc,USD)

show-fic:	$(EQU) fic/fic.dsc
		(echo '#ORD'; grep '^FIC ' gta02-core.ord ; ) | \
		  $(BOOM) prettyord '-s/OldPn (\S+)/FIC/' \
		  - $(EQU) fic/fic.dsc | \
		  sed 's/^...  \(.\{,78\}\).*/\1/'

#ANN = pmu
ANN = audio
ann:		../$(ANN).sch fic/fic.dsc gta02-core.par $(EQU)
		$(BOOM) annotate ../$(ANN).sch fic/fic.dsc gta02-core.par \
		  $(EQU) >$(ANN)-bom.sch || { rm -f $(ANN)-bom.sch; exit 1; }

$(SCH:%=%-bom.sch): \
		$(SCH:%=../%.sch) fic/fic.dsc gta02-core.$(ANNOTATE) $(EQU)
		for n in $(SCH); do \
		    $(BOOM) annotate '-s/OldPn (\S+)/FIC/' \
		      ../$$n.sch fic/fic.dsc \
		      gta02-core.$(ANNOTATE) $(EQU) >$$n-bom.sch \
		      || { rm -f $$n-bom.sch; exit 1; }; \
		done

gta02-core-bom.pro:	../gta02-core.pro
		sed 's|\./|../|' <../gta02-core.pro >$@ || \
		  { rm -f $@; exit 1; }

gta02-core-bom.sch:	../gta02-core.sch
		sed 's/\.sch"/-bom.sch"/' <$< | \
		  sed 's/"GTA02-CORE"/"GTA02-CORE (ANNOTATED)"/' | \
		  sed 's/\$$EndDescr/&\nText Notes 2500 1300 0 300 ~ 60\nANNOTATED VIEW - FOR BOM REVIEW/' >$@ || \
		  { rm -f $@; exit 1; }

sch:		gta02-core-bom.pro gta02-core-bom.sch
		eeschema `pwd`/gta02-core-bom.sch

gen generate:	gta02-core-bom.pro gta02-core-bom.sch $(SCH:%=%-bom.sch)
		eeschema --plot `pwd`/gta02-core-bom.sch
		../scripts/all-sheets --shrink gta02-core-bom
		gzip -f -9 gta02-core-bom-all.ps
		../scripts/all-sheets --shrink --pdf gta02-core-bom

gv:		gta02-core-bom-all.ps.gz
		gv gta02-core-bom-all.ps.gz

xpdf:		gta02-core-bom-all.pdf
		xpdf gta02-core-bom-all.pdf

upload:		generate
		$(COPY) gta02-core-bom-all.ps.gz gta02-core-bom-all.pdf \
		  $(UPLOAD)
