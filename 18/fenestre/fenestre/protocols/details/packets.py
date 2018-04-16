# -*- coding: utf-8 -*-
import struct

import fenestre as ft

class _request(ft.net.request):
    def send(self, data):
        header = self.protocol.header + struct.pack('!I', len(data))
        self.parent.send(header + data)

class packets:
    _protocol_class = None
    max_size = None
    header = None

    def __init__(self):
        self.protocol = self._protocol_class()
        self.reset()

    def reset(self):
        self.buffer = b''
        self.size = None

    def _get_size_from_buffer(self):
        while len(self.buffer) >= 8:
            payload = self.buffer[:4]
            self.buffer = self.buffer[4:]
            if payload != self.header:
                continue

            n = struct.unpack('!I', self.buffer[:4])[0]
            self.buffer = self.buffer[4:]
            if n > self.max_size:
                continue

            return n
        return None

    def _forward_request(self, request):
        data = self.buffer[:self.size]
        self.protocol.event(_request(self, parent=request, data=data))

        self.buffer = self.buffer[self.size:]
        self.size = None
        return

    def event(self, request):
        from fenestre.network import events

        if request.event is events.receive:
            self.buffer += request.data

            while True:
                if self.size is None:
                    self.size = self._get_size_from_buffer()
                    if self.size is None:
                        return

                if len(self.buffer) >= self.size:
                    self._forward_request(request)
                else:
                    return
        else:
            self.protocol.event(_request(self, parent=request))
