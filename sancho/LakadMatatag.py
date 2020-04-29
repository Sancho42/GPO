import sys
import wave

import GPO.release.programConfig as conf # Здесь лежит путь до файла



def main():
    wav = wave.open(conf.file_path + 'test.wav', 'r')
    (nchannels, sampwidth, framerate, nframes, comptype, compname) = wav.getparams()
    #content = wav.readframes(nframes)
    wav.close()
    print(nchannels, sampwidth, framerate, nframes, comptype, compname)


if __name__ == "__main__":
    sys.exit(main())
