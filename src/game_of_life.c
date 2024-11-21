/* See LICENSE file for copyright and license details.
 * Copyright (C) 2023-2024  Henrique Almeida <me@h3nc4.com> */

#include <stdlib.h>

// Global grid
int **grid = NULL;
int **next_grid = NULL;

// Grid dimensions
int width = 0;
int height = 0;

void allocate_grids()
{
	grid = malloc(height * sizeof(int *));
	next_grid = malloc(height * sizeof(int *));
	for (int y = 0; y < height; y++)
	{
		grid[y] = malloc(width * sizeof(int));
		next_grid[y] = malloc(width * sizeof(int));
	}
}

void free_grids()
{
	for (int y = 0; y < height; y++)
	{
		free(grid[y]);
		free(next_grid[y]);
	}
	free(grid);
	free(next_grid);
}

void init_grid()
{
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			grid[y][x] = rand() % 2;
}

int count_neighbors(int y, int x)
{
	int count = 0;
	for (int dy = -1; dy <= 1; dy++)
		for (int dx = -1; dx <= 1; dx++)
		{
			if (dy == 0 && dx == 0)
				continue;
			count += grid[(y + dy + height) % height][(x + dx + width) % width];
		}
	return count;
}

void update_grid()
{
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
		{
			int alive_neighbors = count_neighbors(y, x);
			if (grid[y][x] == 1)
				next_grid[y][x] = (alive_neighbors == 2 || alive_neighbors == 3) ? 1 : 0;
			else
				next_grid[y][x] = (alive_neighbors == 3) ? 1 : 0;
		}
	for (int y = 0; y < height; y++)
		for (int x = 0; x < width; x++)
			grid[y][x] = next_grid[y][x];
}

void initialize_game(int w, int h)
{
	width = w;
	height = h;
	allocate_grids();
	init_grid();
}

int get_cell_state(int y, int x)
{
	if (y < 0 || y >= height || x < 0 || x >= width)
		return 0;
	return grid[y][x];
}

void set_cell_state(int y, int x, int state)
{
	if (y < 0 || y >= height || x < 0 || x >= width)
		return;
	grid[y][x] = state;
}
