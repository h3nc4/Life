/* See LICENSE file for copyright and license details.
 * Copyright (C) 2023-2024  Henrique Almeida <me@h3nc4.com> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Global grid
static unsigned char *grid = NULL;
static unsigned char *current_grid = NULL;
static unsigned char *prev_grid = NULL;
static unsigned char grid_state = 0; // 0 or 1

// Grid dimensions
static unsigned int width = 0;
static unsigned int height = 0;
static unsigned int size = 0; // width * height for flat array indexing

// Macros for index calculation and neighbor counting
#define IDX(y, x) (((y) * width + (x)))
#define PREV_Y(y) (((y) - 1 + height) % height)
#define NEXT_Y(y) (((y) + 1) % height)
#define PREV_X(x) (((x) - 1 + width) % width)
#define NEXT_X(x) (((x) + 1) % width)
#define COUNT_LIVING_NEIGHBORS(y, x) \
	(prev_grid[IDX(PREV_Y(y), PREV_X(x))] + prev_grid[IDX(PREV_Y(y), (x))] + prev_grid[IDX(PREV_Y(y), NEXT_X(x))] + \
	prev_grid[IDX((y), PREV_X(x))] + prev_grid[IDX((y), NEXT_X(x))] + \
	prev_grid[IDX(NEXT_Y(y), PREV_X(x))] + prev_grid[IDX(NEXT_Y(y), (x))] + prev_grid[IDX(NEXT_Y(y), NEXT_X(x))])

void free_grid()
{
	free(grid);
}

// Aligned allocation for SIMD
static void allocate_grid()
{
	grid = aligned_alloc(32, 2 * size * sizeof(char));
	if (!grid)
	{
		fprintf(stderr, "Error: Failed to allocate memory.\n");
		exit(EXIT_FAILURE);
	}
}

static void init_grid()
{
	srand(time(NULL));
	for (unsigned int i = 0; i < size; i++)
		grid[i] = rand() % 2;
	memcpy(grid + size, grid, size * sizeof(char));
	current_grid = grid;
}

void update_grid()
{
	current_grid = grid + size * grid_state;
	prev_grid = grid + size * (1 - grid_state);
#pragma omp parallel for schedule(static) // Parallelize loop
	for (unsigned int idx = 0; idx < size; idx++)
	{
		// Count living neighbors
		char living_neighbors = COUNT_LIVING_NEIGHBORS(idx / width, idx % width);

		// A cell becomes alive if it has exactly 3 neighbors (birth)
		// A cell stays alive if it is currently alive and has exactly 2 neighbors (survival)
		// Otherwise, it dies.
		current_grid[idx] = (living_neighbors == 3) | (prev_grid[idx] & (living_neighbors == 2));
	}
	grid_state = 1 - grid_state;
}

void initialize_game(unsigned int w, unsigned int h)
{
	if (grid)
		free_grid();
	width = w;
	height = h;
	size = width * height;
	allocate_grid();
	init_grid();
}

unsigned char **ptr_to_current_grid()
{
	return &current_grid;
}

void clear_grid()
{
	memset(grid, 0, size * sizeof(char));
	grid_state = 0;
}

void toggle_cell_state(unsigned int y, unsigned int x)
{
	unsigned int idx = IDX(y, x);
	current_grid[idx] = 1 - current_grid[idx]; // Toggle between 0 and 1
}
