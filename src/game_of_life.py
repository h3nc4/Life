# See LICENSE file for copyright and license details.
# Copyright (C) 2023-2024  Henrique Almeida <me@h3nc4.com>

import ctypes
import pygame

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
pygame.init()
info = pygame.display.Info()
screen = pygame.display.set_mode((info.current_w, info.current_h), pygame.FULLSCREEN)
clock = pygame.time.Clock()

ALIVE_COLOR = (0, 0, 0)  # Living cells
DEAD_COLOR = (255, 255, 255)  # Dead cells
CELL_SIZE = 8  # Pixels
WIDTH, HEIGHT = info.current_w // CELL_SIZE, info.current_h // CELL_SIZE
paused = False


def draw_grid():
	for y in range(HEIGHT):
		for x in range(WIDTH):
			state = game_of_life.get_cell_state(y, x)
			color = ALIVE_COLOR if state == 1 else DEAD_COLOR
			pygame.draw.rect(
				screen, color, (x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE)
			)


def handle_mouse_click():
	mx, my = pygame.mouse.get_pos()
	x, y = mx // CELL_SIZE, my // CELL_SIZE
	if 0 <= x < WIDTH and 0 <= y < HEIGHT:
		new_state = 1 - game_of_life.get_cell_state(y, x)  # Flip between alive and dead
		game_of_life.set_cell_state(y, x, new_state)  # Update


running = True
game_of_life.initialize_game(WIDTH, HEIGHT)
while running:
	screen.fill((255, 255, 255))  # Fill the screen with white
	draw_grid()  # Draw the grid based on the current state
	keys = pygame.key.get_pressed()
	if keys[pygame.K_SPACE]:
		paused = not paused  # Pause the game on space key press
	if not paused:
		game_of_life.update_grid()  # Update the grid if not paused
	for event in pygame.event.get():
		if event.type == pygame.QUIT:
			running = False
		elif event.type == pygame.MOUSEBUTTONDOWN and event.button == 1:
			handle_mouse_click()  # Toggle cell on mouse click
		elif event.type == pygame.MOUSEMOTION and pygame.mouse.get_pressed()[0]:
			handle_mouse_click()  # Toggle cell while holding the mouse button
	pygame.display.flip()  # Update the display
	clock.tick(10)  # Limit the frame rate to 10 FPS
pygame.quit()
