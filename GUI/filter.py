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


class Filter:
    def __init__(self):
        self.content = None

    def filtered(self, file_names):
        with open(file_names, "rb") as file:
            self.content = file.read()

        self.samples: numpy.nbarray = numpy.frombuffer(self.content, dtype=numpy.double)
        print(type(self.samples))

        print(len(self.samples))
        print(len(self.samples) // 256)
        size_of_arr = len(self.samples) // 256

        # self.samples.resize((size_of_arr, 256))
        self.samples = numpy.resize(self.samples, (size_of_arr, 256))
        samples = self.samples.transpose()

        # считаем длину записи в секундах
        self.durationSec = size_of_arr / 12000

        self.dat = samples

        self.fig, self.axes = plt.subplots(nrows=1, ncols=1)

        cmaplist = plt.cm.datad

        for ax in self.fig.axes:
            cf = ax.imshow(self.dat, cmap='plasma', interpolation='gaussian', origin="lower", extent=(0, 500, 255, 0))

            ax.xaxis.set_major_locator(ticker.MultipleLocator(50))
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

            ax.set_title('')
            ax.set_xlabel('время (с)')
            ax.set_ylabel('Номер канала')
            ax.grid()
            #  Устанавливаем интервал вспомогательных делений:
            ax.xaxis.set_minor_locator(ticker.MultipleLocator(10))
            ax.yaxis.set_minor_locator(ticker.MultipleLocator(10))
            #  Включаем видимость вспомогательных делений:
            ax.minorticks_on()
            #  Теперь можем отдельно задавать внешний вид
            #  вспомогательной сетки:
            ax.grid(which='minor',
                    color='gray',
                    linestyle=':')

            self.fig.colorbar(cf, ax=ax)

        plt.suptitle(u'Свртка с фильтром')  # единый заголовок рисунка

        # save('pic_4_3', fmt='png')
        # save('pic_4_3', fmt='pdf')

        plt.show()


def main():
    filter_ = Filter()
    filter_.filtered(file_names[0])


if __name__ == "__main__":
    sys.exit(main())
