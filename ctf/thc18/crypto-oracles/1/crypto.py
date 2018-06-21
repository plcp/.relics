import os
import cryptography
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import cmac
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives import constant_time
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.primitives.kdf.pbkdf2 import PBKDF2HMAC

def equals(a, b):
    return constant_time.bytes_eq(a, b)

def hash(msg):
    ctx = hashes.Hash(hashes.SHA256(), backend=default_backend())
    ctx.update(msg)
    return ctx.finalize()

def stretch(key, length, salt=None, iterations=10):
    if len(key) < 10 and salt is None:
        raise RuntimeError("Input space too small (specify a salt?)")
    if salt is None:
        salt = bytes(16)

    ctx = PBKDF2HMAC(
            algorithm=hashes.SHA256(), length=length, salt=salt,
            iterations=iterations, backend=default_backend())
    return ctx.derive(key)

class mac:
    @staticmethod
    def sign(key, msg):
        ctx = cmac.CMAC(algorithms.AES(key), backend=default_backend())
        ctx.update(msg)
        return ctx.finalize()

    @staticmethod
    def verify(key, msg, tag):
        ctx = cmac.CMAC(algorithms.AES(key), backend=default_backend())
        ctx.update(msg)
        try:
            ctx.verify(tag)
        except cryptography.exceptions.InvalidSignature:
            return False
        return True

class ecb:
    @staticmethod
    def encrypt(key, msg):
        ctx = Cipher(algorithms.AES(key), modes.ECB(),
                backend=default_backend()).encryptor()
        return ctx.update(msg) + ctx.finalize()

    @staticmethod
    def decrypt(key, msg):
        ctx = Cipher(algorithms.AES(key), modes.ECB(),
                backend=default_backend()).decryptor()
        return ctx.update(msg) + ctx.finalize()

class cbc:
    @staticmethod
    def encrypt(key, msg):
        iv = os.urandom(16)
        ctx = Cipher(algorithms.AES(key), modes.CBC(iv),
                backend=default_backend()).encryptor()
        return iv + ctx.update(msg) + ctx.finalize()

    @staticmethod
    def decrypt(key, msg):
        iv = msg[:16]
        ctx = Cipher(algorithms.AES(key), modes.CBC(iv),
                backend=default_backend()).decryptor()
        return ctx.update(msg[16:]) + ctx.finalize()
