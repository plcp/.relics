# hn
HackerNews survey

# Quick Setup
Here:

```
git clone https://github.com/plcp/hn
cd hn
virtualenv venv
source venv/bin/activate
pip install -r requirements.txt
python poll.py
```

# TL;DR

The `poll.py` script will track HackerNews entries as they get posted:
 - downloading each entry once
 - tracking each entry evolution during its first two hours
 - tracking each entry with a score above 10 during four extra hours
 - store all the raw data for further use
 - precomputes differences between updates

Everything is stored within `entries.pkl` as a giant pickled dictionary:

```
$ python -i -c 'from poll import *'
>>> bot = crawler()
>>> bot.load('entries.pkl')
12798
>>> bot.entries[16984662].title
'Gaia’s Map of 1.3B Stars Makes for a Milky Way in a Bottle'
```

You can resume polling at any time by restarting `poll.py`.
