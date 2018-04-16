import sntt.random.bytes
import sntt.random.noise

source = open('randomness', 'rb')
def get(nb_bytes):
    return source.read(nb_bytes)
sntt.random.bytes.get = get

prng = sntt.random.noise.gaussian(20, 128, 2**14)
print(prng.nb_barriers)
for idx, barrier in enumerate(prng.barriers):
    for word in barrier:
        print("{}, ".format(word), end='')
    print('')
