import math

with open('ciphertext', 'r') as f:
    ct = f.read().split('\n')

ciphers = [list(reversed([c[i:i+2] for i in range(0, len(c), 2)])) for c in ct]
ciphers = [[int(d, 16) for d in c] for c in ciphers]
ciphers = [int.from_bytes(bytes(c), byteorder='big') for c in ciphers]
ciphers = [c for c in ciphers if c > 0]

gcds = []
for c in range(len(ciphers)):
    for d in range(c + 1, len(ciphers)):
        x = math.gcd(ciphers[c], ciphers[d])
        if x not in gcds:
            gcds.append(x)

def decrypt(cipher, key):
    n = 32
    while True:
        n *= 2
        try:
            plain = (cipher // key).to_bytes(n, byteorder='big')
            return plain.replace(b'\x00', b'')
        except OverflowError:
            pass

for g in gcds:
    input('Try next?')
    for c in ciphers:
        print(decrypt(c, g))
