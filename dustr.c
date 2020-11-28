#include <SDL2/SDL.h>
#include <stdio.h>

#include "img.h"
#include "util.h"

// TODO match window size to loaded file
#define WIDTH 800
#define HEIGHT 600
#define FPS 30

typedef struct {
	int x;
	int y;
} Vec2i;

static SDL_Window *gWindow = NULL;
static SDL_Renderer *gRenderer = NULL;
static SDL_Texture *gTexture = NULL;
static uint32_t *pixels = NULL;
static Vec2i winsize = {WIDTH, HEIGHT};
static Vec2i winpos = {0, 0};
static Vec2i mouse = {0, 0};
static SDL_Rect croprect = {0, 0, 1, 1};
static SDL_Rect imgrect = {0, 0, 1, 1};
static SDL_Rect dispcroprect = {0, 0, 1, 1};
static SDL_Rect dispimgrect = {0, 0, 1, 1};
static char *outputfile = NULL;

static int
init(void)
{
	uint32_t sdlflags = SDL_INIT_VIDEO;
	if (!SDL_WasInit(sdlflags))
		if (SDL_Init(sdlflags))
			sdldie("cannot initialize SDL");
	atexit(SDL_Quit);

	gWindow = SDL_CreateWindow("dustr", SDL_WINDOWPOS_UNDEFINED,
							 SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT,
							 SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
	if (!gWindow)
		sdldie("cannot create SDL window");

	gRenderer = SDL_CreateRenderer(gWindow, -1, 0);
	if (!gRenderer)
		sdldie("cannot create SDL renderer");
	SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);

	gTexture = SDL_CreateTexture(gRenderer, SDL_PIXELFORMAT_ABGR8888,
								 SDL_TEXTUREACCESS_STATIC, imgrect.w, imgrect.h);
	if (!gTexture)
		sdldie("cannot create SDL texture");

	SDL_UpdateTexture(gTexture, NULL, pixels, imgrect.w * sizeof(uint32_t));

	return 1;
}

void
quit(void)
{
	free(pixels);
	SDL_DestroyTexture(gTexture);
	SDL_DestroyRenderer(gRenderer);
	SDL_DestroyWindow(gWindow);
	gTexture = NULL;
	gRenderer = NULL;
	gWindow = NULL;
	exit(0);
}

void
mouseevent(SDL_Event *event)
{
	mouse.x = event->motion.x;
	mouse.y = event->motion.y;

	switch (event->type) {
	case SDL_MOUSEBUTTONUP:
		printf("%dx%d@+%d+%d\n", croprect.w, croprect.h, croprect.x - dispimgrect.x, croprect.y - dispimgrect.y);
		write_png(outputfile, pixels, dispimgrect.w, croprect.w, croprect.h,
				croprect.x - dispimgrect.x, croprect.y - dispimgrect.y);
		quit();
		break;
	}
}

void
keyevent(SDL_Event *event)
{
	// int shift = SDL_GetModState() & KMOD_LSHIFT || SDL_GetModState() & KMOD_RSHIFT;

	switch(event->key.keysym.sym) {
	case SDLK_q:
	case SDLK_ESCAPE:
		quit();
		break;
	}
}

void
draw()
{
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xff);
	SDL_RenderClear(gRenderer);

	// Draw image background
	dispimgrect = imgrect;
	dispimgrect.x = (winsize.x - (int)dispimgrect.w) * 0.5;
	dispimgrect.y = (winsize.y - (int)dispimgrect.h) * 0.5;
	SDL_RenderCopy(gRenderer, gTexture, NULL, &dispimgrect);

	// Draw crop rectangle
	croprect.x = mouse.x - croprect.w * 0.5;
	croprect.y = mouse.y - croprect.h * 0.5;
	if (croprect.x < dispimgrect.x)
		croprect.x = dispimgrect.x;
	else if (croprect.x + croprect.w > dispimgrect.x + dispimgrect.w)
		croprect.x = dispimgrect.x + dispimgrect.w - croprect.w;
	if (croprect.y < dispimgrect.y)
		croprect.y = dispimgrect.y;
	else if (croprect.y + croprect.h > dispimgrect.y + dispimgrect.h)
		croprect.y = dispimgrect.y + dispimgrect.h - croprect.h;

	SDL_SetRenderDrawColor(gRenderer, 0xff, 0xff, 0xff, 0xff);
	SDL_Rect rect1 = croprect;
	rect1.x -= 1;
	rect1.y -= 1;
	rect1.w += 2;
	rect1.h += 2;
	SDL_Rect rect2 = rect1;
	rect2.x -= 1;
	rect2.y -= 1;
	rect2.w += 2;
	rect2.h += 2;
	SDL_RenderDrawRect(gRenderer, &rect1);
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xff);
	SDL_RenderDrawRect(gRenderer, &rect2);

	SDL_RenderPresent(gRenderer);
}

void
usage(const char* name)
{
	die("Usage: %s [-g <width>x<height>] <img in> <img out>", name);
}

int
main(int argc, char *argv[])
{
	int ticknext = 0;

	char *inputfile = NULL;

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-g")) {
			if (argc < i + 4 || sscanf(argv[++i], "%ux%u", (uint32_t*)&croprect.w, (uint32_t*)&croprect.h) < 2)
				usage(argv[0]);
		// TODO add option to choose an aspect ratio rather than geometry
		} else if (!inputfile) {
			inputfile = argv[i];
		} else if (!outputfile) {
			outputfile = argv[i];
		}
	}

	if (!inputfile || !outputfile)
		usage(argv[0]);

	read_png(inputfile, &pixels, (uint32_t*)&imgrect.w, (uint32_t*)&imgrect.h);

	/* Initialize GUI */
	init();

	while (1) {
		int tick = SDL_GetTicks();
		SDL_Event event;
		if (tick < ticknext)
			SDL_Delay(ticknext - tick);
		ticknext = tick + (1000 / FPS);

		draw(pixels);

		while (SDL_PollEvent(&event) != 0) {
			if (event.type == SDL_QUIT)
				quit();
			else if (event.type == SDL_MOUSEBUTTONUP ||
					 event.type == SDL_MOUSEBUTTONDOWN ||
					 event.type == SDL_MOUSEMOTION) {
				mouseevent(&event);
			} else if (event.type == SDL_KEYDOWN) {
				keyevent(&event);
			} else if (event.type == SDL_WINDOWEVENT) {
				if (event.window.event == SDL_WINDOWEVENT_EXPOSED ||
						event.window.event == SDL_WINDOWEVENT_RESIZED) {
					SDL_GetWindowSize(gWindow, &winsize.x, &winsize.y);
					SDL_GetWindowPosition(gWindow, &winpos.x, &winpos.y);
					draw(pixels);
				}
			}
		}
	}
	quit();
	return 0;
}
