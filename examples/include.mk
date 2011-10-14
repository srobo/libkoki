
EXAMPLE_BINS := $(addprefix examples/,marker_info realtime_quads)
GL_EXAMPLE_BINS := $(addprefix examples/,realtime_gl)

CLEAN += $(EXAMPLE_BINS) examples/*.o

GL_CFLAGS +=  `pkg-config --cflags gl glu`
GL_LDFLAGS += `pkg-config --libs   gl glu` -lglut

include examples/depend

examples/depend: src/*.c
	rm -f examples/depend
	for file in src/*.c; do \
		$(CC) $(CFLAGS) -MM $$file -o - >> $@ ; \
	done ;

examples: $(EXAMPLE_BINS)

$(EXAMPLE_BINS): % : %.o lib/libkoki.so
	$(CC) $(LDFLAGS) -lkoki -o $@ $<

$(GL_EXAMPLE_BINS): % : %.o lib/libkoki.so
	$(CC) $(LDFLAGS) -lkoki -o $@ $<

examples/realtime_gl.o: examples/realtime_gl.c
	$(CC) $(CFLAGS) -o $@ $^

examples/%.o: examples/%.c
