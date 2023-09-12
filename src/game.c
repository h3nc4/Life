/*
 *  Copyright 2023 Henrique Almeida <me@h3nc4.com>
 *
 * This file is part of Life.
 *
 * Life is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * Life is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU
 * General Public License along with Life. If not, see
 * <https://www.gnu.org/licenses/>.
 */

#include "life.h"

/**
 * \brief Gets adjacent live cells of a cell
 * \return int Total adjacent live cells
 * \details This function runs through the 3x3 grid around the cell and counts the number of live cells
 */
int get_adjacent_cells(void)
{
    int neighbors = 0;
    // check if the cell is in the first row
    if (i > 0)
    {
        // check if the cell is in the first column
        if (j > 0)
            neighbors += grid[i - 1][j - 1]; // top left
        neighbors += grid[i - 1][j];         // top
        // check if the cell is in the last column
        if (j < Y - 1)
            neighbors += grid[i - 1][j + 1]; // top right
    }
    // check if the cell is in the last row
    if (i < X - 1)
    {
        // check if the cell is in the first column
        if (j > 0)
            neighbors += grid[i + 1][j - 1]; // bottom left
        neighbors += grid[i + 1][j];         // bottom
        // check if the cell is in the last column
        if (j < Y - 1)
            neighbors += grid[i + 1][j + 1]; // bottom right
    }
    // check if the cell is in the first column
    if (j > 0)
        neighbors += grid[i][j - 1]; // left
    // check if the cell is in the last column
    if (j < Y - 1)
        neighbors += grid[i][j + 1]; // right
    return neighbors;
}

/**
 * \brief Get the board's next generation
 * \details This function runs through the board and gets the number of adjacent live cells for each cell.
 *         Then, it applies the rules of the game of life to determine if the cell will be alive or dead in the next generation.
 */
void next_generation(void)
{
    gboolean new_board[X][Y];
    int adjacent_cells;
    // run through the board
    for (i = 0, j = 0; i < X; j = (j + 1) % Y, i += j == 0)
    {
        // get adjacent cells
        adjacent_cells = get_adjacent_cells();
        // if the cell is alive and has 2 or 3 adjacent cells, or if its dead and has 3 adjacent cells, then it's alive
        new_board[i][j] = (grid[i][j] && (adjacent_cells == 2 || adjacent_cells == 3)) || (!grid[i][j] && adjacent_cells == 3);
    }

    // copy contents of new board to the original board
    for (i = 0, j = 0; i < X; j = (j + 1) % Y, i += j == 0)
        grid[i][j] = new_board[i][j];
}
