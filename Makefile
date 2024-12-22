# Compiler and flags
CC = cc
CFLAGS = -Wall -O3 -ftree-vectorize -ffast-math -fPIC -fopenmp $(DEBUG_FLAGS)
LDFLAGS = -fopenmp -lX11 -lXinerama -lXext
TARGET = life
SRCS = life.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

clean:
	rm -f $(OBJS) $(TARGET)

install: $(TARGET)
	install -Dm755 $(TARGET) /usr/local/bin/$(TARGET)

uninstall:
	rm -f /usr/local/bin/$(TARGET)

.PHONY: all clean install uninstall
