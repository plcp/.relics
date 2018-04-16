
A small game for Christmas written in few days.

# sown

Someone invited me to make a game for Christmas and here's the result:

 - You're alone in the forest.
 - You have a fire and some food.
 - You have to go outside to find more food.
 - You wait for help as the snowstorm intensifies.

**Spoilers:** Help never come, but consider yourself successful if you survive
the given number of days — the higher, the better.

# Quick setup

You'll need `gcc 7.2.0` (or any compiler with a proper support of `--std=c++17`
and `std::variant`) and `sfml 2.4.2` installed on your computer.

Then, use the `Makefile` to build the game and play:
```
git clone --recursive https://github.com/plcp/sown
cd sown && make
./build/sown
```

If you get the following error message:
```
Failed to open sound file "./resources/sounds/rocks.wav" (couldn't open stream)
...
Failed to load font "./resources/fonts/Inconsolata.otf" (failed to create the font face)
terminate called after throwing an instance of 'std::runtime_error'
  what():  Default font not found.
```

Verify that the resource folder is accessible here:

 - `./resources/` (relative path)

(Just check that you're in the game's root
directory: `cd sown && ./build/sown`)

# Keyboard Controls

You don't have to read this section to play, please discover the game by
playing it. Most of the gameplay is about figuring out how the world works and
how to survive inside it, **don't spoil yourself**.

If you're still wondering the keyboard controls of the game after playing it
for a while, here are a quick summary:

 - `left` and `right` to walk.
 - `up` to jump or climb trees.
 - `down` to pick an item.
 - `space` to put an item.
 - `tab` to cycle through your inventory.
 - `R` to restart with a fresh game.
 - `V` to view the whole map.

Here are also an exhaustive list of craft recipes available to you:

 - `leaf + leaf` gives you `food`
 - `wood + wood` gives you `plank`

Note that you can't jump while climbing trees or structures and that you may
freeze to death if you play a bit too much with the snow.

# Abstract

The main motive for making this game was to put
the [clenche](https://github.com/plcp/clenche) library to the test in a
practical and complete application.

I didn't learn much, but the experience was interesting as a whole.

Most of the mechanics were put together in two days, several design choices are
debatable but the overall system works well. From this baseline `~2702 loc`,
many adjustments were made to address gameplay and aesthetic
concerns `+888 loc` over the next few days.

# License

The whole game is a *libre* software (a `F/LOSS` with `GNU GPL v3` attached),
except some sounds effects (mostly `snowstorm.ogg` and `fireplace.ogg`). If you
are one of the authors and you find this abuse to be unacceptable, please
contact me and I will remove these sound effects from the repository.

# Contribute

Feel free to contribute, to open issues or to ask me for help building or
playing this little piece of code. I'll be happy to help. : )
