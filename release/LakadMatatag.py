import sys
import wave

import numpy
import matplotlib.pyplot as plt




def main():
    #wav = wave.open(conf.file_path + 'test.wav', 'r')
    wav = wave.open(r'../File/signal.wav', 'r')
    (nchannels, sampwidth, framerate, nframes, comptype, compname) = wav.getparams()
    content = wav.readframes(nframes)
    wav.close()
    print('nchannels : ', nchannels,
          '\nsampwidth : ',sampwidth,
          '\nframerate : ', framerate,
          '\nnframes : ', nframes,
          '\ncomptype : ', comptype,
          '\ncompname : ', compname)

    print(content)
    samples = numpy.frombuffer(content, dtype=numpy.int16)
    print(samples)

    plt.plot(samples)
    plt.ylabel('some numbers')
    plt.show()

    txt = open(r'../File/spectrum.bin', 'r').read(100)
    print(txt)


if __name__ == "__main__":
    sys.exit(main())

