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

// Iterators
int i, j;

// Declare the boolean board
gboolean **grid;

// Boolean value to indicate if the game is paused
gboolean game_paused = TRUE;

/**
 * \brief Callback function for the timer to update the board
 * \param data The data associated with the timer
 * \return G_SOURCE_CONTINUE to indicate that the timer should continue running
 */
gboolean timer_callback(gpointer data)
{
    if (game_paused)
    {
        show_cursor();
    }
    else
    {
        hide_cursor();
        next_generation();
        gtk_widget_queue_draw(GTK_WIDGET(data));
    }
    return G_SOURCE_CONTINUE;
}

/**
 * \brief Callback function for the key press event
 * \param widget The GTK widget
 * \param event The GDK key event
 * \param user_data Additional user data
 */
void on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
    if (event->keyval == GDK_KEY_space)
    {
        game_paused = !game_paused;
    }
    else if (event->keyval == GDK_KEY_Escape)
    {
        show_cursor();
        free_grid();
        gtk_main_quit(); // quit the app
    }
}

/**
 * \brief The main function of the application
 * \param argc The number of command-line arguments
 * \param argv The array of command-line arguments
 * \return The exit code for the app
 */
int main(int argc, char *argv[])
{
    initialize_grid();

    // init GTK
    gtk_init(&argc, &argv);

    // create the main window
    GtkWidget *window = create_main_window();
    gtk_window_set_title(GTK_WINDOW(window), "Conway's Game of Life");
    gtk_container_set_border_width(GTK_CONTAINER(window), 0);

    // Set the window to be fullscreen
    gtk_window_fullscreen(GTK_WINDOW(window));

    // connect the destroy event to gtk_main_quit
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // create a drawing area for the grid
    GtkWidget *grid = gtk_drawing_area_new();
    gtk_widget_set_size_request(grid, X * 5, Y * 10);
    // Create a grid layout container and add the drawing area to it
    GtkWidget *grid_container = gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(grid_container), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid_container), TRUE);
    gtk_grid_attach(GTK_GRID(grid_container), grid, 0, 0, 1, 1);

    // Add the grid container to the main window
    gtk_container_add(GTK_CONTAINER(window), grid_container);

    // connect the draw event
    g_signal_connect(grid, "draw", G_CALLBACK(draw_board), NULL);

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
