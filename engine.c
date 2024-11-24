/* See LICENSE file for copyright and license details.
 * Copyright (C) 2023-2024  Henrique Almeida <me@h3nc4.com> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#ifndef uint8_t
    typedef unsigned char uint8_t;
#endif
#ifdef _WIN32
#include <malloc.h>
#define aligned_alloc(alignment, size) _aligned_malloc(size, alignment)
#endif

typedef uint8_t u8int;

// Global grid and its pointers
static u8int *grid = NULL;
static u8int *current_grid = NULL;
static u8int *prev_grid = NULL;

typedef struct
{
	u8int grid_state; // Current grid state (0 or 1)
	unsigned int width;		  // Grid width
	unsigned int height;	  // Grid height
	unsigned int size;		  // Total grid size (width * height)
} rules;

static rules game;

// Macros for index calculation and neighbor counting
#define IDX(y, x) (((y) * game.width + (x)))
#define PREV_Y(y) (((y) - 1 + game.height) % game.height)
#define NEXT_Y(y) (((y) + 1) % game.height)
#define PREV_X(x) (((x) - 1 + game.width) % game.width)
#define NEXT_X(x) (((x) + 1) % game.width)
#define COUNT_LIVING_NEIGHBORS(y, x)                                                                                \
	(prev_grid[IDX(PREV_Y(y), PREV_X(x))] + prev_grid[IDX(PREV_Y(y), (x))] + prev_grid[IDX(PREV_Y(y), NEXT_X(x))] + \
	 prev_grid[IDX((y), PREV_X(x))] + prev_grid[IDX((y), NEXT_X(x))] +                                              \
	 prev_grid[IDX(NEXT_Y(y), PREV_X(x))] + prev_grid[IDX(NEXT_Y(y), (x))] + prev_grid[IDX(NEXT_Y(y), NEXT_X(x))])

void free_grid()
{
	free(grid);
}

// Aligned allocation for SIMD
static void allocate_grid()
{
	grid = aligned_alloc(32, 2 * game.size * sizeof(u8int));
	if (!grid)
	{
		fprintf(stderr, "Error: Failed to allocate memory.\n");
		exit(EXIT_FAILURE);
	}
}

static void init_grid()
{
	srand(time(NULL));
	for (unsigned int i = 0; i < game.size; i++)
		grid[i] = rand() % 2;
	memcpy(grid + game.size, grid, game.size * sizeof(u8int));
	current_grid = grid;
}

void update_grid()
{
	current_grid = grid + game.size * game.grid_state;
	prev_grid = grid + game.size * (1 - game.grid_state);
#pragma omp parallel for schedule(static) // Parallelize loop
	for (unsigned int idx = 0; idx < game.size; idx++)
	{
		u8int living_neighbors = COUNT_LIVING_NEIGHBORS(idx / game.width, idx % game.width);

		// A cell becomes alive if it has exactly 3 neighbors (birth)
		// A cell stays alive if it is currently alive and has exactly 2 neighbors (survival)
		// Otherwise, it dies.
		current_grid[idx] = (living_neighbors == 3) | (prev_grid[idx] & (living_neighbors == 2));
	}
	game.grid_state = 1 - game.grid_state;
}

void initialize_game(unsigned int w, unsigned int h)
{
	if (grid)
		free_grid();
	game.width = w;
	game.height = h;
	game.size = game.width * game.height;
	allocate_grid();
	init_grid();
}

u8int **ptr_to_current_grid()
{
	return &current_grid;
}

void clear_grid()
{
	memset(grid, 0, 2 * game.size * sizeof(u8int));
}

void restart_game()
{
	init_grid();
}

void toggle_cell_state(unsigned int y, unsigned int x)
{
	unsigned int idx = IDX(y, x);
	current_grid[idx] = 1 - current_grid[idx]; // Toggle between 0 and 1
}
