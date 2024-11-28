# See LICENSE file for copyright and license details.
# Copyright (C) 2023-2024  Henrique Almeida <me@h3nc4.com>

from ctypes import CDLL, c_uint, c_ubyte, POINTER, cast
from traceback import print_exc
from argparse import ArgumentParser
from os import getenv, environ
from platform import system
import numpy
import time

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
ENGINE = CDLL("./engine.dll" if system() == "Windows" else "./engine.so")
ENGINE.initialize_game.argtypes = [c_uint, c_uint]
ENGINE.initialize_game.restype = None
ENGINE.update_grid.argtypes = []
ENGINE.update_grid.restype = None
ENGINE.ptr_to_both_grids.argtypes = []
ENGINE.ptr_to_both_grids.restype = POINTER(c_ubyte)
ENGINE.toggle_cell_state.argtypes = [c_uint, c_uint]
ENGINE.toggle_cell_state.restype = None
ENGINE.free_grid.argtypes = []
ENGINE.free_grid.restype = None
ENGINE.clear_grid.argtypes = []
ENGINE.clear_grid.restype = None
ENGINE.restart_game.argtypes = []
ENGINE.restart_game.restype = None

# Pygame setup
pygame.display.init()
DISPLAY_INFO = pygame.display.Info()
flags = pygame.FULLSCREEN | pygame.HWSURFACE | pygame.DOUBLEBUF
try:
    GAME_SCREEN = pygame.display.set_mode((DISPLAY_INFO.current_w, DISPLAY_INFO.current_h), flags)
except pygame.error:  # Fallback to software rendering if hardware acceleration fails
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

SCALED_SURFACE = pygame.Surface((DISPLAY_INFO.current_w, DISPLAY_INFO.current_h))
GAME_SURFACE = pygame.Surface((WIDTH, HEIGHT), depth=32)
GAME_PIXELS = pygame.surfarray.pixels2d(GAME_SURFACE)
ENGINE.initialize_game(WIDTH, HEIGHT)

# This is a pointer to a pointer to the current generation grid
# To access the it, its value must be dereferenced twice
# Then, it must be cast to a 2D array
GRIDS = cast(ENGINE.ptr_to_both_grids(), POINTER(POINTER(c_ubyte * (WIDTH * HEIGHT)))).contents
GAME_MATRIX1 = numpy.array(GRIDS[1], copy=False).reshape(WIDTH, HEIGHT)
GAME_MATRIX2 = numpy.array(GRIDS[0], copy=False).reshape(WIDTH, HEIGHT)
COLOR_LUT = numpy.array([DEAD_PIXEL, ALIVE_PIXEL], dtype=numpy.uint32)

def draw_grid():
	"""
	Draws cells based on their current state.

	Uses numpy to directly manipulate the pixel array in a shared memory buffer.
	"""
	GAME_PIXELS[:, :] = COLOR_LUT[GAME_MATRIX1]
	pygame.transform.scale(GAME_SURFACE, (DISPLAY_INFO.current_w, DISPLAY_INFO.current_h), SCALED_SURFACE)
	GAME_SCREEN.blit(SCALED_SURFACE, (0, 0))


def handle_mouse_click_or_drag():
	"""Handle mouse input for toggling cell states."""
	global last_selected_cell
	mx, my = pygame.mouse.get_pos()
	x, y = mx // CELL_SIZE, my // CELL_SIZE
	if (0 <= x < WIDTH and 0 <= y < HEIGHT and last_selected_cell != (x, y)):  # Only toggle if this is a new cell
		ENGINE.toggle_cell_state(x, y)
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
			elif event.key == pygame.K_r:
				ENGINE.restart_game()  # Restart the game on `r` key press
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


def pyrules():
	pygame.event.set_allowed([pygame.QUIT, pygame.KEYDOWN, pygame.MOUSEBUTTONDOWN, pygame.MOUSEBUTTONUP, pygame.MOUSEMOTION])


def debug_print(*args, **kwargs):
	"""Print debug messages if the DEBUG environment variable is set."""
	if getenv("DEBUG"):
		print(*args, **kwargs)


start_time = time.time()
frame_count = 0
running = True
last_update_time = pygame.time.get_ticks()
update_interval = 100
pyrules()
try:
	while running:
		current_time = pygame.time.get_ticks()  # Define current_time at the start of the loop
		handle_events()  # Process events
		if current_time - last_update_time >= update_interval and not paused:
			update_time = time.time()
			GAME_MATRIX1, GAME_MATRIX2 = GAME_MATRIX2, GAME_MATRIX1  # Swap the grids
			ENGINE.update_grid()
			debug_print(f"Update time: {time.time() - update_time}")
			last_update_time = current_time
		draw_grid_time = time.time()
		draw_grid()  # Draw cells
		debug_print(f"Draw grid time: {time.time() - draw_grid_time}")
		display_time = time.time()
		pygame.display.flip()  # Update display
		debug_print(f"Display time: {time.time() - display_time}")
		GAME_CLOCK.tick(60)  # Run at 60 FPS
		if not getenv("DEBUG"):
			continue
		frame_count += 1
		current_time = time.time()
		if current_time - start_time >= 1.0:  # Every second
			print(f"FPS: {frame_count}")
			frame_count = 0
			start_time = current_time
except Exception:
	if getenv("DEBUG"):
		print_exc()  # Print the full exception traceback
finally:
	print("\nExiting Game of Life. Goodbye!")
	ENGINE.free_grid()  # Free memory allocated for grids
	pygame.quit()
