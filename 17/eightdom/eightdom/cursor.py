import sys

import eightdom.unit as unit


class cursor:
    up = [259, 169, 122, 119]
    down = [258, 117, 115, 115]
    left = [260, 97, 113, 97]
    right = [261, 105, 100, 100]
    space = [32]
    exit = [27]

    mrate = 16

    def __init__(self, world, owner):
        self.world = world
        self.ops = []

        self.reset(owner)

    def reset(self, owner):
        self.owner = owner
        self.unit = unit.default
        self.rate = self.mrate
        self.pos = (self.world.width // 2, self.world.width // 2)
        self.n = 0
        self.m = 0
        self.p = 0

        self.permute(owner)
        self.pop()
        self.move(self.space[0], False)

    def permute(self, p):
        w = self.world.width
        c = self.world.coind

        if self.p > 0:
            self.home = (1 + c, w - 2)

            self.up = self.__class__.up
            self.down = self.__class__.down
            self.left = self.__class__.left
            self.right = self.__class__.right

            self.p = 0
            return

        if False:
            pass
        elif p == 1:
            # left down down
            self.home = (1 + c, w - 2)
        elif p == self.world.config[0]:
            # right down down
            self.home = (w - 2 - c, w - 2)
            self.up, self.down = self.__class__.up, self.__class__.down
            self.left, self.right = self.__class__.right, self.__class__.left
        elif p == self.world.config[1]:
            # right down up
            self.home = (w - 2, w - 2 - c)
            self.up, self.down = self.__class__.right, self.__class__.left
            self.left, self.right = self.__class__.up, self.__class__.down
        elif p == self.world.config[2]:
            # right up down
            self.home = (w - 2, 1 + c)
            self.up, self.down = self.__class__.left, self.__class__.right
            self.left, self.right = self.__class__.up, self.__class__.down
        elif p == self.world.config[3]:
            # right up up
            self.home = (w - 2 - c, 1)
            self.up, self.down = self.__class__.down, self.__class__.up
            self.left, self.right = self.__class__.right, self.__class__.left
        elif p == self.world.config[4]:
            # left up up
            self.home = (1 + c, 1)
            self.up, self.down = self.__class__.down, self.__class__.up
            self.left, self.right = self.__class__.left, self.__class__.right
        elif p == self.world.config[5]:
            # left up down
            self.home = (1, 1 + c)
            self.up, self.down = self.__class__.left, self.__class__.right
            self.left, self.right = self.__class__.down, self.__class__.up
        elif p == self.world.config[6]:
            # left down up
            self.home = (1, w - 2 - c)
            self.up, self.down = self.__class__.right, self.__class__.left
            self.left, self.right = self.__class__.down, self.__class__.up
        else:
            raise BaseException("p={} unknown".format(p))

        self.p = p

    def pop(self, what=None):
        if what is None:
            what = ('1', 1, self.owner)

        target = self.world.world[self.home[0]][self.home[1]]
        if target[2] in [0, self.owner] and what[1] > target[1]:
            self.world.world[self.home[0]][self.home[1]] = what
            if self.pos == self.home:
                self.repick()

            c = 0
            for i in range(0, self.world.width):
                for j in range(0, self.world.width):
                    t = self.world.world[i][j]
                    if t[1] == 1 and t[2] == self.owner:
                        c += 1
            self.rate = self.mrate - c // 2

        elif target[2] != self.owner:

            if target[1] == 1:
                cnew = (' ', 0, 0)
            elif target[1] == 2:
                cnew = ('1', 1, target[2])
            else:
                return

            self.world.world[self.home[0]][self.home[1]] = cnew

            c = self.world.get_cursor(target[2])
            if c.pos == self.home:
                c.repick()

        else:
            pass

    def get(self, x=0, y=0):
        return self.world.world[self.pos[0] + x][self.pos[1] + y]

    def set(self, value, x=0, y=0):
        self.world.world[self.pos[0] + x][self.pos[1] + y] = value
        return self.get(x, y)

    def tick(self):
        if len(self.ops) == 0:
            return
        if len(self.ops) > self.n:
            self.move(self.ops[self.n], False)
            self.n += 1
        elif len(self.ops) == self.n:
            self.reset(self.owner)

    def repick(self):
        if self.get()[2] == self.owner:
            self.unit = unit.pick(self.get()[1])
        else:
            self.unit = unit.default

    def apply(self, move, neg=False):
        if neg:
            self.pos = (self.pos[0] - move[0], self.pos[1] - move[1])
        else:
            self.pos = (self.pos[0] + move[0], self.pos[1] + move[1])

    def move(self, key, add=True):

        if False:
            pass
        elif key in self.up:
            self.unit(self, (0, -1))
        elif key in self.down:
            self.unit(self, (0, 1))
        elif key in self.left:
            self.unit(self, (-1, 0))
        elif key in self.right:
            self.unit(self, (1, 0))
        elif key in self.space:
            if self.unit != unit.default:
                self.unit = unit.default
            elif self.get()[2] == 0:
                self.pos = self.home
                self.repick()
            else:
                self.repick()
        elif key in self.exit:
            sys.exit()
        elif not add:
            raise BaseException(
                ("Keystroke {} recorded by {}:" +
                 "\n\tup {}\n\tdown {}\n\tleft {}\n\tright {}").format(
                     key, self.owner, self.up, self.down, self.left,
                     self.right))
        else:
            return

        if add:
            self.ops.append(key)

        p = max(4, self.rate)
        if self.m % p == 0:
            self.pop()

        p = max(4, self.mrate * 4 + self.rate)
        if (self.m + 1) % p == 0:
            self.pop(('2', 2, self.owner))

        p = max(4, self.mrate * 16 + self.rate)
        if (self.m + 3) % p == 0:
            self.pop(('3', 3, self.owner))

        self.m += 1
