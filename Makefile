LINUX_PROJECT = linux
WINDOWS_PROJECT = windows
MAC_PROJECT = mac

ICONS_FOLDER = icons

all: linux windows
# The mac target requires an actual Mac

linux:
	@$(MAKE) -C $(LINUX_PROJECT) dist

linux-clean:
	@$(MAKE) -C $(LINUX_PROJECT) clean-all

windows:
	@$(MAKE) -C $(WINDOWS_PROJECT) dist

windows-clean:
	@$(MAKE) -C $(WINDOWS_PROJECT) clean-all

mac:
	@$(MAKE) -C $(MAC_PROJECT) dist

mac-clean:
	@$(MAKE) -C $(MAC_PROJECT) clean-all

icons:
	@$(MAKE) -C $(ICONS_FOLDER) install

clean: linux-clean windows-clean

.PHONY: all linux linux-clean windows windows-clean mac mac-clean icons clean
