import sys
import time
import wave
import pickle
import copy

import numpy
import matplotlib as mpl
import matplotlib.pyplot as plt
from matplotlib.ticker import NullFormatter, FixedLocator
from matplotlib.pyplot import xticks
import matplotlib.ticker as ticker


def toFixed(numObj, digits=0):
    return f"{numObj:.{digits}f}"


file_names = [r'../File/spectrum1.bin', r'../File/spectrum_solntse_selo.bin', r'../File/spectrum_tselebny_kolodets.bin',
              r'../File/spectrum_varilas_ukha.bin']


class Pitcher:
    def __init__(self):
        self.content = None

    def pitched(self, file_names):
        with open(file_names, "rb") as file:
            self.content = file.read()

        self.samples: numpy.nbarray = numpy.frombuffer(self.content, dtype=numpy.double)
        # #test
        # self.samples = list(i for i in range(12000))
        #

        print(type(self.samples))

        print(len(self.samples))
        print(len(self.samples) // 256)
        size_of_arr = len(self.samples)

        # считаем длину записи в секундах
        self.durationSec = size_of_arr / 12000

        self.dat = []
        for i in self.samples:
            if i != 0:
                self.dat.append(i)
            else:
                self.dat.append(numpy.nan)

        self.fig, self.axes = plt.subplots(nrows=1, ncols=1)

        cmaplist = plt.cm.datad

        for ax in self.fig.axes:
            cf = ax.plot(self.dat)

            ax.xaxis.set_major_locator(ticker.MultipleLocator(1200 * self.durationSec))
            ax.xaxis.set_major_formatter(mpl.ticker.FixedFormatter([0, self.durationSec * 0,
                                                                    toFixed(self.durationSec * 0.1, 3),
                                                                    toFixed(self.durationSec * 0.2, 3),
                                                                    toFixed(self.durationSec * 0.3, 3),
                                                                    toFixed(self.durationSec * 0.4, 3),
                                                                    toFixed(self.durationSec * 0.5, 3),
                                                                    toFixed(self.durationSec * 0.6, 3),
                                                                    toFixed(self.durationSec * 0.7, 3),
                                                                    toFixed(self.durationSec * 0.8, 3),
                                                                    toFixed(self.durationSec * 0.9, 3),
                                                                    toFixed(self.durationSec * 1, 3)]))

            ax.yaxis.set_major_locator(ticker.MultipleLocator(25))
            ax.yaxis.set_major_formatter(mpl.ticker.FixedFormatter([255, 230, 205, 180, 255, 130, 105, 80, 55, 30, 5]))

            ax.set_title('')
            ax.set_xlabel('время (с)')
            ax.set_ylabel('Номер канала')
            ax.grid()
            # #  Устанавливаем интервал вспомогательных делений:
            # ax.xaxis.set_minor_locator(ticker.MultipleLocator(10))
            # ax.yaxis.set_minor_locator(ticker.MultipleLocator(10))
            # #  Включаем видимость вспомогательных делений:
            # ax.minorticks_on()
            # #  Теперь можем отдельно задавать внешний вид
            # #  вспомогательной сетки:
            # ax.grid(which='minor',
            #         color='gray',
            #         linestyle=':')

        plt.suptitle(u'Вокализация')  # единый заголовок рисунка

        # save('pic_4_3', fmt='png')
        # save('pic_4_3', fmt='pdf')

        plt.show()


def main():
    filter_ = Pitcher()
    filter_.pitched(r'../File/vocal.bin')


if __name__ == "__main__":
    sys.exit(main())
