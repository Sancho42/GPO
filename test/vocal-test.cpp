#include "test.h"
#include "../core/scale.h"
#include "../core/vocal.h"
#include "../io/iofile.h"

NAMESPACE_TEST_BEGIN;

using namespace spl;
using namespace io;

namespace {

    const char *scale_std = "E:/testdata/scale-model-freq.bin";
    const char *spectrum_std = "E:/testdata/spectrum.bin";
    const char *mask_byte_std = "E:/testdata/sync-mask.bin";
    const char *mask_bit_std = "E:/testdata/sync-mask.bit";

    const char *pitch_chan_std = "E:/testdata/pitch-chan.bin";
    const char *pitch_freq_std = "E:/testdata/pitch-freq.bin";
    const char *vocal_chan_std = "E:/testdata/vocal-chan.bin";

    const char *pitch_chan_test = "E:/testdata/test-pitch-chan.bin";
    const char *pitch_freq_test = "E:/testdata/test-pitch-freq.bin";
    const char *vocal_chan_test = "E:/testdata/test-vocal-chan.bin";

    struct original_t {
        size_t K;
        freq_t Fn, Fv, F;
        double ksi, rho, delta;
        int border_effect;

        int Ng;
        freq_t Fon, Fov;
        double minV, minNV;

        original_t() {
            K = 256;
            Fn = 50;
            Fv = 4000;
            ksi = 0.001;
            F = 12000;
            border_effect = 1;
            rho = 1.0;
            delta = 1.0;
            Ng = 2;
            Fon = 70;
            Fov = 400;
            minV = 0.030;
            minNV = 0.030;
        }

    } original;

}

class test_pitch_track_t : public test_error_t
{
    const char *name() { return "pitch_track"; }
    double max_error() { return 0.0; }
    double error() {

        {
            // открываем шкалу частот
            freq_scale_t sc = freq_scale_t::load(scale_std);

            // открываем маску
            ifstream<mask_t> input(mask_byte_std);

            // выходной файл
            ofstream<short> output(pitch_chan_test);

            // параметры генерации маски
            mask_params_t pm;
            pm.border_effect = original.border_effect != 0.0;
            pm.ksi = original.ksi;
            pm.rho = original.rho;
            pm.delta = original.delta;

            // параметры генерации шаблонов
            pitch_params_t pp;
            pp.Nh = original.Ng;
            pp.F1 = original.Fon;
            pp.F2 = original.Fov;

            pitch_calculator calc(sc, pm, pp);

            // собственно выделение
            tic();
            size_t written = calc.execute(input, output);
            set_execution_time(toc());
        }

        return compare_streams<short>(pitch_chan_std, pitch_chan_test);
    }
} test_pitch_track;

class test_vocal_segment_t : public test_error_t
{
    const char *name() { return "vocal_segment"; }
    double max_error() { return 0.0; }
    double error() {

        {
            // открываем шкалу частот
            freq_scale_t sc = freq_scale_t::load(scale_std);

            // открываем каналы
            ifstream<short> input(pitch_chan_test);

            // выходной файл
            ofstream<short> output(vocal_chan_test);

            // параметры сегментации
            vocal_params_t p;
            p.minV = original.minV;
            p.minNV = original.minNV;

            // частота дискретизации
            freq_t F = original.F;

            // собственно сегментация
            size_t written = vocal_segment(input, output, F, p);
        }

        return compare_streams<short>(vocal_chan_std, vocal_chan_test);
    }
} test_vocal_segment;

NAMESPACE_TEST_END;