CFLAGS += -g -Wall -O3 -std=gnu99
LDFLAGS += -L./lib -lm -lyaml

#CFLAGS += -DKOKI_DEBUG_LEVEL=KOKI_DEBUG_INFO

CFLAGS += -I. -I./include

CLEAN :=

CFLAGS+=`pkg-config --cflags glib-2.0 opencv`
LDFLAGS+=`pkg-config --libs glib-2.0 opencv`

world: solib examples tests

include */include.mk

all: solib examples tests docs docs_latex bugs_html AUTHORS

AUTHORS: tools/generate_authors
	tools/generate_authors AUTHORS

.PHONY: clean

clean:
	-rm -rf $(CLEAN)
