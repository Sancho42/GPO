import sys
import time
import wave
import pickle

import numpy
import matplotlib as mpl
import matplotlib.pyplot as plt
from matplotlib.ticker import NullFormatter, FixedLocator
from matplotlib.pyplot import xticks
import matplotlib.ticker as ticker


def toFixed(numObj, digits=0):
    return f"{numObj:.{digits}f}"


file_names = [r'../File/mask.bin', r'../File/mask_solntse_selo.bin', r'../File/mask_tselebny_kolodets.bin',
              r'../File/mask_varilas_ukha.bin']


class Mask:
    def __init__(self):
        self.content = None

    def masked(self, file_names):
        with open(file_names, "rb") as file:
            self.content = file.read()

        self.samples = numpy.frombuffer(self.content, dtype=numpy.bool)
        print(type(self.samples))

        print(len(self.samples))
        print(len(self.samples) // 256)
        self.size_of_arr = len(self.samples) // 256

        self.samples.resize((self.size_of_arr, 256))
        samples = self.samples.transpose()

        # считаем длину записи в секундах
        self.durationSec = self.size_of_arr / 12000

        self.dat = samples

        self.fig, self.axes = plt.subplots(nrows=1, ncols=1)

        for ax in self.fig.axes:

            cf = ax.imshow(self.dat, cmap='copper', interpolation='gaussian', origin="lower", extent=(0, 500, 255, 0))
            ax.xaxis.set_major_locator(ticker.MultipleLocator(50))
            ax.xaxis.set_major_formatter(mpl.ticker.FixedFormatter([0, self.durationSec * 0, toFixed(self.durationSec * 0.1, 3),
                                                                    toFixed(self.durationSec * 0.2, 3),
                                                                    toFixed(self.durationSec * 0.3, 3),
                                                                    toFixed(self.durationSec * 0.4, 3),
                                                                    toFixed(self.durationSec * 0.5, 3),
                                                                    toFixed(self.durationSec * 0.6, 3),
                                                                    toFixed(self.durationSec * 0.7, 3),
                                                                    toFixed(self.durationSec * 0.8, 3),
                                                                    toFixed(self.durationSec * 0.9, 3),
                                                                    toFixed(self.durationSec * 1, 3)]))
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

            # ax.yaxis.set_major_locator(FixedLocator(numpy.arange(0, 255, 10)))

        plt.suptitle(u'Свёртка с маской')  # единый заголовок рисунка

        # save('pic_4_3', fmt='png')
        # save('pic_4_3', fmt='pdf')

        plt.show()

def main():
    mask_ = Mask()
    mask_.masked(file_names[0])

if __name__ == "__main__":
    sys.exit(main())
