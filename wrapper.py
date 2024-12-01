# See LICENSE file for copyright and license details.
# Copyright (C) 2023-2024  Henrique Almeida <me@h3nc4.com>

from ctypes import CDLL, c_uint, c_ubyte, POINTER, cast
from platform import system
import numpy


class GameEngine:
	def __init__(self, width, height):
		self.ENGINE = CDLL("./engine.dll" if system() == "Windows" else "./engine.so")
		WIDTH = width
		HEIGHT = height

		# Define argument and return types for the C functions
		self.ENGINE.initialize_game.argtypes = [c_uint, c_uint]
		self.ENGINE.initialize_game.restype = None
		self.ENGINE.update_grid.argtypes = []
		self.ENGINE.update_grid.restype = None
		self.ENGINE.ptr_to_both_grids.argtypes = []
		self.ENGINE.ptr_to_both_grids.restype = POINTER(c_ubyte)
		self.ENGINE.toggle_cell_state.argtypes = [c_uint, c_uint]
		self.ENGINE.toggle_cell_state.restype = None
		self.ENGINE.free_grid.argtypes = []
		self.ENGINE.free_grid.restype = None
		self.ENGINE.clear_grid.argtypes = []
		self.ENGINE.clear_grid.restype = None
		self.ENGINE.restart_game.argtypes = []
		self.ENGINE.restart_game.restype = None

		# Initialize the game engine
		self.ENGINE.initialize_game(WIDTH, HEIGHT)

		# Get pointers to the grids
		GRIDS = cast(self.ENGINE.ptr_to_both_grids(), POINTER(POINTER(c_ubyte * (WIDTH * HEIGHT)))).contents
		self.GAME_MATRIX = numpy.array(GRIDS[1], copy=False).reshape(WIDTH, HEIGHT)
		self.GAME_MATRIX_NEXT = numpy.array(GRIDS[0], copy=False).reshape(WIDTH, HEIGHT)
		self.current_grid_index = 1


	def update(self):
		"""Updates the grid with the new generation."""
		self.ENGINE.update_grid()
		self.current_grid_index ^= 1


	def toggle_cell(self, x, y):
		"""Toggle the state of a cell at (x, y)."""
		self.ENGINE.toggle_cell_state(x, y)


	def clear(self):
		"""Clear the grid."""
		self.ENGINE.clear_grid()


	def restart(self):
		"""Restart the game."""
		self.ENGINE.restart_game()
		self.current_grid_index = 1


	def get_current_grid(self):
		"""Get the current active grid as a numpy array."""
		return self.GAME_MATRIX if self.current_grid_index == 1 else self.GAME_MATRIX_NEXT


	def free_grid(self):
		"""Free the memory allocated for the grids."""
		self.ENGINE.free_grid()
