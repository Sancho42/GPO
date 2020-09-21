#include "test.h"
#include "../core/scale.h"
#include <cmath>

NAMESPACE_TEST_BEGIN;

using namespace spl;

const char *scale_std = "E:/testdata/scale-model-freq.bin";
const char *scale_test = "E:/testdata/test-scale-model-freq.bin";

class test_scale_generate_t : public test_error_t
{
    const char *name() { return "scale_generate"; }
    double max_error() { return 2E-10; }
    double error() {
        freq_scale_t sc = freq_scale_t::generate(spl_params_t::DEFAULT.scale);
        sc.save(scale_test);

        return compare_streams<freq_t>(scale_std, scale_test);
    }
} test_scale_generate;

class test_scale_form_estimate_t : public test_t
{
public:
    const char *name() { return "scale_form_estimate"; }
    void test() {
        freq_scale_t sc(256);
        scale_point_t point1(0, 50.0), point2(0, 4000.0);

        scale_form_t form;
        scale_form_t forms[] = {
            scale_form_t::linear, 
            scale_form_t::log, 
            scale_form_t::mel, 
            scale_form_t::bark, 
            scale_form_t::model, 
            scale_form_t::unknown 
        };
        for (int i = 0; forms[i] != scale_form_t::unknown; i++) {
            sc.generate(forms[i], point1, point2);
            if (!sc.generate(forms[i], point1, point2)) {
                fail("Can't generate scale '%s'", freq_scale_t::scale_form_name(forms[i]));
            }
            form = sc.estimate_form();
            assert(form == forms[i], "Generated scale '%s', but estimated scale '%s'", 
                freq_scale_t::scale_form_name(forms[i]), 
                freq_scale_t::scale_form_name(form));
        }
    }
} test_scale_form_estimate;

class test_scale_interp_t : public test_t
{
    const char *name() { return "scale_interp"; }
    double max_error() { return 1E-10; }
    void test() {
        int K = 256, dk = 50;
        freq_t F1 = 50.0, F2 = 4000.0;
        freq_scale_t base_scale = freq_scale_t::generate(K, scale_form_t::model, F1, F2);
        freq_scale_t extended_scale = freq_scale_t::generate(K + 2*dk, scale_form_t::model,
            scale_point_t(dk, F1), scale_point_t(dk + K - 1, F2));

        // scale_generate test
        for (int m = 0; m < K; m++) {
            freq_t e = abs(base_scale.get_freq(m) - extended_scale.get_freq(m + dk));
            assert(e <= max_error(), "scale extrapolation failed at %d: %lf > %lf", m, e, max_error());
        }
        
        // interpolate_freq test
        for (int m = 0; m < K; m++) {
            freq_t fm = base_scale.interpolate_freq(m);
            freq_t e = fabs(fm - extended_scale.get_freq(m + dk));
            assert(e <= max_error(), "scale_interp_k2f failed at %d: %lf > %lf", m, e, max_error());
        }
        
        // interpolate_index test
        for (int m = 0; m < K; m++) {
            double k = base_scale.interpolate_index(extended_scale.get_freq(m + dk));
            freq_t e = fabs(k - m);
            assert(e <= max_error(), "scale_interp_f2k failed at %d: %lf > %lf", m, e, max_error());
        }
    }
} test_scale_interp;

NAMESPACE_TEST_END;