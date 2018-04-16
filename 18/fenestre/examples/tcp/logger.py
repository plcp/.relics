import binascii
import fenestre as ft

class logger:
    def event(self, request):
        if request.event is ft.net.events.connect:
            print(' > connected: {}'.format(request.addr))
        if request.event is ft.net.events.disconnect:
            print(' > disconnected: {}'.format(request.addr))
        if request.event is ft.net.events.receive:
            print(' >> received:')
            print('    - raw: {}'.format(request.data))
            print('    - hex: {}'.format(binascii.hexlify(request.data)))

if __name__ == '__main__':
    ft.net.tcp.bind(ft.net.server(ft.p.sep(logger)), '6667')
