import librosa as rosa
import numpy as np

def load(filename, sample_rate=22050):
    data, sr = rosa.load(filename, sr=None)
    return rosa.resample(data, sr, sample_rate)

def norm(data, start=2800, end=3200):
    data = data - np.mean(data)

    sigma = np.std(data)
    data[sigma * 3 < data] = sigma * 3
    data[sigma * 3 < -data] = -sigma * 3

    data = data / (3 * sigma)

    tdata = np.arange(len(data))
    tdata = tdata[np.abs(data) > sigma]
    tmean = int(np.mean(tdata))

    data = data[tmean - start:tmean + end]
    return rosa.util.pad_center(data, start + end)

class reference:
    sweight = 289000
    eweight = 690
    fweight = 706000
    mweight = 1870
    rweight = 289

    def __init__(self, filename, start=2800, end=3200, sample_rate=22050):
        self.sample_rate = sample_rate
        self.filename = filename
        self.start = start
        self.len = start + end
        self.end = end

        self.data = norm(load(filename, sample_rate), start, end)

        self.evalue = np.abs(self.data)
        self.svalue = np.sum(self.evalue)

        self.fvalue = rosa.magphase(rosa.stft(self.data))[0]
        self.mvalue = rosa.feature.melspectrogram(S=self.fvalue)
        self.rvalue = rosa.feature.rmse(S=self.fvalue)

        width = self.fvalue.shape[1]
        self.fvalue = self.fvalue * np.arange(0, width) / (width - 1)
        self.mvalue = self.mvalue * np.arange(0, width) / (width - 1)

    def score(self, candidate):
        evalue = np.abs(candidate)
        svalue = np.sum(evalue)

        fvalue = rosa.magphase(rosa.stft(candidate))[0]
        mvalue = rosa.feature.melspectrogram(S=fvalue)
        rvalue = rosa.feature.rmse(S=fvalue)

        width = fvalue.shape[1]
        fvalue = fvalue * np.arange(0, width) / (width - 1)
        mvalue = mvalue * np.arange(0, width) / (width - 1)

        sdist = np.sum((self.svalue - svalue)**2) / self.sweight
        edist = np.sum((self.evalue - evalue)**2) / self.eweight
        fdist = np.sum((self.fvalue - fvalue)**2) / self.fweight
        mdist = np.sum((self.mvalue - mvalue)**2) / self.mweight
        rdist = np.sum((self.rvalue - rvalue)**2) / self.rweight

        dist = (edist + fdist + rdist + sdist) / 2 + mdist * 4
        return 100 / np.exp(dist**2 / 25)





