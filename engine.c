/* See LICENSE file for copyright and license details.
 * Copyright (C) 2023-2024  Henrique Almeida <me@h3nc4.com> */

#include <stdlib.h>
#include <string.h>

// Global grid
static int *grid = NULL;
static int *next_grid = NULL;

// Grid dimensions
static int width = 0;
static int height = 0;
static int size = 0; // width * height for flat array indexing

#define IDX(y, x) (((y) * width + (x)))

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
	for (int i = 0; i < size; i++)
		grid[i] = rand() % 2;
}

static int count_living_neighbors(int y, int x)
{
	int living_neighbors = 0;
	int prev_y = (y - 1 + height) % height;
	int next_y = (y + 1) % height;
	int prev_x = (x - 1 + width) % width;
	int next_x = (x + 1) % width;
	living_neighbors += grid[IDX(prev_y, prev_x)] + grid[IDX(prev_y, x)] + grid[IDX(prev_y, next_x)];
	living_neighbors += grid[IDX(y, prev_x)] + grid[IDX(y, next_x)];
	living_neighbors += grid[IDX(next_y, prev_x)] + grid[IDX(next_y, x)] + grid[IDX(next_y, next_x)];
	return living_neighbors;
}

void update_grid()
{
#pragma omp parallel for schedule(static) // Parallelize the loop
	for (int idx = 0; idx < size; idx++)
	{
		int living_neighbors = count_living_neighbors(idx / width, idx % width);
		next_grid[idx] = (living_neighbors == 3) || (grid[idx] && living_neighbors == 2);
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

int **get_grids()
{
	static int *grids[2];
	grids[0] = grid;
	grids[1] = next_grid;
	return grids;
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
