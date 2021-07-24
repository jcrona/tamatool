WIN_ARCH ?= 64

ifeq ($(WIN_ARCH), 64)
CROSS_COMPILER = x86_64-w64-mingw32
else ifeq ($(WIN_ARCH), 32)
CROSS_COMPILER = i686-w64-mingw32
else
$(error Bad WIN_ARCH value (only 64 and 32 are supported))
endif
GNU_PREFIX = $(CROSS_COMPILER)-

CC=$(GNU_PREFIX)gcc
LD=$(GNU_PREFIX)ld
WINDRES=$(GNU_PREFIX)windres

SDL_ROOT = libs/SDL2-2.0.14/$(CROSS_COMPILER)
SDL_IMAGE_ROOT = libs/SDL2_image-2.0.5/$(CROSS_COMPILER)
LIBPNG_ROOT = libs/libpng-1.6.37/$(CROSS_COMPILER)

SDL_LIB = $(SDL_ROOT)/bin/SDL2.dll
SDL_IMAGE_LIB = $(SDL_IMAGE_ROOT)/bin/SDL2_image.dll
LIBPNG_LIB = $(SDL_IMAGE_ROOT)/bin/libpng16-16.dll
ZLIB_LIB = $(SDL_IMAGE_ROOT)/bin/zlib1.dll

DIST_PATH = dist
DIST_FILE = tamatool-win$(WIN_ARCH).zip

RES_PATH = ../res

CFLAGS += -I$(SDL_ROOT)/include/SDL2 -I$(SDL_IMAGE_ROOT)/include/SDL2 -I$(LIBPNG_ROOT)/include -Wall
LDLIBS = -L$(SDL_ROOT)/lib -L$(SDL_IMAGE_ROOT)/lib -L$(LIBPNG_ROOT)/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lpng16 -mwindows -mconsole

include ../src/common.mk

BUILD_FOLDER = build

TARGET := $(TARGET).exe

OBJECTS += tamatool.res.o
OBJECTS := $(addprefix $(BUILD_FOLDER)/, $(OBJECTS))

all: $(TARGET)

dist: all
	@rm -rf $(DIST_PATH)
	@mkdir -p $(DIST_PATH)
	@install -s -m 0755 $(TARGET) $(DIST_PATH)
	@install -m 0644 $(SDL_LIB) $(DIST_PATH)
	@install -m 0644 $(SDL_IMAGE_LIB) $(DIST_PATH)
	@install -m 0644 $(LIBPNG_LIB) $(DIST_PATH)
	@install -m 0644 $(ZLIB_LIB) $(DIST_PATH)
	@cp -a $(RES_PATH) $(DIST_PATH)/
	@rm -f $(DIST_FILE)
	@cd $(DIST_PATH) && zip -r ../$(DIST_FILE) *

dist-clean:
	@rm -rf $(DIST_PATH) $(DIST_FILE)

$(TARGET): $(BUILD_FOLDER) $(OBJECTS)
	@echo
	@echo -n "Linking ..."
	@$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ $(LDLIBS)
	@echo " -> $@"
	@echo

clean:
	$(RM) -rf $(BUILD_FOLDER) $(TARGET)

clean-all: dist-clean clean

$(BUILD_FOLDER):
	@mkdir -p $(BUILD_FOLDER)/lib

$(BUILD_FOLDER)/%.res.o : %.rc
	@echo "[$@] ..."
	@$(WINDRES) $< -O coff -o $@

$(BUILD_FOLDER)/%.o : ../src/%.c
	@echo "[$@] ..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_FOLDER)/lib/%.o : ../src/lib/%.c
	@echo "[$@] ..."
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all dist dist-clean clean clean-all
