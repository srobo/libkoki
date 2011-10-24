SO_OBJS := $(addprefix src/,labelling.o contour.o quad.o pca.o marker.o \
	unwarp.o code_grid.o threshold.o pose.o crc12.o rotation.o bearing.o \
	v4l.o yaml_config.o image.o integral-image.o)

CLEAN += $(SO_OBJS) src/*.o lib

include src/depend

src/depend: src/*.c
	rm -f src/depend
	for file in src/*.c; do \
		$(CC) $(CFLAGS) -MM $$file -o - >> $@ ; \
	done ;

solib: lib/libkoki.so

lib/libkoki.so: $(SO_OBJS)
	mkdir -p `dirname $@`
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -Wl,-soname,libkoki.so \
		-o $@ $^



