dustr: dustr.o util.o img.o
	cc -std=c99 -g -Wall -Wpedantic -Wextra -lSDL2 -lpng $^ -o $@

#%.o: %.h
util.o: util.h
img.o: img.h
%.o: %.c
	cc -std=c99 -g -Wall -Wpedantic -Wextra -c $< -o $@

clean:
	rm -f ./dustr
	rm -f ./*.o

.PHONY: clean
