#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <jpeglib.h>
#include <png.h>

#include "img.h"

static void
png_err(png_struct *pngs, const char *msg)
{
	(void)pngs;
	die("libpng: %s", msg);
}

static void
jpeg_err(j_common_ptr js)
{
	fprintf(stderr, "libjpeg: ");
	(*js->err->output_message)(js);
	exit(1);
}

void
read_png(FILE *fd, uint32_t **pixels, Vec2i *size)
{
	png_struct *pngs;
	png_info *pngi;
	size_t rowlen, r;
	uint8_t **pngrows;

	/* prepare */
	pngs = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, png_err, NULL);
	pngi = png_create_info_struct(pngs);

	if (!pngs || !pngi)
		die("Failed to initialize libpng");

	png_init_io(pngs, fd);
	if (png_get_valid(pngs, pngi, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(pngs);
	}
	png_set_add_alpha(pngs, 0xffff, PNG_FILLER_AFTER);
	png_set_expand_gray_1_2_4_to_8(pngs);
	png_set_gray_to_rgb(pngs);
	png_set_packing(pngs);
	// TODO Support 16 bit color depth
	png_read_png(pngs, pngi, PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_16, NULL);
	size->x = png_get_image_width(pngs, pngi);
	size->y = png_get_image_height(pngs, pngi);
	rowlen = size->x * (sizeof("RGBA") - 1);
	pngrows = png_get_rows(pngs, pngi);

	/* Read data */
	*pixels = ecalloc(size->x * size->y, sizeof(uint32_t));
	for (r = 0; r < (size_t)size->y; ++r)
		memcpy(*pixels + r * size->x, pngrows[r], rowlen);

	/* clean up */
	png_destroy_read_struct(&pngs, &pngi, NULL);
}

static void
write_png(FILE *fd, uint32_t *pixels, uint32_t stride, Vec2i size, Vec2i off)
{
	png_struct *pngs;
	png_info *pngi;
	int i;

	/* Prepare */
	pngs = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, png_err, NULL);
	pngi = png_create_info_struct(pngs);

	if (!pngs || !pngi)
		die("Failed to initialize libpng");

	png_init_io(pngs, fd);
	png_set_IHDR(pngs, pngi, size.x, size.y, 8, PNG_COLOR_TYPE_RGB_ALPHA,
	             PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
	             PNG_FILTER_TYPE_BASE);
	png_write_info(pngs, pngi);

	/* write data */
	for (i = off.y; i < size.y + off.y; ++i)
		png_write_row(pngs, (uint8_t *)(pixels + i * stride + off.x));

	/* clean up */
	png_write_end(pngs, NULL);
	png_destroy_write_struct(&pngs, NULL);
}

void
read_jpg(FILE *fd, uint32_t **pixels, Vec2i *size)
{
	struct jpeg_decompress_struct js;
	struct jpeg_error_mgr jerr;
	uint8_t *row;
	size_t i;

	/* prepare */
	jpeg_create_decompress(&js);
	jerr.error_exit = jpeg_err;
	js.err = jpeg_std_error(&jerr);

	jpeg_stdio_src(&js, fd);
	jpeg_read_header(&js, 1);
	size->x = js.image_width;
	size->y = js.image_height;
	js.output_components = 3;     /* color components per pixel */
	js.out_color_space = JCS_RGB; /* libjpeg output color space */

	jpeg_start_decompress(&js);
	row = ecalloc(size->x, (sizeof("RGB") - 1) * sizeof(uint8_t));
	*pixels = ecalloc(size->x * size->y, sizeof(uint32_t));

	/* write data */
	while (js.output_scanline < js.output_height) {
		jpeg_read_scanlines(&js, &row, 1);

		for (i = 0; i < (size_t)size->x; ++i) {
			/* Don't forget about that little endianness */
			uint32_t px = row[3*i] | row[3*i+1] << 8 | row[3*i+2] << 16 | 0xff000000;
			(*pixels)[(js.output_scanline - 1) * size->x + i] = px;
		}
	}

	/* clean up */
	jpeg_finish_decompress(&js);
	jpeg_destroy_decompress(&js);
}

