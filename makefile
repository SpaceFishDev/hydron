src = $(wildcard src/*.c)
out = hydron
cflags = -O3

all: build run

build:
	gcc $(src) -o $(out) $(cflags)

run:
	./$(out)