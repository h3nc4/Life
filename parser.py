# See LICENSE file for copyright and license details.
# Copyright (C) 2023-2024  Henrique Almeida <me@h3nc4.com>

from argparse import ArgumentParser


def parse_args():
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
	return args.alive_color, args.dead_color, args.cell_size
