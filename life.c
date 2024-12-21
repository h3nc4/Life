/* See LICENSE file for copyright and license details.
 * Copyright (C) 2023-2024  Henrique Almeida <me@h3nc4.com> */

#define UPDATE_FPS 10 // Frames per second for grid update
#define FPS 60		  // Frames per second for rendering

#include <X11/Xutil.h>
#include <X11/extensions/Xinerama.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

static unsigned int CELL_SIZE = 10; // Default size of each cell in pixels

// Type definitions
typedef uint8_t u8int;

typedef struct
{
	u8int grid_state;	 // Define grid currently being updated
	unsigned int width;	 // Grid width
	unsigned int height; // Grid height
	unsigned int size;	 // Total grid size (width * height)
	bool running;
	bool paused;
	int last_triggered_x;
	int last_triggered_y;
	bool mouse_pressed; // Moved mouse_pressed into the struct
} rules;

// Global grid and its pointers
static u8int *grid = NULL;
static u8int *current_grid = NULL;
static u8int *prev_grid = NULL;
// This array stores the offset of each row in the grid
static unsigned int *row_offsets = NULL;

static rules game = {
	.running = true,
	.paused = false,
	.last_triggered_x = -1,
	.last_triggered_y = -1,
	.mouse_pressed = false,
};

// Global X11 variables
static Display *display = NULL;
static Window window;
static Pixmap pixmap;
static GC gc;
static XRectangle *rectangles = NULL;

#ifdef DEBUG_FPS_LOGGING
unsigned int frame_count = 0;
unsigned long long fps_timer_start = 0;
#endif

#define MAX_RECTANGLES_PER_BATCH 100000

// Macros for index calculation and neighbor counting
#define PREV_Y(y) (((y) - 1 + game.height) % game.height)
#define NEXT_Y(y) (((y) + 1) % game.height)
#define PREV_X(x) (((x) - 1 + game.width) % game.width)
#define NEXT_X(x) (((x) + 1) % game.width)
#define COUNT_LIVING_NEIGHBORS(y, x)                 \
	(prev_grid[row_offsets[PREV_Y(y)] + PREV_X(x)] + \
	 prev_grid[row_offsets[PREV_Y(y)] + (x)] +       \
	 prev_grid[row_offsets[PREV_Y(y)] + NEXT_X(x)] + \
	 prev_grid[row_offsets[(y)] + PREV_X(x)] +       \
	 prev_grid[row_offsets[(y)] + NEXT_X(x)] +       \
	 prev_grid[row_offsets[NEXT_Y(y)] + PREV_X(x)] + \
	 prev_grid[row_offsets[NEXT_Y(y)] + (x)] +       \
	 prev_grid[row_offsets[NEXT_Y(y)] + NEXT_X(x)])
// A cell becomes alive if it has exactly 3 neighbors (birth)
// A cell stays alive if it is currently alive and has exactly 2 neighbors (survival)
// Otherwise, it dies.
#define IS_ALIVE(living_neighbors, cell) ((living_neighbors == 3) | (cell & (living_neighbors == 2)))

static void free_grid()
{
	free(grid);
	free(row_offsets);
	free(rectangles);
}

// Aligned allocation for SIMD
static void allocate_grid()
{
	grid = aligned_alloc(32, 2 * game.size * sizeof(u8int));
	row_offsets = malloc(game.height * sizeof(unsigned int));
	if (!grid || !row_offsets)
	{
		fprintf(stderr, "Error: Failed to allocate memory.\n");
		exit(EXIT_FAILURE);
	}
	rectangles = malloc(game.size * sizeof(XRectangle));
	if (rectangles == NULL)
	{
		fprintf(stderr, "Error: Failed to allocate memory for rectangles.\n");
		exit(EXIT_FAILURE);
	}
}

static void init_grid()
{
	srand(time(NULL));
	for (unsigned int i = 0; i < game.size; i++)
		grid[i] = rand() % 2;
	memcpy(grid + game.size, grid, game.size * sizeof(u8int));
	current_grid = grid;

	// Precompute row offsets
	for (unsigned int y = 0; y < game.height; y++)
		row_offsets[y] = y * game.width;
}

