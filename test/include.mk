EXAMPLE_BINS := $(addprefix test/,example realtime debug_img marker_info)

CLEAN += $(EXAMPLE_BINS)

examples: $(EXAMPLE_BINS)

$(EXAMPLE_BINS): % : %.o lib/libkoki.so
