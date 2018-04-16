# -*- coding: utf-8 -*-
import inspect
import enum
import zope

from twisted.internet import reactor, task, defer, ssl, error
from twisted.internet.interfaces import ISSLTransport, IHandshakeListener
from twisted.internet.protocol import (
    Protocol, ReconnectingClientFactory, Factory)

import fenestre as ft

events = ft.etc.make_events('connect', 'disconnect', 'receive')

class factory:
    def build(self, local):
        raise NotImplementedError

class _default_factory(factory):
    def __init__(self, _class):
        assert inspect.isclass(_class)
        self._class = _class

    def build(self, local):
        try:
            return self._class(local)
        except TypeError:
            return self._class()

class request:
    def __init__(self, protocol, parent=None, event=None, data=None):
        handle = None
        if parent is not None:
            handle = parent.handle
            if data is None:
                data = parent.data
            if event is None:
                event = parent.event

        if handle is None:
            handle = self

        self.protocol = protocol
        self.handle = handle
        self.parent = parent
        self.event = event
        self.data = data

    @property
    def event(self):
        return self._event

    @property
    def state(self):
        if self.handle is not None:
            return self.handle.local.state
        return self.parent.state

    @property
    def handler(self):
        if self.handle is None or self.handle is self:
            return self
        return self.handle.handler

    @event.setter
    def event(self, value):
        assert isinstance(value, int)
        self._event = value

    @handler.setter
    def handler(self, value):
        if not (self.handle is None or self.handle is self):
            self.handle.handler = value

    def __getattr__(self, attr):
        if self.parent is None:
            raise AttributeError(
                "request has no attribute '{}' nor parent".format(attr))
        return getattr(self.parent, attr)

class state:
    def __init__(self, local):
        pass

    def die(self):
        pass

    def connect(self, peer):
        pass

    def activate(self, peer):
        pass

    def disconnect(self, peer):
        pass

    def terminate(self, peer):
        pass

class server(Factory):
    def __init__(self, protocol_factory, state_factory=None):
        if state_factory is None:
            state_factory = _default_factory(state)

        if not isinstance(state_factory, factory):
            state_factory = _default_factory(state_factory)

        if not isinstance(protocol_factory, factory):
            protocol_factory = _default_factory(protocol_factory)

        self.protocol_factory = protocol_factory
        self.state_factory = state_factory
        self.state = None
        self.task = None
        self.reset()

    def reset(self):
        if self.state is not None:
            self.state.die()
        self.state = self.state_factory.build(self)

    def connect(self, peer):
        self.state.connect(peer)

    def activate(self, peer):
        self.state.activate(peer)

    def disconnect(self, peer):
        self.state.disconnect(peer)

    def terminate(self, peer):
        self.state.terminate(peer)
        if self.task is not None:
            self.task.callback(None)
            self.task = None

    def buildProtocol(self, addr):
        return protocol(addr, self, self.protocol_factory.build(self))

class client(ReconnectingClientFactory):
    def __init__(self, protocol_factory, state_factory=None):
        if state_factory is None:
            state_factory = _default_factory(state)

        if not isinstance(state_factory, factory):
            state_factory = _default_factory(state_factory)

        if not isinstance(protocol_factory, factory):
            protocol_factory = _default_factory(protocol_factory)

        self.protocol_factory = protocol_factory
        self.state_factory = state_factory
        self.active = None
        self.state = None
        self.task = None
        self.reset()

    def reset(self):
        if self.state is not None:
            self.state.die()
        self.state = self.state_factory.build(self)

    def connect(self, peer):
        self.state.connect(peer)

    def activate(self, peer):
        self.state.activate(peer)
        self.resetDelay()

    def disconnect(self, peer):
        self.state.disconnect(peer)

    def terminate(self, peer):
        self.state.terminate(peer)
        self.stopTrying()
        if self.task is not None:
            self.task.callback(None)
            self.task = None

    def buildProtocol(self, addr):
        if self.active is None:
            self.active = self.protocol_factory.build(self)
        return protocol(addr, self, self.active)

@zope.interface.implementer(IHandshakeListener)
class protocol(Protocol):
    def __init__(self, addr, local, protocol):
        self.addr = addr
        self.local = local
        self.handle = self
        self.handler = self
        self.active = False
        self.protocol = protocol
        self.reset()

    def reset(self, event=None, data=None):
        self.data = data
        self.event = event

    @property
    def tls(self):
        return ISSLTransport.providedBy(self.transport)

    def later(self, time, callback, *args):
        return reactor.callLater(time, callback, *args)

    def terminate(self, abort=False):
        if abort:
            self.abort()
        else:
            self.disconnect()
        self.local.terminate(self)

    def disconnect(self):
        self.transport.loseConnection()

    def abort(self):
        self.transport.abortConnection()

    def send(self, data):
        self.transport.write(data)

    def connectionMade(self):
        if not self.tls:
            self.connect_event()

    def handshakeCompleted(self):
        self.connect_event()

    def connect_event(self, retries=0):
        self.reset(events.connect)
        self.protocol.event(self)

        self.local.connect(self)

    def connectionLost(self, reason):
        self.disconnect_event(reason)

    def disconnect_event(self, reason):
        self.reset(events.disconnect, reason)
        self.protocol.event(self)
        self.local.disconnect(self)

    def dataReceived(self, data):
        self.reset(events.receive, data)
        self.protocol.event(self)

        if not self.active:
            self.active = True
            self.local.activate(self)

class BaseError(BaseException):
    def __init__(self, report):
        self.report = report

class BindError(BaseError):
    def __str__(self):
        return str(self.report.socketError)

class ConnectError(BaseError):
    def __str__(self):
        return str(self.report.osError)

def _try(callback, *args):
    try:
        return callback(*args)
    except error.BindError as e:
        raise BindError(e)
    except error.ConnectError as e:
        raise ConnectError(e)

class tcp:
    @staticmethod
    def connect(local, port, host):
        port = int(port)

        @defer.inlineCallbacks
        def _task(reactor):
            local.task = defer.Deferred()
            _try(reactor.connectTCP, host, port, local)
            yield local.task
        task.react(_task)

    @staticmethod
    def bind(local, port):
        port = int(port)

        @defer.inlineCallbacks
        def _task(reactor):
            local.task = defer.Deferred()
            _try(reactor.listenTCP, port, local)
            yield local.task
        task.react(_task)

class tls:
    @staticmethod
    def connect(store, client, server, local, port, host, dhparam_id=0):
        port = int(port)

        @defer.inlineCallbacks
        def _task(reactor):
            opt = ssl.optionsForClientTLS(
                store.remote[server].name,
                store.authority.cert,
                store.hosted[client].cert,
                extraCertificateOptions=dict(
                    dhParameters=store.diffie[dhparam_id]))

            local.task = defer.Deferred()
            _try(reactor.connectSSL, host, port, local, opt)
            yield local.task
        task.react(_task)

    @staticmethod
    def bind(store, server, local, port, dhparam_id=0):
        port = int(port)

        @defer.inlineCallbacks
        def _task(reactor):
            opt = ssl.CertificateOptions(
                privateKey=store.hosted[server].cert.privateKey.original,
                certificate=store.hosted[server].cert.original,
                trustRoot=ssl.trustRootFromCertificates([
                    store.authority.cert]),
                dhParameters=store.diffie[dhparam_id])

            local.task = defer.Deferred()
            _try(reactor.listenSSL, port, local, opt)
            yield local.task
        task.react(_task)
