import librosa as rosa
import numpy as np
import os

from reference import load, norm, reference

sources = {}
for source in os.listdir():
    if not os.path.isfile(source):
        continue
    if not source.startswith('source') or not source.endswith('.wav'):
        continue

    sources[source] = reference(source)

samples = []
for sample in os.listdir():
    if not os.path.isfile(sample):
        continue
    if not sample.startswith('sample') or not sample.endswith('.wav'):
        continue

    data = norm(load(sample))

    scores = []
    for source in sources.values():
        scores.append(source.score(data))
    samples.append((sample, np.max(scores), data))
samples.sort(key=lambda x: x[1])

pause = 1000
track = np.zeros(pause)
for sample, score, cdata in samples:
    track = np.concatenate([track, cdata, np.zeros(pause)])
    print('{}: {}'.format(sample, score))

rosa.output.write_wav('track.wav', track, 22050)
