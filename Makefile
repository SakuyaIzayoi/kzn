debug ?= 0
NAME := kzn
CC := clang
SRC_DIR := src
BUILD_DIR := build

OBJ := $(patsubst %.c,%.o, $(wildcard $(SRC_DIR)/*.c))

CFLAGS := -std=c17 -Wall -pedantic -Wshadow -Wstrict-aliasing -Wstrict-overflow \
		-Wformat -Wformat=2 -Wconversion -Wimplicit-fallthrough -Werror=format-security \
		-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3 -fPIE -fcf-protection=full \
		-Werror=implicit -Werror=incompatible-pointer-types -Werror=int-conversion
LDFLAGS := -lvulkan -pie
SDL2FLAGS := $(shell pkg-config sdl2 --cflags --libs)

ifeq ($(debug), 1)
	CFLAGS := $(CFLAGS) -g -O0
else
	CFLAGS := $(CFLAGS) -O2
endif

#

$(NAME): dir shaders $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(patsubst %, build/%, $(OBJ)) $(SDL2FLAGS) $(LDFLAGS) 

$(OBJ): dir
	@mkdir -p $(BUILD_DIR)/$(@D)
	@$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ -c $*.c

dir:
	@mkdir -p $(BUILD_DIR)

shaders: frag.spv vert.spv

frag.spv: $(patsubst %, $(SRC_DIR)/shaders/%, shader.frag)
	glslc $< -o $@

vert.spv: $(patsubst %, $(SRC_DIR)/shaders/%, shader.vert)
	glslc $< -o $@

clean:
	@rm -rf $(BUILD_DIR) $(NAME) *.spv compile_commands.json

bear: clean
	bear -- make

.PHONY: all shaders dir clean bear
