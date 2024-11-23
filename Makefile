# Compiler and flags
CC = cc
CFLAGS = -Wall -O3 -ftree-vectorize -ffast-math -fPIC -fopenmp
LDFLAGS = -shared -fopenmp

# Target shared library and source files
TARGET = engine.so
SRCS = engine.c
OBJS = $(SRCS:.c=.o)

# Python setup for PyInstaller
PYTHON = python3
PYINSTALLER = pyinstaller
PY_SRCS = ui.py

# Default target
all: life

# Build shared library
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $(TARGET)

# Compile source files into object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build Python executable using PyInstaller
life: $(TARGET) $(PY_SRCS)
	$(PYINSTALLER) --distpath . --onefile --add-data "$(TARGET):." $(PY_SRCS)
	mv ui life

clean:
	rm -fr $(OBJS) $(TARGET) build/ ui.spec life

install: life
	install -Dm755 life /usr/local/games/life

uninstall:
	rm -f /usr/local/games/life

.PHONY: all clean install uninstall
