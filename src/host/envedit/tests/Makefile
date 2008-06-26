.PHONY:	tests clean

# Explicitly enumerate the lower case letters to escape localization weirdness.

tests:
	for n in [abcdefghijklmnopqrstuvwxyz]*; do \
	  SCRIPT=$$n . ./$$n; done; \
	  echo "Passed all $$passed tests" 2>&1

clean:
	rm -f _*
