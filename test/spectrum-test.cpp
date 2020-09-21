#include "test.h"
#include "../core/scale.h"
#include "../core/spectrum.h"
#include "../io/iowave.h"

NAMESPACE_TEST_BEGIN;

using namespace spl;

namespace {

    const freq_t sampling_freq_std = 12000;
    const double spectrum_ksi_std = spl_params_t::DEFAULT.spectrum.ksi;
    const char *signal_std = "E:/testdata/signal.bin";
    const char *signal_wav_std = "E:/testdata/signal.wav";
    const char *scale_std = "E:/testdata/scale-model-freq.bin";

    const char *spectrum_std = "E:/testdata/spectrum.bin";
    const char *spectrum_filters_std = "E:/testdata/spectrum-filters.bin";
    const char *spectrum_test = "E:/testdata/test-spectrum.bin";
    const char *spectrum_filters_test = "E:/test-spectrum-filters.bin";

}

class test_filter_binary_file_t : public test_error_t
{
    const char *name() { return "spectrum_binary"; }
    double max_error() { return 1E+0; }
    double error() {
        
        {
            freq_scale_t sc = freq_scale_t::load(scale_std);
            spectrum_calculator spec_calc(sc, sampling_freq_std, spectrum_ksi_std);
            io::ifstream<signal_t> signal(signal_std);
            io::ofstream<spectrum_t> spectrum(spectrum_test);

            tic();
            size_t written = spec_calc.execute(signal, spectrum);
            set_execution_time(toc());
        }

        return compare_streams<spectrum_t>(spectrum_std, spectrum_test);
    }
} test_filter_binary_file;


class test_filter_wav_file_t : public test_error_t
{
    const char *name() { return "spectrum_wav"; }
    double max_error() { return 1E+0; }
    double error() {

        {
            
            io::iwstream<signal_t> signal(signal_wav_std);
            io::ofstream<spectrum_t> spectrum(spectrum_test);
            freq_scale_t sc = freq_scale_t::load(scale_std);

            spectrum_calculator spec_calc(sc, signal.freq(), spectrum_ksi_std);

            tic();
            size_t written = spec_calc.execute(signal, spectrum);
            set_execution_time(toc());
        }

        return compare_streams<spectrum_t>(spectrum_std, spectrum_test);
    }
} test_filter_wav_file;

NAMESPACE_TEST_END;