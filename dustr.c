#include <SDL2/SDL.h>
#include <stdio.h>

#include "img.h"
#include "util.h"

// TODO match window size to loaded file
#define WIDTH 800
#define HEIGHT 600
#define FPS 30

static SDL_Window *gWindow = NULL;
static SDL_Renderer *gRenderer = NULL;
static SDL_Texture *gTexture = NULL;
static uint32_t *pixels = NULL;
static Vec2i winsize = {WIDTH, HEIGHT};
static Vec2i winpos = {0, 0};
static Vec2i mouse = {0, 0};
static Vec2i requestedcrop = {1, 1};
static Vec2i originalsize = {0, 0};
static SDL_Rect croprect = {0, 0, 1, 1};
static SDL_Rect imgrect = {0, 0, 1, 1};
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
								 SDL_TEXTUREACCESS_STATIC, originalsize.x, originalsize.y);
	if (!gTexture)
		sdldie("cannot create SDL texture");

	SDL_UpdateTexture(gTexture, NULL, pixels, originalsize.x * sizeof(uint32_t));

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

Vec2i
getcroppos()
{
	Vec2i pos;
	pos.x = (croprect.x - imgrect.x) * originalsize.x / imgrect.w;
	pos.y = (croprect.y - imgrect.y) * originalsize.y / imgrect.h;
	return pos;
}

void
mouseevent(SDL_Event *event)
{
	mouse.x = event->motion.x;
	mouse.y = event->motion.y;

	switch (event->type) {
	case SDL_MOUSEBUTTONUP: {
		Vec2i pos = getcroppos();
		printf("%dx%d@+%d+%d\n", requestedcrop.x, requestedcrop.y, pos.x, pos.y);
		write_png(outputfile, pixels, originalsize.x, requestedcrop, pos);
		quit();
		break;
	}
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
	if (originalsize.x > winsize.x || originalsize.y > winsize.y) {
		if (winsize.x / (float)originalsize.x > winsize.y / (float)originalsize.y) {
			imgrect.w = winsize.y * originalsize.x / originalsize.y;
			imgrect.h = winsize.y;
		} else {
			imgrect.w = winsize.x;
			imgrect.h = winsize.x * originalsize.y / originalsize.x;
		}
	} else {
		imgrect.w = originalsize.x;
		imgrect.h = originalsize.y;
	}
	imgrect.x = (winsize.x - (int)imgrect.w) * 0.5;
	imgrect.y = (winsize.y - (int)imgrect.h) * 0.5;
	SDL_RenderCopy(gRenderer, gTexture, NULL, &imgrect);

	// Draw crop rectangle
	croprect.w = requestedcrop.x * imgrect.w / originalsize.x;
	croprect.h = requestedcrop.y * imgrect.h / originalsize.y;
	croprect.x = mouse.x - croprect.w * 0.5;
	croprect.y = mouse.y - croprect.h * 0.5;
	if (croprect.x < imgrect.x)
		croprect.x = imgrect.x;
	else if (croprect.x + croprect.w > imgrect.x + imgrect.w)
		croprect.x = imgrect.x + imgrect.w - croprect.w;
	if (croprect.y < imgrect.y)
		croprect.y = imgrect.y;
	else if (croprect.y + croprect.h > imgrect.y + imgrect.h)
		croprect.y = imgrect.y + imgrect.h - croprect.h;

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
			if (argc < i + 4 || sscanf(argv[++i], "%ux%u", (uint32_t*)&requestedcrop.x, (uint32_t*)&requestedcrop.y) < 2)
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

	read_png(inputfile, &pixels, (uint32_t*)&originalsize.x, (uint32_t*)&originalsize.y);

	/* Initialize GUI */
	init();

	while (1) {
		int tick = SDL_GetTicks();
		SDL_Event event;
		if (tick < ticknext)
			SDL_Delay(ticknext - tick);
		ticknext = tick + (1000 / FPS);

		draw();

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
					draw();
				}
			}
		}
	}
	quit();
	return 0;
}
