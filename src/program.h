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
#ifndef _PROGRAM_H_
#define _PROGRAM_H_

#include "hal_types.h"

#define MAX_SPRITES			256

typedef enum {
	ROM_TYPE_P1 = 0,
	ROM_TYPE_P2 = 1,
	ROM_TYPE_MAX
} rom_type_t;


u12_t * program_load(char *path, uint32_t *size);
void program_save(char *path, u12_t *program, uint32_t size);
void program_to_header(u12_t *program, uint32_t size);
void program_get_data(u12_t *program, uint32_t size, char *path);
void program_set_data(u12_t *program, uint32_t size, char *path);
rom_type_t program_detect_type(u12_t *program);
char * program_detect_type_str(u12_t *program);
char * program_validate_type_str(char *type);

#endif /* _PROGRAM_H_ */
