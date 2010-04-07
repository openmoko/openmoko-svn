CACHE=query.data

.PHONY:		update regen regenerate clean spotless

all:		digi-key.dsc digi-key.inv

$(CACHE):	digi-key.equ
		awk '/^#END/ { exit } /^DIGI-KEY / { print $$2 }' \
		  digi-key.equ | \
		  perl ./dk-db.pl query \
		  `[ -r $(CACHE) ] && echo '' -i $(CACHE)` >_$@ || \
		  { rm -f $@ _$@; exit 1; }
		mv _$@ $@

digi-key.dsc:	$(CACHE)
		perl ./dk-db.pl dsc $(CACHE) >$@ || { rm -f $@; exit 1; }
	
digi-key.inv:	$(CACHE)
		perl ./dk-db.pl inv $(CACHE) >$@ || { rm -f $@; exit 1; }

update:
		$(MAKE) clean all

regen regenerate:
		rm -f digi-key.dsc digi-key.inv
		$(MAKE) all

clean:
		rm -f $(CACHE) _$(CACHE)

#
# we don't do a "make clean" on "make spotless", so that things like "make
# optimist" don't erase the cache.
#

spotless:
		rm -f digi-key.dsc digi-key.inv _$(CACHE)
