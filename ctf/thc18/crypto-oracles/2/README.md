# oracles 2

Someone is waiting for you here: `<host> <port>`

## Files provided to the challengers

- None

## Hints

These hints basically gives the answer of the challenge:

- aes-ctr
- one-byte nonce

## Deployment

Running `./server.py localhost <port> [<level>]` starts a fork server (ready to
accept a lot of connections) listening on port `<port>` with logging level of
`<level>` (in `WARNING`, `INFO` or `DEBUG`).

## Update the flag

Put the flag in a file named `flag` in the working directory of the server.
