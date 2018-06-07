# tim

Randomly search for the slowest permutation of `list(range(1024))` when sorted
with python's sort (also known
as [Timsort](https://en.wikipedia.org/wiki/Timsort)).

# Quick setup

Here:
```
git clone https://github.com/plcp/tim
cd tim
python tim.py # wait for few days during random search
python select.py # wait for few hours during sorting candidates
python evaluation.py # wait for few minutes during evaluation
```

You should get a permutation that takes a little longer time to be sorted:
```
$ python evaluation.py
Base: 274.25ms
Best: 291.92ms
Gain: 1.06
```

Some results are already included within the repository (see `best.py` and
`candidates.py`) and you should be able to beat them easily.

# How does it work?

We model the event `list A sorts slower than list B` as biased
[coin flip](https://en.wikipedia.org/wiki/Checking_whether_a_coin_is_fair),
enabling us to decide between two very similar candidates – making measured
approximations through
[confidence intervals](https://en.wikipedia.org/wiki/Confidence_interval) and
sparing us a lot of calculus.

Being able to arrange "duels" between our candidates, we run an
[elo rating system](https://en.wikipedia.org/wiki/Elo_rating_system) that gives
us bad candidates and good ones, thus enabling us to greedily converge slowly
to slower and slower lists.
