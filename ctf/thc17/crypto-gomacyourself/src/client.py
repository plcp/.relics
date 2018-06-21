#!/usr/bin/env python3

from twisted.internet.protocol import ClientFactory
from twisted.protocols.basic import LineReceiver
from twisted.internet import reactor
from binascii import unhexlify, hexlify
from crypto import *
import os

port = 1058
mtod = "Well, it's fine."

class ask4flag(LineReceiver):
    initquery = 1
    handshake = 2
    initchall = 3
    authchall = 4
    byeserver = 5
    mkey = None

    def __init__(self):
        self.delimiter = b'\n'

    def connectionMade(self):
        print("> Connection made: " + str(self.transport.getPeer()))
        self.state = self.initquery

    def kexchange(self):
        self.mkey = unhexlify(input("> Server key: "))
        context = get_context()

        # client to server integrity key
        self.ikey_c2s = derivate(
                self.mkey, 256, context, b"integrity", b"client2server")

        # server to client integrity key
        self.ikey_s2c = derivate(
                self.mkey, 256, context, b"integrity", b"server2client")

    def answer(self, payload, willtag = True):
        print("+ " + payload)
        padded = bytes(("+ " + payload).ljust(b64mac_len, ' '), "utf8")
        if(willtag):
            tag = b64encode(mac(self.ikey_c2s, padded))
        else:
            tag = b''
        self.sendLine(tag + padded)

    def lineReceived(self, line):
        if self.mkey is None:
            self.kexchange()
        print(str(line, "utf8"))

        if len(line) < b64mac_len + 1:
            print("> Invalid server payload: Invalid length")
            return

        tag = b64decode(line[:b64mac_len])
        msg = line[b64mac_len:]
        if not is_authenticated(self.ikey_c2s, tag, msg):
            print("> Invalid server payload: Invalid tag")
            return

        if self.state == self.initquery:
            self.answer(mtod, False)
            self.state += 1
        elif self.state == self.handshake:
            self.answer(mtod)
            self.state += 1
        elif self.state == self.initchall:
            self.answer("Fine.")
            self.state += 1
        elif self.state == self.authchall:
            challenge = get_challenge(unhexlify(msg[-64:]))
            self.answer("Here is the challenge: "
                    + str(hexlify(challenge), "ascii"))
            self.state += 1
        elif self.state == self.byeserver:
            self.answer("Goodbye !")
            self.state += 1
        else:
            return

    def connectionLost(self, reason):
        reactor.stop()

class factory(ClientFactory):
    protocol = ask4flag

factory = factory()
reactor.connectTCP("localhost", port, factory)
reactor.run()
