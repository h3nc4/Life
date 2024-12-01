import pytest
import sys
from os.path import dirname, abspath

sys.path.insert(0, dirname(dirname(dirname(abspath(__file__)))))
from wrapper import GameEngine
from game import PygameManager


def test_game_engine():
	ENGINE = GameEngine(10, 10)
	assert ENGINE.WIDTH == 10
	assert ENGINE.HEIGHT == 10
	assert ENGINE.GAME_MATRIX1.shape == (10, 10)
	assert ENGINE.GAME_MATRIX2.shape == (10, 10)

def test_pygame_manager():
	MANAGER = PygameManager(1366, 768, 1, (255, 255, 255), (0, 0, 0))
	assert MANAGER.width == 1366
	assert MANAGER.height == 768
	assert MANAGER.cell_size == 1
	assert MANAGER.alive_pixel == 16777215
	assert MANAGER.dead_pixel == 0
