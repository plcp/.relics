# hayfield

Will you manage to decrypt the flag?

## Files provided to the challengers

- `./src/hayfield.c`
- `./src/ciphertext`

## Hints

- `./src/hint`

## Regenerate the files

`make`

## Update the flag

Update `flag` constant in `./generate.c` before running `make`. If someone
leaks the encryption key used to encrypt the flag, you want to change `key`
and the other plaintexts used.
