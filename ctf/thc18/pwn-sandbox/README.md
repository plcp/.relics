# sandbox

Will you manage to escape the sandbox?

http://localhost:8080

Note: the content of the sandboxing "library" used is mirrored on my github
account [here](https://github.com/plcp/sandkox).

## Files provided to the challengers

- `./sandbox.c`

## Hints

Several hints:

1. Hosted on github
2. Environment variables
3. `gcc -nostartfiles -shared -fPIC -lcap -ldl sandbox.c -o sandbox.so`
4. `man ld.so|grep LD_P`

## Deployment

Running `./server.py` starts a HTTP server on port 8080 which executes any
arbitrary binary submitted by the participant as root: the twist here is that
a shared library (`./sandkox/sandkox-preload.so`) is loaded via `LD_PRELOAD`
and drops all the privileges by overriding the `_init` symbol.

The shared library `./sandkox/sandkox-preload.so` **must** be accessible from
the working directory of the server – if not, the challenge will be far easier
as it will not try to sandbox the binary and will only execute it as root.

As the participant can manage to escalate privileges and to become root, the
challenge shall be properly packaged – for example, in a read-only virtualised
environment.

Use `make -C ./sandkox` to build `./sandkox/sandkox-preload.so` and don't
forget to run the server as `root`.

## Local setup

Execute `sudo python server.py` and then connect to `localhost:8080`.

Now, try to get root on your own computer from your favorite browser.

## Deployment with docker

Just use `make build` and `make up` as usual.

Warning: The challenge requires a privileged container (also called "yolo-mode").

## Update the flag

Update `flag` and rebuild the docker image.
