/*
 *  Copyright 2023 Henrique Almeida
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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// constants
#define X 46
#define Y 167
#define buffer_size (X + 1) * Y + 1

// clear screen command
#ifdef _WIN32
#define CLEAR "cls"
#else
#define CLEAR "clear"
#endif

// board
int board[X][Y];

// iterators
int i, j;

/**
 * \brief Gets adjacent live cells of a cell
 * \return int Total adjacent live cells
 * \details This function runs through the 3x3 grid around the cell and counts the number of live cells,
 *          deprecated for a more efficient function
 */
int get_adjacent_cells_deprecated(void)
{
    int neighbors = 0;
    for (int i0 = i - 1; i0 <= i + 1; i0++)
        for (int j0 = j - 1; j0 <= j + 1; j0++)
            if (i0 >= 0 && i0 < X && j0 >= 0 && j0 < Y && board[i0][j0])
                neighbors++;
    return board[i][j] ? neighbors - 1 : neighbors; // if the current cell is alive, subtract 1 (since it counts itself)
}

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
            neighbors += board[i - 1][j - 1]; // top left
        neighbors += board[i - 1][j];         // top
        // check if the cell is in the last column
        if (j < Y - 1)
            neighbors += board[i - 1][j + 1]; // top right
    }
    // check if the cell is in the last row
    if (i < X - 1)
    {
        // check if the cell is in the first column
        if (j > 0)
            neighbors += board[i + 1][j - 1]; // bottom left
        neighbors += board[i + 1][j];         // bottom
        // check if the cell is in the last column
        if (j < Y - 1)
            neighbors += board[i + 1][j + 1]; // bottom right
    }
    // check if the cell is in the first column
    if (j > 0)
        neighbors += board[i][j - 1]; // left
    // check if the cell is in the last column
    if (j < Y - 1)
        neighbors += board[i][j + 1]; // right
    return neighbors;
}

/**
 * \brief Get the board's next generation
 * \details This function runs through the board and gets the number of adjacent live cells for each cell.
 *         Then, it applies the rules of the game of life to determine if the cell will be alive or dead in the next generation.
 */
void next_generation(void)
{
    int new_board[X][Y], adjacent_cells;
    // run through the board
    for (i = 0, j = 0; i < X; j = (j + 1) % Y, i += j == 0)
    {
        // get adjacent cells
        adjacent_cells = get_adjacent_cells();
        // if the cell is alive and has 2 or 3 adjacent cells, or if its dead and has 3 adjacent cells, then becomes alive
        new_board[i][j] = (board[i][j] && (adjacent_cells == 2 || adjacent_cells == 3)) || (!board[i][j] && adjacent_cells == 3);
    }

    // copy contents of new board to the original board
    for (i = 0, j = 0; i < X; j = (j + 1) % Y, i += j == 0)
        board[i][j] = new_board[i][j];
}

/**
 * \brief Print the board's next generation
 * \details This function runs through the board and builds a visual output of the board.
 *         Then, it clears the last table and prints the next generation.
 */
void print_next_board(void)
{
    char buffer[buffer_size];
    int index = 0;
    // run through the board and build a visual output
    for (i = 0; i < X; i++)
    {
        buffer[index++] = '\n';
        for (j = 0; j < Y; j++)
            buffer[index++] = board[i][j] ? 219 : ' '; // 219 is the ascii code for a filled square
    }
    buffer[0] = 0; // remove first \n
    // clear last table and print next
    system(CLEAR);
    fwrite(buffer, 1, index, stdout);
}

/**
 * \brief Sleep for a given amount of milliseconds
 * \param milliseconds The amount of milliseconds to sleep for
 * \details This function gets the current time and adds the given amount of milliseconds to it.
 *         Then, it runs a loop until the current time is equal to the end time.
 */
void sleep(int milliseconds)
{
    clock_t end = clock() + milliseconds; // get end time from current time + milliseconds
    while (clock() < end)
        ;
}

/**
 * \brief Main function
 * \return int The exit code
 * \details This function initializes a random board and runs the game of life.
 */
int main(void)
{
    // initialize a random board
    srand(time(0));
    for (i = 0, j = 0; i < X; j = (j + 1) % Y, i += j == 0)
        board[i][j] = rand() & 1; // 1 or 0
    print_next_board();           // print first board

    // Larger time on start so user can see his initial board with detail
    sleep(1000);

    // Life begins
    while (1)
    {
        sleep(300);
        next_generation();
        print_next_board();
    }

    // end of program (unreachable)
    return 0;
}
