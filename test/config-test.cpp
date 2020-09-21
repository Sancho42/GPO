#include "test.h"
#include "../core/config.h"

namespace spl { 
    bool is_config_index_sorted(); 
}
namespace {
    const char *config_test = "config-test.txt";
}

NAMESPACE_TEST_BEGIN;


using namespace spl;

class test_config_rw_c : public test_t
{
    const char *name() { return "config_rw"; }
    void test() {
        spl_params_t params1;
        memset(&params1, 0, sizeof(spl_params_t));
        params1.scale.K = 100;
        params1.scale.form = scale_form_t::mel;
        params1.scale.point1.freq = 100;
        params1.scale.point2.freq = 1000;
        params1.spectrum.ksi = 0.02;
        params1.freq_mask.ksi = 0.02;
        params1.freq_mask.rho = 0.2;
        params1.freq_mask.delta = 1.0;
        params1.freq_mask.border_effect = false;
        params1.pitch.Nh = 3;
        params1.pitch.F1 = 50;
        params1.pitch.F2 = 400;
        params1.vocal.minV = 0.033;
        params1.vocal.minNV = 0.033;
        save_params(&params1, config_test);

        spl_params_t params2;
        memset(&params2, 0, sizeof(spl_params_t));
        load_params(&params2, config_test);

        int diff = memcmp(&params1, &params2, sizeof(spl_params_t));
        assert(diff == 0, "Written config differs from read config");
    }
} test_config_rw;

class test_config_index_c : public test_t {
    const char *name() { return "config_index"; }
    void test() {
        assert(spl::is_config_index_sorted(), "Config field index should be sorted");
    }
} test_config_index;

NAMESPACE_TEST_END;