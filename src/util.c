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
 * \brief Initialize the grid with random values
 */
void initialize_grid()
{
    srand(time(0));
    for (i = 0, j = 0; i < X; j = (j + 1) % Y, i += j == 0)
        grid[i][j] = rand() & 1;
}

/**
 * \brief Hide the mouse cursor
 * \details This function hides the mouse cursor by setting it to a blank cursor.
 */
void hide_cursor()
{
    GdkDisplay *display = gdk_display_get_default();
    GdkCursor *invisible_cursor = gdk_cursor_new_for_display(display, GDK_BLANK_CURSOR);
    gdk_window_set_cursor(gdk_get_default_root_window(), invisible_cursor);
    g_object_unref(invisible_cursor);
}

/**
 * \brief Show the mouse cursor
 * \details This function is called when on app closure to show the mouse cursor again.
 */
void show_cursor()
{
    GdkDisplay *display = gdk_display_get_default();
    GdkCursor *default_cursor = gdk_cursor_new_for_display(display, GDK_LEFT_PTR);
    gdk_window_set_cursor(gdk_get_default_root_window(), default_cursor);
    g_object_unref(default_cursor);
}

/**
 * \brief Create the main window for the app
 * \return The new window
 */
GtkWidget *create_main_window()
{
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Conway's Game of Life");
    gtk_container_set_border_width(GTK_CONTAINER(window), 0);
    gtk_window_fullscreen(GTK_WINDOW(window));
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    return window;
}
