PREFIX = /usr/local

dustr: dustr.o util.o img.o
	cc -std=c99 -g -Wall -Wpedantic -Wextra -lSDL2 -lpng -ljpeg $^ -o $@

#%.o: %.h
util.o: util.h
img.o: img.h
%.o: %.c
	cc -std=c99 -g -Wall -Wpedantic -Wextra -c $< -o $@

install: dustr
	mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	cp -f $< "$(DESTDIR)$(PREFIX)/bin"
	chmod 755 "$(DESTDIR)$(PREFIX)/bin/$<"

clean:
	rm -f ./dustr
	rm -f ./*.o

.PHONY: clean
