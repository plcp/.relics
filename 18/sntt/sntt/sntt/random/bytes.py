import salsa20
import os

_internal_key = None
_internal_nonce = 0

def get(nb_bytes):
    global _internal_key, _internal_nonce
    if _internal_key is None:
        _internal_key = os.urandom(32)

    nonce = _internal_nonce.to_bytes(8, byteorder='big')
    return salsa20.Salsa20_keystream(nb_bytes, nonce, _internal_key)
