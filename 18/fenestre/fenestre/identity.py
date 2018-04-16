# -*- coding: utf-8 -*-
import os

import OpenSSL
from twisted.internet import ssl
from twisted.python.filepath import FilePath

def is_identity(item):
    return issubclass(item.__class__, _base_identity)

class _base_identity:
    def __init__(self):
        raise NotImplementedError('(private API)')

    @property
    def id(self):
        return str(self.cert.digest(method='sha256'), 'utf8')

    @property
    def ca(self):
        return str(self.cert.getIssuer()['commonName'], 'utf8')

    @property
    def name(self):
        return str(self.cert.getSubject()['commonName'], 'utf8')

class remote(_base_identity):
    def __init__(self, filename):
        if isinstance(filename, hosted):
            filename = filename.filename

        self.filename = filename
        with open(filename, 'r') as fp:
            self.cert = ssl.Certificate.loadPEM(fp.read())

class hosted(_base_identity):
    def __init__(self, filename):
        if isinstance(filename, remote):
            filename = filename.filename

        self.filename = filename
        with open(filename, 'r') as fp:
            self.cert = ssl.PrivateCertificate.loadPEM(fp.read())

class store:
    def __init__(self, authority):
        assert isinstance(authority, remote)
        self.store = OpenSSL.crypto.X509Store()
        self.store.add_cert(authority.cert.original)

        self.authority = authority
        self.remote = {}
        self.hosted = {}
        self.diffie = []

    def add_identity(self, identity, force='auto'):
        assert force in ['remote', 'hosted', 'auto']

        # Handle « force == 'auto' » case
        assert is_identity(identity)
        if force == 'auto':
            if isinstance(identity, remote):
                force = 'remote'
            elif isinstance(identity, hosted):
                force = 'hosted'
            else:
                raise RuntimeError('Unknown: {}'.format(repr(identity)))

        # Verify certificate signature against stored authority
        try:
            ctx = OpenSSL.crypto.X509StoreContext(
                self.store,
                identity.cert.original)
            ctx.verify_certificate()
        except BaseException as e:
            raise RuntimeError((identity.filename, e))

        # Pick the right location to store the identity (remote or hosted)
        if force == 'remote':
            assert isinstance(identity, remote)
            location = self.remote
        else:
            assert isinstance(identity, hosted)
            location = self.hosted

        # Check for duplicates
        if identity.name in location:
            raise RuntimeError('Duplicate: {}'.format(identity.name))

        # Then add for by-name lookups
        location[identity.name] = identity

        return self

    def add_dhparameters(self, filename):
        assert filename.endswith('.dh.pem')
        filepath = FilePath(filename)
        self.diffie.append(ssl.DiffieHellmanParameters.fromFile(filepath))
        return self

    def add_file(self, filename, force='auto'):
        assert force in ['remote', 'hosted', 'diffie', 'auto']

        # Handle « force == 'auto' » case
        raised = []
        identity = None
        if force == 'auto':

            try:
                return self.add_dhparameters(filename)
            except BaseException as e:
                raised.append(e)
                try:
                    identity = hosted(filename)
                except BaseException as e:
                    raised.append(e)
                    try:
                        identity = remote(filename)
                    except BaseException as f:
                        raised.append(f)
                        raise RuntimeError(raised)

        # Handle other cases
        elif force == 'diffie':
            return self.add_dhparameters(filename)
        elif force == 'hosted':
            identity = hosted(filename)
        elif force == 'remote':
            identity = remote(filename)

        return self.add(identity, force=force)

    def add_directory(self, directory, force='auto'):
        assert force in ['remote', 'hosted', 'diffie', 'auto']

        # For each .pem file found
        for suffix in os.listdir(directory):
            if not suffix.endswith('.pem'):
                continue

            filename = os.path.join(directory, suffix)
            if not os.path.isfile(filename):
                continue

            # add to store
            self.add_file(filename)
        return self

    def add(self, various, force='auto'):
        assert force in ['remote', 'hosted', 'auto']

        # Handle raw identities
        if is_identity(various):
            return self.add_identity(various, force=force)

        # Handle unknown types
        if not isinstance(various, str):
            raise RuntimeError('Expected types: identity, str (got {})'.format(
                repr(various)))

        # Handle files
        if os.path.isfile(various) and various.endswith('.pem'):
            return self.add_file(various, force=force)

        # Handle directories
        if os.path.isdir(various):
            return self.add_directory(various, force=force)

        RuntimeError('Unable to add {} to store.'.format(repr(various)))

def load(*sources, authority='./certs/ca.d/ca_public.pem'):
    _store = store(remote(authority))
    for source in sources:
        _store.add(source)
    return _store
