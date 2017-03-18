CLEAN :=

all: AUTHORS

AUTHORS: tools/generate_authors
	tools/generate_authors AUTHORS

.PHONY: clean

clean:
	-rm -rf $(CLEAN)