static void update_grid()
{
	current_grid = grid + game.size * game.grid_state;
	prev_grid = grid + game.size * (game.grid_state ^ 1);
#pragma omp parallel for schedule(guided) // Parallelize loop
	for (unsigned int idx = 0; idx < game.size; idx++)
		current_grid[idx] = IS_ALIVE(COUNT_LIVING_NEIGHBORS(idx / game.width, idx % game.width), prev_grid[idx]);
	game.grid_state ^= 1;
}

static void init_rules(unsigned int w, unsigned int h)
{
	if (grid)
		free_grid();
	game.width = w;
	game.height = h;
	game.size = game.width * game.height;
	allocate_grid();
	init_grid();
}

static void clear_grid()
{
	memset(grid, 0, 2 * game.size * sizeof(u8int));
}

static void restart_game()
{
	init_grid();
}

static int toggle_cell_state(unsigned int y, unsigned int x)
{
	if (y >= game.height || x >= game.width)
		return 1;
	if (game.last_triggered_x == x && game.last_triggered_y == y)
		return 0;
	current_grid[row_offsets[y] + x] ^= 1; // Toggle between 0 and 1
	game.last_triggered_x = x;
	game.last_triggered_y = y;
	return 0;
}

static void draw_grid()
{
	XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
	XFillRectangle(display, pixmap, gc, 0, 0, game.width * CELL_SIZE, game.height * CELL_SIZE);
	XSetForeground(display, gc, WhitePixel(display, DefaultScreen(display)));
	int rect_count = 0;
	for (unsigned int i = 0; i < game.size; i++)
		if (current_grid[i])
		{
			rectangles[rect_count].x = (i % game.width) * CELL_SIZE;
			rectangles[rect_count].y = (i / game.width) * CELL_SIZE;
			rectangles[rect_count].width = CELL_SIZE;
			rectangles[rect_count].height = CELL_SIZE;
			rect_count++;
			if (rect_count >= MAX_RECTANGLES_PER_BATCH)
			{
				XFillRectangles(display, pixmap, gc, rectangles, rect_count);
				rect_count = 0;
			}
		}
	if (rect_count > 0)
		XFillRectangles(display, pixmap, gc, rectangles, rect_count);
}

static void handle_key_press(KeySym key)
{
	switch (key)
	{
	case XK_q: // Quit
		game.running = false;
		break;
	case XK_space: // Pause
		game.paused = !game.paused;
		break;
	case XK_r: // Restart
		restart_game();
		break;
	case XK_c: // Clear
		clear_grid();
		break;
	}
}

static void handle_events()
{
	XEvent event;
	while (XPending(display))
	{
		XNextEvent(display, &event);
		if (event.type == Expose)
			XCopyArea(display, pixmap, window, gc, 0, 0, game.width * CELL_SIZE, game.height * CELL_SIZE, 0, 0);
		else if (event.type == KeyPress)
			handle_key_press(XLookupKeysym(&event.xkey, 0));
		else if (event.type == ButtonPress)
		{
			game.mouse_pressed = true;
			toggle_cell_state(event.xbutton.y / CELL_SIZE, event.xbutton.x / CELL_SIZE);
		}
		else if (event.type == MotionNotify && game.mouse_pressed)
			toggle_cell_state(event.xmotion.y / CELL_SIZE, event.xmotion.x / CELL_SIZE);
		else if (event.type == ButtonRelease)
		{
			game.mouse_pressed = false;
			game.last_triggered_x = -1;
			game.last_triggered_y = -1;
		}
	}
}

