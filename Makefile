PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man
CFLAGS   = -std=c99 -pedantic -Wall -Wextra -Os
LDFLAGS  = -s
LIBS = -lSDL2 -lpng -ljpeg

dustr: dustr.o util.o img.o
	cc $(LDFLAGS) $(LIBS) $^ -o $@

%.o: %.c util.h img.h
	cc $(CFLAGS) -c $< -o $@

install: dustr
	mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	cp -f $< "$(DESTDIR)$(PREFIX)/bin"
	chmod 755 "$(DESTDIR)$(PREFIX)/bin/$<"
	cp -f dustr.1 "$(DESTDIR)$(MANPREFIX)/man1"
	chmod 644 "$(DESTDIR)$(MANPREFIX)/man1/dustr.1"

clean:
	rm -f ./dustr
	rm -f ./*.o

.PHONY: clean
