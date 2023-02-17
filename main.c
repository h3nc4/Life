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

int get_adjacent_cells()
{
    int out = 0;
    // run through the 3x3 grid around the cell
    for (int i0 = i - 1; i0 <= i + 1; i0++)
        for (int j0 = j - 1; j0 <= j + 1; j0++)
            // if the next cell is not the current cell or not beyond the board and is alive, then add 1 to the total
            if (!((i0 == i && j0 == j) || i0 < 0 || i0 >= X || j0 < 0 || j0 >= Y) && board[i0][j0])
                out++;
    return out;
}

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
            if ((board[i][j] && (adjacent_cells == 2 || adjacent_cells == 3)) || (!board[i][j] && adjacent_cells == 3))
                new_board[i][j] = 1;
            else
                new_board[i][j] = 0;
        }

    // copy contents of new board to the original board
    for (i = 0; i < X; i++)
        for (j = 0; j < Y; j++)
            board[i][j] = new_board[i][j];
}

void print_next_board()
{
    char buffer[buffer_size];
    int index = 0;
    // run through the board and build a visual output
    for (i = 0; i < X; i++)
    {
        buffer[index++] = '\n';
        for (j = 0; j < Y; j++)
            if (board[i][j])
                buffer[index++] = 219;
            else
                buffer[index++] = ' ';
    }
    buffer[0] = 0;
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
            board[i][j] = rand() % 2; // 1 or 0

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