static void start_game()
{
	const unsigned int render_interval = 1000000 / FPS;
	const unsigned int update_interval = 1000000 / UPDATE_FPS;
	unsigned long long last_render_time = 0;
	unsigned long long last_update_time = 0;
	struct timespec current_time;

#ifdef DEBUG_FPS_LOGGING
	struct timespec start_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	fps_timer_start = start_time.tv_sec;
#endif

	while (game.running)
	{
		clock_gettime(CLOCK_MONOTONIC, &current_time);
		unsigned long long now = current_time.tv_sec * 1000000LL + current_time.tv_nsec / 1000;
		handle_events();
		if (!game.paused && now - last_update_time >= update_interval)
		{
			update_grid();
			last_update_time = now;
		}
		if (now - last_render_time >= render_interval)
		{
			draw_grid();
			XCopyArea(display, pixmap, window, gc, 0, 0, game.width * CELL_SIZE, game.height * CELL_SIZE, 0, 0);

#ifdef DEBUG_FPS_LOGGING
			frame_count++;
			if (now / 1000000LL - fps_timer_start >= 1)
			{
				fprintf(stdout, "FPS: %u\n", frame_count);
				frame_count = 0;
				fps_timer_start = now / 1000000LL;
			}
#endif

			last_render_time = now;
		}
		usleep(1000); // Sleep for 1 ms to avoid busy waiting
	}
}

static void set_fullscreen()
{
	Atom wm_state = XInternAtom(display, "_NET_WM_STATE", False);
	Atom fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
	XEvent xev = {0};
	xev.xclient.type = ClientMessage;
	xev.xclient.window = window;		 // Set the window this message applies to
	xev.xclient.message_type = wm_state; // Set the message type to the "_NET_WM_STATE" atom
	xev.xclient.format = 32;			 // Set the data format to 32-bit
	xev.xclient.data.l[0] = 1;			 // Add the fullscreen state
	xev.xclient.data.l[1] = fullscreen;	 // Set the second data element to the fullscreen atom
	xev.xclient.data.l[2] = 0;			 // Source indication is the application itself
	XSendEvent(display, DefaultRootWindow(display), False, SubstructureNotifyMask | SubstructureRedirectMask, &xev);
}

static int *get_screen_size()
{
	int screen = DefaultScreen(display);
	static int screen_size[2];
	XineramaScreenInfo *screen_info;
	if (XineramaIsActive(display))
	{
		int screen_count;
		screen_info = XineramaQueryScreens(display, &screen_count);
		screen_size[0] = screen_info[0].width;
		screen_size[1] = screen_info[0].height;
		XFree(screen_info);
	}
	else
	{
		screen_size[0] = DisplayWidth(display, screen);
		screen_size[1] = DisplayHeight(display, screen);
	}
	return screen_size;
}

static Display *get_display()
{
	Display *display = XOpenDisplay(getenv("DISPLAY"));
	if (!display)
	{
		fprintf(stderr, "Error: Unable to open X display.\n");
		exit(EXIT_FAILURE);
	}
	return display;
}

static void init_game()
{
	display = get_display();
	int *screen_size = get_screen_size();
	int screen = DefaultScreen(display);
	window = XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0, screen_size[0], screen_size[1], 1, WhitePixel(display, screen), BlackPixel(display, screen));
	XSelectInput(display, window, ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
	XMapWindow(display, window);
	set_fullscreen();
	pixmap = XCreatePixmap(display, window, screen_size[0], screen_size[1], DefaultDepth(display, screen));
	gc = XCreateGC(display, window, 0, NULL);
	XSetForeground(display, gc, WhitePixel(display, screen));
	init_rules(screen_size[0] / CELL_SIZE, screen_size[1] / CELL_SIZE);
}

static void free_x11()
{
	XFreePixmap(display, pixmap);
	XFreeGC(display, gc);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
}

int main(int argc, char *argv[])
{
#ifdef DEBUG_FPS_LOGGING
	setbuf(stdout, NULL);
#endif
	if (argc == 2)
	{
		CELL_SIZE = atoi(argv[1]);
		if (CELL_SIZE <= 0)
		{
			fprintf(stderr, "Error: Cell size must be positive.\n");
			return EXIT_FAILURE;
		}
	}
	init_game();
	start_game();
	free_x11();
	free_grid();
	return EXIT_SUCCESS;
}
