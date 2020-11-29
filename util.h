#pragma once

#include <sys/types.h>
#include <stdio.h>

#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B) ((A) <= (X) && (X) <= (B))
#define LENGTH(X) (sizeof(X) / sizeof(X[0]))
/* Sets F flag in X depending on bool B state */
/* https://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching
 */
#define SETFLAG(X, F, B) ((X) ^= ((~(B) + 1) ^ (X)) & (F))

#define NO_VALUE -1

typedef struct {
	int x;
	int y;
} Vec2i;

void die(const char *fmt, ...);
void *ecalloc(size_t nmemb, size_t size);
void sdldie(const char *msg);
void sdlerrcheck(int line);
char *readfile(const char *file);
void efread(void *p, size_t s, size_t n, FILE *f);
void efwrite(const void *p, size_t s, size_t n, FILE *f);
