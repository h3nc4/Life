# See LICENSE file for copyright and license details.
# Copyright (C) 2023-2024  Henrique Almeida <me@h3nc4.com>

import numpy
from os import environ

environ["PYGAME_HIDE_SUPPORT_PROMPT"] = "1"
import pygame


class PygameManager:
	def __init__(self, width, height, cell_size, alive_color, dead_color):
		pygame.display.init()
		self.display_info = pygame.display.Info()
		self.cell_size = cell_size
		self.width = width
		self.height = height
		self.running = True
		self.paused = False
		self.mouse_dragging = False
		self.last_selected_cell = None

		# Screen setup
		flags = pygame.FULLSCREEN | pygame.HWSURFACE | pygame.DOUBLEBUF
		try:
			self.screen = pygame.display.set_mode((self.display_info.current_w, self.display_info.current_h), flags)
		except pygame.error:
			self.screen = pygame.display.set_mode((self.display_info.current_w, self.display_info.current_h), pygame.FULLSCREEN)

		self.clock = pygame.time.Clock()

		# Surfaces and drawing
		self.scaled_surface = pygame.Surface((self.display_info.current_w, self.display_info.current_h))
		self.game_surface = pygame.Surface((width, height), depth=32)
		self.game_pixels = pygame.surfarray.pixels2d(self.game_surface)

		# Colors
		self.alive_pixel = (alive_color[0] << 16) | (alive_color[1] << 8) | alive_color[2]
		self.dead_pixel = (dead_color[0] << 16) | (dead_color[1] << 8) | dead_color[2]
		self.color_lut = numpy.array([self.dead_pixel, self.alive_pixel], dtype=numpy.uint32)


	def draw_grid(self, game_matrix):
		"""Draw the game grid."""
		self.game_pixels[:, :] = self.color_lut[game_matrix]
		pygame.transform.scale(self.game_surface, (self.display_info.current_w, self.display_info.current_h), self.scaled_surface)
		self.screen.blit(self.scaled_surface, (0, 0))


	def update_display(self):
		"""Update the Pygame display."""
		pygame.display.flip()


	def tick(self, fps):
		"""Control the framerate."""
		self.clock.tick(fps)


	def get_ticks(self):
		"""Get the current time in milliseconds."""
		return pygame.time.get_ticks()


	def handle_events(self, engine):
		"""Handle Pygame events."""
		for event in pygame.event.get():
			event_handler(event, engine, self)


	def _handle_mouse_click_or_drag(self, engine):
		"""Handle mouse input for toggling cell states."""
		mx, my = pygame.mouse.get_pos()
		x, y = mx // self.cell_size, my // self.cell_size
		if (0 <= x < self.width and 0 <= y < self.height and self.last_selected_cell != (x, y)):
			engine.toggle_cell(x, y)
			self.last_selected_cell = (x, y)


	def quit(self):
		"""Clean up Pygame."""
		pygame.quit()


def event_handler(event, engine, pygame_manager):
	if event.type == pygame.QUIT:
		pygame_manager.running = False
	elif event.type == pygame.KEYDOWN:
		if event.key == pygame.K_SPACE:
			pygame_manager.paused = not pygame_manager.paused
		elif event.key in (pygame.K_q, pygame.K_ESCAPE):
			pygame_manager.running = False
		elif event.key == pygame.K_c:
			engine.clear()
		elif event.key == pygame.K_r:
			engine.restart()
	elif event.type == pygame.MOUSEBUTTONDOWN:
		if event.button == 1:
			pygame_manager._handle_mouse_click_or_drag(engine)
			pygame_manager.mouse_dragging = True
	elif event.type == pygame.MOUSEBUTTONUP:
		if event.button == 1:
			pygame_manager.mouse_dragging = False
			pygame_manager.last_selected_cell = None
	elif event.type == pygame.MOUSEMOTION:
		if pygame_manager.mouse_dragging:
			pygame_manager._handle_mouse_click_or_drag(engine)
