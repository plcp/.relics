CC=g++
OPT=-std=c++17 -Wall -Wextra -pedantic -O3 -I ../
LD=-lmpfr -lgmp

all: checksums

barriers:
	$(CC) $(OPT) $(LD) barriers.cpp -o barriers

randomness:
	cat /dev/urandom | head -c 1M > randomness

barriers-nfllib: barriers randomness
	./barriers > barriers-nfllib

barriers-python:
	python barriers.py > barriers-python

checksums: barriers-nfllib barriers-python
	sed -i 's/[0-9]\{2\},\s*$$/xx/g' barriers-nfllib barriers-python
	#
	# We strip here the last digit as we sometimes have the last bits of
	# precision flipped due to rounding errors between mpfr & mpmath.
	#
	# This is not a problem as we never use these bits during calculus.
	#
	sha256sum barriers-nfllib barriers-python

clean:
	rm -f randomness barriers-nfllib barriers-python

mr_proper: clean
	rm -f barriers
