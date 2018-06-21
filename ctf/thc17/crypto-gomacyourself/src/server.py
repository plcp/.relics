#!/usr/bin/env python3

from twisted.internet.protocol import Factory
from twisted.protocols.basic import LineReceiver
from twisted.internet import reactor
from binascii import hexlify
from base64 import b64encode, b64decode
from crypto import *
import os, sys

flagone = 'THC{iS_tHis_4_fLAg_??_gOMAcyoUrsElF}'
flagtwo = 'THC{aRe_U_r3ally_sURe_GoMacYouRSeLf}'

port = 1058
max_size = 1024
max_history = 256

class serve(LineReceiver):
    error = -1
    initquery = 1
    handshake = 2
    initchall = 3
    authchall = 4
    challenge = None

    def __init__(self):
        self.delimiter = b'\n'
        self.state = self.initquery
        self.kexchange()

    def kexchange(self):
        context = get_context()

        # TODO: Add key exchange to get rid of pre-shared secrets
        self.mkey = os.urandom(16)
        print(" > Give this key to the client: "
                + str(hexlify(self.mkey), "ascii"), file=sys.stderr)

        # client to server integrity key
        self.ikey_c2s = derivate(
                self.mkey, 256, context, b"integrity", b"client2server")

        # server to client integrity key
        self.ikey_s2c = derivate(
                self.mkey, 256, context, b"integrity", b"server2client")

    def mac(self, *karg):
        tag = mac(*karg)
        if tag is None:
            self.transport.loseConnection()
            return b''
        else:
            return tag

    def answer(self, payload):
        try:
            padded = bytes(str(self.state) + "\r" +
                (": " + payload).ljust(b64mac_len + 1, ' '), "utf8")
            tag = b64encode(self.mac(self.ikey_c2s, padded))
        except:
            self.transport.loseConnection()
            return

        self.sendLine(tag + padded)
        self.history.append(tag)

    def connectionMade(self):
        self.history = []
        self.answer("What do you- what do you think of this f-flag server?")
        print("Connection made with: " + str(self.transport.getPeer()),
                file=sys.stderr)

    def connectionLost(self, reason):
        print("Connection lost [" + reason.getErrorMessage() + "]: "
                + str(self.transport.getPeer()), file=sys.stderr)

    # handle client requests
    def lineReceived(self, line):
        if len(line) > max_size:
            self.transport.loseConnection()
            return

        if len(self.history) > max_history:
            self.transport.loseConnection()
            return

        if len(line) >= b64mac_len and line[:b64mac_len] in self.history:
            self.answer("Well, you're retarded, just r-restart from"
                    + " the beg-beginning.")
            self.state = self.initquery
            return

        if self.state == self.initquery:
            self.answer("Look, I'll give you the f-flag, just say something"
                    + ", but authenticated.")
            self.state += 1
        elif self.state == self.handshake:
            if len(line) < b64mac_len:
                self.answer("No tag f-found.")
                self.transport.loseConnection()
                return

            try:
                tag = b64decode(line[:b64mac_len])
            except:
                self.transport.loseConnection()
                return

            msg = line[b64mac_len:]
            if not is_authenticated(self.ikey_c2s, tag, msg):
                self.answer("Invalid.")
                return

            self.answer("Well played, here- here is the first flag: " + flagone)
            self.history.clear()
            self.state += 1
        elif self.state == self.initchall:
            nounce = os.urandom(32)
            self.challenge = hexlify(get_challenge(nounce))
            self.answer("You want m-more ? Authenticate this challenge: "
                    + str(hexlify(nounce), "ascii"))
            self.state += 1
        elif self.state == self.authchall:
            if len(line) < b64mac_len:
                self.answer("No tag f-found.")
                self.transport.loseConnection()
                return

            try:
                tag = b64decode(line[:b64mac_len])
            except:
                self.transport.loseConnection()
                return

            msg = line[b64mac_len:]
            if ((not is_authenticated(self.ikey_c2s, tag, msg))
                    or (not self.challenge in line)):
                self.answer("Invalid.")
                return

            self.answer("Here- here is the second f-flag, my buddy: " + flagtwo)
            self.state += 1
        else:
            self.transport.loseConnection()
            return

# state-machine factory to handle clients
class flagserver(Factory):
    def __init__(self):
        pass

    def buildProtocol(self, addr):
        return serve()

if __name__ == "__main__":
    # retrieve the flags ; )
    with open('flag-1.txt', 'r') as rfile:
        flagone = rfile.read().replace('\n', ' ')
    with open('flag-2.txt', 'r') as rfile:
        flagtwo = rfile.read().replace('\n', ' ')

    # run the server
    reactor.listenTCP(port, flagserver())
    reactor.run()
