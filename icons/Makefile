SRC_ICON = source-1024x1024.png

LINUX_ICON_BASE_FOLDER = ../linux
WINDOWS_ICON_BASE_FOLDER = ../windows
MAC_ICON_BASE_FOLDER = ../mac/tamatool.iconset

all: linux windows mac

install: linux-install windows-install mac-install


linux: icon-128x128.png

linux-install: linux
	@cp icon-128x128.png $(LINUX_ICON_BASE_FOLDER)/tamatool.png


windows: icon-256x256.ico

windows-install: windows
	@cp icon-256x256.ico $(WINDOWS_ICON_BASE_FOLDER)/tamatool.ico


mac: icon-16x16.png icon-32x32.png icon-64x64.png icon-128x128.png icon-256x256.png icon-512x512.png

mac-install: mac
	@cp icon-16x16.png $(MAC_ICON_BASE_FOLDER)/icon_16x16.png
	@cp icon-32x32.png $(MAC_ICON_BASE_FOLDER)/icon_16x16@2x.png
	@cp icon-32x32.png $(MAC_ICON_BASE_FOLDER)/icon_32x32.png
	@cp icon-64x64.png $(MAC_ICON_BASE_FOLDER)/icon_32x32@2x.png
	@cp icon-128x128.png $(MAC_ICON_BASE_FOLDER)/icon_128x128.png
	@cp icon-256x256.png $(MAC_ICON_BASE_FOLDER)/icon_128x128@2x.png
	@cp icon-256x256.png $(MAC_ICON_BASE_FOLDER)/icon_256x256.png
	@cp icon-512x512.png $(MAC_ICON_BASE_FOLDER)/icon_256x256@2x.png
	@cp icon-512x512.png $(MAC_ICON_BASE_FOLDER)/icon_512x512.png
	@cp $(SRC_ICON) $(MAC_ICON_BASE_FOLDER)/icon_512x512@2x.png


clean:
	@$(RM) -rf icon-*.*


icon-%: $(SRC_ICON)
	@echo "[$@] ..."
	@convert $< -resize $$(echo $@ | cut -d '-' -f 2 | cut -d '.' -f 1)\> $@


.PHONY: all install linux linux-install windows windows-install mac mac-install clean
