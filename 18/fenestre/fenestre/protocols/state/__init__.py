# -*- coding: utf-8 -*-
import fenestre as ft

class bypeer(ft.net.state):
    def __init__(self, local):
        self.clients = {}
        self.local = local

    def get(self, peer, *keys):
        ret = []
        for key in keys:
            ret.append(self.clients[peer.handle][key])

        if len(ret) > 0:
            return ret
        return None

    def set(self, peer, **keys):
        for key in keys:
            self.clients[peer.handle][key] = keys[key]
        return self.clients[peer.handle]

    def add(self, peer, **keys):
        self.clients[peer.handle] = {}
        self.set(peer, **keys)

    def remove(self, peer):
        del self.clients[peer.handle]

    def has(self, peer):
        return peer.handle in self.clients

    def connect(self, peer):
        self.add(peer, ip=peer.addr.host, port=peer.addr.port)

    def disconnect(self, peer):
        self.remove(peer)

