from tim import *
import best

if __name__ == '__main__':
    psize = len(best.candidates)
    scores = [1000 for _ in range(psize)]
    population = [list(c) for c in best.candidates]

    for gen in range(320000):
        for src in range(psize):
            dst = random.randint(0, psize - 1)
            scores[src], scores[dst] = scores[dst], scores[src]
            population[src], population[dst] = population[dst], population[src]

        src = random.randint(0, psize - 1)
        dst = (random.randint(1, psize - 1) + src) % psize

        r, c = compare(population[src], population[dst])
        scores[src], scores[dst] = elo(scores[src], scores[dst], r)
        if r == 0.5:
            scores[src] -= psize / 8
            scores[dst] -= psize / 8
            for j in range(psize):
                scores[j] += 1 / 4

        log('{:4} {:8.2f} {:8.2f} {:8.2f}'.format(
            gen, min(scores), sum(scores) / psize, max(scores)))

        indexes = list(range(psize))
        indexes.sort(key=lambda x: -scores[x])

        result = []
        for idx in indexes:
            result.append(population[idx])

        with open('candidates.py', 'w') as f:
            f.write('sorted_best = [' + ', '.join([repr(p) for p in result]) + ']')

