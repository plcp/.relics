import random
import struct
import fenestre as ft

class client:
    n = 1
    def event(self, request):
        if request.event is ft.net.events.connect:
            request.send(struct.pack('!I', self.n))

        if request.event is ft.net.events.receive:
            self.n, = struct.unpack('!I', request.data)
            request.send(struct.pack('!I', self.n + 1))

        if self.n > 256:
            request.terminate()

class server:
    n = 0
    def event(self, request):
        if request.event is ft.net.events.connect:
            print('/ in: {}'.format(request.addr))

        if request.event is ft.net.events.receive:
            request.send(request.data)

            self.n, = struct.unpack('!I', request.data)
            if random.randint(0, self.n) == 0:
                request.disconnect()

        if request.event is ft.net.events.disconnect:
            print('| score: {}'.format(self.n))
            print('\ out: {}\n'.format(request.addr))

if __name__ == '__main__':
    import sys
    assert sys.argv[1] in ['client', 'server']
    host = sys.argv[2:4]

    if sys.argv[1] == 'server':
        ft.net.tcp.bind(ft.net.server(ft.p.packets(server)), host[0])
    else:
        ft.net.tcp.connect(ft.net.client(ft.p.packets(client)), *host)
