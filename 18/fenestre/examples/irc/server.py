import binascii
import random
import time

import fenestre as ft
import fenestre.protocols.irc
import fenestre.protocols.state

# Server internal state
class state(ft.p.state.bypeer):
    def __init__(self, local):
        ft.p.state.bypeer.__init__(self, local)
        self.nicks = dict()
        self.rooms = dict()

    # Remove peer from nicks & room before cleaning it up.
    def remove(self, peer):
        for nick in list(self.nicks):
            if peer.handle is self.nicks[nick]:
                del self.nicks[nick]

        for room, peers in self.rooms.items():
            if peer.handle in peers:
                self.rooms[room] = [h for h in peers if h != peer.handle]

        ft.p.state.bypeer.remove(self, peer)

    # Send a message to either a room or a nick.
    def sendto(self, peer, room=None, nick=None, data=None):
        if data is None:
            data = b''
        elif data.startswith(b':'):
            data = data[1:]

        # (to a room)
        if room is not None:
            fdst = format_dst(peer)
            for other in self.rooms[room]:
                if other is peer.handle:
                    continue

                other.handler.send(
                    src=fdst, code='PRIVMSG', dst=room, data=data)

        # (to a nick)
        if nick is not None:
            fdst = format_dst(peer)
            self.nicks[nick].handler.send(
                src=fdst, code='PRIVMSG', dst=nick, data=data)

    # Add a peer to a given room.
    def join(self, peer, room):
        if room not in self.rooms:
            self.rooms[room] = []

        if peer.handle not in self.rooms[room]:
            self.rooms[room].append(peer.handle)
            return True
        else:
            return False

    # Set a peer's « realname » user
    def user(self, peer, user=None):
        if user is None:
            try:
                return self.get(peer, 'user')[0]
            except KeyError:
                self.set(peer, user=b'None')
                return b'None'

        self.set(peer, user=user)
        return user

    # Set a peer's hostname
    def host(self, peer, host=None):
        if host is None:
            try:
                return self.get(peer, 'host')[0]
            except KeyError:
                host = b'%f' % time.time()
                host = [b'ano', b'nymo', b'us', host[-10:-3]]
                host = b'.'.join(host)

                self.set(peer, host=host)
                return host

        self.get(peer, host=host)
        return host

    # Generate a « guest_nick » for a given peer
    def guest_nick(self, peer):
        for i in range(0, 10):
            nick = 'Guest{}'.format(random.randint(10000, 99999))
            nick = ft.etc.tobytes(nick)

            if not nick in self.nicks:
                self.nicks[nick] = peer.handle
                self.set(peer, nick=nick)
                return nick

        # Upon failure, disconnect the peer.
        peer.disconnect()
        return None

    # Set a peer's nickname
    def nick(self, peer, nick=None, guest=True):
        if nick is None:
            try:
                return self.get(peer, 'nick')[0]
            except KeyError:
                if guest:
                    return self.guest_nick(peer)
                return None

        if nick in self.nicks:
            return self.nicks[nick]

        peer_nick = self.nick(peer)
        del self.nicks[peer_nick]

        self.nicks[nick] = peer.handle
        self.set(peer, nick=nick)

        return peer.handle

# Format « nick!user@hostname »
def format_dst(request, nick=None):
    if nick is None:
        nick = request.state.nick(request)
    user = request.state.user(request)
    host = request.state.host(request)
    return b''.join([nick, b'!', user, b'@', host])

# Handle each client as a separate instance
class server:
    def event(self, request):

        # Handle CONNECT events
        if request.event is ft.net.events.connect:
            request.send(dst="Auth", data="Welcome to basic IRC server™ !")

        # Update the request's handler
        if request.event in ft.p.irc.events:
            request.handler = request
            args = request.data.split()

        # Handle QUIT events
        if request.event is ft.p.irc.events.quit:
            request.disconnect()

        # Handle PRIVMSG events
        if request.event is ft.p.irc.events.privmsg:
            nick = request.state.nick(request)
            target, data = args[:2]

            # Handle rooms
            if target.startswith(b'#') and target in request.state.rooms:
                request.state.sendto(request, room=target, data=data)

            # Handle private messages
            elif target in request.state.nicks:
                request.state.sendto(request, nick=target, data=data)

            # Handle « 401 not found » errors
            else:
                data = b' '.join([target, b':No such nick/channel'])
                request.send(code='401', dst=nick, data=data, raw=True)

        # Handle JOIN events
        if request.event is ft.p.irc.events.join:
            room = args[0]
            nick = request.state.nick(request)

            # Add a # to the room's name if nescessary
            if not room.startswith(b'#'):
                room = b''.join([b'#', room])

            newr = request.state.join(request, room)
            if newr:
                # Send a JOIN to each peer in the room
                fdst = format_dst(request)
                for peer in request.state.rooms[room]:
                    peer.handler.send(
                        src=fdst, code='JOIN', dst=b''.join([b':', room]))

                # Send a NAMES list to the newcomer
                names = []
                for peer in request.state.rooms[room]:
                    names.append(request.state.nick(peer))
                data = b''.join([b'= ', room, b' :', b' '.join(names)])
                request.send(code='353', dst=nick, data=data, raw=True)

                # Finish the list
                data = b' '.join([room, b':End of /NAMES list.'])
                request.send(code='366', dst=nick, data=data, raw=True)

        # Handle WHO events
        if request.event is ft.p.irc.events.who:
            room = args[0]
            nick = request.state.nick(request)

            # Enumerate each user in the room
            if room.startswith(b'#') or room in request.state.rooms:
                for peer in request.state.rooms[room]:
                    user = request.state.user(peer)
                    host = request.state.host(peer)
                    hname = request.protocol.hostname
                    onick = request.state.nick(peer)

                    data = b' '.join([room, user, host, hname, onick,
                        b'H :0 realname'])
                    request.send(code='352', dst=nick, data=data, raw=True)

            # Finish the list
            data = b' '.join([room, b':End of /WHO list.'])
            request.send(code='315', dst=nick, data=data, raw=True)

        # Handle USER events
        if request.event is ft.p.irc.events.user:
            request.state.user(request, args[0])

        # Handle NICK events
        if request.event is ft.p.irc.events.nick:
            reqn = args[0]

            # Tell the client few things if it is the first NICK command
            nick = request.state.nick(request, guest=False)
            if nick is None:
                data = 'Type « /join <chan> » to chat.'
                request.send(code="001", dst=reqn, data=data)

                # Hostname
                host = request.state.host(request)
                data = b' '.join([host, b':is now your displayed host'])
                request.send(code='386', dst=reqn, data=data, raw=True)

                # Terminate connection if nick already taken (simpler)
                if request.handle is not request.state.nick(request, reqn):
                    data = b' '.join([reqn, b':Nickname is already in use'])
                    request.send(code="433", dst=nick, data=data, raw=True)
                    request.later(0.2, request.disconnect)

            # Handle regular NICK commands
            else:
                fdst = format_dst(request)
                if request.handle is request.state.nick(request, reqn):
                    request.send(src=fdst, code='NICK', dst=reqn)
                else:
                    data = b' '.join([reqn, b':Nickname is already in use'])
                    request.send(code="433", dst=nick, data=data, raw=True)

# Bootstrap the server
if __name__ == '__main__':
    protocol = ft.p.sep(ft.p.irc.server(server))
    ft.net.tcp.bind(ft.net.server(protocol, state), '6667')
