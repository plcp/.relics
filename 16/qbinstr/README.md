# qbinstr

Quick & Basic DynamoRIO instrumentation, clone with:
```sh
  git clone --recursive https://github.com/plcp/qbinstr.git
```

# Build

Build with:
```sh
  make
```

# Use

You may want to try:
```sh
  ./qbinstr --help
```

For example, you may trace `ls`, register's value and memory access:
```sh
  ./qbinstr -mg -- ls
```
