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
#include <stdarg.h>
#include <string.h>
#include <getopt.h>
#if !defined(__WIN32__)
#include <time.h>
#endif

#if defined(__WIN32__)
#include <windows.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#endif

#include "SDL.h"
#include "SDL_image.h"

#include "lib/tamalib.h"

#include "program.h"
#include "state.h"
#include "mem_edit.h"

#define APP_NAME			"TamaTool"
#define APP_VERSION			"0.1" // Major, minor
#define COPYRIGHT_DATE			"2021"
#define AUTHOR_NAME			"Jean-Christophe Rona"

#define ROM_PATH			"rom.bin"

#define ROM_NOT_FOUND_TITLE		"Tamagotchi ROM not found"
#define ROM_NOT_FOUND_MSG		"You need to place a Tamagotchi P1 ROM called \"rom.bin\" inside TamaTool's folder/package first !"

#define DEFAULT_WINDOW_SIZE		321

#define DEFAULT_LCD_OFFSET_X		1
#define DEFAULT_LCD_OFFSET_Y		82

#define ICON_NUM			8
#define ICON_SRC_SIZE			64
#define DEFAULT_ICON_DEST_SIZE		64
#define DEFAULT_ICON_OFFSET_X		25
#define DEFAULT_ICON_OFFSET_Y		14
#define DEFAULT_ICON_STRIDE_X		71
#define DEFAULT_ICON_STRIDE_Y		242

#define DEFAULT_PIXEL_STRIDE		10
#define DEFAULT_PIXEL_PADDING		1

#define DEFAULT_LCD_ALPHA_ON		255
#define DEFAULT_LCD_ALPHA_OFF		20

#define PIXEL_STRIDE_MIN		1
#define PIXEL_STRIDE_MAX		30

#define RES_PATH			"./res"
#define BACKGROUND_PATH			RES_PATH"/background.png"
#define ICONS_PATH			RES_PATH"/icons.png"

#define AUDIO_FREQUENCY			48000
#define AUDIO_SAMPLES			480 // 10 ms @ 48000 Hz
#define AUDIO_VOLUME			0.2f

typedef enum {
	SPEED_UNLIMITED = 0,
	SPEED_1X = 1,
	SPEED_10X = 10,
} emulation_speed_t;

static breakpoint_t *g_breakpoints = NULL;

static u12_t *g_program = NULL;		// The actual program that is executed
static uint32_t g_program_size = 0;

static bool_t memory_editor_enable = 0;

static SDL_Window *window = NULL;
static SDL_Renderer* renderer = NULL;

static SDL_Texture *bg;
static SDL_Texture *icons;
static SDL_Rect bg_rect;

static SDL_AudioSpec audio_spec;
static SDL_AudioDeviceID audio_dev;
static u32_t current_freq = 0; // in dHz
static unsigned int sin_pos = 0;
static bool_t is_audio_playing = 0;

static bool_t matrix_buffer[LCD_HEIGTH][LCD_WIDTH] = {{0}};
static bool_t icon_buffer[ICON_NUM] = {0};

static u8_t log_levels = LOG_ERROR | LOG_INFO;

static emulation_speed_t speed = SPEED_1X;

static timestamp_t mem_dump_ts = 0;

static uint16_t pixel_stride = DEFAULT_PIXEL_STRIDE;
static uint16_t window_size, lcd_offset_x, lcd_offset_y, icon_dest_size, icon_offset_x, icon_offset_y, icon_stride_x, icon_stride_y, pixel_size;
static uint16_t pixel_alpha_on, pixel_alpha_off, icon_alpha_on, icon_alpha_off;

#if defined(__WIN32__)
static LARGE_INTEGER counter_freq;
#endif

static void sdl_release(void);
static bool_t sdl_init(void);


static void * hal_malloc(u32_t size)
{
	return SDL_malloc(size);
}

static void hal_free(void *ptr)
{
	SDL_free(ptr);
}

static void hal_halt(void)
{
	exit(EXIT_SUCCESS);
}

static bool_t hal_is_log_enabled(log_level_t level)
{
	return !!(log_levels & level);
}

static void hal_log(log_level_t level, char *buff, ...)
{
	va_list arglist;

	if (!(log_levels & level)) {
		return;
	}

	va_start(arglist, buff);

	vfprintf((level == LOG_ERROR) ? stderr : stdout, buff, arglist);

	va_end(arglist);
}

