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

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// constants
#define X 30
#define Y 30
#define buffer_size (X + 1) * Y + 1

// clear screen command
#ifdef _WIN32
#define CLEAR "cls"
#else
#define CLEAR "clear"
#endif

// iterators
int i, j;

// Define the boolean board
gboolean grid[X][Y] = {0};

/**
 * \brief Gets adjacent live cells of a cell
 * \return int Total adjacent live cells
 * \details This function runs through the 3x3 grid around the cell and counts the number of live cells
 */
static int get_adjacent_cells(void)
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
static void next_generation(void)
{
    gboolean new_board[X][Y];
    int adjacent_cells;
    // run through the board
    for (i = 0, j = 0; i < X; j = (j + 1) % Y, i += j == 0)
    {
        // get adjacent cells
        adjacent_cells = get_adjacent_cells();
        // if the cell is alive and has 2 or 3 adjacent cells, or if its dead and has 3 adjacent cells, then becomes alive
        new_board[i][j] = (grid[i][j] && (adjacent_cells == 2 || adjacent_cells == 3)) || (!grid[i][j] && adjacent_cells == 3);
    }

    // copy contents of new board to the original board
    for (i = 0, j = 0; i < X; j = (j + 1) % Y, i += j == 0)
        grid[i][j] = new_board[i][j];
}

/**
 * \brief Sleep for a given amount of milliseconds
 * \param milliseconds The amount of milliseconds to sleep for
 * \details This function gets the current time and adds the given amount of milliseconds to it.
 *         Then, it runs a loop until the current time is equal to the end time.
 */
static void sleeping(int milliseconds)
{
    clock_t end = clock() + milliseconds; // get end time from current time + milliseconds
    while (clock() < end)
        ;
}

// Callback function for the click event
static void toggle_cell(GtkWidget *widget, gpointer data)
{
    int x = GPOINTER_TO_INT(data) / Y;
    int y = GPOINTER_TO_INT(data) % Y;
    grid[x][y] = !grid[x][y];
    gtk_widget_queue_draw(widget);
}

// Callback function to draw the grid
static gboolean draw_board(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    // Get the current color
    GdkRGBA color;

    // Get the size of the drawing area
    guint width = gtk_widget_get_allocated_width(widget);
    guint height = gtk_widget_get_allocated_height(widget);

    // Calculate cell size
    double cell_width = (double)width / X;
    double cell_height = (double)height / Y;

    // Draw the next generation
    for (int x = 0; x < X; x++)
        for (int y = 0; y < Y; y++)
        {
            // Set cell color based on boolean value
            if (grid[x][y])
                gdk_rgba_parse(&color, "white"); // Set to white for alive cells
            else
                gdk_rgba_parse(&color, "black"); // Set to black for dead cells

            // Fill the cell with the selected color
            cairo_set_source_rgb(cr, color.red, color.green, color.blue);
            cairo_rectangle(cr, x * cell_width, y * cell_height, cell_width, cell_height);
            cairo_fill(cr);
        }
    return FALSE;
}

// Callback function for the timer
static gboolean timer_callback(gpointer data)
{
    next_generation();
    gtk_widget_queue_draw(GTK_WIDGET(data));
    return G_SOURCE_CONTINUE;
}

// Callback function for the key press event
static void on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    if (event->keyval == GDK_KEY_Escape)
        gtk_main_quit(); // quit the app
}

int main(int argc, char *argv[])
{
    // initialize a random board
    srand(time(0));
    for (i = 0, j = 0; i < X; j = (j + 1) % Y, i += j == 0)
        grid[i][j] = rand() & 1;

    // init GTK
    gtk_init(&argc, &argv);

    // create the main window
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Conway's Game of Life");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);

    // Set the window to be fullscreen
    gtk_window_fullscreen(GTK_WINDOW(window));

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // create a drawing area for the grid
    GtkWidget *grid = gtk_drawing_area_new();
    gtk_widget_set_size_request(grid, X * 20, Y * 20);
    gtk_container_add(GTK_CONTAINER(window), grid);

    // connect the draw event
    g_signal_connect(grid, "draw", G_CALLBACK(draw_board), NULL);

    // connect the click event to toggle cells
    g_signal_connect(grid, "button-press-event", G_CALLBACK(toggle_cell), NULL);

    // timer to update the board every 300 milliseconds
    guint timer_id = g_timeout_add(300, timer_callback, grid);

    // connect the key press event
    g_signal_connect(G_OBJECT(window), "key-press-event", G_CALLBACK(on_key_press), NULL);

    // show all widgets
    gtk_widget_show_all(window);

    // start the GTK main loop
    gtk_main();

    // cleanup and remove the timer on exit
    g_source_remove(timer_id);

    return 0;
}
