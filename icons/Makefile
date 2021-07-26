SRC_ICON = source-1024x1024.png

LINUX_ICON_BASE_FOLDER = ../linux
WINDOWS_ICON_BASE_FOLDER = ../windows

all: linux windows

install: linux-install windows-install


linux: icon-128x128.png

linux-install: linux
	@cp icon-128x128.png $(LINUX_ICON_BASE_FOLDER)/tamatool.png


windows: icon-256x256.ico

windows-install: windows
	@cp icon-256x256.ico $(WINDOWS_ICON_BASE_FOLDER)/tamatool.ico


clean:
	@$(RM) -rf icon-*.*


icon-%: $(SRC_ICON)
	@echo "[$@] ..."
	@convert $< -resize $$(echo $@ | cut -d '-' -f 2 | cut -d '.' -f 1)\> $@


.PHONY: all install linux linux-install windows windows-install clean
