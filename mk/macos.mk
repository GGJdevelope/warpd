.PHONY: rel all install clean

CFILES=$(shell find src/*.c)
OBJCFILES=$(shell find src/platform/macos -name '*.m')
OBJECTS=$(CFILES:.c=.o) $(OBJCFILES:.m=.o)

RELFLAGS=-Wl,-adhoc_codesign -framework cocoa -framework carbon -Wl,-sectcreate,__TEXT,__info_plist,files/Info.plist

all: $(OBJECTS)
	-mkdir -p bin
	$(CC) -o bin/warpd-bin $(OBJECTS) -framework cocoa -framework carbon -Wl,-sectcreate,__TEXT,__info_plist,files/Info.plist
	-rm -rf bin/warpd.app
	mkdir -p bin/warpd.app/Contents/MacOS
	mkdir -p bin/warpd.app/Contents/Resources
	cp bin/warpd-bin bin/warpd.app/Contents/MacOS/warpd
	cp files/Info.plist bin/warpd.app/Contents/Info.plist
	chmod +x bin/warpd.app/Contents/MacOS/warpd
	./codesign/sign.sh bin/warpd.app/Contents/MacOS/warpd
	ln -sf warpd.app/Contents/MacOS/warpd bin/warpd
rel: clean
	$(CC) -o bin/warpd-arm $(CFILES) $(OBJCFILES) -target arm64-apple-macos $(CFLAGS) $(RELFLAGS)
	$(CC) -o bin/warpd-x86  $(CFILES) $(OBJCFILES) -target x86_64-apple-macos $(CFLAGS) $(RELFLAGS)
	lipo -create bin/warpd-arm bin/warpd-x86 -output bin/warpd-bin && rm -r bin/warpd-*
	-rm -rf bin/warpd.app
	mkdir -p bin/warpd.app/Contents/MacOS
	mkdir -p bin/warpd.app/Contents/Resources
	cp bin/warpd-bin bin/warpd.app/Contents/MacOS/warpd
	cp files/Info.plist bin/warpd.app/Contents/Info.plist
	chmod +x bin/warpd.app/Contents/MacOS/warpd
	./codesign/sign.sh bin/warpd.app/Contents/MacOS/warpd
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
