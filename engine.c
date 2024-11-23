/* See LICENSE file for copyright and license details.
 * Copyright (C) 2023-2024  Henrique Almeida <me@h3nc4.com> */

#include <stdlib.h>
#include <string.h>
#include <time.h>

// Global grid
static int *grid = NULL;
static int *next_grid = NULL;

// Grid dimensions
static int width = 0;
static int height = 0;
static int size = 0; // width * height for flat array indexing

// Macros for index calculation and neighbor counting
#define IDX(y, x) (((y) * width + (x)))
#define PREV_Y(y) (((y) - 1 + height) % height)
#define NEXT_Y(y) (((y) + 1) % height)
#define PREV_X(x) (((x) - 1 + width) % width)
#define NEXT_X(x) (((x) + 1) % width)
#define COUNT_LIVING_NEIGHBORS(y, x) \
	(grid[IDX(PREV_Y(y), PREV_X(x))] + grid[IDX(PREV_Y(y), (x))] + grid[IDX(PREV_Y(y), NEXT_X(x))] + \
	grid[IDX((y), PREV_X(x))] + grid[IDX((y), NEXT_X(x))] + \
	grid[IDX(NEXT_Y(y), PREV_X(x))] + grid[IDX(NEXT_Y(y), (x))] + grid[IDX(NEXT_Y(y), NEXT_X(x))])

// Aligned allocation for SIMD
static void allocate_grids()
{
	grid = aligned_alloc(32, size * sizeof(int));
	next_grid = aligned_alloc(32, size * sizeof(int));
	memset(grid, 0, size * sizeof(int));
	memset(next_grid, 0, size * sizeof(int));
}

void free_grids()
{
	free(grid);
	free(next_grid);
}

static void init_grid()
{
	srand(time(NULL));
	for (int i = 0; i < size; i++)
		grid[i] = rand() % 2;
}

void update_grid()
{
#pragma omp parallel for schedule(static) // Parallelize loop
	for (int idx = 0; idx < size; idx++)
	{
		// Count living neighbors
		int living_neighbors = COUNT_LIVING_NEIGHBORS(idx / width, idx % width);

		// A cell becomes alive if it has exactly 3 neighbors (birth)
		// A cell stays alive if it is currently alive and has exactly 2 neighbors (survival)
		// Otherwise, it dies.
		next_grid[idx] = (living_neighbors == 3) | (grid[idx] & (living_neighbors == 2));
	}

	// Swap grids
	int *temp = grid;
	grid = next_grid;
	next_grid = temp;
}

void initialize_game(int w, int h)
{
	if (grid)
		free_grids();
	width = w;
	height = h;
	size = width * height;
	allocate_grids();
	init_grid();
}

int **ptr_to_current_grid()
{
	return &grid;
}

void clear_grid()
{
	memset(grid, 0, size * sizeof(int));
}

void flip_cell_state(int y, int x)
{
	int idx = IDX(y, x);
	grid[idx] = 1 - grid[idx]; // Toggle between 0 and 1
}
