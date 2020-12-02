#pragma once

#include <stdint.h>
#include <stdio.h>

#include "util.h"

enum ImgFmt {IMG_PNG, IMG_JPG};

enum ImgFmt read_img(const char *file, uint32_t **pixels, Vec2i *size);
void write_img(const char *file, uint32_t *pixels, uint32_t stride, Vec2i size, Vec2i off, enum ImgFmt infmt);

void print_img(uint32_t *pixels, uint32_t w, uint32_t h, uint32_t maxw);
