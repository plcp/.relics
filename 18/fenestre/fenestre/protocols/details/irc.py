# -*- coding: utf-8 -*-
import fenestre as ft

events = ft.etc.make_events(
    'ping', 'cap', 'nick',
    'user', 'join', 'quit',
    'mode', 'who', 'part',
    'privmsg', 'notice')

class _request_client(ft.net.request):
    def send(self, data=None, code=None, dst=None, raw=False):
        assert code is not None

        if data is None:
            data = b''
        if isinstance(code, int):
            code = ft.etc.event(code)
        code = code.upper()

        code, dst, data = ft.etc.tobytes(code, dst, data)
        if not raw and len(data) > 0:


class client:
    _protocol_class = None
    server_hostname = None
    user = None
    nick = None

    def event(self, request):
        from fenestre.network import events as network_events

        if request.event is network_events.connect:
            pass

class _request_server(ft.net.request):
    def send(self, data=None, src=None, code=None, dst=None, raw=False):
        if src is None:
            src = self.protocol.hostname
        if code is None:
            code = b'NOTICE'
        if dst is None:
            dst = self.protocol.hostname
        if data is None:
            data = b''

        src, code, dst, data = ft.etc.tobytes(src, code, dst, data)

        src = b':' + src
        if not raw and len(data) > 0:
            data = b':' + data

        self.parent.send(b' '.join([src, code, dst, data]))

class server:
    _protocol_class = None
    hostname = None

    def __init__(self):
        self.protocol = self._protocol_class()
        self.hostname = ft.etc.tobytes(self.hostname)

    def event(self, request):
        from fenestre.network import events as network_events

        if request.event is network_events.receive:
            req = None
            try:
                event, data = request.data.split(maxsplit=1)
                if event in events:
                    data = data
                    event = events[event]
                else:
                    data = None
                    event = network_events.receive
                req = _request_server(self,
                                        parent=request, event=event, data=data)
            except ValueError:
                self.protocol.event(request)
                return

            if req.event is events.ping:
                req.send(code=b'PONG', data=req.data)
            else:
                self.protocol.event(req)
        else:
            self.protocol.event(_request_server(self, parent=request))
