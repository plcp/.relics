#!/usr/bin/env python3

from twisted.internet.protocol import DatagramProtocol, ProcessProtocol
from twisted.internet import reactor, stdio
from readchar import readchar
from base64 import b64encode, b64decode
from time import time

import os, sys, tty, hmac, json, signal
import crypto

server_port = 48000
client_port = 4848

mkey = None
ikey = None
manager = None
userlist = None
pickuser = None
pickserver = None

class shellclient(DatagramProtocol):

    def answer(self, payload):
        try:
            if self.port is None or self.ikey is None:
                return
        except:
            return

        if isinstance(payload, str):
            payload = bytes(payload, 'utf8')

        mac = b64encode(hmac.new(self.ikey, payload).digest())
        payload += b'\n' + mac + b'\n\n'

        self.transport.write(payload, (self.serverhost, self.port))

    def startProtocol(self):
        username = None
        password = None
        for user in userlist:
            if user['username'] == pickuser:
                username = user['username']
                password = user['password']

        if username is None or username == b'':
            print('!! unknown user !!', file=sys.stderr)
            username = bytes(input('Username: '), 'utf8')

        if password is None or password == b'':
            from getpass import getpass
            password = bytes(getpass('Password: '), 'utf8')

        challenge, response = crypto.challenge(
                password,
                os.urandom(4),
                bytes(),
                username).digest()

        payload = (b"USER "
                + username
                + b" " + b64encode(challenge)
                + b" " + b64encode(response)
                )

        self.serverhost = pickserver
        self.username   = username
        self.password   = password
        self.response   = response

        self.skey = None
        self.ikey = None
        self.port = None
        self.drop = False
        self.carr = False

        self.transport.write(payload, (self.serverhost, server_port))

    def datagramReceived(self, payload, addr):
        raw = payload.split(b'\n')
        if len(raw) != 5 or raw[0][-10:] != b'ciphertext':
            print("!! Unexpected server payload !!", file=sys.stderr)
            try:
                print("\n" + str(payload, 'utf8'), file=sys.stderr)
            except:
                print(payload, file=sys.stderr)
            return

        if self.skey is None or self.ikey is None or self.port is None:
            plaintext = crypto.decrypt(
                    self.password + self.response,
                    b64decode(raw[1]))
        else:
            plaintext = crypto.decrypt(
                    self.skey,
                    b64decode(raw[1]))
        if plaintext is None:
            return

        req = plaintext[0:4]
        if req == b'ACPT':
            body = plaintext[5:].split(b'\n', 1)
            if(len(body) != 2):
                print("!! Invalid ACPT request !!", file=sys.stderr)
                return

            self.accept(body[1])
            return

        if self.drop:
            try:
                plaintext = plaintext.split(b'\n', 2)[2]
            except:
                pass
            self.drop = False

        try:
            print(str(plaintext, 'utf8'), end='', flush=True)
        except:
            print(plaintext, end='', flush=True)

        if(plaintext[-3:] == b']$ '):
            self.readline()
            self.drop = True

    def accept(self, payload):
        try:
            if args['session_key'] == self.skey:
                self.port = args['port'] + server_port
        except:
            self.port = None
            self.skey = None
            self.ikey = None

        try:
            args = json.loads(str(payload, 'utf8'))
        except:
            return

        try:
            self.port = args['port'] + server_port
            self.skey = b64decode(args['session_key'])
            self.ikey = hmac.new(self.skey, b'integrity_key').digest()
        except:
            self.port = None
            self.skey = None
            self.ikey = None
            return
        return

    def readline(self):
        key = ''
        nkey = 0
        while key != '\x0d':
            key = readchar()
            if key == '\x03' or key == '\x04' or key == '\x1a':
                self.detach()
                return
            if key == '\x1b':
                readchar()
                readchar()
                continue

            if key == '\x7f':
                if nkey > 0:
                    print('\b \b', end='', flush=True)
                    nkey -= 1
                else:
                    continue
            else:
                print(key, end='', flush=True)
                nkey += 1

            self.sendkey(key)

        if self.carr:
            print('\x1b[2K\r\x1b[1A', end='', flush=True)
        else:
            print('\n', end='', flush=True)
            self.carr = True

    def answ(self, payload, mode='plaintext', request='ANSW'):
        return request + ' ' + str(int(time())) + ' ' + mode + '\n' + payload

    def sendkey(self, key):
        ptxt = b'KEY' + bytes(key, 'utf8')
        ctxt = b64encode(crypto.encrypt(self.skey, ptxt))
        self.answer(self.answ(str(ctxt, 'utf8'), 'ciphertext'))

    def detach(self):
        sys.stdin.flush()
        sys.stdin.close()
        os.kill(os.getpid(), signal.SIGINT)

if __name__ == '__main__':

    try:
        client_port = int(sys.argv[2])
    except:
        client_port = 4848

    try:
        server_port = int(sys.argv[3])
    except:
        server_port = 48000

    manager = shellclient()
    with open('key.b64', 'rb') as keyfile:
        mkey = b64decode(keyfile.read())
        ikey = hmac.new(mkey, b'integrity_key').digest()

    with open('users.json', 'r') as userfile:
        userlist = json.loads(userfile.read())
        for user in userlist:
            user['username'] = bytes(user['username'], 'utf8')
            user['password'] = b64decode(user['password'])

    try:
        args = sys.argv[1].split('@')
        pickuser = bytes(args[0], 'utf8')
        pickserver = args[1]
    except:
        print('Usage: moche user@server'
            + '[<client_port>, <server_port>]')
        exit(1)


    reactor.listenUDP(client_port, manager)
    reactor.run()
