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

#define IDX(y, x) (((y + height) % height) * width + ((x + width) % width))

void allocate_grids() {
	grid = calloc(size, sizeof(int));
	next_grid = calloc(size, sizeof(int));
}

void free_grids() {
	free(grid);
	free(next_grid);
}

void init_grid() {
	for (int i = 0; i < size; i++)
		grid[i] = rand() % 2;
}

void update_grid() {
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int idx = IDX(y, x);
			int living_neighbors = 0;
			for (int dy = -1; dy <= 1; dy++)
				for (int dx = -1; dx <= 1; dx++)
					if (dy || dx) // Skip the cell itself
						living_neighbors += grid[IDX(y + dy, x + dx)];

			// Apply the Game of Life rules
			next_grid[idx] = (grid[idx] == 1)
				? (living_neighbors == 2 || living_neighbors == 3)
				: (living_neighbors == 3);
		}
	}

	// Swap grids
	int *temp = grid;
	grid = next_grid;
	next_grid = temp;
}

void initialize_game(int w, int h) {
	if (grid) free_grids();
	width = w;
	height = h;
	size = width * height;
	allocate_grids();
	init_grid();
}

int get_cell_state(int y, int x) {
	return grid[IDX(y, x)];
}

void set_cell_state(int y, int x, int state) {
	grid[IDX(y, x)] = state;
}
