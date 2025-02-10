/*
 * TamaTool - A Cross-Platform Explorer for First-Gen Tamagotchi
 *
 * Copyright (C) 2021 Jean-Christophe Rona <jc@rona.fr>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <png.h>

#include "SDL.h"

#include "image.h"


void image_alloc(image_t *image)
{
	unsigned int y;

	image->row_pointers = (png_bytepp) SDL_calloc(sizeof(png_bytep) * image->height, 1);
	for (y = 0; y < image->height; y++) {
		image->row_pointers[y] = (png_byte*) SDL_calloc(image->stride, 1);
	}
}

void image_free(image_t *image)
{
	unsigned int y;

	for (y = 0; y < image->height; y++) {
		SDL_free(image->row_pointers[y]);
	}
	SDL_free(image->row_pointers);
}

void image_read_file(char* file_name, image_t *image)
{
	png_byte header[8];
	png_structp png_ptr;
	png_infop info_ptr;

	FILE *fp = fopen(file_name, "rb");
	if (!fp) {
		fprintf(stderr, "[read_png_file] File %s could not be opened for reading", file_name);
		return;
	}

	fread(header, 1, 8, fp);
	if (png_sig_cmp(header, 0, 8)) {
		fprintf(stderr, "[read_png_file] File %s is not recognized as a PNG file", file_name);
		fclose(fp);
		return;
	}

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr) {
		fprintf(stderr, "[read_png_file] png_create_read_struct failed");
		fclose(fp);
		return;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		fprintf(stderr, "[read_png_file] png_create_info_struct failed");
		fclose(fp);
		return;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "[read_png_file] Error during init_io");
		fclose(fp);
		return;
	}

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	image->width = png_get_image_width(png_ptr, info_ptr);
	image->height = png_get_image_height(png_ptr, info_ptr);
	image->color_type = png_get_color_type(png_ptr, info_ptr);
	image->bit_depth = png_get_bit_depth(png_ptr, info_ptr);
	image->stride = png_get_rowbytes(png_ptr,info_ptr);

	png_read_update_info(png_ptr, info_ptr);

	/* read file */
	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "[read_png_file] Error during read_image");
		fclose(fp);
		return;
	}

	image_alloc(image);

	png_read_image(png_ptr, image->row_pointers);

	fclose(fp);
}

void image_write_file(char* file_name, image_t *image)
{
	png_structp png_ptr;
	png_infop info_ptr;

	FILE *fp = fopen(file_name, "wb");
	if (!fp) {
		fprintf(stderr, "[write_png_file] File %s could not be opened for writing", file_name);
		return;
	}

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr) {
		fprintf(stderr, "[write_png_file] png_create_write_struct failed");
		fclose(fp);
		return;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		fprintf(stderr, "[write_png_file] png_create_info_struct failed");
		fclose(fp);
		return;
	}

	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "[write_png_file] Error during init_io");
		fclose(fp);
		return;
	}

	png_init_io(png_ptr, fp);

	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "[write_png_file] Error during writing header");
		fclose(fp);
		return;
	}

	png_set_IHDR(png_ptr, info_ptr, image->width, image->height,
		image->bit_depth, image->color_type, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	png_write_info(png_ptr, info_ptr);

	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "[write_png_file] Error during writing bytes");
		fclose(fp);
		return;
	}

	png_write_image(png_ptr, image->row_pointers);

	if (setjmp(png_jmpbuf(png_ptr))) {
		fprintf(stderr, "[write_png_file] Error during end of write");
		fclose(fp);
		return;
	}

	png_write_end(png_ptr, NULL);

	fclose(fp);
}
