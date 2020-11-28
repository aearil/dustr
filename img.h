#pragma once

#include <png.h>
#include <stdint.h>

#include "util.h"

void read_png(const char *file, uint32_t **pixels, uint32_t *width, uint32_t *height);
void write_png(const char *file, uint32_t *pixels, uint32_t stride, Vec2i size, Vec2i off);
void print_img(uint32_t *pixels, uint32_t w, uint32_t h, uint32_t maxw);
