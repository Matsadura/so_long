NAME        := so_long

CC          := cc
CFLAGS      := -Wall -Wextra -Werror -std=c11 -D_DEFAULT_SOURCE -Iincludes
ifeq ($(DEBUG),1)
CFLAGS      += -g3 -O0
else
CFLAGS      += -O2
endif

# SDL2 + SDL2_image discovery (override on the command line if needed).
SDL_CFLAGS  ?= $(shell pkg-config --cflags sdl2 SDL2_image 2>/dev/null || sdl2-config --cflags 2>/dev/null)
SDL_LIBS    ?= $(shell pkg-config --libs sdl2 SDL2_image 2>/dev/null || echo "$$(sdl2-config --libs 2>/dev/null) -lSDL2_image")

SRC         := src/main.c \
               src/core/game.c \
               src/core/enemies.c \
               src/core/player.c \
               src/core/lanes.c \
               src/core/scoring.c \
               src/map/map_read.c \
               src/map/map_validate.c \
               src/map/map_path.c \
               src/map/map_lanes.c \
               src/map/levels.c \
               src/platform/sdl_platform.c \
               src/platform/sdl_input.c \
               src/platform/sdl_audio.c \
               src/ui/render.c \
               src/ui/hud.c \
               src/ui/font.c \
               src/ui/screens.c \
               src/ui/sound.c

BUILD_DIR   := build/native
OBJ         := $(SRC:%.c=$(BUILD_DIR)/%.o)
DEP         := $(OBJ:.o=.d)

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(SDL_LIBS) -lm -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(SDL_CFLAGS) -MMD -MP -c $< -o $@

# --- WebAssembly (requires emsdk_env.sh sourced) -------------------------
EMCC        ?= emcc
WEB_DIR     := web
WEB_OUT     := $(WEB_DIR)/index.html
WEB_SHELL   := $(WEB_DIR)/shell.html
WASM_FLAGS  := -Wall -Wextra -Werror -std=c11 -D_DEFAULT_SOURCE -Iincludes -O2 \
               -sUSE_SDL=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS='["png"]' \
               -sALLOW_MEMORY_GROWTH=1 -sENVIRONMENT=web \
               -sEXPORTED_FUNCTIONS='["_main","_web_action","_web_is_muted"]' \
               -sEXPORTED_RUNTIME_METHODS='["ccall"]' \
               --preload-file assets/sprites --preload-file assets/maps \
               --preload-file assets/sounds \
               --shell-file $(WEB_SHELL)

wasm: $(WEB_OUT)

$(WEB_OUT): $(SRC) $(wildcard includes/*.h) $(WEB_SHELL) \
            $(wildcard assets/sprites/*.png) $(wildcard assets/maps/*.ber) \
            $(wildcard assets/sounds/*.wav)
	$(EMCC) $(WASM_FLAGS) $(SRC) -o $@

test: $(NAME)
	./tests/run_tests.sh

clean:
	rm -rf build

fclean: clean
	rm -f $(NAME) $(WEB_DIR)/index.html $(WEB_DIR)/index.js \
		$(WEB_DIR)/index.wasm $(WEB_DIR)/index.data

re: fclean
	$(MAKE) all

-include $(DEP)

.PHONY: all wasm test clean fclean re
