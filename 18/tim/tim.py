import math
import time
import best
import random

def log(*kargs, **kwargs):
    print(*kargs, **kwargs)
    with open('report.log', 'a+') as f:
        f.write(' '.join([k.replace('\r', '\n') for k in kargs]))
        if 'end' not in kwargs:
            f.write('\n')
        else:
            f.write(kwargs['end'])

def fitness(candidate, avg):
    copy = list(candidate)
    date = time.perf_counter()
    for i in range(avg):
        list(copy).sort()
    return time.perf_counter() - date

def single_compare(a, b, avg=50):
    return int(fitness(a, avg) > fitness(b, avg)), avg * 2

# 1.6449 90.000%
# 1.9599 95.000%
# 2.0000 95.450%
# 2.5759 99.000%
# 3.0000 99.730%
# 3.2905 99.900%
# 3.8906 99.990%
# 4.0000 99.993%
# 4.4172 99.999%

default_z = 1.6449
def error(count, z=default_z):
    return z / (2 * math.sqrt(count))

def error_small(z=default_z):
    for i in range(1, 1000):
        if 0.5 > error(i, z):
            return i
mz = error_small(default_z)

def compare(a, b, minq=mz, fail=40, retry=5, foo=single_compare, stop=True):
    if minq is None:
        minq = error_small()

    n = 0
    cost = 0
    tails = 0
    while True:
        n += 1
        if random.randint(0, 1) == 0:
            r, c = foo(a, b)
            tails += r
        else:
            r, c = foo(b, a)
            tails += (1 - r)
        cost += c

        if n > minq:
            e = error(n)
            emin = 0.5 - e
            emax = 0.5 + e
            ratio = tails / n
            if ratio < emin or ratio > emax:
                return int(ratio > 0.5), cost

        if n > fail:
            if retry > 0:
                r, c = compare(a, b, minq, fail, retry - 1, foo)
                return r, (cost + c)
            elif stop:
                return 0.5, cost

def meta_compare(a, b, minq=20, fail=100, retry=5, foo=single_compare):
    def sub_compare(a, b):
        if random.randint(0, 1) == 0:
            return compare(a, b, minq, fail, retry, foo)
        else:
            r, c = compare(b, a, minq, fail, retry, foo)
            return 1 - r, c
    return compare(a, b, minq, fail, retry, sub_compare)

def create(size=1024):
    candidate = list(range(size))
    random.shuffle(candidate)
    return candidate

def mutate(candidate, rate=10, expn=5):
    n = 0
    while True:
        q = random.randint(0, rate)
        if q > expn:
            rate *= 2
            expn *= 2
            continue
        n += q
        break

    sz = len(candidate)
    copy = list(candidate)
    for _ in range(n):

        src = random.randint(0, sz - 1)
        dst = (random.randint(1, sz - 1) + src) % sz

        copy[src], copy[dst] = copy[dst], copy[src]
    return copy

def elorep(d):
    return 1 / (1 + 10 ** (-d / 400))

def elo(a, b, w, k=20):
    delta = a - b
    na = a + k * (w - elorep(delta))
    nb = b + k * ((1 - w) - elorep(-delta))
    return na, nb

if __name__ == '__main__':
    psize = 32
    mrate = 2

    scores = [1000 for _ in range(psize)]
    population = list(best.candidates)
    while len(population) < psize:
        population.append(create())

    gen = 0
    tcost = 0
    start = time.time()
    while True:
        gen += 1
        nul = 0
        last = time.time()

        for src in range(psize):
            dst = random.randint(0, psize - 1)
            scores[src], scores[dst] = scores[dst], scores[src]
            population[src], population[dst] = population[dst], population[src]

        cost = 0
        for i in range(psize):
            src = random.randint(0, psize - 1)
            dst = (random.randint(1, psize - 1) + src) % psize

            r, c = compare(population[src], population[dst])
            log('\r {:3} vs {:3} - {:1.1f} - {:8.2f}/{:8.2f} to '.format(
                src, dst, r, scores[src], scores[dst]), end='')

            scores[src], scores[dst] = elo(scores[src], scores[dst], r)
            if r == 0.5:
                nul += 1
                scores[src] -= psize / 2
                scores[dst] -= psize / 2
                for j in range(psize):
                    scores[j] += 1

            log('{:8.2f}/{:8.2f} - {:8} - {:2}'.format(
                scores[src], scores[dst], c, i), end='', flush=True)

            cost += c
        tcost += cost

        avg = sum(scores) / psize
        for _ in range(mrate):
            who = random.randint(0, psize - 1)
            if scores[who] < avg:
                best = scores.index(max(scores))
                population[who] = mutate(population[best])
            else:
                worst = scores.index(min(scores))
                population[who] = mutate(population[who])

        log(('\n{:6}: [{:8.2f}; {:8.2f}; {:8.2f}] {:2} nul ({:8} cost)'
            ).format(gen, min(scores), avg, max(scores), nul, cost), end='')

        elapsed = time.time() - start
        duration = time.time() - last
        per_iter = elapsed / tcost

        log('- {:5.1f}s last {:6.1f}s total {:4.1f}us cost'.format(
            duration, elapsed, per_iter * 1000000))

        with open('candidate.py', 'w') as f:
            f.write(repr(population[scores.index(max(scores))]))
