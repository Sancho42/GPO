import sys
import wave

#import programConfig as conf # Здесь лежит путь до файла



def main():
    wav = wave.open('testdata/signal.wav', 'r')
    (nchannels, sampwidth, framerate, nframes, comptype, compname) = wav.getparams()
    content = wav.readframes(nframes)
    wav.close()
    print(nchannels, sampwidth, framerate, nframes, comptype, compname)


if __name__ == "__main__":
    try:
        sys.exit(main())
    except:
        pass

