# See LICENSE file for copyright and license details.
# Copyright (C) 2023-2024  Henrique Almeida <me@h3nc4.com>

from wrapper import GameEngine
from traceback import print_exc
from game import PygameManager
from argparse import ArgumentParser
from os import getenv
import time

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

# Initialize the game engine and Pygame manager
ENGINE = GameEngine(1920 // CELL_SIZE, 1080 // CELL_SIZE)
MANAGER = PygameManager(ENGINE.WIDTH, ENGINE.HEIGHT, CELL_SIZE, ALIVE_COLOR, DEAD_COLOR)

GAME_MATRIX, GAME_MATRIX_NEXT = ENGINE.get_grids()


def debug_print(*args, **kwargs):
	"""Print debug messages if the DEBUG environment variable is set."""
	if getenv("DEBUG"):
		print(*args, **kwargs)


start_time = time.time()
frame_count = 0
running = True
last_update_time = MANAGER.get_ticks()
update_interval = 100
try:
	while MANAGER.running:
		current_time = MANAGER.get_ticks()
		MANAGER.handle_events(ENGINE)
		if current_time - last_update_time >= update_interval and not MANAGER.paused:
			update_time = time.time()
			GAME_MATRIX, GAME_MATRIX_NEXT = GAME_MATRIX_NEXT, GAME_MATRIX  # Swap the grids
			ENGINE.update()
			debug_print(f"Update time: {time.time() - update_time}")
			last_update_time = current_time
		draw_grid_time = time.time()
		MANAGER.draw_grid(GAME_MATRIX)
		display_time = time.time()
		debug_print(f"Draw grid time: {display_time - draw_grid_time}")
		MANAGER.update_display()
		debug_print(f"Display time: {time.time() - display_time}")
		MANAGER.tick(60)
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
	MANAGER.quit()
