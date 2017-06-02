# .firefox-nightly

A drop-in repository to setup clean firefox nightly builds in a package-manager-agnostic way for the local user.

## Quick setup

First clone the repository in your home:
```bash
cd
git clone https://github.com/plcp/.firefox-nightly.git
```

Then `make`:
```bash
cd ~/.firefox-nightly
make
```

## Pulling firefox nightly everyday

You can setup a `cron` job:
```bash
cd ~/.firefox-nightly
make crontab
```
This will try to retrieve then install a clean firefox nightly build everyday at 10am.

**Note:** Feel free to keep or remove older builds present in `~/.firefox-nightly/firefox/`.

## Misc: Why `gpg --verify` often fails to verify Mozilla's signature ?

This is somehow related to [this comment](https://bugzilla.mozilla.org/show_bug.cgi?id=1305139#c63) and [this issue](https://bugzilla.mozilla.org/show_bug.cgi?id=1336732). As it is annoying, we are obliged to trust certificate-signing authorities, `https` and mozilla's CDN to gives us the right file.
