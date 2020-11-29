#pragma once

#include <stdint.h>
#include <stdio.h>

#include "util.h"

void read_png(FILE *fd, uint32_t **pixels, Vec2i *size);
void read_jpg(FILE *fd, uint32_t **pixels, Vec2i *size);
void read_img(const char *file, uint32_t **pixels, Vec2i *size);

void write_png(const char *file, uint32_t *pixels, uint32_t stride, Vec2i size, Vec2i off);
void write_jpg(const char *file, uint32_t *pixels, uint32_t stride, Vec2i size, Vec2i off);

void print_img(uint32_t *pixels, uint32_t w, uint32_t h, uint32_t maxw);
