# See LICENSE file for copyright and license details.
# Copyright (C) 2023-2024  Henrique Almeida <me@h3nc4.com>

import ctypes
import argparse
import sys
import os

os.environ["PYGAME_HIDE_SUPPORT_PROMPT"] = "1"
import pygame

parser = argparse.ArgumentParser(description="Conway's Game of Life")
parser.add_argument(
	"--alive-color",
	type=str,
	default="255,255,255",
	help="Color of living cells in RGB format, e.g., '255,255,255' (default: black)",
)
parser.add_argument(
	"--dead-color",
	type=str,
	default="0,0,0",
	help="Color of dead cells (background) in RGB format, e.g., '0,0,0' (default: white)",
)
parser.add_argument(
	"--cell-size", type=int, default=8, help="Size of each cell in pixels (default: 8)"
)
args = parser.parse_args()


# Convert the RGB strings into tuples
ALIVE_COLOR = tuple(map(int, args.alive_color.split(",")))
DEAD_COLOR = tuple(map(int, args.dead_color.split(",")))
CELL_SIZE = args.cell_size

# Load the shared library
game_of_life = ctypes.CDLL("./game_of_life.so")
game_of_life.initialize_game.argtypes = [ctypes.c_int, ctypes.c_int]
game_of_life.initialize_game.restype = None
game_of_life.update_grid.argtypes = []
game_of_life.update_grid.restype = None
game_of_life.get_grid.argtypes = []
game_of_life.get_grid.restype = ctypes.POINTER(ctypes.c_int)
game_of_life.flip_cell_state.argtypes = [ctypes.c_int, ctypes.c_int]
game_of_life.flip_cell_state.restype = None
game_of_life.free_grids.argtypes = []
game_of_life.free_grids.restype = None

# Pygame setup
pygame.display.init()
info = pygame.display.Info()
screen = pygame.display.set_mode((info.current_w, info.current_h), pygame.FULLSCREEN)
clock = pygame.time.Clock()

# Convert colors to the pixel format used by pygame
ALIVE_PIXEL = (ALIVE_COLOR[0] << 16) | (ALIVE_COLOR[1] << 8) | ALIVE_COLOR[2]
DEAD_PIXEL = (DEAD_COLOR[0] << 16) | (DEAD_COLOR[1] << 8) | DEAD_COLOR[2]

# Define the width and height based on the cell size and screen size
WIDTH, HEIGHT = info.current_w // CELL_SIZE, info.current_h // CELL_SIZE
paused = False
mouse_dragging = False
last_selected_cell = None

surface = pygame.Surface((WIDTH, HEIGHT))
pixels = pygame.surfarray.pixels2d(surface)
grid_array = None  # Cache the grid array


def draw_grid():
	"""Draw cells based on their current state."""
	global grid_array
	if grid_array is None:
		grid_ptr = game_of_life.get_grid()
		grid_array = ctypes.cast(
			grid_ptr, ctypes.POINTER(ctypes.c_int * (WIDTH * HEIGHT))
		).contents
	for y in range(HEIGHT):
		for x in range(WIDTH):
			state = grid_array[y * WIDTH + x]
			pixels[x, y] = ALIVE_PIXEL if state == 1 else DEAD_PIXEL
	screen.blit(
		pygame.transform.scale(surface, (info.current_w, info.current_h)), (0, 0)
	)


def handle_mouse_click_or_drag():
	"""Handle mouse input for toggling cell states."""
	global last_selected_cell
	mx, my = pygame.mouse.get_pos()
	x, y = mx // CELL_SIZE, my // CELL_SIZE
	if 0 <= x < WIDTH and 0 <= y < HEIGHT and last_selected_cell != (x, y):  # Only toggle if this is a new cell
		game_of_life.flip_cell_state(y, x)
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
try:
	game_of_life.initialize_game(WIDTH, HEIGHT)
	while running:
		handle_events()  # Process events
		if not paused:
			game_of_life.update_grid()
			clock.tick(10)  # Limit to 10 FPS when running
		else:
			clock.tick(60)  # Increase to 60 FPS when paused for smoother interaction
		draw_grid()  # Draw cells
		pygame.display.flip()  # Update display
finally:
	print("\nExiting Game of Life. Goodbye!")
	game_of_life.free_grids()  # Free memory allocated for grids
	pygame.quit()
	sys.exit()
