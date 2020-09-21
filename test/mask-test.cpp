#include "test.h"
#include "../core/scale.h"
#include "../core/mask.h"
#include "../io/iobit.h"

NAMESPACE_TEST_BEGIN;

using namespace spl;

namespace {

    const char *scale_std = "E:/testdata/scale-model-freq.bin";
    const char *spectrum_std = "E:/testdata/spectrum.bin";

    const char *mask_byte_std = "E:/testdata/sync-mask.bin";
    const char *mask_bit_std = "E:/testdata/sync-mask.bit";

    const char *mask_byte_test = "E:/testdata/test-sync-mask.bin";
    const char *mask_bit_test = "E:/testdata/test-sync-mask.bit";
    const char *mask_fast_test = "E:/testdata/test-sync-mask.bin";
    const char *mask_fast_bit_test = "E:/testdata/test-sync-mask.bit";

}

/*
void test_mask_extend_reduce() {
    int Ws = 12;
    int K = int(spl_params_t::DEFAULT.K);
    {
        io::ifstream<spectrum_t> in(spectrum_std);
        io::ofstream<spectrum_t> out(spectrum_test);
        spl::istream_block_extend<spectrum_t> in1(in, K, Ws);
        spl::ostream_block_reduce<spectrum_t> out1(out, K, Ws);

        tic();
        spectrum_t buf[1500];
        while (!in1.eos()) {
            size_t N = in1.read(buf, 1500);
            size_t M = out1.write(buf, N);
        }
        print_time();
    }

    spectrum_t e = compare_streams<spectrum_t>(spectrum_std, spectrum_test);
    print_error(e);
    print_status(e <= 0.0);
}*/


class test_mask_t : public test_error_t
{
    const char *_name;
    bool bit, fast;

public:
    typedef io::filter<spectrum_t, mask_t> mask_calc_t;
    
    test_mask_t(const char *n, bool b, bool f) :
        _name(n), bit(b), fast(f) {}

    const char *name() { return _name; }
    double max_error() { return 0; }
    double error() {
        // standard scale
        freq_scale_t sc = freq_scale_t::load(scale_std);

        // mask parameters
        mask_params_t p = spl_params_t::DEFAULT.freq_mask;

        mask_calc_t *calc = nullptr;
        if (fast)
            calc = new freq_mask_calculator_fast(sc, p);
        else 
            calc = new freq_mask_calculator(sc, p);

        const char *std_file, *test_file;

        io::ifstream<spectrum_t> spectrum(spectrum_std);

        if (bit) {
            std_file = mask_bit_std;
            test_file = mask_bit_test;
            io::ofstream<unsigned char> out_str(test_file);
            io::obitwrap8 out_bit_str(out_str);

            tic();
            size_t written = calc->execute(spectrum, out_bit_str);
            set_execution_time(toc());

            out_bit_str.flush();
        }
        else {
            std_file = mask_byte_std;
            test_file = mask_byte_test;
            io::ofstream<mask_t> out_str(test_file);

            tic();
            size_t written = calc->execute(spectrum, out_str);
            set_execution_time(toc());
        }

        delete calc;

        return compare_streams<unsigned char>(std_file, test_file);
    }
}
test_mask_byte_naive("mask_byte_naive", false, false),
test_mask_bit_naive("mask_bit_naive", true, false);
//test_mask_byte_fast("mask_byte_fast", false, true),
//test_mask_bit_fast("mask_bit_fast", true, true);


NAMESPACE_TEST_END;