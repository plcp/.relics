CC=g++
SRC=./clenche/src/
OFLAGS=-O3 -march=native
CFLAGS=--std=c++17 -Wall -Wextra -Werror -pedantic
LDFLAGS=-lsfml-graphics -lsfml-window -lsfml-system

all: cars cars-wobble cars-collide

cars: cars.cpp
	$(CC) $(CFLAGS) $(OFLAGS) $(LDFLAGS) -I $(SRC) -o build/$@ $<

cars-wobble: cars.cpp
	$(CC) $(CFLAGS) $(OFLAGS) $(LDFLAGS) -I $(SRC) -DWOBBLE -o build/$@ $<

cars-collide: cars.cpp
	$(CC) $(CFLAGS) $(OFLAGS) $(LDFLAGS) -I $(SRC) -DCOLLIDE -o build/$@ $<