static timestamp_t hal_get_timestamp(void)
{
#if defined(__WIN32__)
	LARGE_INTEGER count;

	QueryPerformanceCounter(&count);
	return (count.QuadPart * 1000000)/counter_freq.QuadPart;
#else
	struct timespec time;

	clock_gettime(CLOCK_REALTIME, &time);
	return (time.tv_sec * 1000000 + time.tv_nsec/1000);
#endif
}

static void hal_usleep(timestamp_t us)
{
	timestamp_t start = hal_get_timestamp();

	/* Since very high accuracy is required here, nanosleep() is not an option
	 * TODO: find a way to actually sleep
	 */
	while (hal_get_timestamp() - start < us);
}

static void hal_update_screen(void)
{
	unsigned int i, j;
	SDL_Rect r, src_icon_r, dest_icon_r;

	SDL_RenderCopy(renderer, bg, NULL, &bg_rect);

	/* Dot matrix */
	for (j = 0; j < LCD_HEIGTH; j++) {
		for (i = 0; i < LCD_WIDTH; i++) {
			r.w = pixel_size;
			r.h = pixel_size;
			r.x = i * pixel_stride + lcd_offset_x;
			r.y = j * pixel_stride + lcd_offset_y;

			if (matrix_buffer[j][i]) {
				SDL_SetRenderDrawColor(renderer, 0, 0, 128, pixel_alpha_on);
			} else {
				SDL_SetRenderDrawColor(renderer, 0, 0, 128, pixel_alpha_off);
			}

			SDL_RenderFillRect(renderer, &r);
		}
	}

	/* Icons */
	for (i = 0; i < ICON_NUM; i++) {
		src_icon_r.w = ICON_SRC_SIZE;
		src_icon_r.h = ICON_SRC_SIZE;
		src_icon_r.x = (i % 4) * ICON_SRC_SIZE;
		src_icon_r.y = (i / 4) * ICON_SRC_SIZE;

		dest_icon_r.w = icon_dest_size;
		dest_icon_r.h = icon_dest_size;
		dest_icon_r.x = (i % 4) * icon_stride_x + icon_offset_x;
		dest_icon_r.y = (i / 4) * icon_stride_y + icon_offset_y;


		SDL_SetTextureColorMod(icons, 0, 0, 128);
		if (icon_buffer[i]) {
			SDL_SetTextureAlphaMod(icons, icon_alpha_on);
		} else {
			SDL_SetTextureAlphaMod(icons, icon_alpha_off);
		}

		SDL_RenderCopy(renderer, icons, &src_icon_r, &dest_icon_r);
	}

	SDL_RenderPresent(renderer);
}

static void hal_set_lcd_matrix(u8_t x, u8_t y, bool_t val)
{
	matrix_buffer[y][x] = val;
}

static void hal_set_lcd_icon(u8_t icon, bool_t val)
{
	icon_buffer[icon] = val;
}

static void hal_set_frequency(u32_t freq)
{
	if (current_freq != freq) {
		current_freq = freq;
		sin_pos = 0;
	}
}

static void hal_play_frequency(bool_t en)
{
	if (is_audio_playing != en) {
		is_audio_playing = en;
	}
}

static void compute_layout(uint8_t stride)
{
	pixel_stride = stride;
	pixel_size = pixel_stride - pixel_stride/10;

	window_size = pixel_stride * (LCD_WIDTH + 1) - pixel_size;
	lcd_offset_x = pixel_stride - pixel_size;
	lcd_offset_y = (window_size * DEFAULT_LCD_OFFSET_Y)/DEFAULT_WINDOW_SIZE;
	icon_dest_size = (window_size * DEFAULT_ICON_DEST_SIZE)/DEFAULT_WINDOW_SIZE;
	icon_offset_x = (window_size * DEFAULT_ICON_OFFSET_X)/DEFAULT_WINDOW_SIZE;
	icon_offset_y = (window_size * DEFAULT_ICON_OFFSET_Y)/DEFAULT_WINDOW_SIZE;
	icon_stride_x = (window_size * DEFAULT_ICON_STRIDE_X)/DEFAULT_WINDOW_SIZE;
	icon_stride_y = (window_size * DEFAULT_ICON_STRIDE_Y)/DEFAULT_WINDOW_SIZE;

	pixel_alpha_on = DEFAULT_LCD_ALPHA_ON;
	pixel_alpha_off = (pixel_size != pixel_stride) ? DEFAULT_LCD_ALPHA_OFF : 0;
	icon_alpha_on = DEFAULT_LCD_ALPHA_ON;
	icon_alpha_off = DEFAULT_LCD_ALPHA_OFF;
}

