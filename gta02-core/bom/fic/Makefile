BOOM=../../../eda/boom/boom

INV = inventory-fic-ee.csv

.PHONY:		all spotless

all:		fic.equ fic.inv fic.dsc present.inv missing.equ

fic.equ:	fic2vendor $(INV)
		perl fic2vendor $(INV) >$@ || { rm -f $@; exit 1; }

fic.inv:	fic2inv $(INV)
		perl fic2inv $(INV) >$@ || { rm -f $@; exit 1; }

fic.dsc:	fic2dsc $(INV)
		perl fic2dsc $(INV) >$@ || { rm -f $@; exit 1; }

present.inv:	fic.inv
		grep -v ' 0 USD' fic.inv >present.inv || { rm -f $@; exit 1; }

missing.equ:	equ2equ missing.inv fic.equ
		$(BOOM) ./equ2equ missing.inv fic.equ >$@ || \
		  { rm -f $@; exit 1; }

spotless:
		rm -f fic.equ fic.inv fic.dsc present.inv missing.equ
