import fenestre as ft

class client:
    def event(self, request):
        if request.event is ft.net.events.connect:
            request.send(b'ping')

        if request.event is ft.net.events.receive:
            print(request.data)
            request.terminate()

class server:
    def event(self, request):
        if request.event is ft.net.events.receive:
            request.send(request.data)

if __name__ == '__main__':
    import sys
    assert sys.argv[1] in ['client', 'server']
    host = sys.argv[2:4]

    store = ft.id.load('./certs', './certs/frontends.d')
    if sys.argv[1] == 'server':
        ft.net.tls.bind(store, 'server', ft.net.server(server), host[0])
    else:
        ft.net.tls.connect(
            store, 'irssi', 'server', ft.net.client(client), *host)
