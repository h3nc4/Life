/******************************************************************************
	Copyright (C) 2023-2024  Henrique Almeida <me@h3nc4.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <https://www.gnu.org/licenses/>.
******************************************************************************/

#define UPDATE_FPS 10 // Frames per second for grid update
#define FPS 60		  // Frames per second for rendering
#define FPS_THRESHOLD 3.5
#define MAX_RECTANGLES_PER_BATCH 100000
#define RENDER_INTERVAL 1000000 / FPS * (100 - FPS_THRESHOLD) / 100
#define UPDATE_INTERVAL 1000000 / UPDATE_FPS * (100 - FPS_THRESHOLD) / 100

#include <X11/Xutil.h>
#include <X11/extensions/Xinerama.h>
#include <X11/extensions/XShm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/shm.h>

static unsigned int CELL_SIZE = 10; // Default size of each cell in pixels

// Type definitions
typedef uint8_t u8int;
typedef unsigned long long ull;

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
static GC gc;
static XShmSegmentInfo shm_info;
static XImage *ximage = NULL;

#ifdef FPS_LOGGING
unsigned int frame_count = 0;
unsigned int update_count = 0;
ull fps_timer_start = 0;
#endif

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

static void free_game()
{
	if (ximage)
	{
		XDestroyImage(ximage);
		ximage = NULL;
	}
	XShmDetach(display, &shm_info);
	if (shm_info.shmaddr)
	{
		shmdt(shm_info.shmaddr);
		shm_info.shmaddr = NULL;
	}
	if (grid)
	{
		free(grid);
		grid = NULL;
	}
	if (row_offsets)
	{
		free(row_offsets);
		row_offsets = NULL;
	}
	XFreeGC(display, gc);
	XDestroyWindow(display, window);
	XCloseDisplay(display);
}

static void init_grid()
{
	grid = aligned_alloc(32, 2 * game.size * sizeof(u8int));
	row_offsets = malloc(game.height * sizeof(unsigned int));
	if (!grid || !row_offsets)
	{
		fprintf(stderr, "Error: Failed to allocate memory.\n");
		exit(EXIT_FAILURE);
	}
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
	#ifdef UPDATE_CALC_LOGGING
	struct timespec start_time, end_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	#endif
	#pragma omp parallel for schedule(static) // Parallelize loop
	for (unsigned int idx = 0; idx < game.size; idx++)
		current_grid[idx] = IS_ALIVE(COUNT_LIVING_NEIGHBORS(idx / game.width, idx % game.width), prev_grid[idx]);
	game.grid_state ^= 1;
	#ifdef UPDATE_CALC_LOGGING
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	printf("Update time: %ld ns\n", (end_time.tv_sec - start_time.tv_sec) * 1000000L + (end_time.tv_nsec - start_time.tv_nsec) / 1000L);
	#endif
	#ifdef FPS_LOGGING
	update_count++;
	#endif
}

