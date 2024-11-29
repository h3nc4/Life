/* See LICENSE file for copyright and license details.
 * Copyright (C) 2023-2024  Henrique Almeida <me@h3nc4.com> */

#ifndef GAME_OF_LIFE_H
#define GAME_OF_LIFE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#ifdef _WIN32
#include <malloc.h>
#define aligned_alloc(alignment, size) _aligned_malloc(size, alignment)
#endif

// Type definitions
typedef uint8_t u8int;

typedef struct
{
	u8int grid_state;	 // Current grid state (0 or 1)
	unsigned int width;	 // Grid width
	unsigned int height; // Grid height
	unsigned int size;	 // Total grid size (width * height)
} rules;

// Function declarations
void free_grid();
void initialize_game(unsigned int w, unsigned int h);
void update_grid();
u8int **ptr_to_both_grids();
void clear_grid();
void restart_game();
void toggle_cell_state(unsigned int y, unsigned int x);

#endif
