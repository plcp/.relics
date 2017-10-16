import numpy as np
import cervel

class identity(cervel.component):
    def evaluate(self, values, forward):
        return values

class atom(cervel.component):
    def __init__(self, value):
        cervel.component.__init__(self)
        self._value = value

    def evaluate(self, values, forward):
        if forward:
            return [self._value]
        else:
            errors = []
            for peer in self.inputs:
                errors.append(self._value - peer.value(self))
            return errors

class product(cervel.component):
    nb_inputs = 2

    def evaluate(self, values, forward):
        if forward:
            values = [v for v in values if v is not None]
            product = np.ones(self.size)
            for value in values:
                product *= value
            return [product]
        else:
            value, = self.value()
            error, = values
            errors = []
            for peer in self.inputs:
                _error = np.zeros(self.size)
                _input = peer.value(self)
                _where = np.abs(_input) > 1e-10
                _error[_where] = error[_where] * value[_where] / _input[_where]

                errors.append(_error)
            return errors

class state(cervel.component):
    def __init__(self):
        cervel.component.__init__(self)

    def reset(self):
        cervel.component.reset(self)
        default_value = np.random.uniform(low=-1.0, size=self.size)
        self.forward_history[self.date + 1] = default_value

    def forward(self):
        date = self.date + 1
        self.date = date

        peer, = self.inputs
        if peer.date < date:
            peer.forward()

        self.forward_history[date + 1] = [peer.value(self)]

    def backward(self):
        date = self.date - 1

        peer, = self.outputs
        error = peer.error(self, when=2)
        if error is None:
            error = np.zeros(self.size)

        self.backward_history[date] = error
