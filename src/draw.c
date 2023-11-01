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
 * \brief Draw the grid on a GTK widget using Cairo
 * \param widget The GTK widget
 * \param cr The Cairo context for drawing
 * \param data The data associated with the drawing event
 * \return FALSE to indicate that no further drawing is needed
 */
gboolean draw_board(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    // Get the current color
    GdkRGBA color;

    // Get the size of the drawing area
    guint width = gtk_widget_get_allocated_width(widget);
    guint height = gtk_widget_get_allocated_height(widget);

    // Calculate cell size
    double cell_width = (double)width / X;
    double cell_height = (double)height / Y;

    // Disable antialiasing
    cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);

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
