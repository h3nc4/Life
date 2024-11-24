# Compiler and flags
CC = cc
CFLAGS = -Wall -O3 -ftree-vectorize -ffast-math -fPIC -fopenmp
LDFLAGS = -shared -fopenmp

# Linux-specific
TARGET = engine.so

# Windows-specific
WIN_TARGET = engine.dll
WIN_LDFLAGS = -shared -fopenmp

# Common
SRCS = engine.c
OBJS = $(SRCS:.c=.o)

# Python setup for PyInstaller
PYTHON = python3
PYINSTALLER = pyinstaller
PY_SRCS = ui.py

# Default target
all: life

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Linux builds
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

life: $(TARGET) $(PY_SRCS)
	$(PYINSTALLER) --distpath . --onefile --add-data "$(TARGET):." $(PY_SRCS)
	mv ui life

# Windows builds
$(WIN_TARGET): $(OBJS)
	$(CC) $(OBJS) $(WIN_LDFLAGS) -o $@

life-win: $(WIN_TARGET) $(PY_SRCS)
	$(PYINSTALLER) --distpath . --onefile --add-data "$(WIN_TARGET);." $(PY_SRCS)
	mv ui.exe life.exe

# Clean up
clean:
	rm -fr $(OBJS) $(TARGET) $(WIN_TARGET) build/ ui.spec life life.exe

install: life
	install -Dm755 life /usr/local/games/life

uninstall:
	rm -f /usr/local/games/life

.PHONY: all clean install uninstall life life-win
