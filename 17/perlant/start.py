from world import world
import random

w = world(256, 256, random.randint(0, 2**32))
w.run()

