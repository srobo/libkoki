
EXAMPLE_BINS := $(addprefix examples/,realtime_gl marker_info)

CLEAN += $(EXAMPLE_BINS) examples/*.o

CFLAGS +=  `pkg-config --cflags gl glu`
LDFLAGS += `pkg-config --libs   gl glu` -lglut

include examples/depend

examples/depend: src/*.c
	rm -f examples/depend
	for file in src/*.c; do \
		$(CC) $(CFLAGS) -MM $$file -o - >> $@ ; \
	done ;

examples: $(EXAMPLE_BINS)

$(EXAMPLE_BINS): % : %.o lib/libkoki.so
	$(CC) $(LDFLAGS) -lkoki -o $@ $<

examples/%.o: examples/%.c
