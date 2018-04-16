#!/usr/bin/env python3
import salsa20
import sys

if __name__ == "__main__":
    key = sys.stdin.buffer.read(32)

    for nonce in range(0, 1024):
        vnonce = nonce.to_bytes(8, byteorder='little')

        # libsodium's salsa20 (from NaCl)
        buffer = salsa20.Salsa20_keystream(1024, vnonce, key)
        sys.stdout.buffer.write(buffer)
