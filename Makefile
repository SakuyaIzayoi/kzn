CC?=clang
SDL2FLAGS=$(shell pkg-config sdl2 --cflags --libs)
CFLAGS?=-std=c17 -O2 -g -Wall -pedantic -Wshadow -Wstrict-aliasing -Wstrict-overflow
LDFLAGS?=-lvulkan

.PHONY: all shaders debug clean release

all: kzn

kzn: main.c shaders
	${CC} ${CFLAGS} -o $@ $< ${SDL2FLAGS} ${LDFLAGS}

shaders: frag.spv vert.spv

frag.spv: shader.frag
	glslc $< -o $@

vert.spv: shader.vert
	glslc $< -o $@

release: main.c
	${CC} ${CFLAGS} -O3 -g -o kzn $< ${SDL2FLAGS} ${LDFLAGS}

clean:
	rm -f kzn
	rm -f *.o
	rm -f *.spv
