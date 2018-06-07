import argparse
import random

import candidates
import tim

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('navg', nargs='?', type=int, default=100000)
    parser.add_argument('nfit', nargs='?', type=int, default=10)
    sys_argv = parser.parse_args()
    
    width = len(candidates.sorted_best[0])
    base = list(range(width))
    afit = 0
    for n in range(sys_argv.nfit):
        random.shuffle(base)
        afit += tim.fitness(base, sys_argv.navg)

    bfit = 0
    for n in range(sys_argv.nfit):
        bfit += tim.fitness(candidates.sorted_best[n], sys_argv.navg)

    print('Base: {:.2f}ms'.format(10**6 * afit / (sys_argv.nfit * sys_argv.navg)))
    print('Best: {:.2f}ms'.format(10**6 * bfit / (sys_argv.nfit * sys_argv.navg)))
    print('Gain: {:.2f}'.format(bfit / afit))
