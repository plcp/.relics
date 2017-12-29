import curses
import math

import eightdom
import eightdom.world

colors = [
    (1, 7, 1),
    (2, 7, 2),
    (3, 7, 3),
    (4, 7, 4),
    (5, 7, 5),
    (6, 7, 6),
    (7, 0, 7),
    (8, 7, 4),
]


def main(stdscr, config):
    global colors
    for color in colors:
        curses.init_pair(*color)

    w = eightdom.world.world(config, stdscr)
    stdscr.move(0, 0)
    stdscr.nodelay(1)
    while True:

        w.draw(stdscr)
        c = stdscr.getch()

        if c != -1:
            w.moves += 16 / w.width
            w.score += 1 / (1 + math.log(abs(w.moves) + 1))

            w.cursor.move(c)
            for x in w.cursors:
                x.tick()

            if w.defeat(stdscr):
                w = eightdom.world.world(config, stdscr)
                continue

            if w.victory():
                w.reset()


def bootstrap():
    games = {
        'regular': (True, [8, 6, 4, 2, 7, 5, 3]),
        'hostile': (False, [8, 6, 4, 2, 7, 5, 3]),
    }

    print('Pick your favorite:')
    for idx, title in enumerate(games):
        print('{})\t- {}'.format(idx, title))
    c = input('Your choice ? (default: regular) ')

    if len(c) < 1:
        c = 'regular'

    try:
        c = int(c)
        assert 0 <= c < len(games)
        config = games[list(games.keys())[c]]
    except BaseException:
        assert c in games
        config = games[c]

    while True:
        try:
            curses.setupterm()
            curses.wrapper(main, config)
            break
        except curses.error:
            pass

        if eightdom.world.size < 4:
            print('Terminal too small.')
            break

        eightdom.world.size -= 1

if __name__ == '__main__':
    bootstrap()
