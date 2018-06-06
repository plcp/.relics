
# profile.sh

Bourne-again shell profiler.

# Quick setup

Here:
```
git clone https://github.com/plcp/profile.sh
cd profile.sh
./profile ./example test-string
# wait for it
```

# How does it works?

The profiler itself invokes the given script with `bash -x` after doing some
`PS4` hackery, then processes all the output with lots of `bc` and displays a
report with `tput` colors.
