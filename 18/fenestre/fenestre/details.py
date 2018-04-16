# -*- coding: utf-8 -*-

def tobytes(*various):
    for v in various:
        assert isinstance(v, (str, bytes))

    if len(various) == 1:
        if isinstance(various[0], str):
            return bytes(various[0], 'utf8')
        return bytes(various[0])
    else:
        return [tobytes(v) for v in various]

def norm(*various):
    return tobytes(*[v.lower() for v in various])

_event_count = 0
_event_reverse = {}
def event(event):
    if event not in _event_reverse:
        raise AttributeError("Event {} not found".format(event))
    return _event_reverse[event]

def make_events(*options):
    class events:
        def __init__(self):
            global _event_count, _event_reverse
            self._supported = {}
            for s in options:
                s = norm(s) # Event's name MUST be bytes or str

                _event_count += 1
                self._supported[s] = _event_count
                _event_reverse[int(_event_count)] = s

        def __contains__(self, event):
            if isinstance(event, int):
                return _event_reverse[event] in self._supported
            return norm(event) in self._supported

        def __getitem__(self, attr):
            if not self.__contains__(attr):
                raise AttributeError("Event {} not found".format(attr))
            return self._supported[norm(attr)]

        def __getattr__(self, attr):
            return self.__getitem__(attr)

    return events()
