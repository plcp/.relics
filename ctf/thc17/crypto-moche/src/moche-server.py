#!/usr/bin/env python3

from twisted.internet.protocol import DatagramProtocol, ProcessProtocol
from twisted.internet import reactor
from base64 import b64encode, b64decode
from time import time
import os, sys, hmac, json, shlex
import crypto

server_port = 48000
client_port = 4848
rangeofport = 999
remote_host = None

mkey = None
ikey = None
manager = None
userlist = None

class subshell(ProcessProtocol):
    def __init__(self, parent):
        try:
            parent.shell.kill()
        except:
            pass
        finally:
            parent.shell = self
        self.parent = parent

    def answ(self, payload, mode='plaintext', request='ANSW'):
        return request + ' ' + str(int(time())) + ' ' + mode + '\n' + payload

    def outReceived(self, payload):
        self.parent.output(payload)

    def cmd(self, payload):
        if isinstance(payload, str):
            payload = bytes(payload, 'utf8')
        self.transport.write(payload + b'\n')

    def processExited(self, reason):
        self.parent.shell = None

    def processEnded(self, reason):
        self.parent.shell = None

    def kill(self):
        try:
            self.transport.closeStdin()
            self.transport.signalProcess('KILL')
            self.transport.signalProcess('HUP')
            self.transport.loseConnection()
        except:
            pass
        finally:
            self.parent.shell = None

class shellmoche(DatagramProtocol):
    def __init__(self, username, password, jargs, client):
        args = json.loads(jargs)
        rhost, rport = client

        self.username = username
        self.password = password
        self.active = time()
        self.rhost = rhost
        self.rport = rport
        self.shell = None
        self.port = args['port'] + server_port
        self.skey = b64decode(args['session_key'])
        self.ikey = hmac.new(self.skey, b'integrity_key').digest()
        self.num = self.port - server_port

        self.buffer = []
        self.popshell()

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

        self.transport.write(payload, (self.rhost, self.rport))

    def answ(self, payload, mode='plaintext', request='ANSW'):
        return request + ' ' + str(int(time())) + ' ' + mode + '\n' + payload

    def output(self, payload):
        ctxt = b64encode(crypto.encrypt(self.skey, payload))
        self.answer(self.answ(str(ctxt, 'utf8'), 'ciphertext'))

    def datagramReceived(self, payload, addr):
        raw = payload.split(b'\n')
        if len(raw) != 5 or raw[0][-10:] != b'ciphertext':
            return

        plaintext = crypto.decrypt(self.skey, b64decode(raw[1]))[0:4]
        if plaintext is None:
            return

        if plaintext[0:4] == b'KEY\r':
            self.keyreturn(raw, plaintext[3])

        if plaintext[0:3] == b'KEY':
            self.keystroke(raw, plaintext[3], addr)

        return

    def keystroke(self, raw, key, addr):
        mac = hmac.new(self.ikey, raw[0] + b'\n' + raw[1]).digest()
        if b64decode(raw[2]) != mac:
            return

        key = chr(key)
        if key == '\x7f' and len(self.buffer) > 0:
            self.buffer.pop()
            return

        self.rhost, self.rport = addr
        self.buffer.append(key)
        return

    def keyreturn(self, raw, key):
        user_input = ''.join(self.buffer)
        self.buffer.clear()

        if self.shell is None:
            self.popshell()
        self.shell.cmd(user_input)

    def popshell(self):
        penv = os.environ.copy()
        penv["SSHPASS"] = self.password
        cmd = shlex.split(''
                    + 'sshpass -e ssh -t -oStrictHostKeyChecking=no '
                    + str(self.username, 'utf8')
                    + '@' + str(remote_host))

        handler = subshell(self)
        reactor.spawnProcess(handler,
                'sshpass',
                args=cmd,
                env=penv,
                usePTY=True)

    def status(self):
        return (str(self.num) + ': '
                + '[' + str(self.username, 'utf8')
                + '@' + str(self.rhost)
                + ':' + str(self.rport)
                + '] Active since '
                + str(int(time() - self.active)) + 's')

