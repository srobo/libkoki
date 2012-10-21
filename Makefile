CLEAN :=

all: bugs_html AUTHORS

include */include.mk

AUTHORS: tools/generate_authors
	tools/generate_authors AUTHORS

.PHONY: clean

clean:
	-rm -rf $(CLEAN)
