.PHONY:		all ps gv xpdf loe gen generate upload clean

ALL=gta02-core-modules
UPLOAD=werner@sita.openmoko.org:public_html/gta02-core/
COPY=rsync -e ssh --progress

all:
		@echo "make what ? targets: gen gv xpdf loe ps upload clean"
		@exit 1

ps:
		./mkloe --ps

gv:
		gv $(ALL).ps.gz

xpdf:
		xpdf $(ALL).pdf

loe:
		./mkloe

gen generate:
		./mkloe --one-ps $(ALL).ps
		gzip -f -9 $(ALL).ps
		./mkloe --one-pdf $(ALL).pdf

upload:		gen
		$(COPY) $(ALL).ps.gz $(ALL).pdf $(UPLOAD)

clean:
		rm -f *~*.fpd
		rm -f *.mod *.ps
		rm -f $(ALL).ps $(ALL).pdf
