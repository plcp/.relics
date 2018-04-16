# -*- coding: utf-8 -*-
import fenestre as ft

import fenestre.protocols.details
import fenestre.protocols.details.irc as etc

import inspect


events = etc.events

def server(protocol, hostname='localhost'):
    assert inspect.isclass(protocol)
    class _irc_server_details(etc.server):
        pass

    _irc_server_details._protocol_class = protocol
    _irc_server_details.hostname = hostname

    return _irc_server_details
