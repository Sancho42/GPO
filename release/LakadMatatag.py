import sys
import wave

import GPO.release.programConfig as conf # Здесь лежит путь до файла



def main():
    #wav = wave.open(conf.file_path + 'test.wav', 'r')
    wav = wave.open(r'../File/test.wav', 'r')
    (nchannels, sampwidth, framerate, nframes, comptype, compname) = wav.getparams()
    #content = wav.readframes(nframes)
    wav.close()
    print('nchannels : ', nchannels,
          '\nsampwidth : ',sampwidth,
          '\nframerate : ', framerate,
          '\nnframes : ', nframes,
          '\ncomptype : ', comptype,
          '\ncompname : ', compname)

    #txt = open('text.txt', 'w')
    #txt.write("lol")
    #txt.close()


if __name__ == "__main__":
    sys.exit(main())

