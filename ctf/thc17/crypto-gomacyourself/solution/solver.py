from twisted.internet.protocol import ClientFactory
from twisted.protocols.basic import LineReceiver
from twisted.internet import reactor
from binascii import unhexlify, hexlify
from crypto import *
import os, sys

port = 1058

# stabilizing payload
stabilizer = (b''              # This payload exploit 0 as a stable fixed point
    + b'\x00' * 128            # Increment as much as possible permutation 0
    + bytes(range(0x01, 0x0A)) # Disrupt perm. where perm. 0 may got stuck
    + bytes(range(0x0B, 0xFF)) # (Without \n in your payload, cf. LineReceiver)
    + b'\x00' * 128            # Push forward again perm. 0 by incrementing it
    + bytes(range(0x7F, 0xFF)) # Disrupt again (high prob. to be in 2nd half)
    + b'\x00' * 256            # Finalize your stabilization with one more pass
    + b'')

# stabilized tag
stag = b'\x00' * 16 # 0...0 used as nounce, stabilized payload tagged with 0...0

# reuse the provided client
class ask4flag(LineReceiver):
    initquery = 1  # two states needed : initquery and authenticating payloads
    byeserver = 64 # try 64 times then restart to try with a renewed key

    def __init__(self):
        self.delimiter = b'\n'            # delimiter (cf. LineReceiver)
        self.challenge = b''              # the challenge to authenticate
        self.incr      = os.urandom(1)[0] # initial value of the increment

        # The so-called increment is needed to get different payloads to
        # authenticate, as we may be unable to stabilize a given payload.
        #
        # Each time we will try to authenticate a payload, we will increment
        # this nounce-like value to ensure that we will authenticate a new
        # payload the next try.

    def connectionMade(self):
        self.state = self.initquery

    def answer(self, payload, tag = b''):
        tag = b64encode(tag)
        self.sendLine(tag + payload)

    def lineReceived(self, line):

        # display server output only if we got a flag
        if b'here' in line:
            print('\x1b[1K\r', end='', flush=True)
            print(str(line, 'utf8'))

        # welcome the server with an empty payload
        if self.state == self.initquery:
            self.answer(b'', b'')
            self.state += 1

        # try self.byeserver times to authenticate some stabilized payloads
        elif self.state < self.byeserver:

            # update the nounce-like value to get new payload each time we try
            self.incr = (self.incr + 1) % 256
            if self.incr == ord(self.delimiter): # cf. LineReceiver
                self.incr += 1
            self.state += 1

            # update our challenge with the one given by the server, if given
            if b'Authenticate this challenge: ' in line:
                self.challenge = hexlify(get_challenge(unhexlify(line[-64:])))
                self.state = self.initquery + 1  # reset the try-again counter

            # try to authenticate a stabilized payload
            self.answer(b''
                + self.challenge     # arbitrary payload we try to authenticate
                + bytes([self.incr]) # seed the payload with a nounce-like value
                + stabilizer         # stabilize the payload
                , stag)              # the valid tag is this one with high-prob.

            # try to fool muggles into believing that our exploit is a magic one
            print('.', end='', flush=True)
            if self.incr % 80 == 0:
                print('\x1b[1K\r', end='', flush=True)
        else:

            # restart the exploit we a renewed key, as we failed with this one
            self.transport.loseConnection()
            os.execlp('python', 'python', *sys.argv)
            return

    def connectionLost(self, reason):
        reactor.stop()

class factory(ClientFactory):
    protocol = ask4flag

factory = factory()
reactor.connectTCP("localhost", port, factory)
reactor.run()
