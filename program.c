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
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "program.h"
#include "image.h"

typedef struct map {
	uint32_t ref;
	uint32_t width;
	uint32_t height;
} map_t;

static map_t g_map[MAX_SPRITES];


u12_t * program_load(char *path, uint32_t *size)
{
	FILE *f;
	uint32_t i;
	uint8_t buf[2];
	u12_t *program;

	f = fopen(path, "r");
	if (f == NULL) {
		fprintf(stderr, "FATAL: Cannot open ROM \"%s\" !\n", path);
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	*size = ftell(f)/2;
	fseek(f, 0, SEEK_SET);

	//fprintf(stdout, "ROM size is %u * 12bits\n", *size);

	program = (u12_t *) malloc(*size * sizeof(u12_t));
	if (program == NULL) {
		fprintf(stderr, "FATAL: Cannot allocate ROM memory !\n");
		fclose(f);
		return NULL;
	}

	for (i = 0; i < *size; i++) {
		if (fread(buf, 2, 1, f) != 1) {
			fprintf(stderr, "FATAL: Cannot read program from ROM !\n");
			free(program);
			fclose(f);
			return NULL;
		}

		program[i] = buf[1] | ((buf[0] & 0xF) << 8);
	}

	fclose(f);
	return program;
}

void program_save(char *path, u12_t *program, uint32_t size)
{
	FILE *f;
	uint32_t i;
	uint8_t buf[2];

	f = fopen(path, "w");
	if (f == NULL) {
		fprintf(stderr, "FATAL: Cannot open ROM \"%s\" !\n", path);
		return;
	}

	for (i = 0; i < size; i++) {
		buf[0] = (program[i] >> 8) & 0xF;
		buf[1] = program[i] & 0xFF;

		if (fwrite(buf, 2, 1, f) != 1) {
			fprintf(stderr, "FATAL: Cannot write program from ROM !\n");
			fclose(f);
			return;
		}
	}

	fclose(f);
}

void program_to_header(u12_t *program, uint32_t size)
{
	uint32_t i;

	fprintf(stdout, "static u12_t program[] = {");

	for (i = 0; i < size; i++) {
		if (!(i % 16)) {
			fprintf(stdout, "\n\t");
		} else {
			fprintf(stdout, " ");
		}

		fprintf(stdout, "0x%03X,", program[i]);
	}

	fprintf(stdout, "\n};\n");
}

static uint32_t generate_data_map(map_t *map, u12_t *program, uint32_t size)
{
	uint32_t i, j, k;
	uint32_t count = 0;
	uint32_t width = 0;

	/* Parse the program to get a map */
	for (i = 0; i < size; i++) {
		if ((program[i] >> 8) == 0x9) {
			/* LBPX */
			if (width == 0) {
			}

			width++;
		} else {
			/* RETD */
			if ((program[i] >> 8) == 0x1 && width != 0) {
				map[count].ref = i - width;
				map[count].width = width + 1;
				map[count].height = 8;

				for (k = 0; k < map[count].height; k++) {
					for (j = 0; j < map[count].width; j++) {
					}
				}

				count++;
			}

			width = 0;
		}
	}

	return count;
}

void program_get_data(u12_t *program, uint32_t size, char *path)
{
	uint32_t i, j, k;
	uint8_t depth;
	uint32_t sprite_num = 0;
	image_t img;

	sprite_num = generate_data_map(g_map, program, size);

	/* Create an image file from the map */
	img.width = 18;
	img.height = sprite_num * 10;
	img.color_type = PNG_COLOR_TYPE_RGBA;
	img.bit_depth = 8;
	depth = img.bit_depth/8 * 4;
	img.stride = 18 * depth;

	image_alloc(&img);

	for (i = 0; i < sprite_num; i++) {
		/* Bounds */
		for (k = 0; k < g_map[i].height + 2; k++) {
			img.row_pointers[i * 10 + k][0] = 0xFF;
			img.row_pointers[i * 10 + k][1] = 0x00;
			img.row_pointers[i * 10 + k][2] = 0x00;
			img.row_pointers[i * 10 + k][3] = 0xFF;

			img.row_pointers[i * 10 + k][depth * (g_map[i].width + 1)] = 0xFF;
			img.row_pointers[i * 10 + k][depth * (g_map[i].width + 1) + 1] = 0x00;
			img.row_pointers[i * 10 + k][depth * (g_map[i].width + 1) + 2] = 0x00;
			img.row_pointers[i * 10 + k][depth * (g_map[i].width + 1) + 3] = 0xFF;
		}
		for (j = 0; j < g_map[i].width + 2; j++) {
			img.row_pointers[i * 10][depth * j] = 0xFF;
			img.row_pointers[i * 10][depth * j + 1] = 0x00;
			img.row_pointers[i * 10][depth * j + 2] = 0x00;
			img.row_pointers[i * 10][depth * j + 3] = 0xFF;

			img.row_pointers[i * 10 + (g_map[i].height + 1)][depth * j] = 0xFF;
			img.row_pointers[i * 10 + (g_map[i].height + 1)][depth * j + 1] = 0x00;
			img.row_pointers[i * 10 + (g_map[i].height + 1)][depth * j + 2] = 0x00;
			img.row_pointers[i * 10 + (g_map[i].height + 1)][depth * j + 3] = 0xFF;
		}

		/* Sprite */
		for (k = 0; k < g_map[i].height; k++) {
			for (j = 0; j < g_map[i].width; j++) {
				if ((program[g_map[i].ref + j] >> k) & 0x1) {
					img.row_pointers[i * 10 + k + 1][depth * (j + 1)] = 0x00;
					img.row_pointers[i * 10 + k + 1][depth * (j + 1) + 1] = 0x00;
					img.row_pointers[i * 10 + k + 1][depth * (j + 1) + 2] = 0x00;
					img.row_pointers[i * 10 + k + 1][depth * (j + 1) + 3] = 0xFF;
				} else {
					img.row_pointers[i * 10 + k + 1][depth * (j + 1)] = 0x00;
					img.row_pointers[i * 10 + k + 1][depth * (j + 1) + 1] = 0x00;
					img.row_pointers[i * 10 + k + 1][depth * (j + 1) + 2] = 0x00;
					img.row_pointers[i * 10 + k + 1][depth * (j + 1) + 3] = 0x00;
				}
			}
		}
	}

	printf("Writing %u sprites to file %s ...\n", sprite_num, path);
	image_write_file(path, &img);

	image_free(&img);
}

void program_set_data(u12_t *program, uint32_t size, char *path)
{
	uint32_t i, j, k;
	uint8_t depth;
	uint32_t sprite_num = 0;
	image_t img;

	image_read_file(path, &img);
	printf("Reading %u sprites from file %s ...\n", img.height/10, path);

	sprite_num = generate_data_map(g_map, program, size);

	if (sprite_num != img.height/10)  {
		fprintf(stderr, "FATAL: Invalid number of sprites (%u != %u) !\n", img.height/10, sprite_num);
		return;
	}

	depth = img.bit_depth/8 * 4;

	for (i = 0; i < sprite_num; i++) {
		/* Sprite */
		for (k = 0; k < g_map[i].height; k++) {
			for (j = 0; j < g_map[i].width; j++) {
				program[g_map[i].ref + j] &= ~(0x1 << k);

				/* Use alpha channel only */
				if (img.row_pointers[i * 10 + k + 1][depth * (j + 1) + 3] != 0x00) {
					program[g_map[i].ref + j] |= 0x1 << k;
				}
			}
		}
	}

	image_free(&img);
}
