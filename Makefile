CFLAGS += -g -Wall -O2 -std=c99
LDFLAGS += -L./lib -lkoki -lm

CFLAGS += -I. -I./include

CLEAN :=

CFLAGS+=`pkg-config --cflags glib-2.0 opencv`
LDFLAGS+=`pkg-config --libs glib-2.0 opencv`

world: solib examples

include */include.mk

all: solib examples docs docs_latex bugs_html AUTHORS

AUTHORS: tools/generate_authors
	tools/generate_authors AUTHORS

.PHONY: clean

clean:
	-rm -rf $(CLEAN)
