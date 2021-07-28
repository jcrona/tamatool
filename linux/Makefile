GNU_PREFIX =

CC=$(GNU_PREFIX)gcc
LD=$(GNU_PREFIX)ld

DESKTOP_FILE = tamatool.desktop
ICON_FILE = tamatool.png

VERSION = $(shell git describe --tags --always --dirty)

DIST_PATH = dist
DIST_FILE = tamatool-linux-$(VERSION).zip

RES_PATH = ../res

LDLIBS = -lSDL2 -lSDL2_image -lpng16
CFLAGS += -Wall -I/usr/include/SDL2/

include ../src/common.mk

BUILD_FOLDER = build

OBJECTS := $(addprefix $(BUILD_FOLDER)/, $(OBJECTS))

all: $(TARGET)

dist: all
	@rm -rf $(DIST_PATH)
	@mkdir -p $(DIST_PATH)
	@install -s -m 0755 $(TARGET) $(DIST_PATH)
	@install -m 0644 $(DESKTOP_FILE) $(DIST_PATH)
	@install -m 0644 $(ICON_FILE) $(DIST_PATH)
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

$(BUILD_FOLDER)/%.o : ../src/%.c
	@echo "[$@] ..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_FOLDER)/lib/%.o : ../src/lib/%.c
	@echo "[$@] ..."
	@$(CC) $(CFLAGS) -c $< -o $@

.PHONY: all dist dist-clean clean clean-all
