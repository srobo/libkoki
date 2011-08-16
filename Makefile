CFLAGS+=-g -Wall -O2 -std=c99
LDFLAGS+=-L./lib -lkoki
INCLUDES+=-I. -I./include

OBJECTS=labelling.o contour.o quad.o pca.o marker.o unwarp.o

SRC_DIR=./src
LIB_DIR=./lib
TEST_DIR=./test
DOCS_DIR=./docs
BUGS_DIR=./bugs
BUGS_HTML_DIR=$(BUGS_DIR)/html

CFLAGS+=`pkg-config --cflags glib-2.0 opencv`
LDFLAGS+=`pkg-config --libs glib-2.0 opencv`

all: solib example docs docs_latex bugs_html

%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -c $< -o $@

solib: $(OBJECTS)
	mkdir -p $(LIB_DIR)
	$(CC) -shared -Wl,-soname,libkoki.so \
		-o $(LIB_DIR)/libkoki.so $(OBJECTS)

example: solib
	$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) \
		$(TEST_DIR)/example.c -o $(TEST_DIR)/example

run_example:
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(LIB_DIR) \
		$(TEST_DIR)/example

docs:
	doxygen $(DOCS_DIR)/Doxyfile

docs_latex: docs
	cd $(DOCS_DIR)/latex ; make

bugs_html:
	ditz html $(BUGS_HTML_DIR)

clean:
	rm -rf $(LIB_DIR) $(TEST_DIR)/example *.o
	rm -rf $(DOCS_DIR)/html $(DOCS_DIR)/latex
	rm -rf $(BUGS_HTML_DIR)


.PHONY: clean solib example run_example docs docs_latex bugs_html