static void init_rules(unsigned int w, unsigned int h)
{
	if (grid)
		free_game();
	game.width = w;
	game.height = h;
	game.size = game.width * game.height;
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

static inline void draw(int i)
{
	for (unsigned int cy = 0; cy < CELL_SIZE; cy++)
		for (unsigned int cx = 0; cx < CELL_SIZE; cx++)
			XPutPixel(ximage, (i % game.width) * CELL_SIZE + cx, (i / game.width) * CELL_SIZE + cy, WhitePixel(display, DefaultScreen(display)));
}

static void draw_grid()
{
	memset(ximage->data, 0, ximage->bytes_per_line * ximage->height);
	#ifdef UPDATE_DRAW_LOGGING
	struct timespec start_time, end_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	#endif
	#pragma omp parallel for schedule(static) // Parallelize loop
	for (unsigned int i = 0; i < game.size; i++)
		if (current_grid[i])
			draw(i);
	#ifdef UPDATE_DRAW_LOGGING
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	printf("Draw time: %ld ns\n", (end_time.tv_sec - start_time.tv_sec) * 1000000L + (end_time.tv_nsec - start_time.tv_nsec) / 1000L);
	#endif
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
			XShmPutImage(display, window, gc, ximage, 0, 0, 0, 0, game.width * CELL_SIZE, game.height * CELL_SIZE, False);
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

#ifdef FPS_LOGGING
static void debug_fps_logging(ull now)
{
	frame_count++;
	static bool first = true;
	if (now / 1000000LL - fps_timer_start >= 1)
	{
		if (first)
		{
			frame_count = 0;
			update_count = 0;
			fps_timer_start = now / 1000000LL;
			first = false;
			return;
		}
		fprintf(stdout, "FPS: %u | Updates: %u\n", frame_count, update_count);
		frame_count = 0;
		update_count = 0;
		fps_timer_start = now / 1000000LL;
	}
}
#endif

static void game_loop(struct timespec *current_time, ull *last_render, ull *last_update)
{
	clock_gettime(CLOCK_MONOTONIC, current_time);
	ull now = current_time->tv_sec * 1000000LL + current_time->tv_nsec / 1000;
	handle_events();
	if (!game.paused && now - *last_update >= UPDATE_INTERVAL)
	{
		update_grid();
		*last_update = now;
	}
	if (now - *last_render >= RENDER_INTERVAL)
	{
		draw_grid();
		XShmPutImage(display, window, gc, ximage, 0, 0, 0, 0, game.width * CELL_SIZE, game.height * CELL_SIZE, False);
		#ifdef FPS_LOGGING
		debug_fps_logging(now);
		#endif
		*last_render = now;
	}
	usleep(1000); // Sleep for 1 ms to avoid busy waiting
}

static void start_game()
{
	ull last_render = 0;
	ull last_update = 0;
	struct timespec current_time;

	#ifdef FPS_LOGGING
	struct timespec start_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	fps_timer_start = start_time.tv_sec;
	#endif

	while (game.running)
		game_loop(&current_time, &last_render, &last_update);
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

static void init_game()
{
	if (!(display = XOpenDisplay(getenv("DISPLAY"))))
		goto x_error;
	int screen = DefaultScreen(display);
	int *screen_size = get_screen_size();
	window = XCreateSimpleWindow(display, RootWindow(display, screen), 0, 0, screen_size[0], screen_size[1], 1, WhitePixel(display, screen), BlackPixel(display, screen));
	XSelectInput(display, window, ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
	XMapWindow(display, window);
	set_fullscreen();
	gc = XCreateGC(display, window, 0, NULL);
	XSetForeground(display, gc, WhitePixel(display, screen));
	init_rules(screen_size[0] / CELL_SIZE, screen_size[1] / CELL_SIZE);
	if (!(ximage = XShmCreateImage(display, DefaultVisual(display, screen), DefaultDepth(display, screen), ZPixmap, NULL, &shm_info, game.width * CELL_SIZE, game.height * CELL_SIZE)))
		goto x_error;
	if ((shm_info.shmid = shmget(IPC_PRIVATE, ximage->bytes_per_line * ximage->height, IPC_CREAT | 0600)) < 0 ||
		(shm_info.shmaddr = (char *)shmat(shm_info.shmid, NULL, 0)) == (char *)-1)
		goto x_error;
	shm_info.readOnly = False;
	ximage->data = shm_info.shmaddr;
	if (!XShmAttach(display, &shm_info))
		goto x_error;
	shmctl(shm_info.shmid, IPC_RMID, NULL);
	return;
x_error:
	perror("Error initializing X resources");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	#ifdef FPS_LOGGING
	setbuf(stdout, NULL);
	#endif
	if (argc == 2 && (CELL_SIZE = atoi(argv[1])) <= 0)
	{
		fprintf(stderr, "Error: Cell size must be positive.\n");
		return EXIT_FAILURE;
	}
	init_game();
	start_game();
	free_game();
	return EXIT_SUCCESS;
}
