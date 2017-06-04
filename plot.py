import matplotlib
matplotlib.use('TkAgg')
import matplotlib.pyplot as plt
import numpy as np
import struct
import wave

scale = 0.01
wav = wave.open('output.wav', 'r')

print 'channels %d'%wav.getnchannels()
print 'smpl width %d'%wav.getsampwidth()
print 'frame rate %f'%wav.getframerate()
nframes = wav.getnframes()
print 'frames %d'%nframes

data = wav.readframes(nframes)

data = scale * np.array(struct.unpack('<%dh'%nframes, data)) / float((1 << 14))

plt.plot(data)
plt.show()
