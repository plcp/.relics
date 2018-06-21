import binascii
import socket
import sys

# nice display (patch print to only display progress & flag)
printb = print
def print(*kargs, **kwargs):
    s = ''.join([str(k, 'utf8') for k in kargs])
    if 'meta' in s or 'will' in s:
        kwargs['end'] = ''
        printb(s, **kwargs)
    elif 'thc{' in s:
        printb('\n\n' + s, **kwargs)
    return

# connect to server
with socket.socket() as server:
    server.connect((sys.argv[1], int(sys.argv[2])))

    # use the chosen-plaintext oracle to build nonce->aes(key, nonce) dict
    i = 0
    history = {}
    print(server.recv(32))
    while True:
        print(server.recv(36), bytes(str(len(history.keys())), 'utf8'))
        print(server.recv(26))

        # use a counter to ask encrypt different payloads each time
        # (the oracle don't accept duplicates)
        i += 1
        chl = i.to_bytes(32, byteorder='big')
        server.sendall(binascii.hexlify(chl) + b'\n')

        # retrieve our plaintext ciphered
        print(server.recv(38))
        ans = binascii.unhexlify(server.recv(66))
        print(server.recv(54))

        # sanity
        assert len(ans) == 33
        assert len(chl) == 32

        # exit when our one-byte nonce dictionary is complete
        if len(history.keys()) > 255:
            server.sendall(b'0\n')
            break

        # continue & build our dictionary of OTP-ed AES-CTR payloads
        server.sendall(b'1\n')
        history[ans[0]] = bytes(l ^ r for l, r in zip(chl, ans[1:]))

    # solve the challenge equipped with our dictionary
    for j in range(1, 128 + 1):
        print(server.recv(66))
        print(server.recv(26))

        # send a payload
        chl = (i + j).to_bytes(32, byteorder='big')
        server.sendall(binascii.hexlify(chl) + b'\n')

        # retrieve one challenge
        print(server.recv(22))
        ans = binascii.unhexlify(server.recv(66))
        print(server.recv(46))

        # from the one-byte nonce, retrieve recorded aes(key, nonce)
        h = history[ans[0]]
        c = bytes([l ^ r for l, r in zip(ans[-32:], h)])

        # do we retrieve the original payload or garbage?
        if c == chl:
            print(b'[aes-ctr]')
            server.sendall(b'1\n')
        else:
            print(b'[entropy]')
            server.sendall(b'0\n')

    # display the flag
    print(server.recv(128))
    print(server.recv(128))
    print(server.recv(128))
