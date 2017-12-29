CC=g++
SRC=./clenche/src/
OFLAGS= -O3 -march=native
CFLAGS=--std=c++17 -Wall -Wextra -Werror -pedantic
LDFLAGS=-lsfml-graphics -lsfml-window -lsfml-system

all: flocks flocks-few flocks-gravity

flocks: flocks.cpp
	$(CC) $(CFLAGS) $(OFLAGS) $(LDFLAGS) -I $(SRC) -o build/$@ $<

flocks-few: flocks.cpp
	$(CC) $(CFLAGS) $(OFLAGS) $(LDFLAGS) -DFEW -I $(SRC) -o build/$@ $<

flocks-gravity: flocks.cpp
	$(CC) $(CFLAGS) $(OFLAGS) $(LDFLAGS) -DGRAVITY -I $(SRC) -o build/$@ $<
