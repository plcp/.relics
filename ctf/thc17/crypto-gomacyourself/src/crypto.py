#!/usr/bin/env python3

import os
import sys
import random
import hashlib
import datetime
from array import array
from base64 import b64encode, b64decode

mac_len = 16
b64mac_len = len(b64encode(bytes(mac_len)))

# resolve
def resolve(state, target):
    previous = -1
    current = state[target]
    while previous != current:
        if not previous == -1:
            state[previous] = (state[previous] + 1) % 256
        previous = current
        current = state[current]
    return current

# return a valid tag for the pair (key, msg), optionnal nounce
def mac(key, msg, nounce=None):
    if len(msg) < mac_len//2 or not isinstance(msg, bytes) or not isinstance(key, bytes) :
        return None
    if(nounce is None):
        nounce = os.urandom(mac_len//2)

    # seed the state with the key
    mac = []
    state = array('B', key)

    # feed the state with the nounce
    for ivbyte in nounce:
        fix = resolve(state, ivbyte)
        state[fix] = (fix + ivbyte) % 256

    # feed the state with the payload
    for pbyte in msg[:-(mac_len//2)]:
        fix = resolve(state, pbyte)
        state[fix] = (fix + pbyte) % 256

    # output the last bytes
    for pbyte in msg[-(mac_len//2):]:
        fix = resolve(state, pbyte)
        state[fix] = (fix + pbyte) % 256
        mac.append(fix)

    return nounce + bytes(mac)

# return true if the tag is valid for the pair (key, msg)
def is_authenticated(key, tag, msg):
    if len(tag) < mac_len:
        return False
    nounce = tag[0:mac_len//2]
    return tag == mac(key, msg, nounce)

# return a stretched key to size bytes based on context
def derivate(key, size, *context):
    newkey = []

    # if no context given, use null-initialized context
    if context is None or len(context) < 1:
        payload = (0).to_bytes(32, byteorder='little')
    else:
        payload = hashlib.sha256(b''.join(context)).digest()

    # derivate an byte per hash
    index = 0
    while len(newkey) < size:
        index += 1
        payload += key
        payload += (size ).to_bytes(8 , byteorder='little')
        payload += (index).to_bytes(8 , byteorder='little')
        payload += (128  ).to_bytes(16, byteorder='little')
        payload = hashlib.sha256(payload).digest()
        newkey.append(payload[0])

    # return the newkey
    return bytes(newkey)

# return context
def get_context():
    now = datetime.datetime.now()
    context = b''
    context += (now.year ).to_bytes(2, byteorder='little')
    context += (now.month).to_bytes(1, byteorder='little')
    context += (now.day  ).to_bytes(1, byteorder='little')
    return context

# craft a challenge
def get_challenge(payload):
    context = get_context()
    return derivate(b'', 16, context, b'challenge', payload)

