/* See LICENSE file for copyright and license details.
 * Copyright (C) 2023-2024  Henrique Almeida <me@h3nc4.com> */

#include "engine.h"

// Global grid and its pointers
static u8int *grid = NULL;
static u8int *current_grid = NULL;
static u8int *prev_grid = NULL;
// This array stores the offset of each row in the grid
static unsigned int *row_offsets = NULL;

static rules game;

// Macros for index calculation and neighbor counting
#define PREV_Y(y) (((y) - 1 + game.height) % game.height)
#define NEXT_Y(y) (((y) + 1) % game.height)
#define PREV_X(x) (((x) - 1 + game.width) % game.width)
#define NEXT_X(x) (((x) + 1) % game.width)
#define COUNT_LIVING_NEIGHBORS(y, x)                 \
	(prev_grid[row_offsets[PREV_Y(y)] + PREV_X(x)] + \
	 prev_grid[row_offsets[PREV_Y(y)] + (x)] +       \
	 prev_grid[row_offsets[PREV_Y(y)] + NEXT_X(x)] + \
	 prev_grid[row_offsets[(y)] + PREV_X(x)] +       \
	 prev_grid[row_offsets[(y)] + NEXT_X(x)] +       \
	 prev_grid[row_offsets[NEXT_Y(y)] + PREV_X(x)] + \
	 prev_grid[row_offsets[NEXT_Y(y)] + (x)] +       \
	 prev_grid[row_offsets[NEXT_Y(y)] + NEXT_X(x)])
// A cell becomes alive if it has exactly 3 neighbors (birth)
// A cell stays alive if it is currently alive and has exactly 2 neighbors (survival)
// Otherwise, it dies.
#define IS_ALIVE(living_neighbors, cell) ((living_neighbors == 3) | (cell & (living_neighbors == 2)))

void free_grid()
{
	free(grid);
	free(row_offsets);
}

// Aligned allocation for SIMD
static void allocate_grid()
{
	grid = aligned_alloc(32, 2 * game.size * sizeof(u8int));
	row_offsets = malloc(game.height * sizeof(unsigned int));
	if (!grid || !row_offsets)
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

	// Precompute row offsets
	for (unsigned int y = 0; y < game.height; y++)
		row_offsets[y] = y * game.width;
}

void update_grid()
{
	current_grid = grid + game.size * game.grid_state;
	prev_grid = grid + game.size * (game.grid_state ^ 1);
#pragma omp parallel for schedule(static) // Parallelize loop
	for (unsigned int idx = 0; idx < game.size; idx++)
		current_grid[idx] = IS_ALIVE(COUNT_LIVING_NEIGHBORS(idx / game.width, idx % game.width), prev_grid[idx]);
	game.grid_state ^= 1;
}

void initialize_game(unsigned int w, unsigned int h)
{
	if (grid)
		free_grid();
	// Width and height are swapped to optimize cache access
	game.width = h;
	game.height = w;
	game.size = game.width * game.height;
	allocate_grid();
	init_grid();
}

u8int **ptr_to_both_grids()
{
	static u8int *ptrs[2];
	ptrs[0] = current_grid;
	ptrs[1] = prev_grid;
	return ptrs;
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
	current_grid[row_offsets[y] + x] ^= 1; // Toggle between 0 and 1
}
