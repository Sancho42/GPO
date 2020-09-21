#include "spl_cpp.h"

#include "../core/spectrum.h"
#include "../core/mask.h"
#include "../core/vocal.h"
#include "../io/iobit.h"
#include "../io/iofile.h"
#include "../io/iomem.h"
#include "../io/iowave.h"
#include "../io/iobuf.h"
#include "../io/io.h"

#include <thread>

NAMESPACE_SPL_BEGIN;

spl_calc_t::spl_calc_t() : p(spl_params_t::DEFAULT)
{
    sc = new freq_scale_t(freq_scale_t::generate(p.scale));
}

spl_calc_t::spl_calc_t(const spl_params_t& params) : p(params)
{
    sc = new freq_scale_t(freq_scale_t::generate(p.scale));
}

spl_calc_t::spl_calc_t(spl_calc_t& that) : p(that.p), sc(that.sc) 
{
    that.sc = nullptr;
}

spl_calc_t::~spl_calc_t() 
{
    if (sc) delete sc;
}

spl_calc_t& spl_calc_t::with_params(const spl_params_t& params)
{
    return with_scale(params.scale).
        with_spectrum(params.spectrum.ksi).
        with_freq_mask(params.freq_mask).
        with_pitch(params.pitch);
}

spl_calc_t& spl_calc_t::with_params(const char *params_file)
{
    spl_params_t params;
    load_params(&params, params_file);
    return with_params(params);
}

spl_calc_t& spl_calc_t::with_scale(const scale_params_t& s)
{
    p.scale = s;
    if (sc->size() != s.K) {
        delete sc;
        sc = new freq_scale_t(s.K);
    }
    sc->generate(s.form, s.point1, s.point2);
    return *this;
}

spl_calc_t& spl_calc_t::with_scale(int K, freq_t F1, freq_t F2)
{
    return with_scale(scale_params_t::create(K, scale_form_t::model, F1, F2));
}

size_t spl_calc_t::calc_spectrum(int num_samples, freq_t sample_freq, const signal_t *signal, spectrum_t *spectrum) const
{
    io::imstream<signal_t> s(signal, num_samples);
    io::omstream<spectrum_t> sp(spectrum, num_samples * sc->size());
    spl::spectrum_calculator calc(*sc, sample_freq, p.spectrum.ksi);
    return calc.execute(s, sp);
}

size_t spl_calc_t::calc_spectrum_bin(freq_t sample_freq, const char *signal_path, const char *spectrum_path) const
{
    io::ifstream<signal_t> s(signal_path);
    io::ofstream<spectrum_t> sp(spectrum_path);
    spl::spectrum_calculator calc(*sc, sample_freq, p.spectrum.ksi);
    return calc.execute(s, sp);
}

size_t spl_calc_t::calc_spectrum_wav(const char *signal_path, const char *spectrum_path) const
{
    io::iwstream<signal_t> s(signal_path);
    io::ofstream<spectrum_t> sp(spectrum_path);
    spl::spectrum_calculator calc(*sc, s.freq(), p.spectrum.ksi);
    return calc.execute(s, sp);
}

size_t spl_calc_t::calc_freq_mask(int num_samples, const spectrum_t *spectrum, mask_t *freq_mask) const
{
    io::imstream<spectrum_t> sp(spectrum, num_samples * sc->size());
    io::omstream<mask_t> m(freq_mask, num_samples * sc->size());
    spl::freq_mask_calculator calc(*sc, p.freq_mask);
    return calc.execute(sp, m);
}

size_t spl_calc_t::calc_freq_mask_bin(const char *spectrum_path, const char *freq_mask_path) const
{
    io::ifstream<spectrum_t> sp(spectrum_path);
    io::ofstream<mask_t> m(freq_mask_path);
    spl::freq_mask_calculator calc(*sc, p.freq_mask);
    return calc.execute(sp, m);
}

size_t spl_calc_t::calc_freq_mask_bit(const char *spectrum_path, const char *freq_mask_path) const
{
    io::ifstream<spectrum_t> sp(spectrum_path);
    io::ofstream<unsigned char> u(freq_mask_path);
    io::obitwrap8 m(u);
    spl::freq_mask_calculator calc(*sc, p.freq_mask);
    return calc.execute(sp, m);
}

size_t spl_calc_t::calc_pitch(int num_samples, const mask_t *freq_mask, freq_t *pitch) const
{
    io::imstream<mask_t> ms(freq_mask, num_samples * sc->size());
    io::omstream<freq_t> ps(pitch, num_samples * sc->size());
    spl::pitch_calculator calc(*sc, p.freq_mask, p.pitch);
    spl::freq_translator trans(ps, sc->frequences());
    return calc.execute(ms, trans);
}

size_t spl_calc_t::calc_pitch_bin(const char *freq_mask_path, const char *pitch_path) const
{
    io::ifstream<mask_t> ms(freq_mask_path);
    io::ofstream<freq_t> ps(pitch_path);

    spl::pitch_calculator calc(*sc, p.freq_mask, p.pitch);
    spl::freq_translator trans(ps, sc->frequences());
    return calc.execute(ms, trans);
}

size_t spl_calc_t::calc_pitch_bit(const char *freq_mask_path, const char *pitch_path) const
{
    io::ifstream<unsigned char> u(freq_mask_path);
    io::ibitwrap8 ms(u);
    io::ofstream<freq_t> ps(pitch_path);

    spl::pitch_calculator calc(*sc, p.freq_mask, p.pitch);
    spl::freq_translator trans(ps, sc->frequences());
    return calc.execute(ms, trans);
}

void spl_calc_t::calc_all_parallel(int num_samples, freq_t sample_freq, const signal_t *signal, freq_t *pitch)
{
	io::imstream<signal_t> signal_st(signal, num_samples);
	io::memory_buffer<spectrum_t> spec_buf = io::memory_buffer<spectrum_t>();
	io::memory_buffer<mask_t> mask_buf = io::memory_buffer<mask_t>();
	io::omstream<freq_t> pitch_st(pitch, num_samples * sc->size());

	std::thread spec_thread(&spl_calc_t::spec_thread_func, this, std::ref(signal_st), std::ref(spec_buf), sample_freq);
	std::thread mask_thread(&spl_calc_t::mask_thread_func, this, std::ref(spec_buf), std::ref(mask_buf));
	std::thread pitch_thread(&spl_calc_t::pitch_thread_func, this, std::ref(mask_buf), std::ref(pitch_st));
	
	spec_thread.join();
	mask_thread.join();
	pitch_thread.join();
}

void spl_calc_t::spec_thread_func(io::istream<signal_t>& signal, io::memory_buffer<spectrum_t>& spec, freq_t sample_freq)
{
	spl::spectrum_calculator calc(*sc, sample_freq, p.spectrum.ksi);
	calc.execute(signal, spec);

}

void spl_calc_t::mask_thread_func(io::memory_buffer<spectrum_t>& spec, io::memory_buffer<mask_t>& freq_mask)
{
	spl::freq_mask_calculator_fast calc(*sc, p.freq_mask);
	calc.execute(spec, freq_mask);
}

void spl_calc_t::pitch_thread_func(io::memory_buffer<mask_t>& freq_mask, io::ostream<freq_t>& pitch)
{
	spl::pitch_calculator calc(*sc, p.freq_mask, p.pitch);
	spl::freq_translator trans(pitch, sc->frequences());
	calc.execute(freq_mask, trans);
}


NAMESPACE_SPL_END;