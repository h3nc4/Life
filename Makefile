CC = gcc
CFLAGS = -Wall -O2 -fPIC
LDFLAGS = -shared

all: dist/game_of_life

src/game_of_life.so: src/game_of_life.c
	$(CC) $(CFLAGS) src/game_of_life.c -o src/game_of_life.so $(LDFLAGS)

dist/game_of_life: src/game_of_life.so src/game_of_life.py
	pyinstaller --onefile --add-data "src/game_of_life.so:." src/game_of_life.py

clean:
	rm -rf src/game_of_life.so build dist *.spec

.PHONY: all clean
