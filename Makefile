LINUX_PROJECT = linux
WINDOWS_PROJECT = windows

all: linux windows

linux:
	@$(MAKE) -C $(LINUX_PROJECT) dist

linux-clean:
	@$(MAKE) -C $(LINUX_PROJECT) clean-all

windows:
	@$(MAKE) -C $(WINDOWS_PROJECT) dist

windows-clean:
	@$(MAKE) -C $(WINDOWS_PROJECT) clean-all

clean: linux-clean windows-clean

.PHONY: all linux linux-clean windows windows-clean clean
