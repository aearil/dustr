#include <SDL2/SDL.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

void *
ecalloc(size_t nmemb, size_t size)
{
	void *p;

	if (!(p = calloc(nmemb, size)))
		die("calloc:");
	return p;
}

void
die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt) - 1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}

	exit(1);
}

void
sdldie(const char *msg)
{
	fprintf(stderr, "sdldie: %s: %s\n", msg, SDL_GetError());
	SDL_Quit();
	exit(1);
}

void
sdlerrcheck(int line)
{
	const char *sdlerror = SDL_GetError();
	if (*sdlerror != '\0') {
		fprintf(stderr, "SDL Error: l.%d: %s\n", line, sdlerror);
		SDL_ClearError();
	}
}

char *
readfile(const char *file)
{
	FILE *f;
	long flen;
	char *str;

	if (!(f = fopen(file, "r")))
		die("%s could not be read\n", file);

	fseek(f, 0, SEEK_END);
	flen = ftell(f); /* get file length */
	rewind(f);
	str = calloc(flen + 2, sizeof(char));
	if (!str)
		die("Insufficient memory to load %s\n", file);

	fread(str, flen, sizeof(char), f);
	fclose(f);
	str[flen + 1] = '\0';
	return str;
}

void
efread(void *p, size_t s, size_t n, FILE *f)
{
	if (fread(p, s, n, f) != n) {
		if (ferror(f))
			die("fread:");
		else
			die("fread: Unexpected end of file");
	}
}

void
efwrite(const void *p, size_t s, size_t n, FILE *f)
{
	if (fwrite(p, s, n, f) != n)
		die("fwrite:");
}
