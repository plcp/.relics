# Go MAC yourself !

Homebrewed crypto is homebrewed: go mac yourself.

## Provided files
- `server.py`: server code
- `crypto.py`: homebrewed crypto module
- `client.py`: client code

## Usage

```bash
# Prod
make export     # build exports
make clean      # cleanup exports & images

# Testing
make up         # boots the challenge on port 1058
make down       # shutdown the challenge (after being booted with `make up`)
make logs       # prints the logs
```

## Change flags

There are two flags for this challenge, a "easier" one and a regular one. Just
edit `flag-1.txt`, `flag-2.txt` than reboot the server to change the flags – no
extra export needed.

## Misc

This is a `crypto` challenge where a server use an unusual
[Message Authentication Code](https://en.wikipedia.org/wiki/Message_authentication_code).
We first need to break its
[Existential Forgery](https://en.wikipedia.org/wiki/Digital_signature_forgery#Existential_forgery)
property to get the "easier" flag, then break its
[Universal Forgery](https://en.wikipedia.org/wiki/Digital_signature_forgery#Universal_forgery) to get the real one.

## Hints

Some help if needed:
 - the first task only requires bits of trickery and diff-ing several tags for
   nearly-identical payloads.
 - the second task may requires understanding how this MAC works and its flaws.

## Authors

Thanks to [toffan](https://github.com/toffan) that got my challenge running the
day of the event. :)

