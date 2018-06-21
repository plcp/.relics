#!/usr/bin/env python3

from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend
from time import time
import os, hmac

backend = default_backend()

def pad(plaintext):
    if len(plaintext) % 16 == 0:
        plaintext += (1).to_bytes(1, byteorder='little')
        plaintext += bytes(15)
    else:
        plaintext += (1).to_bytes(1, byteorder='little')
        plaintext += bytes(16 - (len(plaintext) % 16))
    return plaintext

def unpad(plaintext):
    nzero = 0
    while plaintext[-1] == 0:
        plaintext = plaintext[:-1]
        nzero += 1
    if nzero < 1 or plaintext[-1] != 1:
        return None
    return plaintext[:-1]

def encrypt(mkey, plaintext):
    iv = os.urandom(16)
    key = hmac.new(mkey, iv).digest()[:16]
    cipher = Cipher(algorithms.AES(key), modes.CBC(iv), backend=backend)
    encryptor = cipher.encryptor()
    ciphertext = encryptor.update(pad(plaintext)) + encryptor.finalize()
    return iv + ciphertext

def decrypt(mkey, ciphertext):
    iv = ciphertext[0:16]
    key = hmac.new(mkey, iv).digest()[:16]
    cipher = Cipher(algorithms.AES(key), modes.CBC(iv), backend=backend)
    decryptor = cipher.decryptor()
    plaintext = decryptor.update(ciphertext[16:]) + decryptor.finalize()
    return unpad(plaintext)

class challenge():
    def __init__(self, key, challenge, response, context):
        self.valid = False
        stillvalid = True

        when = int(int(time()) / 10).to_bytes(4, byteorder="little")
        what = challenge[0:4]
        auth = hmac.new(key, when + what).digest()

        if challenge[4:] != auth:
            stillvalid = False

        intermed = hmac.new(key, context + challenge).digest()
        expected = hmac.new(key, when    + intermed ).digest()

        if expected != response:
            stillvalid = False

        self.valid = stillvalid
        self.expected = expected
        self.challenge = challenge[0:4] + auth

    def digest(self):
        return (self.challenge, self.expected)

    def isvalid(self):
        return self.valid
