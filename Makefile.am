DATE = $(shell date +%Y%m%d)

zip_exclude := \*.am \*.in \*Makefile \*~ \*.\*.sw\? \*.gitignore

SUBDIRS = \
	addons \
	src

DIST_SUBDIRS = \
	$(SUBDIRS) \
	data

install-lib:
	$(MAKE) -C src/libxvdr/src install-lib

install-include:
	$(MAKE) -C src/libxvdr/src install-include

dist-zip: all
	cp -r addons/pvr.vdr.xvdr .
	$(STRIP) -s -x pvr.vdr.xvdr/$(ADDONNAME)
	zip -9 -r ./pvr.vdr.xvdr-$(VERSION).zip pvr.vdr.xvdr --exclude $(zip_exclude)
	rm -Rf pvr.vdr.xvdr

dist-zip-daily: all
	cp -r addons/pvr.vdr.xvdr .
	cat pvr.vdr.xvdr/addon.xml | sed "s/$(VERSION)/$(VERSION).$(DATE)/g" > pvr.vdr.xvdr/addon.xml.new
	mv pvr.vdr.xvdr/addon.xml.new pvr.vdr.xvdr/addon.xml
if !APPLE
	$(STRIP) -s -x pvr.vdr.xvdr/$(ADDONNAME)
endif
if MINGW32
	zip -9 -r ./pvr.vdr.xvdr-$(VERSION).$(DATE).zip pvr.vdr.xvdr --exclude $(zip_exclude)
else
	zip -9 -r ./pvr.vdr.xvdr-$(VERSION).$(DATE).zip pvr.vdr.xvdr --exclude $(zip_exclude) \*.dll
endif
	rm -Rf pvr.vdr.xvdr

dist-git: all
	GIT_REV=`git --no-pager log --abbrev=7 -n 1 --pretty=format:"%h %ci" HEAD | cut -f1 -d' '` ; \
	$(MAKE) VERSION=$$GIT_REV dist

repo: all dist-zip
	$(MAKE) -C data all
	rm -Rf addon-repository
	mkdir -p addon-repository/pvr.vdr.xvdr
	cp pvr.vdr.xvdr-$(VERSION).zip addon-repository/pvr.vdr.xvdr
	cp addons/pvr.vdr.xvdr/icon.png addon-repository/pvr.vdr.xvdr
	cp data/addons.xml addon-repository
	cp data/addons.xml.md5 addon-repository

repo-daily: all dist-zip-daily
	$(MAKE) -C data all
	rm -Rf addon-repository
	mkdir -p addon-repository/pvr.vdr.xvdr
	cp pvr.vdr.xvdr-$(VERSION).$(DATE).zip addon-repository/pvr.vdr.xvdr
	cp addons/pvr.vdr.xvdr/icon.png addon-repository/pvr.vdr.xvdr
	cat data/addons.xml | sed "s/$(VERSION)/$(VERSION).$(DATE)/g" > addon-repository/addons.xml
	cd addon-repository && md5sum addons.xml > addons.xml.md5

release-repo-daily: repo-daily
	cp -r addon-repository/* ~/Dropbox/Public/xbmc-addon-xvdr

repo-zip: all
	cd addons && \
	zip -9 -r ../repository.xvdr.linux32.zip repository.xvdr.linux32 --exclude $(zip_exclude) && \
	zip -9 -r ../repository.xvdr.linux64.zip repository.xvdr.linux64 --exclude $(zip_exclude) && \
	zip -9 -r ../repository.xvdr.rpi.zip repository.xvdr.rpi --exclude $(zip_exclude) && \
	zip -9 -r ../repository.xvdr.osx.zip repository.xvdr.osx --exclude $(zip_exclude) && \
	zip -9 -r ../repository.xvdr.osx64.zip repository.xvdr.osx64 --exclude $(zip_exclude) && \
	zip -9 -r ../repository.xvdr.win32.zip repository.xvdr.win32 --exclude $(zip_exclude)
