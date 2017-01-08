import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
import numpy as np
import struct
import wave

class RC(object):
    def __init__(self, tau, fs=44.1E3):
        self.tau = tau
        self.fs = fs
        self.rc_sig = 0

    def work(self, sig):
        self.rc_sig = (sig + self.tau*self.fs*self.rc_sig)/(1+self.tau*self.fs)
        return self.rc_sig

    def reset(self):
        self.rc_sig = 0

class Autogain(object):
    def __init__(self, gain, thresh, fs):
        self.peak_rc = RC(0.005, fs)
        self.cond_rc = RC(0.0075, fs)
        self.comp_rc = RC(0.1, fs)
        self.max_gain = gain
        self.gain = 1.0
        self.thresh = thresh

    def work(self, signal):
        output = np.zeros(len(signal))
        for k in xrange(0, len(signal)):
            outp = self.gain * signal[k]
            rect = abs(outp)
            peak_out = 0
            comp_out = 0
            cond_out = 0
            if rect > self.peak_rc.rc_sig:
                self.peak_rc.rc_sig = rect
                peak_out = rect
            else:
                peak_out = self.peak_rc.work(0.0)

            cond_out = self.cond_rc.work(peak_out)

            output[k] = outp

            if peak_out < self.thresh:
                comp_out = self.max_gain
            else:
                comp_out = 0.0

            self.gain = 1.0 + self.comp_rc.work(comp_out)
#output[k] = self.gain
        return output

scale = 0.0075
wav = wave.open('output.wav', 'r')

print 'channels %d'%wav.getnchannels()
print 'smpl width %d'%wav.getsampwidth()
print 'frame rate %f'%wav.getframerate()
nframes = wav.getnframes()
print 'frames %d'%nframes

data = wav.readframes(nframes)

data = scale * np.array(struct.unpack('<%dh'%nframes, data)) / float((1 << 14))

autoC = Autogain(10.0, 0.01, 44.1E3)
autoB = Autogain(10.0, 0.10, 44.1E3)
autoA = Autogain(10.0, 1.00, 44.1E3)

data = autoA.work(autoB.work(autoC.work(data)))

plt.plot(data)
plt.show()
