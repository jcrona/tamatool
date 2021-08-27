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
#if !defined(__WIN32__)
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <termios.h>

#include "lib/tamalib.h"

#include "mem_edit.h"

static u13_t editor_cursor = 0x0;
static struct termios orig_termios;


void mem_edit_reset_terminal(void)
{
	tcsetattr(0, TCSANOW, &orig_termios);

	/* Clear the console */
	printf("\e[1;1H\e[2J");
}

void mem_edit_configure_terminal(void)
{
	struct termios new_termios;

	/* Backup the terminal configuration */
	tcgetattr(0, &orig_termios);
	memcpy(&new_termios, &orig_termios, sizeof(new_termios));

	/* Register cleanup handler, and set the new terminal mode */
	atexit(mem_edit_reset_terminal);
	cfmakeraw(&new_termios);
	tcsetattr(0, TCSANOW, &new_termios);
}

static int kbhit()
{
	struct timeval tv = { 0L, 0L };

	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(0, &fds);

	return select(1, &fds, NULL, NULL, &tv);
}

static int getch()
{
	int r;
	unsigned char c;

	if ((r = read(0, &c, sizeof(c))) < 0) {
		return r;
	} else {
		return c;
	}
}

static void print_editor_field(char *name, u32_t val, uint8_t depth, u12_t position)
{
	u12_t i;

	printf("\e[1;34m%s:\e[0m 0x", name);
	for (i = 0; i < depth; i++) {
		if (i + MEMORY_SIZE + position == editor_cursor) {
			printf("\e[0;30;42m");
		}

		printf("%X", (val >> (4 * (depth - 1 - i))) & 0xF);

		if (i + MEMORY_SIZE + position == editor_cursor) {
			printf("\e[0m");
		}
	}
}

void mem_edit_update(void)
{
	u12_t i;
	uint8_t key;
	state_t *state = tamalib_get_state();
	int8_t hbyte = -1;

	/* Clear the console */
	printf("\e[1;1H\e[2J");

	/* Memory */
	for (i = 0; i < MEMORY_SIZE; i++) {
		if (!(i % 0x80)) {
			printf("\r\n\e[1;34m0x%03X:\e[0m ", i);
		}

		if (i == editor_cursor) {
			printf("\e[0;30;42m");
		} else if (i < 0x280) {
			/* RAM */
		} else if (i >= 0xE00 && i < 0xE50) {
			/* Display Memory 1 */
			printf("\e[0;35m");
		} else if (i >= 0xE80 && i < 0xED0) {
			/* Display Memory 2 */
			printf("\e[0;36m");
		} else if (i >= 0xF00 && i < 0xF80) {
			/* I/O Memory */
			printf("\e[0;33m");
		} else {
			printf("\e[0;90m");
		}

		printf("%X", state->memory[i]);

		if (i == editor_cursor) {
			printf("\e[0m");
		}
	}

	printf("\r\n");
	printf("\r\n");

	/* Variables */
	print_editor_field("PC", *(state->pc), 4, 0);
	printf("    ");
	print_editor_field("SP", *(state->sp), 2, 4);
	printf("    ");
	print_editor_field("NP", *(state->np), 2, 6);
	printf("    ");
	print_editor_field("X", *(state->x), 3, 8);
	printf("    ");
	print_editor_field("Y", *(state->y), 3, 11);
	printf("    ");
	print_editor_field("A", *(state->a), 1, 14);
	printf("    ");
	print_editor_field("B", *(state->b), 1, 15);
	printf("    ");
	print_editor_field("F", *(state->flags), 1, 16);
	printf("\r\n");

	printf("\r\n");

	/* Cursor position */
	if (editor_cursor < MEMORY_SIZE) {
		printf("\e[1;32mCursor:\e[0m 0x%04X", editor_cursor);
	} else {
		printf("\e[1;32mCursor:\e[0m Variable");
	}

	printf("    [ \e[1;37mRAM\e[0m    \e[1;35mDisplay 1\e[0m    \e[1;36mDisplay 2\e[0m    \e[1;33mI/O\e[0m    \e[1;90mInvalid\e[0m ]\r\n");

	while (kbhit()) {
		key = getch();
		switch (key) {
			/* Arrows */
			case 65:
				if ((editor_cursor >> 7) == 0x20) {
					editor_cursor = MEMORY_SIZE - 0x80;
				} else if ((editor_cursor >> 7) > 0) {
					editor_cursor -= 0x80;
				}
				break;

			case 66:
				if ((editor_cursor >> 7) < 0x1F) {
					editor_cursor += 0x80;
				} else if ((editor_cursor >> 7) == 0x1F) {
					editor_cursor = MEMORY_SIZE;
				}
				break;

			case 67:
				if (editor_cursor >= MEMORY_SIZE) {
					if ((editor_cursor & 0x7F) < 16) {
						editor_cursor++;
					}
				} else if ((editor_cursor & 0x7F) < 0x7F) {
					editor_cursor++;
				}
				break;

			case 68:
				if ((editor_cursor & 0x7F) > 0) {
					editor_cursor--;
				}
				break;

			/* Numbers */
			case 48:
			case 49:
			case 50:
			case 51:
			case 52:
			case 53:
			case 54:
			case 55:
			case 56:
			case 57:
				hbyte = key - 48;
				break;

			/* Lowercase letters */
			case 97:
			case 98:
			case 99:
			case 100:
			case 101:
			case 102:
				hbyte = key - 97 + 10;
				break;
		}

		if (hbyte >= 0) {
			if (editor_cursor < MEMORY_SIZE) {
				/* Memory */
				state->memory[editor_cursor] = hbyte;
			} else {
				/* Variables */
				if ((editor_cursor & 0xFFF) < 4) {
					*(state->pc) = (*(state->pc) & ~(0xF << (4 * (3 - editor_cursor)))) | (hbyte << (4 * (3 - editor_cursor)));
				} else if ((editor_cursor & 0xFFF) < 6) {
					*(state->sp) = (*(state->sp) & ~(0xF << (4 * (1 - editor_cursor + 4)))) | (hbyte << (4 * (1 - editor_cursor + 4)));
				} else if ((editor_cursor & 0xFFF) < 8) {
					*(state->np) = (*(state->np) & ~(0xF << (4 * (1 - editor_cursor + 6)))) | (hbyte << (4 * (1 - editor_cursor + 6)));
				} else if ((editor_cursor & 0xFFF) < 11) {
					*(state->x) = (*(state->x) & ~(0xF << (4 * (2 - editor_cursor + 8)))) | (hbyte << (4 * (2 - editor_cursor + 8)));
				} else if ((editor_cursor & 0xFFF) < 14) {
					*(state->y) = (*(state->y) & ~(0xF << (4 * (2 - editor_cursor + 11)))) | (hbyte << (4 * (2 - editor_cursor + 11)));
				} else if ((editor_cursor & 0xFFF) < 15) {
					*(state->a) = hbyte;
				} else if ((editor_cursor & 0xFFF) < 16) {
					*(state->b) = hbyte;
				} else if ((editor_cursor & 0xFFF) < 17) {
					*(state->flags) = hbyte;
				}
			}

			editor_cursor++;
			hbyte = -1;
		}
	}
}
#else
void mem_edit_reset_terminal(void) {}
void mem_edit_configure_terminal(void) {}
void mem_edit_update(void) {}
#endif
