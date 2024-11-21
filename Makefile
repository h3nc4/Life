# Compiler and flags
CC = cc
CFLAGS = -Wall -O2 -fPIC -fopenmp
LDFLAGS = -shared

# Target shared library and source files
TARGET = game_of_life.so
SRCS = game_of_life.c
OBJS = $(SRCS:.c=.o)

# Python setup for PyInstaller
PYTHON = python3
PYINSTALLER = pyinstaller
PY_SRCS = game_of_life.py

# Default target
all: dist/game_of_life

# Build the shared library
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(TARGET)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build the Python executable using PyInstaller
dist/game_of_life: $(TARGET) $(PY_SRCS)
	$(PYINSTALLER) --distpath . --onefile --add-data "$(TARGET):." $(PY_SRCS)

# Clean up generated files
clean:
	rm -fr $(OBJS) $(TARGET) build/ game_of_life.spec game_of_life

# Phony targets
.PHONY: clean
