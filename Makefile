PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man
CFLAGS   = -std=c99 -pedantic -Wall -Wextra -Os
LDFLAGS  = -s
LIBS = -lSDL2 -lpng -ljpeg
CC ?= cc

dustr: dustr.o util.o img.o
	$(CC) $(LDFLAGS) $(LIBS) $^ -o $@

%.o: %.c util.h img.h
	$(CC) $(CFLAGS) -c $< -o $@

install: dustr
	mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	cp -f $< "$(DESTDIR)$(PREFIX)/bin"
	chmod 755 "$(DESTDIR)$(PREFIX)/bin/$<"
	mkdir -p "$(DESTDIR)$(MANPREFIX)/man1"
	cp -f dustr.1 "$(DESTDIR)$(MANPREFIX)/man1"
	chmod 644 "$(DESTDIR)$(MANPREFIX)/man1/dustr.1"

clean:
	rm -f ./dustr
	rm -f ./*.o

.PHONY: clean