class shellmaker(DatagramProtocol):
    def __init__(self):
        self.nport = 1
        self.shells = []

    def answer(self, payload, addr):
        if isinstance(payload, str):
            payload = bytes(payload, 'utf8')

        mac = b64encode(hmac.new(ikey, payload).digest())
        payload += b'\n' + mac + b'\n\n'

        host, port = addr
        self.transport.write(payload, (host, client_port))

    def datagramReceived(self, payload, addr):
        req = payload[0:4]

        if req == b'HELP':
            self.answer(self.help(), addr)

        if req == b'LIST':
            self.answer(self.list(), addr)

        if req == b'USER':
            self.answer(self.user(payload[5:], addr), addr)

    def answ(self, payload, mode='plaintext', request='ANSW'):
        return request + ' ' + str(int(time())) + ' ' + mode + '\n' + payload

    def help(self):
        return self.answ(''
                + 'Usage:'                        + '\n'
                + ' - HELP: get this help'        + '\n'
                + ' - LIST: list active sessions' + '\n'
                + ' - USER: connect as real user')

    def list(self):
        answer = ''
        answer += 'Active sessions:'               + '\n'
        for shell in self.shells:
            answer += ' - ' + shell.status()       + '\n'
        if len(self.shells) == 0:
            answer += ' - (None)'                  + '\n'
        answer += 'Total: '
        answer += str(len(self.shells))
        return self.answ(answer)

    def user(self, payload, addr):
        raw = payload.split(b' ')
        if len(raw) != 3:
            return self.answ('!! Invalid payload !!', 'error')

        username = None
        password = None
        for user in userlist:
            if user['username'] == raw[0]:
                username = user['username']
                password = user['password']

        if username is None or password is None:
            return self.answ('!! Invalid user !!', 'error')

        challenge = b64decode(raw[1])
        response  = b64decode(raw[2])
        if crypto.challenge(password, challenge, response, username).isvalid():
            return self.answ('!! Invalid challenge !!', 'error')

        if self.nport + 1 > 999:
            self.shells.clear()
            self.nport = 1
            return self.answ('!! No available slot !!', 'error')

        skey = os.urandom(16)
        jmsg = ('{"port":'
                    + str(self.nport)
                    + ', "session_key":"'
                    + str(b64encode(skey), 'utf8')
                    + '"}')

        shell = shellmoche(username, password, jmsg, addr)
        self.shells.append(shell)
        self.nport += 1

        bmsg = bytes(self.answ(jmsg, request='ACPT'), 'utf8')
        ctxt = b64encode(crypto.encrypt(password + response, bmsg))

        reactor.listenUDP(shell.port, shell)

        return self.answ(str(ctxt, 'utf8'), 'ciphertext')

if __name__ == '__main__':

    try:
        args = sys.argv[1]
        remote_host = args
    except:
        print('Usage: moche-server <remote_host>'
            + '[<client_port>, <server_port>, <rangeofport>]')
        exit(1)

    try:
        client_port = int(sys.argv[2])
    except:
        client_port = 4848

    try:
        server_port = int(sys.argv[3])
    except:
        server_port = 48000

    try:
        rangeofport = int(sys.argv[4])
    except:
        rangeofport = 999

    with open('key.b64', 'rb') as keyfile:
        mkey = b64decode(keyfile.read())
        ikey = hmac.new(mkey, b'integrity_key').digest()

    with open('users.json', 'r') as userfile:
        userlist = json.loads(userfile.read())
        for user in userlist:
            user['username'] = bytes(user['username'], 'utf8')
            user['password'] = b64decode(user['password'])
    manager = shellmaker()

    reactor.listenUDP(server_port, manager)
    reactor.run()

