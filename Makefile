VERSION := 0.1
CC      := gcc
CFLAGS  := -Wall -fPIC -fPIE
LDLAGS  := -pie 
DESTDIR :=
PREFIX  := /usr
BINDIR  := /bin
MANDIR  := /share/man
APPNAME := augrep

TARGET	:= augrep
SOURCES := $(shell find src/ -type f -name *.c)
OBJECTS := $(patsubst src/%,build/%,$(SOURCES:.c=.o))
DEPS	:= $(OBJECTS:.o=.deps)

$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(LDFLAGS) $(OBJECTS) 

build/%.o: src/%.c
	@mkdir -p build/
	$(CC) $(CFLAGS) -g -DVERSION=\"$(VERSION)\" -DAPPNAME=\"$(APPNAME)\" -MD -MF $(@:.o=.deps) -c -o $@ $< 

clean:
	$(RM) -r build/ $(TARGET) 

install:
	mkdir -p $(DESTDIR)/$(PREFIX)/$(BINDIR)
	cp -p $(APPNAME) $(DESTDIR)/$(PREFIX)/$(BINDIR)
	mkdir -p $(DESTDIR)/$(PREFIX)/$(MANDIR)/man1/
	cp -pr man1/* $(DESTDIR)/$(PREFIX)/$(MANDIR)/man1/

web: 
	cp README_$(APPNAME).html /home/kevin/docs/kzone5/source
	(cd /home/kevin/docs/kzone5; ./make.pl medgrep)

-include $(DEPS)

.PHONY: clean
