import binascii
import logging
import crypto
import socket
import signal
import atexit
import time
import sys
import os

log = None
mkey = None
ppid = None
childs = []

def accept(client, addr):
    log.info(addr)
    client.sendall(b'...do computers have free will?\n')

    for i in range(128):
        key = os.urandom(16)

        motd = (b"\nLet's"
            + b' find out: '
            + bytes('({:3d}/128)'.format(i), 'utf8'))
        client.sendall(motd)

        client.sendall(b'\n - Please send 32 bytes: ')
        chl = binascii.unhexlify(client.recv(65)[:-1])
        if len(chl) != 32:
            client.sendall(b'!! Expected format: 64 nibbles + \\x0a\n')
            return

        client.sendall(b' - Here is my choice: ')
        pick = (ord(os.urandom(1)) & 1 == 0)

        if pick:
            client.sendall(binascii.hexlify(os.urandom(32)))
        else:
            client.sendall(binascii.hexlify(crypto.ecb.encrypt(key, chl)))

        client.sendall(b'\n - Was it entropy?')
        client.sendall(b'\n   - Yes (0)')
        client.sendall(b'\n   - No  (1) ')
        if int(b'1' == client.recv(1)) ^ int(not pick) != 0:
            client.sendall(b'\n...eh, told ya!\n\nYour oracle failed, ')
            client.sendall(b'computers do have free will, right?\n')
            return

        if client.recv(1) != b'\n':
            client.sendall(b'!! Missing \\x0a in answer!')
            return

    client.sendall(b"...meh, I guess that's how it should be?\n\n")
    with open('./flag', 'rb') as kfile:
        client.sendall(b'Here is the flag: ' + kfile.read() + b'\n')
        return

def wait(options=0, timeout=None):
    if timeout is not None:
        time.sleep(timeout)

    try:
        pid, status = os.waitpid(-1, options)
    except ChildProcessError:
        return False

    if os.WIFEXITED(status) or os.WIFSIGNALED(status):
        try:
            childs.remove(pid)
        except ValueError:
            pass
        return True

    return False

def fork(server, prefork=10, postfork=40, rate=0.1):
    server.listen()

    while True:
        if len(childs) < prefork:
            pid = os.fork()
            if pid > 0:
                childs.append(pid)
                continue

            for _ in range(postfork):
                client, addr = server.accept()
                try:
                    with client as client:
                        accept(client, addr)
                        time.sleep(0.1)
                except BaseException:
                    if logging.getLogger().level > logging.DEBUG:
                        break

                    import traceback
                    traceback.print_exc(file=sys.stderr)
            break

        while wait():
            pass

def collect(rate):
    signal.signal(signal.SIGINT, signal.SIG_IGN)
    signal.signal(signal.SIGTERM, signal.SIG_IGN)
    if ppid != os.getpid():
        return

    os.killpg(0, signal.SIGTERM)
    while wait(os.WNOHANG, rate):
        os.killpg(0, signal.SIGTERM)

    if len(childs) > 0:
        os.killpg(0, signal.SIGKILL)

if __name__ == "__main__":

    level = logging.ERROR
    if len(sys.argv) > 3:
        level = getattr(logging, sys.argv[3].upper())
    logging.basicConfig(level=level)
    log = logging.getLogger('server')

    try:
        os.setpgrp()
        ppid = os.getpid()
        atexit.register(collect, 0.1)

        with socket.socket() as server:
            server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
            server.bind((sys.argv[1], int(sys.argv[2])))
            fork(server)
    except KeyboardInterrupt:
        sys.exit()
