import numpy as np

class component:
    size = 8
    nb_inputs = 1
    nb_outputs = 1

    def __init__(self):
        self.inputs = []
        self.outputs = []
        self.reset()

    def reset(self):
        self.date = 0
        self.forward_history = {}
        self.backward_history = {}
        for _input in self.inputs:
            _input.reset()
        for _output in self.outputs:
            _output.reset()

    def value(self, whom=None, when=0):
        date = self.date - when
        if not date in self.forward_history:
            return None

        if whom is None:
            return [np.copy(v) for v in self.forward_history[date]]

        if not whom in self.outputs:
            raise RuntimeError("Unable to route {}".format(whom))
        whom = self.outputs.index(whom)

        return np.copy(self.forward_history[date][whom])

    def error(self, whom=None, when=1):
        date = self.date - when
        if not date in self.backward_history:
            return None

        if whom is None:
            return [np.copy(v) for v in self.backward_history[date]]

        if not whom in self.inputs:
            raise RuntimeError("Unable to route {}".format(whom))
        whom = self.inputs.index(whom)

        return np.copy(self.backward_history[date][whom])

    def input(self, *peer, reverse=True):
        if len(peer) == 1:
            self.inputs.insert(0, peer[0])
            if len(self.inputs) > self.nb_inputs:
                self.inputs.pop()

            if reverse:
                peer[0].output(self, reverse=False)
        else:
            for p in peer:
                self.input(p, reverse=reverse)

    def output(self, *peer, reverse=True):
        if len(peer) == 1:
            self.outputs.insert(0, peer[0])
            if len(self.outputs) > self.nb_outputs:
                self.outputs.pop()

            if reverse:
                peer[0].input(self, reverse=False)
        else:
            for p in peer:
                self.output(p, reverse=reverse)

    def forward(self):
        date = self.date + 1
        self.date = date

        inputs = []
        for peer in self.inputs:
            if peer.date < date:
                peer.forward()
            inputs.append(peer.value(self))

        self.forward_history[date] = self.evaluate(inputs, forward=True)
        assert isinstance(self.forward_history[date], list)

    def backward(self):
        date = self.date - 1

        errors = []
        for peer in self.outputs:
            if peer.error(self) is None:
                peer.backward()
            errors.append(peer.error(self))

        self.backward_history[date] = self.evaluate(errors, forward=False)
        assert isinstance(self.backward_history[date], list)

    def evaluate(self, values, forward):
        raise NotImplementedError
