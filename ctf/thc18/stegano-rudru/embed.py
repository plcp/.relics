#!/usr/bin/env python3
import PIL
from PIL import Image

import numpy as np
import json
import io

pad = 1300
embedding_size = 2
entropy_size = 2**8
score_slice = lambda img: [img[8 * i] for i in range(16384)]

rawref = None
reference = None
entropy = [None] * 256

def bits(src):
    for byte in src:
        byte = int(byte)
        for i in reversed(range(8)):
            b = 0
            if (2**i) & byte:
                b = 1
            yield b

    idealsize = round((len(source) * 8) / 3 + 0.7)
    if idealsize != (len(source) * 8) // 3:
        yield 0

def pits(src):
    n = 0
    h = 0
    for b in bits(src):
        h *= 2
        h += b

        n += 1
        if n == 3:
            n = 0
            yield h
            h = 0
    if n != 0:
        h *= 2**(3 - n)
        yield h

def fits(src):
    for b in pits(src):
        yield 2**b

src_bits = None
def embed(src, dst, embedding):
    global src_bits
    if src_bits is None:
        src_bits = np.array(list(fits(src)), dtype='uint8')

    global rawref
    dst = np.copy(rawref)
    mbd = np.cumsum(embedding) + pad
    dst[mbd] = dst[mbd] ^ src_bits[:len(mbd)]
    return dst.tobytes()

current_img = None
def valid_jpg(candidate):
    global current_img
    current_img = Image.open(io.BytesIO(candidate))
    try:
        current_img = current_img.getdata()
        if current_img[-1] == (128, 128, 128):
            return False
        return True
    except BaseException:
        pass

    return False

def diffscore(source, target, embedding):
    candidate = embed(source, target, embedding)
    if not valid_jpg(candidate):
        return None

    return np.sum(np.abs(score_slice(current_img) - reference))

def initial_search(source, target):
    n = embedding_size

    idealsize = round((len(source) * 8) / 3 + 0.7)
    embedding = np.array([], dtype='uint8')
    for i in range(idealsize):
        print('\rGetting initial {}-embedding... '.format(n), end='')
        print('{}/{}...      '.format(i, idealsize), end='', flush=True)

        embedding = np.concatenate([embedding, np.zeros(1, dtype='uint8')])
        for j in range(n, 256):
            embedding[-1] = j
            candidate = embed(source, target, embedding)
            score = valid_jpg(candidate)
            if score:
                break

    print('\rGetting initial embedding... ', end='')
    print('Finished!                 ', flush=True)
    return embedding

def greedy_search(source, target, embedding, window=32, step=8, skip=256):
    entropy[window] = np.random.randint(0, window, entropy_size)
    best = diffscore(source, target, embedding)
    if best is None:
        best = 10 ** 10 - 1

    c = 0
    d = 0
    i = 0
    while i < min(skip, len(embedding) - window):
        j = 0
        rst = False
        while j < step:
            src = None
            while src is None or embedding[i + src] < 2:
                src = entropy[window][c]
                c = (c + 1) % entropy_size
            dst = (src + entropy[window][d]) % window
            d = (d + 1) % entropy_size

            embedding[i + src] -= 1
            embedding[i + dst] += 1

            score = diffscore(source, target, embedding)
            if score is not None and score < best:
                best = score
                rst = True
                j = 0
            else:
                embedding[i + src] += 1
                embedding[i + dst] -= 1

            print('\r[{:3d}] Best score: {:9d} x{:2d}'.format(
                    i, best, abs(j)), end='', flush=True)
            j += 1

        if rst:
            rst = False
            i = 0

            final = embed(source, target, embedding)
            with open('shrdlu82.jpeg', 'wb') as f:
                f.write(final)
            with open('embedding.json', 'w') as f:
                json.dump([int(x) for x in embedding], f)
        else:
            entropy[window] = np.random.randint(0, window, entropy_size)
            c = np.random.randint(0, entropy_size)
            d = np.random.randint(0, entropy_size)
            i += 1

    print('\r[{:3d}] Best score: {:9d} x{:1d}'.format(i, best, j), end='')
    print(' Finished.')

    return embedding

source = None
with open('flag.tar.gz', 'rb') as f:
    source = f.read()

target = None
with open('shrdlu82.jpeg.orig', 'rb') as f:
    target = f.read()
    rawref = np.array(list(target), dtype='uint8')

rdata = Image.open('shrdlu82.jpeg.orig').getdata()
reference = np.array(score_slice(rdata), dtype='uint8')

embd = None
try:
    with open('../embedding.json', 'r') as f:
        embd = list(json.load(f))
    print('Using previously found embedding as starting point...')
except BaseException:
    embd = initial_search(source, target)

embd = greedy_search(source, target, embd, step=64, skip=16)
embd = greedy_search(source, target, embd, step=32, skip=32)
embd = greedy_search(source, target, embd, window=64, step=16, skip=64)
embd = greedy_search(source, target, embd, window=128)

final = embed(source, target, embd)
with open('shrdlu82.jpeg', 'wb') as f:
    f.write(final)
