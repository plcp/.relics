import curses

import eightdom.unit as unit
from eightdom.cursor import cursor

size = 32


class world:
    def __init__(self, config, stdscr):
        assert len(config[1]) == 7
        assert all(2 <= c < 9 for c in config[1])
        assert all(c in config[1] for c in range(2, 9))
        self.complete, self.config = config
        self.width = size * 4 + 2
        self.coind = size

        stdscr.clear()
        stdscr.refresh()

        self.moves = 0
        self.score = 0
        self.world = None
        self.target = None
        self.cursor = None
        self.cursors = None
        self.nb_victory = 0
        self.reset()

    def defeat(self, stdscr):
        for i in range(0, self.width):
            for j in range(0, self.width):
                if self.world[i][j][2] == 1:
                    return False

        stdscr.move(self.width, 0)
        stdscr.addstr(str("You died."), 0)
        stdscr.refresh()

        while stdscr.getch() == -1:
            pass

        stdscr.move(self.width, 0)
        stdscr.addstr(str(" " * 10), 0)
        stdscr.refresh()
        return True

    def victory(self):
        for i in range(0, self.width):
            for j in range(0, self.width):
                if self.complete:
                    if self.world[i][j][2] > 1:
                        return False
                else:
                    if self.world[i][j][2] == self.target:
                        return False

        self.moves = self.moves // 2 + 1
        if self.complete:
            self.moves = self.moves // 4 + 1
        self.score += 128 * (1 + self.nb_victory)**2 / self.moves
        self.nb_victory += 1
        return True

    def reset(self):
        w, c = (self.width, self.coind)
        self.world = [[(' ', 0, 0)] * w for _ in range(0, w)]
        for i in range(0, w):
            if i <= c or i >= w - c - 1:
                self.world[i][c] = ('+', 10, 0)
                self.world[c][i] = ('+', 10, 0)
                self.world[i][w - c - 1] = ('+', 10, 0)
                self.world[w - c - 1][i] = ('+', 10, 0)
            else:
                self.world[i][0] = ('+', 10, 0)
                self.world[0][i] = ('+', 10, 0)
                self.world[i][w - 1] = ('+', 10, 0)
                self.world[w - 1][i] = ('+', 10, 0)

        if self.cursor is None:
            self.cursor = cursor(self, 1)

        if self.cursors is None:
            self.cursors = []
            for i in range(0, 7):
                self.cursors.append(cursor(self, i + 2))

        if self.target is None:
            self.target = 2
            self.cursors[self.target - 2] = self.cursor
        else:
            self.cursors[self.target - 2] = self.cursor
            self.target = ((self.target - 1) % 7) + 2

        for i, c in enumerate(self.cursors):
            c.reset(i + 2)
        self.cursor = cursor(self, 1)

    def get_cursor(self, owner):
        if owner == 1:
            return self.cursor
        else:
            return self.cursors[owner - 2]

    def draw(self, stdscr):
        w = self.width
        for i in range(0, w):
            for j in range(0, w):
                target = self.world[i][j]
                flags = 0

                stdscr.move(j, i)
                if (i, j) == self.cursor.pos:
                    flags |= curses.A_UNDERLINE

                if target[2] > 0:
                    if (self.complete or
                            target[2] in [1, self.target] or
                            self.cursor.unit == unit.default):
                        flags |= curses.color_pair(target[2])
                else:
                    flags |= curses.A_DIM

                stdscr.addstr(target[0], flags)

        stdscr.move(w + 1, 0)
        stdscr.addstr("Score: {}".format(int(self.score)), flags)
        stdscr.addstr(" " * 20, flags)
        stdscr.move(w + 1, w - 4)
        stdscr.addstr("Rate: {}".format(self.cursor.rate), flags)
        stdscr.move(0, w - 4)
        stdscr.addstr("Turn: {}/8".format(1 + self.nb_victory % 8), flags)
        stdscr.move(1, w - 4)
        stdscr.addstr("Game: {}/8".format(1 + self.nb_victory // 8), flags)
        stdscr.move(0, 0)
        stdscr.refresh()
