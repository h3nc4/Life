# See LICENSE file for copyright and license details.
# Copyright (C) 2023-2024  Henrique Almeida <me@h3nc4.com>

from ctypes import CDLL, c_uint, c_ubyte, POINTER, cast
from traceback import print_exc
from argparse import ArgumentParser
from os import getenv, environ

environ["PYGAME_HIDE_SUPPORT_PROMPT"] = "1"
import pygame

parser = ArgumentParser(description="Conway's Game of Life")
parser.add_argument(
	"--alive-color",
	type=lambda s: tuple(map(int, s.split(","))),
	default=(255, 255, 255),
	help="Color of living cells in RGB format, e.g., '255,255,255' (default: white)",
)
parser.add_argument(
	"--dead-color",
	type=lambda s: tuple(map(int, s.split(","))),
	default=(0, 0, 0),
	help="Color of dead cells (background) in RGB format, e.g., '0,0,0' (default: black)",
)
parser.add_argument(
	"--cell-size", type=int, default=8, help="Size of each cell in pixels (default: 8)"
)
args = parser.parse_args()


# Set easy-to-use variables for colors and cell size
ALIVE_COLOR = args.alive_color
DEAD_COLOR = args.dead_color
CELL_SIZE = args.cell_size

# Load the shared library
ENGINE = CDLL("./engine.so")
ENGINE.initialize_game.argtypes = [c_uint, c_uint]
ENGINE.initialize_game.restype = None
ENGINE.update_grid.argtypes = []
ENGINE.update_grid.restype = None
ENGINE.ptr_to_current_grid.argtypes = []
ENGINE.ptr_to_current_grid.restype = POINTER(POINTER(c_ubyte))
ENGINE.toggle_cell_state.argtypes = [c_uint, c_uint]
ENGINE.toggle_cell_state.restype = None
ENGINE.free_grids.argtypes = []
ENGINE.free_grids.restype = None
ENGINE.clear_grid.argtypes = []
ENGINE.clear_grid.restype = None

# Pygame setup
pygame.display.init()
DISPLAY_INFO = pygame.display.Info()
GAME_SCREEN = pygame.display.set_mode((DISPLAY_INFO.current_w, DISPLAY_INFO.current_h), pygame.FULLSCREEN)
GAME_CLOCK = pygame.time.Clock()

# Convert colors to the pixel format used by pygame
ALIVE_PIXEL = (ALIVE_COLOR[0] << 16) | (ALIVE_COLOR[1] << 8) | ALIVE_COLOR[2]
DEAD_PIXEL = (DEAD_COLOR[0] << 16) | (DEAD_COLOR[1] << 8) | DEAD_COLOR[2]

# Define the width and height based on the cell size and screen size
WIDTH, HEIGHT = DISPLAY_INFO.current_w // CELL_SIZE, DISPLAY_INFO.current_h // CELL_SIZE
paused = False
mouse_dragging = False
last_selected_cell = None

GAME_SURFACE = pygame.Surface((WIDTH, HEIGHT))
GAME_PIXELS = pygame.surfarray.pixels2d(GAME_SURFACE)
ENGINE.initialize_game(WIDTH, HEIGHT)
GRIDS_PTR_PTR = cast(ENGINE.ptr_to_current_grid(), POINTER(POINTER(c_ubyte * (WIDTH * HEIGHT)))).contents


def draw_grid():
	"""Draw cells based on their current state.

	GRIDS_PTR_PTR is a pointer to a pointer to the current generation grid.

	To access it, we need to dereference it twice.

	First, we dereference the pointer to the pointer with [0], then we dereference the pointer to the grid and access it with [y * WIDTH + x].
	"""
	for y in range(HEIGHT):
		for x in range(WIDTH):
			state = GRIDS_PTR_PTR[0][y * WIDTH + x]  # Access directly from pointer
			GAME_PIXELS[x, y] = ALIVE_PIXEL if state == 1 else DEAD_PIXEL
	GAME_SCREEN.blit(
		pygame.transform.scale(GAME_SURFACE, (DISPLAY_INFO.current_w, DISPLAY_INFO.current_h)), (0, 0)
	)


def handle_mouse_click_or_drag():
	"""Handle mouse input for toggling cell states."""
	global last_selected_cell
	mx, my = pygame.mouse.get_pos()
	x, y = mx // CELL_SIZE, my // CELL_SIZE
	if 0 <= x < WIDTH and 0 <= y < HEIGHT and last_selected_cell != (x, y):  # Only toggle if this is a new cell
		ENGINE.toggle_cell_state(y, x)
		last_selected_cell = (x, y)


def handle_events():
	"""Process all Pygame events."""
	global running, paused, mouse_dragging, last_selected_cell
	for event in pygame.event.get():
		if event.type == pygame.QUIT:
			running = False
		elif event.type == pygame.KEYDOWN:
			if event.key == pygame.K_SPACE:
				paused = not paused  # Toggle pause state on space key press
			elif event.key in (pygame.K_q, pygame.K_ESCAPE):
				running = False  # Quit on `q` or `Esc`
			elif event.key == pygame.K_c:
				ENGINE.clear_grid()  # Clear the grid on `c` key press
		elif event.type == pygame.MOUSEBUTTONDOWN:
			if event.button == 1:  # Left click
				handle_mouse_click_or_drag()
				mouse_dragging = True
		elif event.type == pygame.MOUSEBUTTONUP:
			if event.button == 1:  # Left button released
				mouse_dragging = False
				last_selected_cell = None  # Reset the last toggled cell
		elif event.type == pygame.MOUSEMOTION:
			if mouse_dragging:  # If dragging with left button
				handle_mouse_click_or_drag()


running = True
last_update_time = pygame.time.get_ticks()
update_interval = 100
try:
	while running:
		handle_events()  # Process events
		current_time = pygame.time.get_ticks()
		if current_time - last_update_time >= update_interval and not paused:
			ENGINE.update_grid()
			last_update_time = current_time  # Update the last update time
		draw_grid()  # Draw cells
		pygame.display.flip()  # Update display
		GAME_CLOCK.tick(60)  # Run at 60 FPS
except Exception:
	if getenv("DEBUG"):
		print_exc()  # Print the full exception traceback
finally:
	print("\nExiting Game of Life. Goodbye!")
	ENGINE.free_grids()  # Free memory allocated for grids
	pygame.quit()
