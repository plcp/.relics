import matplotlib.pyplot as plt
import matplotlib.animation as animation

class draw:
    def __init__(self, callback):
        self.callback = callback
        self.tick = 0

    def start(self, first):
        self.figure = plt.figure()
        self.axes = plt.axes([0, 0.03, 1, 0.97])
        self.axes.set_axis_off()

        def _draw(event):
            self.tick += 1
            data = self.callback(self, self.tick, event)
            self.im.set_data(data)
            self.figure.canvas.draw_idle()

        self.handler = animation.FuncAnimation(self.figure,
            _draw, repeat=True, interval=30)

        self.im = self.axes.imshow(first, origin='upper')
        plt.show()

