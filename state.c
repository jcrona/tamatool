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

#include "lib/tamalib.h"

#include "state.h"


static uint32_t find_next_slot(void)
{
	char path[256];
	uint32_t i = 0;

	for (i = 0;; i++) {
		sprintf(path, STATE_TEMPLATE, i);
		if (!fopen(path, "r")) {
			break;
		}
	}

	return i;
}

void state_find_next_name(char *path)
{
	sprintf(path, STATE_TEMPLATE, find_next_slot());
}

void state_find_last_name(char *path)
{
	uint32_t num = find_next_slot();

	if (num > 0) {
		sprintf(path, STATE_TEMPLATE, num - 1);
	} else {
		path[0] = '\0';
	}
}

void state_save(char *path)
{
	FILE *f;
	state_t *state;
	uint8_t buf[4];
	uint32_t num = 0;
	uint32_t i;

	state = tamalib_get_state();

	f = fopen(path, "w");
	if (f == NULL) {
		fprintf(stderr, "FATAL: Cannot create state file \"%s\" !\n", path);
		return;
	}

	/* All fields are written as u8, u16 little-endian or u32 little-endian following the struct order */
	buf[0] = *(state->pc) & 0xFF;
	buf[1] = (*(state->pc) >> 8) & 0x1F;
	num += fwrite(buf, 2, 1, f);

	buf[0] = *(state->x) & 0xFF;
	buf[1] = (*(state->x) >> 8) & 0xF;
	num += fwrite(buf, 2, 1, f);

	buf[0] = *(state->y) & 0xFF;
	buf[1] = (*(state->y) >> 8) & 0xF;
	num += fwrite(buf, 2, 1, f);

	buf[0] = *(state->a) & 0xF;
	num += fwrite(buf, 1, 1, f);

	buf[0] = *(state->b) & 0xF;
	num += fwrite(buf, 1, 1, f);

	buf[0] = *(state->np) & 0x1F;
	num += fwrite(buf, 1, 1, f);

	buf[0] = *(state->sp) & 0xFF;
	num += fwrite(buf, 1, 1, f);

	buf[0] = *(state->flags) & 0xF;
	num += fwrite(buf, 1, 1, f);

	buf[0] = *(state->clk_timer_timestamp) & 0xFF;
	buf[1] = (*(state->clk_timer_timestamp) >> 8) & 0xFF;
	buf[2] = (*(state->clk_timer_timestamp) >> 16) & 0xFF;
	buf[3] = (*(state->clk_timer_timestamp) >> 24) & 0xFF;
	num += fwrite(buf, 4, 1, f);

	buf[0] = *(state->prog_timer_timestamp) & 0xFF;
	buf[1] = (*(state->prog_timer_timestamp) >> 8) & 0xFF;
	buf[2] = (*(state->prog_timer_timestamp) >> 16) & 0xFF;
	buf[3] = (*(state->prog_timer_timestamp) >> 24) & 0xFF;
	num += fwrite(buf, 4, 1, f);

	buf[0] = *(state->prog_timer_enabled) & 0x1;
	num += fwrite(buf, 1, 1, f);

	buf[0] = *(state->prog_timer_data) & 0xFF;
	num += fwrite(buf, 1, 1, f);

	buf[0] = *(state->prog_timer_rld) & 0xFF;
	num += fwrite(buf, 1, 1, f);

	buf[0] = *(state->call_depth) & 0xFF;
	buf[1] = (*(state->call_depth) >> 8) & 0xFF;
	buf[2] = (*(state->call_depth) >> 16) & 0xFF;
	buf[3] = (*(state->call_depth) >> 24) & 0xFF;
	num += fwrite(buf, 4, 1, f);

	for (i = 0; i < INT_SLOT_NUM; i++) {
		buf[0] = state->interrupts[i].factor_flag_reg & 0xF;
		num += fwrite(buf, 1, 1, f);

		buf[0] = state->interrupts[i].mask_reg & 0xF;
		num += fwrite(buf, 1, 1, f);

		buf[0] = state->interrupts[i].triggered & 0x1;
		num += fwrite(buf, 1, 1, f);
	}

	for (i = 0; i < MEMORY_SIZE; i++) {
		buf[0] = state->memory[i] & 0xF;
		num += fwrite(buf, 1, 1, f);
	}

	if (num != (14 + INT_SLOT_NUM * 3 + MEMORY_SIZE)) {
		fprintf(stderr, "FATAL: Failed to write to state file \"%s\" !\n", path);
	}

	fclose(f);
}

void state_load(char *path)
{
	FILE *f;
	state_t *state;
	uint8_t buf[4];
	uint32_t num = 0;
	uint32_t i;

	state = tamalib_get_state();

	f = fopen(path, "r");
	if (f == NULL) {
		fprintf(stderr, "FATAL: Cannot open state file \"%s\" !\n", path);
		return;
	}

	/* All fields are read as u8, u16 little-endian or u32 little-endian following the struct order */
	num += fread(buf, 2, 1, f);
	*(state->pc) = buf[0] | ((buf[1] & 0x1F) << 8);

	num += fread(buf, 2, 1, f);
	*(state->x) = buf[0] | ((buf[1] & 0xF) << 8);

	num += fread(buf, 2, 1, f);
	*(state->y) = buf[0] | ((buf[1] & 0xF) << 8);

	num += fread(buf, 1, 1, f);
	*(state->a) = buf[0] & 0xF;

	num += fread(buf, 1, 1, f);
	*(state->b) = buf[0] & 0xF;

	num += fread(buf, 1, 1, f);
	*(state->np) = buf[0] & 0x1F;

	num += fread(buf, 1, 1, f);
	*(state->sp) = buf[0];

	num += fread(buf, 1, 1, f);
	*(state->flags) = buf[0] & 0xF;

	num += fread(buf, 4, 1, f);
	*(state->clk_timer_timestamp) = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);

	num += fread(buf, 4, 1, f);
	*(state->prog_timer_timestamp) = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);

	num += fread(buf, 1, 1, f);
	*(state->prog_timer_enabled) = buf[0] & 0x1;

	num += fread(buf, 1, 1, f);
	*(state->prog_timer_data) = buf[0];

	num += fread(buf, 1, 1, f);
	*(state->prog_timer_rld) = buf[0];

	num += fread(buf, 4, 1, f);
	*(state->call_depth) = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);

	for (i = 0; i < INT_SLOT_NUM; i++) {
		num += fread(buf, 1, 1, f);
		state->interrupts[i].factor_flag_reg = buf[0] & 0xF;

		num += fread(buf, 1, 1, f);
		state->interrupts[i].mask_reg = buf[0] & 0xF;

		num += fread(buf, 1, 1, f);
		state->interrupts[i].triggered = buf[0] & 0x1;
	}

	for (i = 0; i < MEMORY_SIZE; i++) {
		num += fread(buf, 1, 1, f);
		state->memory[i] = buf[0] & 0xF;
	}

	if (num != (14 + INT_SLOT_NUM * 3 + MEMORY_SIZE)) {
		fprintf(stderr, "FATAL: Failed to read from state file \"%s\" !\n", path);
	}

	fclose(f);
}
