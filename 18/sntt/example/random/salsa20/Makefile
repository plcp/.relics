CC=g++
OPT=-std=c++17 -Wall -Wextra -pedantic -O3
LD=-lnfllib

all: checksums

salsa:
	$(CC) $(OPT) $(LD) salsa.cpp -o salsa

key:
	cat /dev/urandom | head -c 32 > key

salsa-nfllib: salsa key
	cat key | ./salsa > salsa-nfllib

salsa-sodium: key
	cat key | ./salsa.py > salsa-sodium

checksums: salsa-nfllib salsa-sodium
	sha256sum salsa-nfllib salsa-sodium

clean:
	rm -f key salsa-nfllib salsa-sodium

mr_proper: clean
	rm -f salsa
