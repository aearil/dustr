#include <png.h>
#include <stdint.h>
#include <string.h>

#include "util.h"

static void
png_err(png_struct *pngs, const char *msg)
{
	(void)pngs;
	die("libpng: %s", msg);
}

static void
png_setup_reader(FILE *fd, png_struct **s, png_info **i, uint32_t *w, uint32_t *h)
{
	*s = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, png_err, NULL);
	*i = png_create_info_struct(*s);

	if (!*s || !*i)
		die("Failed to initialize libpng");

	png_init_io(*s, fd);
	if (png_get_valid(*s, *i, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(*s);
	}
	png_set_add_alpha(*s, 0xffff, PNG_FILLER_AFTER);
	png_set_expand_gray_1_2_4_to_8(*s);
	png_set_gray_to_rgb(*s);
	png_set_packing(*s);
	// TODO Support 16 bit color depth
	png_read_png(*s, *i, PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND | PNG_TRANSFORM_STRIP_16, NULL);
	*w = png_get_image_width(*s, *i);
	*h = png_get_image_height(*s, *i);
}

void
read_png(const char *file, uint32_t **pixels, uint32_t *width, uint32_t *height)
{
	png_struct *pngs;
	png_info *pngi;
	uint32_t rowlen, r;
	uint8_t **pngrows;
	FILE *f;

	if (!(f = fopen(file, "rb")))
		die("%s could not be read", file);

	/* prepare */
	png_setup_reader(f, &pngs, &pngi, width, height);
	rowlen = *width * (sizeof("RGBA") - 1);
	pngrows = png_get_rows(pngs, pngi);

	*pixels = ecalloc(*width * *height, sizeof(uint32_t));
	for (r = 0; r < *height; ++r)
		memcpy(*pixels + r * *width, pngrows[r], rowlen);

	/* clean up */
	png_destroy_read_struct(&pngs, &pngi, NULL);
	fclose(f);
}

static void
png_setup_writer(FILE *fd, png_struct **s, png_info **i, uint32_t w, uint32_t h)
{
	*s = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, png_err, NULL);
	*i = png_create_info_struct(*s);

	if (!*s || !*i)
		die("Failed to initialize libpng");

	png_init_io(*s, fd);
	png_set_IHDR(*s, *i, w, h, 8, PNG_COLOR_TYPE_RGB_ALPHA,
	             PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE,
	             PNG_FILTER_TYPE_BASE);
	png_write_info(*s, *i);
}

void
write_png(const char *file, uint32_t *pixels, uint32_t stride, uint32_t width, uint32_t height, uint32_t offx, uint32_t offy)
{
	png_struct *pngs;
	png_info *pngi;
	uint32_t i;
	FILE *f;

	if (!(f = fopen(file, "wb")))
		die("%s could not be created", file);

	png_setup_writer(f, &pngs, &pngi, width, height);

	/* write data */
	for (i = offy; i < height + offy; ++i)
		png_write_row(pngs, (uint8_t *)(pixels + i * stride + offx));

	/* clean up */
	png_write_end(pngs, NULL);
	png_destroy_write_struct(&pngs, NULL);
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
