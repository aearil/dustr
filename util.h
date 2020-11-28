/* See LICENSE file for copyright and license details. */

#define MAX(A, B) ((A) > (B) ? (A) : (B))
#define MIN(A, B) ((A) < (B) ? (A) : (B))
#define BETWEEN(X, A, B) ((A) <= (X) && (X) <= (B))
#define LENGTH(X) (sizeof X / sizeof X[0])
/* Sets F flag in X depending on bool B state */
/* https://graphics.stanford.edu/~seander/bithacks.html#ConditionalSetOrClearBitsWithoutBranching
 */
#define SETFLAG(X, F, B) ((X) ^= ((~(B) + 1) ^ (X)) & (F))

#define NO_VALUE -1

void die(const char *fmt, ...);
void *ecalloc(size_t nmemb, size_t size);
void sdldie(const char *msg);
void sdlerrcheck(int line);
char *readfile(const char *file);
