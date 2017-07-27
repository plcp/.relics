import numpy as np

def resolve(e, dx, dy):
    w = e.world.size
    x = (e.x + dx) % w
    y = (e.y + dy) % w
    return (x, y)

class entity:
    def __init__(self, world, parent=None):
        self.world = world

        if parent is None:
            self.x = np.random.randint(0, world.size)
            self.y = np.random.randint(0, world.size)
            self.brain = np.random.rand(20, 41)
        else:
            self.x = parent.x
            self.y = parent.y
            self.brain = parent.brain + np.random.rand(20, 41) * 1e-2

        self.dead = False
        self.score = -1.0
        self.state = np.random.rand(16)

    def get(self, dx=0, dy=0):
        x, y = resolve(self, dx, dy)
        return self.world.map[x][y]

    def add(self, v, dx=0, dy=0):
        x, y = resolve(self, dx, dy)
        self.world.map[x][y] += v

    def tick(self):
        x = self.get()
        if self.dead or x < -0.15:
            self.dead = True
            self.score = 1.0
            return

        self.add(1e-2)

        n = 0
        around = np.zeros(25)
        for i in range(-2, 3):
            for j in range(-2, 3):
                around[n] = self.get(i, j)
                n += 1
        _input = np.concatenate([around, self.state])
        _output = np.sum(_input * self.brain, axis=1)

        self.move(_output[:4])
        self.state = np.tanh(_output[4:])

        self.add(1e-2 * -2)

        self.score *= (1 - 1e-4)
        self.score -= x * 1e-3

    def move(self, x):
        soft = np.exp(x) / np.sum(np.exp(x), axis=0)
        sdir = np.random.choice(4, 1, p=soft)[0]

        move = [(1, 0), (0, 1), (-1, 0), (0, -1)][sdir]
        self.x, self.y = resolve(self, *move)

    def draw(self, map_view):
        if not self.dead:
            map_view[self.x][self.y] = -1

