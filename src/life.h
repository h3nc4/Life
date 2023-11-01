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

// constants
#define X 120
#define Y 60

// Iterators
extern int i, j;

// Define the boolean board
// extern gboolean grid[X][Y];
extern gboolean **grid;

// Prototypes for game.c
void next_generation(void);
int get_adjacent_cells(void);

// Prototypes for util.c
void initialize_grid();
void free_grid();
void hide_cursor();
void show_cursor();
GtkWidget *create_main_window();

// Prototypes for draw.c
void toggle_cell(GtkWidget *widget, gpointer data);
gboolean draw_board(GtkWidget *widget, cairo_t *cr, gpointer data);

// Prototypes for life.c
gboolean timer_callback(gpointer data);
void on_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
int main(int argc, char *argv[]);
