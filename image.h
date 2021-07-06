/*
 * TamaTool - A cross-platform Tamagotchi P1 explorer
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
#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <png.h>

typedef struct {
	int width;
	int height;
	int stride;
	png_byte color_type;
	png_byte bit_depth;

	png_bytepp row_pointers;
} image_t;


void image_alloc(image_t *image);
void image_free(image_t *image);
void image_read_file(char* file_name, image_t *image);
void image_write_file(char* file_name, image_t *image);

#endif /* _IMAGE_H_ */
