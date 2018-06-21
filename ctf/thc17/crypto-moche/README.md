# Moche

You sniffed some traffic and found out the implementation behind it, seems like
a secure shell over UDP... but is it secure enough ?

## Provided files
- `moche-server.py`: server code
- `moche.py`: client code
- `crypto.py`: crypto module
- `moche.pcap` : network capture

## Change flags

A file named `flag.gpg` exists in `/home/alice` – `alice` being the owner of
the remote shell we're trying to steal.

To only replace the flag, write something into a file named `flag`, then
encrypt it with `gpg -c flag` and `nsQDVdBklhAtASk274IdUrP` as passphrase.
Please don't forget to replace `flag.gpg` by the new one afterwards.

To also change the passphrase, you have to replay the network capture. In order
to do so, comments the `self.alice_was_here()` line into the server code, then
starts the server altogether with a network capture on ports `4848`, `48000`
and `48001`. Now, run a client with `python moche-client.py alice@<ip-server>`
and perform the following operations through the remote shell: first download a
file called `flag` from a remote HTTP server, encrypt it with GPG and type the
new passphrase, then do some complex things that includes `$()|<>&`.

## Authors

Thanks to [toffan](https://github.com/toffan) and GoFish – both tried amazing
things to get this UDP-based pseudo-secure shell up and running. Next time,
I promise I'll do everything over one single TCP connection. :p
