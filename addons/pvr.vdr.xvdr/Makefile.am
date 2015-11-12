addondir = $(prefix)/addons/pvr.vdr.xvdr

addon_DATA = \
	addon.xml \
	$(ADDONNAME) \
	icon.png

if MINGW32
addon_DATA += \
	pthreadGC2.dll
mingwdlldir := $(shell $(CC) -print-sysroot)/mingw/bin
endif

$(ADDONNAME):
	$(MAKE) -C ../../src
if MINGW32
# TODO: Recursive copying of the dependency DLLs
	for dll in `$(OBJDUMP) -p $(ADDONNAME) | egrep '^[[:space:]]*DLL Name: lib.+\.dll$$' | awk '{print $$3}'`; do \
		if test -f $(mingwdlldir)/$$dll; then \
			cp $(mingwdlldir)/$$dll .; \
		fi; \
	done
endif

install-data-hook: $(ADDONNAME)
	cp -Rf "$(srcdir)/resources" "$(DESTDIR)$(addondir)"
	chmod -R +w "$(DESTDIR)$(addondir)/resources"

uninstall-hook:
	rm -Rf "$(DESTDIR)$(addondir)/resources"

EXTRA_DIST = \
	resources \
	addon.xml \
	icon.png
