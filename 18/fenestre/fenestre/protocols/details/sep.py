# -*- coding: utf-8 -*-
import fenestre as ft

class _request(ft.net.request):
    def send(self, data):
        self.parent.send(data + self.protocol.separator)

class sep:
    _protocol_class = None
    separator = None
    max_size = None

    def __init__(self):
        self.protocol = self._protocol_class()
        self.reset()

    def reset(self):
        self.buffer = b''

    def _forward_request(self, request, idx):
        assert idx > 0

        data = self.buffer[:idx]
        self.protocol.event(_request(self, parent=request, data=data))

        self.buffer = self.buffer[idx + len(self.separator):]
        return

    def event(self, request):
        from fenestre.network import events
        if request.event is events.receive:
            self.buffer += request.data
            if len(self.buffer) >= self.max_size:
                dsize = len(self.buffer) - self.max_size
                self.buffer = self.buffer[dsize:]

            while True:
                idx = self.buffer.find(self.separator)
                if idx > 0:
                    self._forward_request(request, idx)
                else:
                    return
        else:
            self.protocol.event(_request(self, parent=request))
