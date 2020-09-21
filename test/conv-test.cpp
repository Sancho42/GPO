#include "test.h"

#include <fftw3.h>
#include <algorithm>
#include <cmath>
#include <memory>
#include "../core/spl_types.h"
#include "../core/conv.h"
#include "../core/model.h"
#include "conv.h"

NAMESPACE_TEST_BEGIN;

using namespace spl;
using namespace std;

void fill_test_array(fftw_complex *in, double *ri, double *ii) {
    for (int i = 0; i < CONV_WIN_SIZ; i++) {
        double I = double(i);
        in[i][0] = ri[i] = sin(I) * sin(2 * I);
        in[i][1] = ii[i] = sin(I) * sin(2 * I) + 5;
    }
}

double compare_fftw_arrays(int N, const fftw_complex *out, const double *ro, const double *io) {
    double e = 0.0;
    for (int i = 0; i < N; i++) {
        e += fabs((out[i][0] - ro[i]));
        e += fabs((out[i][1] - io[i]));
    }
    return e;
}

class test_fftw_split_t : public test_t
{
    const char *name() { return "fftw_split"; }
    double max_error() { return 5E-10; }
    void test() {
        double e;

        int K = 1;
        double *array = conv_alloc<double>(CONV_WIN_SIZ * 5 * 8 + 2);
        double *ri = SPL_MEMORY_ALIGN(array);
        double *ii = ri + CONV_WIN_SIZ * K;
        double *ro = ii + CONV_WIN_SIZ * K;
        double *io = ro + CONV_WIN_SIZ * K;
        fftw_complex *in = (fftw_complex *)(ro + CONV_WIN_SIZ * K);
        fftw_complex *out = in + CONV_WIN_SIZ * K;



        fftw_iodim dim, howmany;
        dim.n = CONV_WIN_SIZ;
        dim.is = dim.os = 1;
        fftw_plan plan1, plan2;
        howmany.n = K;
        howmany.is = howmany.os = CONV_WIN_SIZ;

        plan1 = fftw_plan_dft_1d(CONV_WIN_SIZ, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
        plan2 = fftw_plan_guru_split_dft(1, &dim, 0, 0, ri, ii, ro, io, FFTW_ESTIMATE);

        fill_test_array(in, ri, ii);
        for (int k = 0; k < K; k++) fftw_execute_dft(plan1, in + k * howmany.is, out + k * howmany.os);
        fftw_execute(plan2);
        fftw_destroy_plan(plan1);
        fftw_destroy_plan(plan2);

        e = compare_fftw_arrays(K * howmany.os, out, ro, io);
        assert(e <= max_error(), "fftw forward split vs. interleaved");

        plan1 = fftw_plan_dft_1d(CONV_WIN_SIZ, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
        plan2 = fftw_plan_guru_split_dft(1, &dim, 0, 0, ii, ri, io, ro, FFTW_ESTIMATE);

        fill_test_array(in, ri, ii);
        for (int k = 0; k < K; k++) fftw_execute_dft(plan1, in + k * howmany.is, out + k * howmany.os);
        fftw_execute(plan2);
        fftw_destroy_plan(plan1);
        fftw_destroy_plan(plan2);

        e = compare_fftw_arrays(K * howmany.os, out, ro, io);
        assert(e <= max_error(), "fftw backward split vs. interleaved");
        
        plan1 = fftw_plan_dft_r2c_1d(CONV_WIN_SIZ, ri, out, FFTW_ESTIMATE);
        plan2 = fftw_plan_guru_split_dft_r2c(1, &dim, 0, 0, ri, ro, io, FFTW_ESTIMATE);

        std::fill(ro, ro + 6 * K * CONV_WIN_SIZ, 0.0);
        fill_test_array(in, ri, ii);
        for (int k = 0; k < K; k++) fftw_execute_dft_r2c(plan1, ri + k * howmany.is, out + k * howmany.os);
        fftw_execute(plan2);
        fftw_destroy_plan(plan1);
        fftw_destroy_plan(plan2);

        e = 0;
        for (int k = 0; k < K; k++) {
            e += compare_fftw_arrays(CONV_WIN_SIZ / 2 + 1, out + k*CONV_WIN_SIZ, ro + k*CONV_WIN_SIZ, io + k*CONV_WIN_SIZ);
        }
        assert(e <= max_error(), "fftw real-to-complex split vs. interleaved");
        
        conv_free(array);
    }
} test_fftw_split;


#define CCONV_TEST_SAMPLE_FREQ 16000
#define CCONV_TEST_SIZE 8192

#define CCONV_TEST_WINDOW_SIZE 999
#define CCONV_TEST_WINDOW_FREQ { 50, 100, 200, 400, 1000, 2000, 4000 }
#define CCONV_TEST_WINDOW_STD 0.01
#define CCONV_TEST_REPEAT 1

#define CCONV_TEST_SIGNAL_SIZE CCONV_TEST_SIZE
#define	CCONV_TEST_SIGNAL_PITCH 100
#define CCONV_TEST_SIGNAL_HARM_POWER { 1, 0.7, 0.3 }
#define CCONV_TEST_SIGNAL_HARM_PHASE { 0, M_PI_4, M_PI/3 }

class test_cconv_t : public test_t
{
protected:

    virtual double max_error() { return 1E-10; }

    template<typename T>
    void generate_test_window(T *window_re, T *window_im, freq_t freq) {
        for (int i = 0; i < CCONV_TEST_WINDOW_SIZE; i++) {
            T t = T(i - CCONV_TEST_WINDOW_SIZE / 2) / CCONV_TEST_SAMPLE_FREQ;
            T power = model::gauss_win<T>(t, 0, CCONV_TEST_WINDOW_STD);
            T omega_t = 2 * M_PI * freq * t;
            window_re[i] = power * cos(omega_t);
            window_im[i] = power * sin(omega_t);
        }
    }

    template<typename T>
    void generate_test_signal(T *signal) {
        size_t size = CCONV_TEST_SIGNAL_SIZE;
        T freq = CCONV_TEST_SAMPLE_FREQ;
        T pitch = CCONV_TEST_SIGNAL_PITCH;
        T power[] = CCONV_TEST_SIGNAL_HARM_POWER;
        T phase[] = CCONV_TEST_SIGNAL_HARM_PHASE;

        for (size_t i = 0; i < size; i++) {
            T t = T(i) / CCONV_TEST_SAMPLE_FREQ;
            T value = 0.0;
            for (int j = 0; j < sizeof(power) / sizeof(T); j++) {
                value += power[j] * sin(2 * M_PI * (1 + j) * pitch * t + phase[j]);
            }
            signal[i] = value;
        }
    }

    void generate_testdata(
        double *signal,
        double *window_re,
        double *window_im,
        double *cconv_re_naive,
        double *cconv_im_naive,
        double window_freq)
    {
        typedef long double extended;

        extended signal_ext[CCONV_TEST_SIZE];
        extended window_ext[2 * CCONV_TEST_SIZE];
        extended cconv_ext[2 * CCONV_TEST_SIZE];
        extended
            *window_re_ext = window_ext,
            *window_im_ext = window_ext + CCONV_TEST_SIZE,
            *cconv_re_ext = cconv_ext,
            *cconv_im_ext = cconv_ext + CCONV_TEST_SIZE;

        // generate inputs
        generate_test_signal(signal_ext);
        std::fill(window_ext, window_ext + 2 * CCONV_TEST_SIZE, 0.0);
        generate_test_window(window_re_ext, window_im_ext, window_freq);

        // compute cconv with long double
        cconv_naive<extended, CCONV_TEST_SIZE>(signal_ext, CCONV_TEST_SIGNAL_SIZE, window_re_ext, CCONV_TEST_WINDOW_SIZE, cconv_re_ext);
        cconv_naive<extended, CCONV_TEST_SIZE>(signal_ext, CCONV_TEST_SIGNAL_SIZE, window_im_ext, CCONV_TEST_WINDOW_SIZE, cconv_im_ext);

        // downgrade long double to double
        std::copy(signal_ext, signal_ext + CCONV_TEST_SIZE, signal);
        std::copy(window_re_ext, window_re_ext + CCONV_TEST_SIZE, window_re);
        std::copy(window_im_ext, window_im_ext + CCONV_TEST_SIZE, window_im);
        std::copy(cconv_re_ext, cconv_re_ext + CCONV_TEST_SIZE, cconv_re_naive);
        std::copy(cconv_im_ext, cconv_im_ext + CCONV_TEST_SIZE, cconv_im_naive);
    }

    template<typename T, typename T2>
    void cconv_calc_error(const T *cconv_re, const T *cconv_im, const T2 *cconv_re_ideal, const T2 *cconv_im_ideal) {

        // calculate error
        T2 e_re = 0.0,
            e_im = 0.0,
            max_re = 0.0, min_re = 0.0,
            max_im = 0.0, min_im = 0.0;
        for (int i = 0; i < CCONV_TEST_SIZE; i++) {
            e_re += fabs((cconv_re[i] - cconv_re_ideal[i]) / cconv_re_ideal[i]);
            e_im += fabs((cconv_im[i] - cconv_im_ideal[i]) / cconv_im_ideal[i]);
            min_re = std::min(min_re, fabs(cconv_re[i]));
            min_im = std::min(min_im, fabs(cconv_im[i]));
            max_re = std::max(max_re, fabs(cconv_re[i]));
            max_im = std::max(max_im, fabs(cconv_im[i]));
        }
        e_re = e_re / CCONV_TEST_SIZE;
        e_im = e_im / CCONV_TEST_SIZE;

        // print error
        assert(e_re < max_error(), "Real error too large: %lg", e_re);
        assert(e_im < max_error(), "Imag error too large: %lg", e_im);
    }

};

class test_cconv_naive_t : public test_cconv_t
{
public:
    const char *name() { return "cconv_naive"; }
    double max_error() { return 1E-100; }
    void test() {

        typedef long double extended;
        typedef double cut_t;

        // generate test data
        extended signal[CCONV_TEST_SIGNAL_SIZE];
        extended window_re[CCONV_TEST_WINDOW_SIZE];
        extended window_im[CCONV_TEST_WINDOW_SIZE];
        generate_test_signal(signal);
        generate_test_window(window_re, window_im, 200);

        // compute cconv
        extended cconv_re[CCONV_TEST_SIZE];
        extended cconv_im[CCONV_TEST_SIZE];
        tic();
        for (int i = 0; i < CCONV_TEST_REPEAT; i++) {
            cconv_naive<extended, CCONV_TEST_SIZE>(signal, CCONV_TEST_SIGNAL_SIZE, window_re, CCONV_TEST_WINDOW_SIZE, cconv_re);
            cconv_naive<extended, CCONV_TEST_SIZE>(signal, CCONV_TEST_SIGNAL_SIZE, window_im, CCONV_TEST_WINDOW_SIZE, cconv_im);
        }
        set_execution_time(toc());
        
        // copy from long double to double
        cut_t cconv_re_cut[CCONV_TEST_SIZE];
        cut_t cconv_im_cut[CCONV_TEST_SIZE];
        std::copy(cconv_re, cconv_re + CCONV_TEST_SIZE, cconv_re_cut);
        std::copy(cconv_im, cconv_im + CCONV_TEST_SIZE, cconv_im_cut);

        // calculate error
        extended e = 0.0;
        for (int i = 0; i < CCONV_TEST_SIZE; i++) {
            extended re = cconv_re[i] - extended(cconv_re_cut[i]);
            extended im = cconv_im[i] - extended(cconv_im_cut[i]);
            e += re > 0 ? re : -re;
            e += im > 0 ? im : -im;
        }

        // print error
        assert(e <= max_error(), "Error too big: %lf > %lf", e, max_error());

        // save computed cconvs
        io::array_to_file(signal, CCONV_TEST_SIGNAL_SIZE, "cconv-naive-signal.bin");
        io::array_to_file(window_re, CCONV_TEST_WINDOW_SIZE, "cconv-naive-window-re.bin");
        io::array_to_file(window_im, CCONV_TEST_WINDOW_SIZE, "cconv-naive-window-im.bin");
        io::array_to_file(cconv_re, CCONV_TEST_SIZE, "cconv-naive-cconv-re.bin");
        io::array_to_file(cconv_im, CCONV_TEST_SIZE, "cconv-naive-cconv-im.bin");
        io::array_to_file(cconv_re_cut, CCONV_TEST_SIZE, "cconv-naive-cconv-re-cut.bin");
        io::array_to_file(cconv_im_cut, CCONV_TEST_SIZE, "cconv-naive-cconv-im-cut.bin");

    }

} test_cconv_naive;


class test_cconv_tc4_t: public test_cconv_t {
public:
    const char *name() { return "cconv_tc4"; }
    void test() {
        double signal[CCONV_TEST_SIZE];
        double window_re[CCONV_TEST_SIZE];
        double window_im[CCONV_TEST_SIZE];
        double cconv_re_naive[CCONV_TEST_SIZE];
        double cconv_im_naive[CCONV_TEST_SIZE];
        freq_t window_freq[] = CCONV_TEST_WINDOW_FREQ;
        int n_freq = sizeof(window_freq) / sizeof(freq_t);

        double cconv_re[CCONV_TEST_SIZE];
        double cconv_im[CCONV_TEST_SIZE];

        unsigned long time = 0;
        for (int i = 0; i < CCONV_TEST_REPEAT; i++) {
            for (int j = 0; j < n_freq; j++) {

                // generate test data
                generate_testdata(signal, window_re, window_im, cconv_re_naive, cconv_im_naive, window_freq[j]);

                // compute cconv with toom-cook 4x4
                tic();
                base<double, CCONV_TEST_SIZE>::cconv(signal, window_re, cconv_re);
                base<double, CCONV_TEST_SIZE>::cconv(signal, window_im, cconv_im);
                time += toc();

                // calculate errors
                //printf("window freq: %lg\n", double(window_freq[j]));
                cconv_calc_error(cconv_re, cconv_im, cconv_re_naive, cconv_im_naive);

            }
        }

        // print calculation time:
        set_execution_time(double(time) / CCONV_TEST_REPEAT / n_freq);

        // save files
        io::array_to_file(cconv_re, CCONV_TEST_SIZE, "cconv-tc4-cconv-re.bin");
        io::array_to_file(cconv_im, CCONV_TEST_SIZE, "cconv-tc4-cconv-im.bin");
    }
} test_cconv_tc4;

class test_cconv_fft_t : public test_cconv_t {
public:
    const char *name() { return "cconv_fft"; }
    void test() {

        // generate test data
        auto_ptr<double> memory(new double[11 * CCONV_TEST_SIZE]);
        double
            *signal = memory.get(),
            *window_re = memory.get() + CCONV_TEST_SIZE,
            *window_im = memory.get() + 2 * CCONV_TEST_SIZE,
            *cconv_re_naive = memory.get() + 3 * CCONV_TEST_SIZE,
            *cconv_im_naive = memory.get() + 4 * CCONV_TEST_SIZE,
            *signal_fft = memory.get() + 5 * CCONV_TEST_SIZE,
            *signal_fft_re = signal_fft,
            *signal_fft_im = signal_fft + CCONV_TEST_SIZE,
            *window_fft_re = memory.get() + 7 * CCONV_TEST_SIZE,
            *window_fft_im = memory.get() + 8 * CCONV_TEST_SIZE,
            *cconv_re = memory.get() + 9 * CCONV_TEST_SIZE,
            *cconv_im = memory.get() + 10 * CCONV_TEST_SIZE;
        freq_t window_freq[] = CCONV_TEST_WINDOW_FREQ;
        int n_freq = sizeof(window_freq) / sizeof(freq_t);

        //	assert(CCONV_TEST_SIZE == CONV_WIN_SIZ);

        // real calculations start
        unsigned long time = 0;
        for (int i = 0; i < CCONV_TEST_REPEAT; i++) {
            for (int j = 0; j < n_freq; j++) {

                // generate test data
                generate_testdata(signal, window_re, window_im, cconv_re_naive, cconv_im_naive, window_freq[j]);

                // compute cconv with toom-cook 4x4
                tic();
                cconv_calc_A(signal, signal_fft_re, signal_fft_im);
                cconv_calc_BC(window_re, window_im, window_fft_re, window_fft_im);
                cconv_normalize(signal_fft, 2 * CONV_WIN_SIZ);
                cconv(signal_fft_re, signal_fft_im, window_fft_re, window_fft_im, cconv_re, cconv_im);
                time += toc();

                // calculate errors
                //printf("window freq: %lg\n", double(window_freq[j]));
                cconv_calc_error(cconv_re, cconv_im, cconv_re_naive, cconv_im_naive);

            }
        }

        // print calculation time:
        set_execution_time(double(time) / CCONV_TEST_REPEAT / n_freq);

        // save files
        io::array_to_file(cconv_re, CCONV_TEST_SIZE, "cconv-fft-cconv-re.bin");
        io::array_to_file(cconv_im, CCONV_TEST_SIZE, "cconv-fft-cconv-im.bin");

    }
} test_cconv_fft;

class test_fft_approx_t : public test_error_t {
public:
    const char *name() { return "fft_approx"; }
    double max_error() { return 1E-10; }
    double error() {

        double array[CONV_WIN_SIZ * 11 + 2];
        double *a = SPL_MEMORY_ALIGN(array);
        double *b = a + CONV_WIN_SIZ;
        double *c = b + CONV_WIN_SIZ;
        double *Ar = c + CONV_WIN_SIZ;
        double *Ai = Ar + CONV_WIN_SIZ;
        double *B = Ai + CONV_WIN_SIZ;
        double *C = B + CONV_WIN_SIZ;
        double *ab1 = C + CONV_WIN_SIZ;
        double *ac1 = ab1 + CONV_WIN_SIZ;
        double *ab2 = ac1 + CONV_WIN_SIZ;
        double *ac2 = ab2 + CONV_WIN_SIZ;

        std::fill(ab1, ab1 + 4 * CONV_WIN_SIZ, 0.0);

        int Ws = 12;
        freq_t Fr = 200, dF = 3; // какие тут частоты указывать - не имеет особого значения
        double std = model::mask_std(Fr, 1.0);
        for (int m = -Ws; m <= Ws; m++) {
            a[Ws + m] = model::gauss_win(Fr + m * dF, Fr, std);
        }
        std::fill(a + 2 * Ws + 1, a + CONV_WIN_SIZ, 0.0);

        fill_test_array((fftw_complex*)B, b, c);

        cconv_calc_A(a, Ar, Ai);
        cconv_normalize(Ar, 2 * CONV_WIN_SIZ);
        cconv_calc_BC(b, c, B, C);
        cconv(Ar, Ai, B, C, ab1, ac1);

        for (int i = Ws; i < CONV_WIN_SIZ - Ws; i++) {
            double sumb = 0.0, sumc = 0.0;
            for (int m = -Ws; m <= Ws; m++) {
                sumb += a[Ws + m] * b[i - m];
                sumc += a[Ws + m] * c[i - m];
            }
            ab2[i] = sumb;
            ac2[i] = sumc;
        }

        io::array_to_file(ab1, CONV_WIN_SIZ, "cconv-1-fft.bin");
        io::array_to_file(ac1, CONV_WIN_SIZ, "cconv-2-fft.bin");
        io::array_to_file(ab2, CONV_WIN_SIZ, "cconv-1-naive.bin");
        io::array_to_file(ac2, CONV_WIN_SIZ, "cconv-2-naive.bin");

        double e1 = 0.0, e2 = 0.0;
        for (int i = Ws; i < CONV_WIN_SIZ - Ws; i++) {
            e1 += fabs(ab1[i + Ws] - ab2[i]);
            e2 += fabs(ac1[i + Ws] - ac2[i]);
        }

        return e1;
    }
} test_fft_approx;


NAMESPACE_TEST_END;