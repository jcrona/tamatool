GNU_PREFIX =

CC=$(GNU_PREFIX)gcc
LD=$(GNU_PREFIX)ld

SDL_ROOT = libs
SDL_IMAGE_ROOT = libs
LIBPNG_ROOT = libs

SDL_LIB = $(SDL_ROOT)/lib/libSDL2-2.0.0.dylib
SDL_IMAGE_LIB = $(SDL_IMAGE_ROOT)/lib/libSDL2_image-2.0.0.dylib
LIBPNG_LIB = $(SDL_IMAGE_ROOT)/lib/libpng16.16.dylib

DIST_PATH = dist
DIST_FILE = tamatool-mac.zip

APP_NAME = TamaTool.app
PLIST_FILE = Info.plist
ICON_FILE = tamatool.icns
ICONSET_FOLDER = tamatool.iconset
WRAPPER_SCRIPT = tamatool.mac

RES_PATH = ../res

CFLAGS += -I$(SDL_ROOT)/include/SDL2 -I$(SDL_IMAGE_ROOT)/include/SDL2 -I$(LIBPNG_ROOT)/include -Wall
LDLIBS = -L$(SDL_ROOT)/lib -L$(SDL_IMAGE_ROOT)/lib -L$(LIBPNG_ROOT)/lib -lSDL2main -lSDL2 -lSDL2_image -lpng16

include ../src/common.mk

BUILD_FOLDER = build

OBJECTS := $(addprefix $(BUILD_FOLDER)/, $(OBJECTS))

all: $(TARGET) $(ICON_FILE)

dist: all
	@rm -rf $(DIST_PATH)
	@mkdir -p $(DIST_PATH)/${APP_NAME}/Contents/MacOS
	@mkdir -p $(DIST_PATH)/${APP_NAME}/Contents/Resources
	@install -s -m 0755 $(TARGET) $(DIST_PATH)/${APP_NAME}/Contents/MacOS
	@install -m 0755 $(WRAPPER_SCRIPT) $(DIST_PATH)/${APP_NAME}/Contents/MacOS
	@install -m 0644 $(SDL_LIB) $(DIST_PATH)/${APP_NAME}/Contents/MacOS
	@install -m 0644 $(SDL_IMAGE_LIB) $(DIST_PATH)/${APP_NAME}/Contents/MacOS
	@install -m 0644 $(LIBPNG_LIB) $(DIST_PATH)/${APP_NAME}/Contents/MacOS
	@install -m 0644 $(PLIST_FILE) $(DIST_PATH)/${APP_NAME}/Contents
	@install -m 0644 $(ICON_FILE) $(DIST_PATH)/${APP_NAME}/Contents/Resources
	@cp -a $(RES_PATH) $(DIST_PATH)/${APP_NAME}/Contents/MacOS
	@rm -f $(DIST_FILE)
	@cd $(DIST_PATH) && zip -r ../$(DIST_FILE) *

dist-clean:
	@rm -rf $(DIST_PATH) $(DIST_FILE)

$(TARGET): $(BUILD_FOLDER) $(OBJECTS)
	@echo
	@echo -n "Linking ..."
	@$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $@ $(LDLIBS)
	echo $(shell pwd)
	@install_name_tool -change $$(otool -L $(TARGET) | grep libSDL2-2.0.0.dylib | cut -d ' ' -f 1 | sed -e 's/	//g') @executable_path/libSDL2-2.0.0.dylib $(TARGET)
	@install_name_tool -change $$(otool -L $(TARGET) | grep libSDL2_image-2.0.0.dylib | cut -d ' ' -f 1 | sed -e 's/	//g') @executable_path/libSDL2_image-2.0.0.dylib $(TARGET)
	@install_name_tool -change $$(otool -L $(TARGET) | grep libpng16.16.dylib | cut -d ' ' -f 1 | sed -e 's/	//g') @executable_path/libpng16.16.dylib $(TARGET)
	@echo " -> $@"
	@echo

$(ICON_FILE): $(ICONSET_FOLDER)/*
	iconutil -c icns $(ICONSET_FOLDER)

clean:
	$(RM) -rf $(BUILD_FOLDER) $(TARGET) $(ICON_FILE)

clean-all: dist-clean sdl-lib-clean glew-lib-clean clean

$(BUILD_FOLDER):
	@mkdir -p $(BUILD_FOLDER)/lib

$(BUILD_FOLDER)/%.o : ../src/%.c
	@echo "[$@] ..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_FOLDER)/lib/%.o : ../src/lib/%.c
	@echo "[$@] ..."
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all dist dist-clean sdl-lib-clean glew-lib-clean clean clean-all
