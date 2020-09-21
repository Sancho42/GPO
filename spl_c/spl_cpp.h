#ifndef _SPL_CPP_API_
#define _SPL_CPP_API_

#include "../core/config.h"
#include "../core/scale.h"
#include "../io/io.h"
#include "../io/iobuf.h"

#ifdef _MSC_VER
#   define EXPORT __declspec(dllexport)
#   define C_CALL 
#else
#   define EXPORT
#   define C_CALL
#endif

NAMESPACE_SPL_BEGIN;

class EXPORT spl_calc_t
{
public:

    const freq_scale_t& scale() const { return *sc; }
    const spl_params_t& params() const { return p; }
    
    size_t calc_spectrum(int num_samples, freq_t sample_freq, const signal_t *signal, spectrum_t *spectrum) const;
    size_t calc_spectrum_bin(freq_t sample_freq, const char *signal_path, const char *spectrum_path) const;
    size_t calc_spectrum_wav(const char *signal_path, const char *spectrum_path) const;

    size_t calc_freq_mask(int num_samples, const spectrum_t *spectrum, mask_t *freq_mask) const;
    size_t calc_freq_mask_bin(const char *spectrum_path, const char *freq_mask_path) const;
    size_t calc_freq_mask_bit(const char *spectrum_path, const char *freq_mask_path) const;

    size_t calc_pitch(int num_samples, const mask_t *freq_mask, freq_t *pitch) const;
    size_t calc_pitch_bin(const char *freq_mask_path, const char *pitch_path) const;
    size_t calc_pitch_bit(const char *freq_mask_path, const char *pitch_path) const;

	void calc_all_parallel(int num_samples, freq_t sample_freq, const signal_t *signal, freq_t *pitch);

    //
    // construction
    //

    spl_calc_t();
    spl_calc_t(const spl_params_t&);
    ~spl_calc_t();

    spl_calc_t& with_params(const char *params_file);
    spl_calc_t& with_params(const spl_params_t& params);

    spl_calc_t& with_scale(const scale_params_t& scale_params);
    spl_calc_t& with_scale(int K, freq_t F1, freq_t F2);
    spl_calc_t& with_spectrum(double ksi) { p.spectrum.ksi = ksi; return *this; }
    spl_calc_t& with_freq_mask(double ksi) { p.freq_mask.ksi = ksi; return *this; }
    spl_calc_t& with_freq_mask(const mask_params_t& mask) { p.freq_mask = mask; return *this; }
    spl_calc_t& with_pitch(freq_t F1, freq_t F2) { p.pitch.F1 = F1; p.pitch.F2 = F2; return *this; }
    spl_calc_t& with_pitch(freq_t F1, freq_t F2, int num_harm) {
        p.pitch.F1 = F1;
        p.pitch.F2 = F2;
        p.pitch.Nh = num_harm;
        return *this;
    }
    spl_calc_t& with_pitch(const pitch_params_t& pitch) { p.pitch = pitch; return *this; }

private:
    spl_calc_t(spl_calc_t&); // move constructor

    spl_params_t p;
    freq_scale_t *sc;

	void spl_calc_t::spec_thread_func(io::istream<signal_t>& signal, io::memory_buffer<spectrum_t>& spec, freq_t sample_freq);
	void spl_calc_t::mask_thread_func(io::memory_buffer<spectrum_t>& spec, io::memory_buffer<mask_t>& freq_mask);
	void spl_calc_t::pitch_thread_func(io::memory_buffer<mask_t>& freq_mask, io::ostream<freq_t>& pitch);
};

NAMESPACE_SPL_END;

#endif//_SPL_CPP_API_