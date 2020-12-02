#include <SDL2/SDL.h>
#include <stdio.h>

#include "img.h"
#include "util.h"

// TODO match window size to loaded file
#define WIDTH 800
#define HEIGHT 600
#define FPS 30

enum Mode {ModeGeom, ModeSelect, ModeInit};

static SDL_Window *gWindow = NULL;
static SDL_Renderer *gRenderer = NULL;
static SDL_Texture *gTexture = NULL;
static uint32_t *pixels = NULL;
static Vec2i winsize = {WIDTH, HEIGHT};
static Vec2i winpos = {0, 0};
static Vec2i mouse = {0, 0};
static Vec2i requestedcrop = {1, 1};
static Vec2i croporigin = {0, 0};
static Vec2i originalsize = {0, 0};
static SDL_Rect croprect = {-999, -999, 1, 1};
static SDL_Rect imgrect = {0, 0, 1, 1};
static char *outputfile = NULL;
static enum Mode mode = ModeInit;
static int drawcrop = 1;
static enum ImgFmt infmt;

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

void
getfinalcrop(Vec2i *pos, Vec2i *size)
{
	*size = requestedcrop;

	pos->x = (croprect.x - imgrect.x) * originalsize.x / imgrect.w;
	pos->y = (croprect.y - imgrect.y) * originalsize.y / imgrect.h;

	if (size->x < 0) {
		size->x = -size->x;
		pos->x -= size->x;
	}

	if (size->y < 0) {
		size->y = -size->y;
		pos->y -= size->y;
	}
}

void
mouseevent(SDL_Event *event)
{
	mouse.x = event->motion.x;
	mouse.y = event->motion.y;

	switch (event->type) {
	case SDL_MOUSEBUTTONDOWN:
		if (mode == ModeSelect) {
			drawcrop = 1;
			croporigin.x = mouse.x;
			croporigin.y = mouse.y;
		}
		break;
	case SDL_MOUSEBUTTONUP: {
		Vec2i pos, size;
		getfinalcrop(&pos, &size);
		fprintf(stderr, "%dx%d@+%d+%d\n", size.x, size.y, pos.x, pos.y);
		write_img(outputfile, pixels, originalsize.x, size, pos, infmt);
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
update()
{
	/* Compute image display geometry */
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

	/* Compute displayed crop rectangle */
	switch (mode) {
	case ModeGeom:
		croporigin.x = mouse.x - croprect.w * 0.5;
		croporigin.y = mouse.y - croprect.h * 0.5;
		croprect.w = requestedcrop.x * imgrect.w / originalsize.x;
		croprect.h = requestedcrop.y * imgrect.h / originalsize.y;
		break;
	case ModeSelect:
		croprect.w = mouse.x - croporigin.x;
		croprect.h = mouse.y - croporigin.y;
		requestedcrop.x = croprect.w * originalsize.x / imgrect.w;
		requestedcrop.y = croprect.h * originalsize.y / imgrect.h;
		break;
	default:
		break;
	}
	croprect.x = croporigin.x;
	croprect.y = croporigin.y;
	if (croprect.x < imgrect.x)
		croprect.x = imgrect.x;
	else if (croprect.x + croprect.w > imgrect.x + imgrect.w)
		croprect.x = imgrect.x + imgrect.w - croprect.w;
	if (croprect.y < imgrect.y)
		croprect.y = imgrect.y;
	else if (croprect.y + croprect.h > imgrect.y + imgrect.h)
		croprect.y = imgrect.y + imgrect.h - croprect.h;
}

void
draw()
{
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xff);
	SDL_RenderClear(gRenderer);

	/* Draw image background */
	SDL_RenderCopy(gRenderer, gTexture, NULL, &imgrect);

	/* Draw crop rectangle */
	if (drawcrop) {
		SDL_SetRenderDrawColor(gRenderer, 0xff, 0xff, 0xff, 0xff);
		SDL_Rect rect1 = {croprect.x - 1, croprect.y - 1, croprect.w + 2, croprect.h + 2};
		SDL_Rect rect2 = {croprect.x - 2, croprect.y - 2, croprect.w + 4, croprect.h + 4};
		SDL_RenderDrawRect(gRenderer, &rect1);
		SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 0xff);
		SDL_RenderDrawRect(gRenderer, &rect2);
	}

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

	/* Parse arguments */
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-g")) {
			if (argc < i + 4 || sscanf(argv[++i], "%ux%u", (uint32_t*)&requestedcrop.x, (uint32_t*)&requestedcrop.y) < 2)
				usage(argv[0]);
			else if (mode != ModeInit)
				die("Options -g and -s are incompatible");
			mode = ModeGeom;
			drawcrop = 1;
		} else if (!strcmp(argv[i], "-s")) {
			if (mode != ModeInit)
				die("Options -g and -s are incompatible");
			mode = ModeSelect;
			drawcrop = 0;
		// TODO add option to choose an aspect ratio rather than geometry
		} else if (!inputfile) {
			inputfile = argv[i];
		} else if (!outputfile) {
			outputfile = argv[i];
		}
	}

	if (!inputfile || !outputfile)
		usage(argv[0]);

	/* Load image file */
	infmt = read_img(inputfile, &pixels, &originalsize);

	/* Initialize GUI */
	init();

	while (1) {
		int tick = SDL_GetTicks();
		SDL_Event event;
		if (tick < ticknext)
			SDL_Delay(ticknext - tick);
		ticknext = tick + (1000 / FPS);

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
				}
			}
		}

		update();
		draw();
	}

	quit();
	return 0;
}
