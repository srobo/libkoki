TOOLS_BINS := $(addprefix tools/,take_photo)

CLEAN += $(TOOLS_BINS) tools/*.o

include tools/depend

tools/depend: src/*.c
	rm -f tools/depend
	for file in src/*.c; do \
		$(CC) $(CFLAGS) -MM $$file -o - >> $@ ; \
	done ;

tools: $(TOOLS_BINS)

$(TOOLS_BINS): % : %.o lib/libkoki.so

tools/%.o: tools/%.c
