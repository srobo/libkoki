EXAMPLE_BINS := $(addprefix test/,example realtime debug_img marker_info speed_test)

CLEAN += $(EXAMPLE_BINS) test/*.o

include test/depend

test/depend: src/*.c
	rm -f test/depend
	for file in src/*.c; do \
		$(CC) $(CFLAGS) -MM $$file -o - >> $@ ; \
	done ;

examples: $(EXAMPLE_BINS)

$(EXAMPLE_BINS): % : %.o lib/libkoki.so

test/%.o: test/%.c
