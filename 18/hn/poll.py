from hackernews import HackerNews
import pickle
import time
import json

class entry:
    def __init__(self, item, author):
        self.created = time.time()
        self.updated = time.time()
        self.updates = [(self.created, json.loads(item.raw))]
        self.author = json.loads(author.raw)
        self.deltas = []

    def update(self, item):
        self.updates.append((time.time(), json.loads(item.raw)))
        self.deltas.append((time.time(), self.last_delta()))
        self.updated = time.time()

    def last_delta(self):
        if len(self.updates) < 2:
            return dict()

        result = dict()
        last_update = self.updates[-1][1]
        for key, value in self.updates[-2][1].items():
            if key not in last_update:
                continue
            elif type(last_update[key]) is not type(value):
                continue
            elif last_update[key] == value:
                continue
            elif isinstance(value, int):
                result[key] = last_update[key] - value
            elif isinstance(value, str):
                result[key] = last_update[key]
            elif isinstance(value, list):
                result[key] = [v for v in last_update[key] if v not in value]
        return result

    def __getattr__(self, name):
        updates = object.__getattribute__(self, 'updates')
        if name not in updates[-1][1]:
            raise AttributeError
        return updates[-1][1][name]

class crawler:
    def __init__(self,
            poll_freq=10,
            poll_limit=20,
            update_freq=1800,
            update_count=4,
            late_freq=3600,
            late_count=4,
            late_score=10):
        self.poll_freq = poll_freq
        self.poll_limit = poll_limit
        self.update_max = update_freq * update_count + poll_freq + 1
        self.update_freq = update_freq
        self.update_count = update_count
        self.late_max = update_freq * update_count + poll_freq + 1
        self.late_freq = 3600
        self.late_count = 4
        self.late_score = 10

        self.recent = []
        self.entries = {}
        self.last_update = 0

    @property
    def next_poll_in(self):
        delta = (self.last_update + self.poll_freq) - time.time()
        if delta < 0:
            return 0.0
        return delta

    def poll(self):
        api = HackerNews()
        start = time.time()
        self.last_update = time.time()

        try:
            get_stories = api.new_stories()
        except BaseException as e:
            print('Polling failed: no internet?')
            return (time.time() - start, 0, 0, 0, 0, 0, 0, 0, len(self.recent))

        get_stories.sort(reverse=True)
        new_stories = [s for s in get_stories if s not in self.entries]

        necount = 0
        for uid in new_stories[:self.poll_limit]:
            print('\r' + ' ' * 40 + '\r', end='')
            print('\r...polling item {}...'.format(uid), end='')

            try:
                necount += 1
                story = api.get_item(uid)
                necount += 1
                author = api.get_user(story.by)
            except BaseException as e:
                print(' {}'.format(str(e)))
                continue

            self.recent.append(uid)
            self.entries[uid] = entry(story, author)

        old_stories = []
        ext_stories = []
        for uid in self.recent:
            print('\r' + ' ' * 40 + '\r', end='')
            print('\r...polling item {}...'.format(uid), end='')

            story = self.entries[uid]
            if story.created + self.update_max < time.time():
                if story.created + self.late_max < time.time():
                    old_stories.append(uid)
                    continue

                try:
                    if story.score < self.late_score:
                        old_stories.append(uid)
                    else:
                        ext_stories.append(uid)
                except AttributeError as e:
                    print(' {}'.format(str(e)))

        upcount = 0
        for uid in self.recent:
            print('\r' + ' ' * 40 + '\r', end='')
            print('\r...polling item {}...'.format(uid), end='')

            if uid in old_stories or uid in ext_stories:
                continue

            story = self.entries[uid]
            if story.updated + self.update_freq < time.time():
                try:
                    upcount += 1
                    story.update(api.get_item(uid))
                except BaseException as e:
                    print(' {}'.format(str(e)))
                    continue

        excount = 0
        for uid in ext_stories:
            print('\r' + ' ' * 40 + '\r', end='')
            print('\r...polling item {}...'.format(uid), end='')

            if uid in old_stories:
                continue

            story = self.entries[uid]
            if story.updated + self.late_freq < time.time():
                try:
                    excount += 1
                    story.update(api.get_item(uid))
                except BaseException as e:
                    print(' {}'.format(str(e)))
                    continue

        self.recent = [s for s in self.recent if s not in old_stories]
        print('\r' + ' ' * 40 + '\r', end='')

        return (time.time() - start, necount, upcount, excount,
                    len(new_stories), len(get_stories),
                    len(ext_stories), len(old_stories), len(self.recent))

    def dump(self, fname):
        with open(fname, 'wb') as f:
            pickle.dump(self.entries, f)

    def load(self, fname, populate=500):
        try:
            with open(fname, 'rb') as f:
                self.entries = pickle.load(f)
        except FileNotFoundError:
            return 0

        stamps = [v.updates[-1][0] for _, v in self.entries.items()]
        stamps.sort(reverse=True)

        recent = []
        for key, value in self.entries.items():
            if value.updates[-1][0] in stamps[:populate]:
                recent.append(key)

        while len(self.recent) < populate:
            self.recent.append(recent.pop(0))

        return len(self.entries)

if __name__ == '__main__':
    bot = crawler()
    pkl_fname = 'entries.pkl'

    n = bot.load(pkl_fname)
    if n > 0:
        print('[{} entries loaded]'.format(n))

    while True:
        seconds = bot.next_poll_in
        if seconds > 0:
            now = time.localtime()
            print('\r[{:02}:{:02}:{:02}] '.format(
                now.tm_hour, now.tm_min, now.tm_sec), end='')
            print('Next poll in {:5.4} seconds... '.format(seconds),
                end='', flush=True)
            time.sleep(seconds)

        try:
            poll_time, ne, re, ex, new, get, ext, old, total = bot.poll()
            if ne + re + ex == 0:
                continue
            else:
                bot.dump(pkl_fname)
        except KeyboardInterrupt:
            print('Pickling entries...')
            while True:
                try:
                    bot.dump(pkl_fname)
                    break
                except KeyboardInterrupt:
                    continue

        print(' ' * 50 + '\r', end='')
        now = time.localtime()
        print('\r[{:02}:{:02}:{:02}] '.format(
            now.tm_hour, now.tm_min, now.tm_sec), end='')
        print('Polling {} entries (out of {})...'.format(total,
            len(bot.entries)))

        if ne + re + ex > 2:
            print(' - {:.4}s to perform {} requests'.format(
                poll_time, 1 + ne + re + ex))
        if new > 0:
            print(' - {} new (out of {})'.format(new, get))
        if ne > 0:
            print(' - {} retrieved (out of {})'.format(ne // 2, new))
        if re > 0:
            print(' - {} updated (out of {})'.format(re, total + old))
        if ex > 0:
            print(' - {} late updates (out of {})'.format(ex, ext))
        if old > 0:
            print(' - {} removed from the pool (out of {})'.format(old,
                total + old))
