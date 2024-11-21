# See LICENSE file for copyright and license details.
# Copyright (C) 2023-2024  Henrique Almeida <me@h3nc4.com>

import ctypes
import pygame
import sys

# Load the shared library
game_of_life = ctypes.CDLL("./game_of_life.so")
game_of_life.initialize_game.argtypes = [ctypes.c_int, ctypes.c_int]
game_of_life.initialize_game.restype = None
game_of_life.update_grid.argtypes = []
game_of_life.update_grid.restype = None
game_of_life.get_cell_state.argtypes = [ctypes.c_int, ctypes.c_int]
game_of_life.get_cell_state.restype = ctypes.c_int
game_of_life.set_cell_state.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int]
game_of_life.set_cell_state.restype = None

# Pygame setup
pygame.display.init()
info = pygame.display.Info()
screen = pygame.display.set_mode((info.current_w, info.current_h), pygame.FULLSCREEN)
clock = pygame.time.Clock()

ALIVE_COLOR = (0, 0, 0)  # Living cells
DEAD_COLOR = (255, 255, 255)  # Dead cells
CELL_SIZE = 8  # Pixels
WIDTH, HEIGHT = info.current_w // CELL_SIZE, info.current_h // CELL_SIZE
paused = False
mouse_dragging = False


def draw_grid():
	"""Draw cells based on the current state."""
	for y in range(HEIGHT):
		for x in range(WIDTH):
			state = game_of_life.get_cell_state(y, x)
			color = ALIVE_COLOR if state == 1 else DEAD_COLOR
			pygame.draw.rect(
				screen, color, (x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE)
			)


def handle_mouse_click_or_drag():
	"""Handle mouse input for toggling cell states."""
	mx, my = pygame.mouse.get_pos()
	x, y = mx // CELL_SIZE, my // CELL_SIZE
	if 0 <= x < WIDTH and 0 <= y < HEIGHT:
		new_state = 1 - game_of_life.get_cell_state(y, x)  # Flip between alive and dead
		game_of_life.set_cell_state(y, x, new_state)  # Update


def handle_events():
	"""Process all Pygame events."""
	global running, paused, mouse_dragging
	for event in pygame.event.get():
		if event.type == pygame.QUIT:
			running = False
		elif event.type == pygame.KEYDOWN:
			if event.key == pygame.K_SPACE:
				paused = not paused  # Toggle pause state on space key press
			elif event.key == pygame.K_q or event.key == pygame.K_ESCAPE:
				running = False  # Quit on `q` or `Esc`
		elif event.type == pygame.MOUSEBUTTONDOWN:
			if event.button == 1:  # Left click
				handle_mouse_click_or_drag()
				mouse_dragging = True
		elif event.type == pygame.MOUSEBUTTONUP:
			if event.button == 1:  # Left button released
				mouse_dragging = False
		elif event.type == pygame.MOUSEMOTION:
			if mouse_dragging:  # If dragging with left button
				handle_mouse_click_or_drag()


running = True
try:
	game_of_life.initialize_game(WIDTH, HEIGHT)
	while running:
		screen.fill(DEAD_COLOR)  # Fill the screen with the dead cell color
		draw_grid()  # Draw the grid based on the current state
		handle_events()  # Process events
		if not paused:
			game_of_life.update_grid()
		pygame.display.flip()  # Update the display
		clock.tick(10)  # Limit the frame rate to 10 FPS
finally:
	print("\nExiting Game of Life. Goodbye!")
	pygame.quit()
	sys.exit()
