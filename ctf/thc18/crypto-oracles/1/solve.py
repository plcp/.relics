import binascii
import socket
import sys

# connect to server
with socket.socket() as server:
    server.connect((sys.argv[1], int(sys.argv[2])))

    # receive payloads
    print(str(server.recv(32)[:-1], 'utf8'))
    for i in range(128):

        # send challenge (64 zeroes)
        print(str(server.recv(27) + server.recv(26), 'utf8'))
        server.sendall(binascii.hexlify(bytes(32)) + b'\n')

        # receive answer (64 nibbles)
        print(str(server.recv(22), 'utf8'), end='')
        chl = server.recv(64)

        # make a guess
        if chl[:32] == chl[32:]:
            print('[aes-ecb]')
            print(str(server.recv(46), 'utf8'))
            server.sendall(b'1\n')
        else:
            print('[entropy]')
            print(str(server.recv(46), 'utf8'))
            server.sendall(b'0\n')

    # display the flag
    print(str(server.recv(128), 'utf8'))
    print(str(server.recv(128), 'utf8'))
    print(str(server.recv(128), 'utf8'))
