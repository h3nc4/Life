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
	for (int dy = -1; dy <= 1; dy++)
		for (int dx = -1; dx <= 1; dx++)
			if (dy || dx) // Skip the cell itself
				living_neighbors += grid[IDX((y + dy + height) % height, (x + dx + width) % width)];
	return living_neighbors;
}

void update_grid()
{
#pragma omp parallel for collapse(2) schedule(static) // Parallelize the nested loop
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
		{
			int idx = IDX(y, x);
			int living_neighbors = count_living_neighbors(y, x);
			next_grid[idx] = (grid[idx] == 1)
				? (living_neighbors == 2 || living_neighbors == 3)
				: (living_neighbors == 3);
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

void flip_cell_state(int y, int x)
{
	int idx = IDX(y, x);
	grid[idx] = 1 - grid[idx]; // Toggle between 0 and 1
}
