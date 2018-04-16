CC=g++
SRC=./clenche/src/
OFLAGS= -O3 -march=native
CFLAGS=--std=c++17 -Wall -Wextra -Werror -pedantic -ftemplate-backtrace-limit=1
LDFLAGS=-lsfml-audio -lsfml-graphics -lsfml-window -lsfml-system

all: sown

sown: src/sown.cpp
	$(CC) $(CFLAGS) $(OFLAGS) $(LDFLAGS) -I $(SRC) -o build/$@ $<