static void
write_jpg(FILE *fd, uint32_t *pixels, uint32_t stride, Vec2i size, Vec2i off)
{
	struct jpeg_compress_struct jcomp;
	struct jpeg_error_mgr jerr;
	uint64_t a;
	size_t i, j, k, l;
	uint8_t *row;
	// TODO provide interface for those settings
	uint8_t mask[3] = { 0xff, 0xff, 0xff };
	int quality = 85;

	/* prepare */
	jpeg_create_compress(&jcomp);
	jerr.error_exit = jpeg_err;
	jcomp.err = jpeg_std_error(&jerr);

	jpeg_stdio_dest(&jcomp, fd);
	jcomp.image_width = size.x;
	jcomp.image_height = size.y;
	jcomp.input_components = 3;     /* color components per pixel */
	jcomp.in_color_space = JCS_RGB; /* output color space */
	jcomp.optimize_coding = 1;      /* Optimize the Huffman table: smaller but slower */
	jpeg_set_defaults(&jcomp);

	jpeg_set_quality(&jcomp, quality, 1);
	jpeg_start_compress(&jcomp, 1);

	row = ecalloc(size.x, (sizeof("RGB") - 1) * sizeof(uint8_t));

	/* write data */
	for (i = off.y; i < (size_t)(size.y + off.y); ++i) {
		for (j = 0, k = 0; j < (size_t)size.x; j++, k += 3) {
			uint8_t *px = (uint8_t*)(pixels + i * stride + off.x + j);
			a = px[3];
			for (l = 0; l < 3; l++)  /* alpha blending */
				row[k + l] = (a * px[l] + (0xff - a) * mask[l]) / 0xff;
		}
		jpeg_write_scanlines(&jcomp, &row, 1);
	}

	/* clean up */
	jpeg_finish_compress(&jcomp);
	jpeg_destroy_compress(&jcomp);
}

enum ImgFmt
read_img(const char *file, uint32_t **pixels, Vec2i *size)
{
	uint8_t header[8];
	uint8_t jpgsig[] = {0xFF, 0xD8};
	uint8_t pngsig[] = {0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n'};
	int i;
	FILE *f;
	enum ImgFmt infmt = 0;

	if (!strcmp("-", file))
		f = stdin;
	else if (!(f = fopen(file, "rb")))
		die("%s could not be read", file);

	efread(header, sizeof(header[0]), LENGTH(header), f);
	/* Put those bytes back where we found them */
	for (i = LENGTH(header) - 1; i >= 0; i--)
		if (ungetc(header[i], f) == EOF)
			die("Failed to put the genie back in the box: ungetc error");

	if (!memcmp(jpgsig, header, sizeof(jpgsig))) {
		read_jpg(f, pixels, size);
		infmt = IMG_JPG;
	} else if (!memcmp(pngsig, header, sizeof(pngsig))) {
		read_png(f, pixels, size);
		infmt = IMG_PNG;
	} else {
		die("%s: unsupported file type", file);
	}

	fclose(f);
	return infmt;
}

void
write_img(const char *file, uint32_t *pixels, uint32_t stride, Vec2i size, Vec2i off, enum ImgFmt infmt)
{
	enum ImgFmt outfmt = IMG_PNG;
	size_t l = strlen(file) + 1;
	FILE *f;

	if (!strcmp(".png", file + l - sizeof(".png")))
		outfmt = IMG_PNG;
	else if (!strcmp(".jpeg", file + l - sizeof(".jpeg")) ||
			!strcmp(".jpg", file + l - sizeof(".jpg")))
		outfmt = IMG_JPG;
	else if (!strcmp("-", file))  /* For stdout we use the input format */
		outfmt = infmt;
	else
		fprintf(stderr, "Couldn't infer file type of %s: defaulting to png\n", file);

	if (!strcmp("-", file)) {
		f = stdout;
	} else if (!(f = fopen(file, "wb"))) {
		die("%s could not be created", file);
	}

	switch (outfmt) {
	case IMG_PNG:
		write_png(f, pixels, stride, size, off);
		break;
	case IMG_JPG:
		write_jpg(f, pixels, stride, size, off);
		break;
	}

	fclose(f);
}

void
print_img(uint32_t *pixels, uint32_t w, uint32_t h, uint32_t maxw)
{
	const char c_int[] = {' ', '.', '-', '*', 'x', 'X', '0'};
	for (uint32_t y = 0; y < h; y++) {
		for (uint32_t x = 0; x < w; x++) {
			uint8_t r, g, b;
			r = pixels[y * w + x] >> 24 & 0xff;
			g = pixels[y * w + x] >> 16 & 0xff;
			b = pixels[y * w + x] >> 8 & 0xff;
			int intensity = (r + g + b) / 128;
			if (x < maxw)
				printf("%c", c_int[intensity]);
		}
		printf("\n");
	}
}
