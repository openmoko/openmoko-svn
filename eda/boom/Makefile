UPLOAD=werner@sita.openmoko.org:public_html/gta02-core/
COPY=rsync -e ssh --progress

.PHONY:		all xpdf spotless upload

all:
		@echo "make what ? xpdf, upload, or spotless ?" 1>&2

workflow.pdf:	workflow.fig
		fig2dev -L pdf $< >$@ || { rm -f $@; exit 1; }

xpdf:		workflow.pdf
		xpdf workflow.pdf

upload:		workflow.pdf
		$(COPY) workflow.pdf $(UPLOAD)/bom-workflow.pdf

spotless:
		rm -f workflow.pdf
