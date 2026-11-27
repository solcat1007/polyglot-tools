CC = gcc
CFLAGS = -O2 -std=c99 -Wall -Wextra
TARGET = bin/tool

.PHONY: all clean windows

all: $(TARGET)

$(TARGET): main.c
	mkdir -p bin
	$(CC) $(CFLAGS) -o $@ main.c -lm

windows: main.c
	mkdir -p bin
	x86_64-w64-mingw32-gcc $(CFLAGS) -o bin/tool.exe main.c -lm

clean:
	rm -f $(TARGET) bin/tool.exe
