/**
 * MIT License
 *
 * Copyright(c) 2023 Henrique Almeida
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <time.h>
#include <windows.h>

// constants
#define X 46
#define Y 167
#define seconds 1000 * 0.3
#define buffer_size (X + 1) * Y + 1

int board[X][Y];

// iterators
int i, j;

/**
 * \brief Get the adjacent live cells of a cell
 * \return int The number of adjacent live cells
 * \details This function runs through the 3x3 grid around the cell and counts the number of live cells
 */
int get_adjacent_cells()
{
    int neighbors = 0;
    // for each neighbor of the current cell (i, j)
    for (int i0 = i - 1; i0 <= i + 1; i0++)
        for (int j0 = j - 1; j0 <= j + 1; j0++)
            // if the next cell is not the current cell and is within the board
            if (i0 >= 0 && i0 < X && j0 >= 0 && j0 < Y && board[i0][j0])
                neighbors++;
    return board[i][j] ? neighbors - 1 : neighbors; // if the current cell is alive, subtract 1 (since it counts itself)
}

/**
 * \brief Get the next generation of the board
 * \details The function runs through the board and gets the number of adjacent live cells for each cell.
 *         Then, it applies the rules of the game of life to determine if the cell will be alive or dead in the next generation.
 */
void next_generation()
{
    int new_board[X][Y], adjacent_cells;
    // run through the board
    for (i = 0; i < X; i++)
        for (j = 0; j < Y; j++)
        {
            // get adjacent cells
            adjacent_cells = get_adjacent_cells();
            // if the cell is alive and has 2 or 3 adjacent cells, or if its dead and has 3 adjacent cells, then becomes alive
            new_board[i][j] = (board[i][j] && (adjacent_cells == 2 || adjacent_cells == 3)) || (!board[i][j] && adjacent_cells == 3);
        }

    // copy contents of new board to the original board
    for (i = 0; i < X; i++)
        for (j = 0; j < Y; j++)
            board[i][j] = new_board[i][j];
}

/**
 * \brief Print the next generation of the board
 * \details The function runs through the board and builds a visual output of the board.
 *         Then, it clears the last table and prints the next generation.
 */
void print_next_board()
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
    system("cls");
    fwrite(buffer, 1, index, stdout);
}

int main()
{
    // initialize a random board
    srand(time(0));
    for (i = 0; i < X; i++)
        for (j = 0; j < Y; j++)
            board[i][j] = rand() & 1; // 1 or 0

    // Larger time on start
    print_next_board();
    Sleep(seconds * 7);
    next_generation();

    // Life begins
    while (1)
    {
        print_next_board();
        Sleep(seconds);
        next_generation();
    }

    // end of program (unreachable)
    return 0;
}
