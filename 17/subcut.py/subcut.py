from base64 import b64encode

import librosa as rosa
import numpy as np
import subprocess
import json
import os

def load(filename, sample_rate=22050, **kwargs):
    data, sr = rosa.load(filename, sr=None, **kwargs)
    return rosa.resample(data, sr, sample_rate)

def running_mean(x, N):
    cumsum = np.cumsum(np.insert(x, 0, 0))
    return (cumsum[N:] - cumsum[:-N]) / float(N)

def pick_file(directory='.'):
    files = [f for f in os.listdir(directory) if os.path.isfile(f)]

    print('Pick a file to process:')
    for idx, name in enumerate(files):
        print('{}) {}'.format(idx + 1, name))

    choice = None
    while choice is None:
        print('Your choice? ', end='')
        try:
            n = int(input()) - 1
        except ValueError:
            continue

        if not len(files) > n >= 0:
            continue

        print("Confirm '{}'? [y/N] ".format(files[n]), end='')
        try:
            c = input()[0]
            if c in ['y', 'Y']:
                choice = files[n]
            break
        except IndexError:
            continue

    return choice

def pick_level(start, end, sigma, stress_size):
    size = end - start
    if size > 2 * stress_size:
        return sigma * 3
    if size > stress_size:
        return sigma * 2
    return sigma

def handle_sample(seconds, data, quiet_size=0.2, stress_size=1.5):
    stress_size = rosa.time_to_samples(stress_size)[0]
    quiet_size = rosa.time_to_samples(quiet_size)[0]

    mean = running_mean(data, quiet_size)
    mean = np.abs(mean)

    dmin, dmax = np.min(mean), np.max(mean)
    mean = (mean - dmin) / (dmax - dmin)
    sigma = np.std(mean)

    sound = -1
    bound = []
    for idx, value in enumerate(mean):

        if sound < 0 and value > sigma * 2:
            rid = max([idx - quiet_size, 0])
            sound = quiet_size
            bound.append(rid)
            continue

        if sound > 0:
            try:
                level = pick_level(bound[-1], idx, sigma, stress_size)
            except IndexError:
                level = sigma

            if value < level:
                sound -= 1
            else:
                sound = quiet_size
            continue

        if sound == 0:
            rid = min([len(mean), idx + quiet_size])
            sound = -1
            bound.append(rid)
            continue

    for start, end in zip(bound[:-1], bound[1:]):
        if end - start < quiet_size:
            continue

        rosa.output.write_wav('out.wav', data[start:end], 22050)
        subprocess.call(['mpv', 'out.wav'])
        input()

if __name__ == '__main__':
    target = pick_file()
    seconds = 0
    finished = False
    while not finished:
        data = load(target, offset=seconds, duration=30)
        time = rosa.get_duration(data)
        handle_sample(seconds, data)

        seconds += (time - 10)
        input('Continue? ({}s)'.format(seconds))
