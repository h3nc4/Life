# Conways Game of Life in C

This is a simple implementation of Conway's Game of Life in C using Pygame for the GUI.

The board is filled with random cells at the start. The user can press space to start the simulation or pause it.

## Building

### Dev Dependencies

* Python 3
* virtualenv
* Ctypes
* Pygame
* Make
* GCC

### Running

To run a development build, call the following command:

```bash
scripts/dev
```

This will create a virtual environment, install the dependencies, compile shared libraries and run the game.

## About Conway's Game of Life

### History

The Game of Life is a cellular automaton devised by the British mathematician John Horton Conway in 1970. It is a zero-player game, meaning that its evolution is determined by its initial state, requiring no further input. One interacts with the Game of Life by creating an initial configuration and observing how it evolves.

### Rules

The universe of the Game of Life is a two-dimensional orthogonal grid of square cells, each of which is in one of two possible states, alive or dead. Every cell interacts with its eight neighbours, which are the cells that are horizontally, vertically, or diagonally adjacent.

At each step in time, the following transitions occur:

* Any live cell with fewer than two live neighbours dies, as if by underpopulation.
* Any live cell with two or three live neighbours lives on to the next generation.
* Any live cell with more than three live neighbours dies, as if by overpopulation.
* Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.

## License

Life is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Life is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

See [LICENSE](LICENSE) for more information.
