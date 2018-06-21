import binascii
import socket
import sys

# nice display (patch print to only display progress & flag)
printb = print
def print(*kargs, **kwargs):
    s = ''.join([str(k, 'utf8') for k in kargs])
    if 'truth' in s or 'will' in s or 'Now' in s:
        kwargs['end'] = ''
        printb(s, **kwargs)
    elif 'thc{' in s:
        printb('\n\n' + s, **kwargs)
    return

# connect to server
with socket.socket() as server:
    server.connect((sys.argv[1], int(sys.argv[2])))

    history = []
    print(server.recv(32))
    for i in range(128):
        print(server.recv(66))
        print(server.recv(26))

        chl = i.to_bytes(16, byteorder='big') + bytes(16)
        server.sendall(binascii.hexlify(chl) + b'\n')

        print(b'\n', server.recv(22))
        history.append(binascii.unhexlify(server.recv(96)))
        print(binascii.hexlify(history[-1][:8]), b'...')

    answers = []
    for i in range(128):
        print(server.recv(36))
        print(server.recv(26))

        chl = i.to_bytes(16, byteorder='big') + bytes(32)
        chl = bytes([a ^ b for a, b in zip(history[i], chl)])
        server.sendall(binascii.hexlify(chl) + b'\n')
        print(binascii.hexlify(chl[:8]), b'...')

        ask = b'\n' + server.recv(22)
        ans = server.recv(64)
        answers.append(ans.count(b'0') < 32)

        if answers[i]:
            print(ask, b'entropy    ', bytes('({:3}/128)'.format(i), 'utf8'))
        else:
            print(ask, b'ciphertext ', bytes('({:3}/128)'.format(i), 'utf8'))

        server.recv(58) # print(server.recv(58))
        if i == 127:
            break
        server.sendall(b'1\n')
    server.sendall(b'0\n')

    for i in range(128):
        print(server.recv(39))
        server.recv(43) # print(server.recv(43))
        if answers[i]:
            server.sendall(b'0\n')
        else:
            server.sendall(b'1\n')

    # display the flag
    print(server.recv(128))
    print(server.recv(128))
    print(server.recv(128))