static int handle_sdl_events(SDL_Event *event)
{
	char save_path[256];

	switch(event->type) {
		case SDL_QUIT:
			return 1;

		case SDL_WINDOWEVENT:
			switch (event->window.event) {
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					break;
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			switch (event->button.button) {
				case SDL_BUTTON_LEFT:
					break;

				case SDL_BUTTON_RIGHT:
					break;

				case SDL_BUTTON_MIDDLE:
					break;
			}
			break;

		case SDL_MOUSEBUTTONUP:
			switch (event->button.button) {
				case SDL_BUTTON_LEFT:
					break;

				case SDL_BUTTON_RIGHT:
					break;

				case SDL_BUTTON_MIDDLE:
					break;
			}
			break;

		case SDL_MOUSEMOTION:
			break;

		case SDL_MOUSEWHEEL:
			break;

		case SDL_KEYDOWN:
			switch (event->key.keysym.sym) {
				case SDLK_AC_BACK:
				case SDLK_ESCAPE:
				case SDLK_q:
					return 1;

				case SDLK_r:
					tamalib_enable_step_by_step(0);
					tamalib_pause(0);
					break;

				case SDLK_s:
					tamalib_enable_step_by_step(1);
					tamalib_pause(0);
					break;

				case SDLK_f:
					switch (speed) {
						case SPEED_1X:
							speed = SPEED_10X;
							break;

						case SPEED_10X:
							speed = SPEED_UNLIMITED;
							break;

						case SPEED_UNLIMITED:
							speed = SPEED_1X;
							break;
					}

					tamalib_set_speed((u8_t) speed);
					break;

				case SDLK_b:
					state_find_next_name(save_path);
					state_save(save_path);
					break;

				case SDLK_n:
					state_find_last_name(save_path);
					if (save_path[0]) {
						state_load(save_path);
					}
					break;

				case SDLK_i:
					if (pixel_stride >= PIXEL_STRIDE_MAX) {
						break;
					}

					sdl_release();
					compute_layout(++pixel_stride);
					sdl_init();
					break;

				case SDLK_d:
					if (pixel_stride <= PIXEL_STRIDE_MIN) {
						break;
					}

					sdl_release();
					compute_layout(--pixel_stride);
					sdl_init();
					break;

				case SDLK_LEFT:
					tamalib_set_button(BTN_LEFT, BTN_STATE_PRESSED);
					break;

				case SDLK_DOWN:
					tamalib_set_button(BTN_MIDDLE, BTN_STATE_PRESSED);
					break;

				case SDLK_RIGHT:
					tamalib_set_button(BTN_RIGHT, BTN_STATE_PRESSED);
					break;
			}
			break;

		case SDL_KEYUP:
			switch (event->key.keysym.sym) {
				case SDLK_LEFT:
					tamalib_set_button(BTN_LEFT, BTN_STATE_RELEASED);
					break;

				case SDLK_DOWN:
					tamalib_set_button(BTN_MIDDLE, BTN_STATE_RELEASED);
					break;

				case SDLK_RIGHT:
					tamalib_set_button(BTN_RIGHT, BTN_STATE_RELEASED);
					break;
			}
			break;
	}

	return 0;
}

static int hal_handler(void)
{
	SDL_Event event;
	timestamp_t ts;

	if (memory_editor_enable) {
		/* Dump memory @ FRAMERATE fps */
		ts = hal_get_timestamp();
		if (ts - mem_dump_ts >= 1000000/FRAMERATE) {
			mem_dump_ts = ts;
			mem_edit_update();
		}
	}

	while (SDL_PollEvent(&event)) {
		if (handle_sdl_events(&event)) {
			return 1;
		}
	}

	return 0;
}

static hal_t hal = {
	.malloc = &hal_malloc,
	.free = &hal_free,
	.halt = &hal_halt,
	.is_log_enabled = &hal_is_log_enabled,
	.log = &hal_log,
	.usleep = &hal_usleep,
	.get_timestamp = &hal_get_timestamp,
	.update_screen = &hal_update_screen,
	.set_lcd_matrix = &hal_set_lcd_matrix,
	.set_lcd_icon = &hal_set_lcd_icon,
	.set_frequency = &hal_set_frequency,
	.play_frequency = &hal_play_frequency,
	.handler = &hal_handler,
};

static void audio_callback(void *userdata, Uint8 *stream, int len)
{
	unsigned int i;
	int samples = len / sizeof(float);

	if (is_audio_playing) {
		/* Generate the required frequency */
		for (i = 0; i < samples; i++) {
			((float *) stream)[i] = AUDIO_VOLUME * SDL_sinf(2 * M_PI * (i + sin_pos) * current_freq / (AUDIO_FREQUENCY * 10));
		}

		sin_pos = (sin_pos + samples) % (AUDIO_FREQUENCY * 10);
	} else {
		/* No sound */
		SDL_memset(stream, 0, len);
		sin_pos = 0;
	}

}

static void sdl_release(void)
{
	SDL_DestroyTexture(icons);
	SDL_DestroyTexture(bg);

	IMG_Quit();

	SDL_DestroyWindow(window);
	SDL_Quit();
}

static bool_t sdl_init(void)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO) != 0) {
		hal_log(LOG_ERROR, "Failed to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG) {
		hal_log(LOG_ERROR, "Failed to initialize SDL_image: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	window = SDL_CreateWindow(APP_NAME, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_size, window_size, SDL_WINDOW_SHOWN);

	renderer =  SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

	bg = IMG_LoadTexture(renderer, BACKGROUND_PATH);
	if(!bg) {
		hal_log(LOG_ERROR, "Failed to load the background image: %s\n", SDL_GetError());
		sdl_release();
		return 1;
	}

	icons = IMG_LoadTexture(renderer, ICONS_PATH);
	if(!icons) {
		hal_log(LOG_ERROR, "Failed to load the icons image: %s\n", SDL_GetError());
		sdl_release();
		return 1;
	}

	bg_rect.x = 0;
	bg_rect.y = 0;
	bg_rect.w = window_size;
	bg_rect.h = window_size;

	SDL_memset(&audio_spec, 0, sizeof(audio_spec));
	audio_spec.freq = AUDIO_FREQUENCY;
	audio_spec.format = AUDIO_F32SYS;
	audio_spec.channels = 1;
	audio_spec.samples = AUDIO_SAMPLES;
	audio_spec.callback = &audio_callback;

	audio_dev = SDL_OpenAudioDevice(NULL, 0, &audio_spec, &audio_spec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
	if(!audio_dev) {
		hal_log(LOG_ERROR, "Failed to open the audio device: %s\n", SDL_GetError());
		sdl_release();
		return 1;
	}

	SDL_PauseAudioDevice(audio_dev, SDL_FALSE);

	return 0;
}

void rom_not_found_msg(void)
{
#if defined(__WIN32__)
	MessageBox(NULL, ROM_NOT_FOUND_MSG, ROM_NOT_FOUND_TITLE, MB_OK);
#elif defined(__APPLE__)
	CFUserNotificationDisplayNotice(0, kCFUserNotificationStopAlertLevel, NULL, NULL, NULL, CFSTR(ROM_NOT_FOUND_TITLE), CFSTR(ROM_NOT_FOUND_MSG), NULL);
#else
	fprintf(stderr, ROM_NOT_FOUND_TITLE": "ROM_NOT_FOUND_MSG"\n");
#endif
}

static void usage(FILE * fp, int argc, char **argv)
{
	fprintf(fp,
		APP_NAME" v"APP_VERSION" - (C)"COPYRIGHT_DATE" "AUTHOR_NAME"\n\n"
		"Usage: %s [options]\n\n"
		"Options:\n"
		"\t-r | --rom <path>             The ROM file to use (default is %s)\n"
		"\t-E | --extract <path>         PNG file to use when extracting the data/sprites from a ROM\n"
		"\t-M | --modify <path>          PNG file to use when modifying the data/sprites of a ROM\n"
		"\t-H | --header                 Generate a header file from the ROM (written to STDOUT)\n"
		"\t-l | --load <path>            Load the given memory state file (save)\n"
		"\t-s | --step                   Enable step by step debugging from the start\n"
		"\t-b | --break <0xXXX>          Add a breakpoint\n"
		"\t-m | --memory                 Show memory access\n"
#if defined(__WIN32__)
		"\t-e | --editor                 Realtime memory editor\n"
#endif
		"\t-c | --cpu                    Show CPU related information\n"
		"\t-v | --verbose                Show all information\n"
		"\t-h | --help                   Print this message\n",
		argv[0], ROM_PATH);
}

static const char short_options[] = "r:E:M:Hl:sb:mecvh";

static const struct option long_options[] = {
	{"rom", required_argument, NULL, 'r'},
	{"extract", required_argument, NULL, 'E'},
	{"modify", required_argument, NULL, 'M'},
	{"header", no_argument, NULL, 'H'},
	{"load", required_argument, NULL, 'l'},
	{"step", no_argument, NULL, 's'},
	{"break", required_argument, NULL, 'b'},
	{"memory", no_argument, NULL, 'm'},
	{"editor", no_argument, NULL, 'e'},
	{"cpu", no_argument, NULL, 'c'},
	{"verbose", no_argument, NULL, 'v'},
	{"help", no_argument, NULL, 'h'},
	{0, 0, 0, 0}
};

int main(int argc, char **argv)
{
	char rom_path[256] = ROM_PATH;
	char sprites_path[256] = {0};
	char save_path[256] = {0};
	bool_t gen_header = 0;
	bool_t extract_sprites = 0;
	bool_t modify_sprites = 0;

#if defined(__WIN32__)
	QueryPerformanceFrequency(&counter_freq);
#endif

	tamalib_register_hal(&hal);

	for (;;) {
		int index;
		int c;

		c = getopt_long(argc, argv, short_options, long_options, &index);

		if (-1 == c)
			break;

		switch (c) {
			case 0:	/* getopt_long() flag */
				break;

			case 'r':
				strncpy(rom_path, optarg, 256);
				break;

			case 'E':
				extract_sprites = 1;
				strncpy(sprites_path, optarg, 256);
				break;

			case 'M':
				modify_sprites = 1;
				strncpy(sprites_path, optarg, 256);
				break;

			case 'H':
				gen_header = 1;
				break;

			case 'l':
				strncpy(save_path, optarg, 256);
				break;

			case 's':
				tamalib_enable_step_by_step(1);
				tamalib_pause(1);
				break;

			case 'b':
				tamalib_add_bp(&g_breakpoints, strtoul(optarg, NULL, 0));
				break;

			case 'm':
				log_levels |= LOG_MEMORY;
				break;

#if defined(__WIN32__)
			case 'e':
				memory_editor_enable = 1;
				break;
#endif

			case 'c':
				log_levels |= LOG_CPU;
				break;

			case 'v':
				log_levels |= LOG_MEMORY | LOG_CPU;
				break;

			case 'h':
				usage(stdout, argc, argv);
				exit(EXIT_SUCCESS);

			default:
				usage(stderr, argc, argv);
				exit(EXIT_FAILURE);
		}
	}

	g_program = program_load(rom_path, &g_program_size);
	if (g_program == NULL) {
		hal_log(LOG_ERROR, "FATAL: Error while loading ROM %s !\n", rom_path);
		tamalib_free_bp(&g_breakpoints);
		rom_not_found_msg();
		return -1;
	}

	if (gen_header || extract_sprites || modify_sprites) {
		/* ROM manipulation only (no emulation) */
		if (gen_header) {
			program_to_header(g_program, g_program_size);
		} else if (extract_sprites) {
			program_get_data(g_program, g_program_size, sprites_path);
		} else if (modify_sprites) {
			program_set_data(g_program, g_program_size, sprites_path);
			program_save(rom_path, g_program, g_program_size);
		}

		SDL_free(g_program);
		tamalib_free_bp(&g_breakpoints);
		return 0;
	}

	compute_layout(pixel_stride);

	if (sdl_init()) {
		hal_log(LOG_ERROR, "FATAL: Error while initializing application !\n");
		SDL_free(g_program);
		tamalib_free_bp(&g_breakpoints);
		return -1;
	}

	if (tamalib_init(g_program, g_breakpoints)) {
		hal_log(LOG_ERROR, "FATAL: Error while initializing tamalib !\n");
		sdl_release();
		SDL_free(g_program);
		tamalib_free_bp(&g_breakpoints);
		return -1;
	}

	if (save_path[0]) {
		state_load(save_path);
	}

	if (memory_editor_enable) {
		/* Logs are not compatible with the memory editor */
		log_levels = LOG_ERROR;
		mem_edit_configure_terminal();
	}

	tamalib_mainloop();

	if (memory_editor_enable) {
		mem_edit_reset_terminal();
	}

	tamalib_release();

	sdl_release();

	SDL_free(g_program);

	tamalib_free_bp(&g_breakpoints);

	return 0;
}
