CFLAGS+=-g -Wall -O2
LDFLAGS+=-L./lib -lkoki
INCLUDES+=-I. -I./include

OBJECTS=

SRC_DIR=./src
LIB_DIR=./lib
TEST_DIR=./test
DOCS_DIR=./docs

all: solib example docs docs_latex

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

clean:
	rm -rf $(LIB_DIR) $(TEST_DIR)/example *.o $(DOCS_DIR)/html $(DOCS_DIR)/latex


.PHONY: clean solib example run_example docs docs_latex
