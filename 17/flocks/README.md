# flocks
Millions of boids flocks around and toward the mouse.

# Quick setup
Use `make` to build:
```sh
git clone --recursive https://github.com/plcp/flocks.git
cd flocks
make
```

Depending on your CPU, you may have between 2¹⁶ and 2²⁰ boids floating around.

# Requirements

Building [clenche](https://github.com/plcp/clenche) and the `SFML`.

Tested with `g++ (GCC) 7.1.1 20170630` and `SFML 2.4.2`.

# Troubleshooting
Try with a lower target framerate:
```sh
cd flocks
sed -i "s, target_framerate =,& 30; //," flocks.cpp
make
```
