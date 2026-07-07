.PHONY: rel all install clean

CFILES=$(shell find src/*.c)
OBJCFILES=$(shell find src/platform/macos -name '*.m')
OBJECTS=$(CFILES:.c=.o) $(OBJCFILES:.m=.o)
MACOS_ICON=files/warpd.icns
MACOS_ICON_SRC=files/warpd-icon.svg
MACOS_ICONSET=tmp/warpd.iconset
MACOS_ICON_RENDER=tmp/warpd-icon-render

RELFLAGS=-Wl,-adhoc_codesign -framework cocoa -framework carbon -Wl,-sectcreate,__TEXT,__info_plist,files/Info.plist

$(MACOS_ICON): $(MACOS_ICON_SRC)
	-rm -rf $(MACOS_ICON_RENDER) $(MACOS_ICONSET)
	mkdir -p $(MACOS_ICON_RENDER) $(MACOS_ICONSET)
	qlmanage -t -s 1024 -o $(MACOS_ICON_RENDER) $(MACOS_ICON_SRC) >/dev/null
	sips -z 16 16 $(MACOS_ICON_RENDER)/warpd-icon.svg.png --out $(MACOS_ICONSET)/icon_16x16.png >/dev/null
	sips -z 32 32 $(MACOS_ICON_RENDER)/warpd-icon.svg.png --out $(MACOS_ICONSET)/icon_16x16@2x.png >/dev/null
	sips -z 32 32 $(MACOS_ICON_RENDER)/warpd-icon.svg.png --out $(MACOS_ICONSET)/icon_32x32.png >/dev/null
	sips -z 64 64 $(MACOS_ICON_RENDER)/warpd-icon.svg.png --out $(MACOS_ICONSET)/icon_32x32@2x.png >/dev/null
	sips -z 128 128 $(MACOS_ICON_RENDER)/warpd-icon.svg.png --out $(MACOS_ICONSET)/icon_128x128.png >/dev/null
	sips -z 256 256 $(MACOS_ICON_RENDER)/warpd-icon.svg.png --out $(MACOS_ICONSET)/icon_128x128@2x.png >/dev/null
	sips -z 256 256 $(MACOS_ICON_RENDER)/warpd-icon.svg.png --out $(MACOS_ICONSET)/icon_256x256.png >/dev/null
	sips -z 512 512 $(MACOS_ICON_RENDER)/warpd-icon.svg.png --out $(MACOS_ICONSET)/icon_256x256@2x.png >/dev/null
	sips -z 512 512 $(MACOS_ICON_RENDER)/warpd-icon.svg.png --out $(MACOS_ICONSET)/icon_512x512.png >/dev/null
	cp $(MACOS_ICON_RENDER)/warpd-icon.svg.png $(MACOS_ICONSET)/icon_512x512@2x.png
	iconutil -c icns $(MACOS_ICONSET) -o $(MACOS_ICON)
	-rm -rf $(MACOS_ICON_RENDER) $(MACOS_ICONSET)

all: $(OBJECTS) $(MACOS_ICON)
	-mkdir -p bin
	$(CC) -o bin/warpd-bin $(OBJECTS) -framework cocoa -framework carbon -Wl,-sectcreate,__TEXT,__info_plist,files/Info.plist
	-rm -rf bin/warpd.app
	mkdir -p bin/warpd.app/Contents/MacOS
	mkdir -p bin/warpd.app/Contents/Resources
	cp bin/warpd-bin bin/warpd.app/Contents/MacOS/warpd
	cp files/Info.plist bin/warpd.app/Contents/Info.plist
	cp $(MACOS_ICON) bin/warpd.app/Contents/Resources/warpd.icns
	chmod +x bin/warpd.app/Contents/MacOS/warpd
	./codesign/sign.sh bin/warpd.app
	ln -sf warpd.app/Contents/MacOS/warpd bin/warpd
rel: clean $(MACOS_ICON)
	$(CC) -o bin/warpd-arm $(CFILES) $(OBJCFILES) -target arm64-apple-macos $(CFLAGS) $(RELFLAGS)
	$(CC) -o bin/warpd-x86  $(CFILES) $(OBJCFILES) -target x86_64-apple-macos $(CFLAGS) $(RELFLAGS)
	lipo -create bin/warpd-arm bin/warpd-x86 -output bin/warpd-bin && rm -r bin/warpd-*
	-rm -rf bin/warpd.app
	mkdir -p bin/warpd.app/Contents/MacOS
	mkdir -p bin/warpd.app/Contents/Resources
	cp bin/warpd-bin bin/warpd.app/Contents/MacOS/warpd
	cp files/Info.plist bin/warpd.app/Contents/Info.plist
	cp $(MACOS_ICON) bin/warpd.app/Contents/Resources/warpd.icns
	chmod +x bin/warpd.app/Contents/MacOS/warpd
	./codesign/sign.sh bin/warpd.app
	ln -sf warpd.app/Contents/MacOS/warpd bin/warpd
	-rm -rf tmp dist
	mkdir tmp dist
	DESTDIR=tmp make install
	cd tmp && tar czvf ../dist/macos-$(VERSION).tar.gz $$(find . -type f -o -type l)
	-rm -rf tmp
install:
	mkdir -p $(DESTDIR)/Applications \
		$(DESTDIR)/usr/local/bin/ \
		$(DESTDIR)/usr/local/share/man/man1/ \
		$(DESTDIR)/Library/LaunchAgents && \
	install -m644 files/warpd.1.gz $(DESTDIR)/usr/local/share/man/man1 && \
	cp -R bin/warpd.app $(DESTDIR)/Applications/ && \
	ln -sf /Applications/warpd.app/Contents/MacOS/warpd $(DESTDIR)/usr/local/bin/warpd && \
	install -m644 files/com.warpd.warpd.plist $(DESTDIR)/Library/LaunchAgents
uninstall:
	rm -f $(DESTDIR)/usr/local/share/man/man1/warpd.1.gz \
		$(DESTDIR)/usr/local/bin/warpd \
		$(DESTDIR)/Library/LaunchAgents/com.warpd.warpd.plist
	rm -rf $(DESTDIR)/Applications/warpd.app
clean:
	-rm $(OBJECTS)
	-rm -rf bin/warpd.app bin/warpd-bin
