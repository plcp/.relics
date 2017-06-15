#!/usr/bin/env python2

# Detect potential redundancy in an arbitrary file
#
# Usage:
#     pydiff.py <ngram-size> <cache-size> <thresold> <plaintext-file>
#
# Example:
#     python2 pydiff.py 16 256 4 ~/thesis.txt
#
# More:
#    - redundancy is detected within a sliding window of <cache-size> size
#    - works by detecting similar n-grams of <ngram-size> within this window
#    - similarity is defined as "a levenshtein distance below <thresold>"
#    - works well for "cache-size >> thresold"
#

from __future__ import print_function
import Levenshtein as lev # python-levenshtein
import sys
import re

width = int(sys.argv[1])
csize = int(sys.argv[2])
thres = int(sys.argv[3])
with open(sys.argv[4], 'r') as f:
    content = f.read()

# handle latex

content = re.sub(r'\\(ref|label|url|cite|begin|end)\{[^}]*\}', '', content)
content = re.sub(r'\\[a-z]*\{TODO\}', '', content)
content = re.sub(r'\\[a-z]*', '', content)

cache = []
nline = 0
forward = 0
for i in range(len(content) - width + 1):
    ngram = content[i:i + width]
    if ngram[0] == '\n':
        nline += 1

    candidates = (
            [c for c in cache[:-thres] if lev.distance(ngram, c[1]) < thres])
    if len(candidates) > 0 and forward < 1:
        cache = [c for c in cache if lev.distance(ngram, c[1]) >= thres]
        forward = width
        printable = ['<{}>: "{}"'.format(
            c[0], c[1].replace('\n', '\\n')) for c in candidates]
        print('Probable redundancy line {}:\n\t+<{}>: "{}"'.format(nline, nline,
                    ngram.replace('\n', '\\n'))
                + '\n\t-'.join([''] + printable)) 
    else:
        forward -= 1

    cache.append((nline, ngram))
    if len(cache) > csize:
        cache.pop(0)
