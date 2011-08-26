SO_OBJS := $(addprefix src/,labelling.o contour.o quad.o pca.o marker.o \
	unwarp.o code_grid.o threshold.o pose.o crc12.o )

CLEAN += $(SO_OBJS) lib

solib: lib/libkoki.so

lib/libkoki.so: $(SO_OBJS)
	mkdir -p `dirname $@`
	$(CC) -shared -Wl,-soname,libkoki.so \
		-o $@ $^



