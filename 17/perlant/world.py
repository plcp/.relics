import numpy as np
import random
import entity
import perlin
import draw

def gen(size, seed):
    return perlin.generate(size, seed) / 3 + 0.1

class world:
    def __init__(self, size, nb_entities, seed=2):
        self.map = gen(size, seed)
        self.seed = seed
        self.size = size
        self.origin = self.map
        self.entities = [entity.entity(self) for i in range(0, nb_entities)]

    def tick(self, tick, world, drawer, event):
        self.entities.sort(key=lambda x: x.score)

        if tick % 8 == 0:
            self.entities.pop()
            self.entities.insert(0, entity.entity(self))

        rate = 2
        for i in range(0, rate):
            self.entities.pop(len(self.entities) -
                np.random.randint(1, len(self.entities) // 2))

        candidates = random.sample(self.entities[:len(self.entities)//2], rate)
        for c in candidates:
            self.entities.insert(0, entity.entity(self, c))

        for e in self.entities:
            e.tick()

        if tick % 8 == 0:
            self.map = (self.map * 31 + self.origin) * (1.0 / 32)

        if tick % 4096 == 0:
            self.origin = gen(self.size, self.seed + tick // 512)

    def map_view(self):
        map_view = self.map.copy()
        for entity in self.entities:
            entity.draw(map_view)
        return map_view

    def run(self):
        def _tick(drawer, tick, event):
            self.tick(tick, self, drawer, event)
            return self.map_view()

        self.drawer = draw.draw(_tick)
        self.drawer.start(self.map)
