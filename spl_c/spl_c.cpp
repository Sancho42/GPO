#include "spl_c.h"

#include "../core/scale.h"
#include "../core/spectrum.h"
#include "../core/mask.h"
#include "../core/vocal.h"
#include "../io/iobit.h"
#include "../io/iofile.h"
#include "../io/iomem.h"
#include "../io/iowave.h"

//
// Frequency scales
//

void C_CALL spl_freq_scale_generate(int num_freqs, freq_t *&freqs, scale_form_t form, freq_t freq_first, freq_t freq_last)
{
    spl::freq_scale_t scale(freqs, num_freqs);
	spl::freq_scale_t scale2 = scale.generate(spl::scale_params_t::create(num_freqs, spl::scale_form_t(form), freq_first, freq_last));
	const freq_t *F = scale2.frequences();
	for (int i = 0; i < num_freqs; i++) {
		freqs[i] = F[i];
	}
}

bool C_CALL spl_freq_scale_load(const char *freq_scale_path, freq_t **freqs, int *num_freqs)
{
    size_t read;
    bool success = io::array_from_file(freqs, &read, freq_scale_path);
    *num_freqs = int(read);
    return success;
}

void C_CALL spl_freq_scale_save(const char *freq_scale_path, const freq_t *freqs, int num_freqs)
{
    io::array_to_file(freqs, num_freqs, freq_scale_path);
}

void C_CALL spl_freq_scale_free(const freq_t *freqs)
{
    delete[] freqs;
}


//
// Spectrum calculations
//

static inline size_t _spl_spectrum_calc(int num_freqs, const freq_t *freqs, io::istream<signal_t>& s, io::ostream<spectrum_t>& sp, freq_t sample_freq, double window_error)
{
    spl::freq_scale_t scale(const_cast<freq_t *>(freqs), num_freqs);
    spl::spectrum_calculator calc(scale, sample_freq, window_error);
    return calc.execute(s, sp);
}

size_t C_CALL spl_spectrum_calc_mem(int num_freqs, const freq_t *freqs, int num_samples, const signal_t *signal, freq_t sampling_freq, spectrum_t *spectrum, double window_error)
{
    io::imstream<signal_t> s(signal, num_samples);
    io::omstream<spectrum_t> sp(spectrum, num_samples * num_freqs);
    return _spl_spectrum_calc(num_freqs, freqs, s, sp, sampling_freq, window_error);
}

size_t C_CALL spl_spectrum_calc_bin_file(int num_freqs, const freq_t *freqs, const char *signal_path, freq_t sampling_freq, const char *spectrum_path, double window_error)
{
    io::ifstream<signal_t> s(signal_path);
    io::ofstream<spectrum_t> sp(spectrum_path);
    return _spl_spectrum_calc(num_freqs, freqs, s, sp, sampling_freq, window_error);
}

size_t C_CALL spl_spectrum_calc_wav_file(int num_freqs, const freq_t *freqs, const char *signal_path, const char *spectrum_path, double window_error)
{
    io::iwstream<signal_t> s(signal_path);
    io::ofstream<spectrum_t> sp(spectrum_path);
    return _spl_spectrum_calc(num_freqs, freqs, s, sp, s.freq(), window_error);
}


//
// Frequency mask calculations
//

static inline size_t _spl_freq_mask_calc(int num_freqs, const freq_t *freqs, 
    io::istream<spectrum_t>& sp, io::ostream<mask_t>& m, double window_error)
{
    spl::freq_scale_t scale(const_cast<freq_t *>(freqs), num_freqs);
    spl::freq_mask_calculator calc(scale, window_error);
    return calc.execute(sp, m);
}

size_t C_CALL spl_freq_mask_calc_mem(int num_freqs, const freq_t *freqs, int num_samples, const spectrum_t *spectrum, mask_t *mask, double window_error)
{
    io::imstream<spectrum_t> sp(spectrum, num_samples * num_freqs);
    io::omstream<mask_t> m(mask, num_samples * num_freqs);
    return _spl_freq_mask_calc(num_freqs, freqs, sp, m, window_error);
}

size_t C_CALL spl_freq_mask_calc_bin_file(int num_freqs, const freq_t *freqs, const char *spectrum_path, const char *freq_mask_path, double window_error)
{
    io::ifstream<spectrum_t> sp(spectrum_path);
    io::ofstream<mask_t> m(freq_mask_path);
    return _spl_freq_mask_calc(num_freqs, freqs, sp, m, window_error);
}

size_t C_CALL spl_freq_mask_calc_bit_file(int num_freqs, const freq_t *freqs, const char *spectrum_path, const char *freq_mask_path, double window_error)
{
    io::ifstream<spectrum_t> sp(spectrum_path);
    io::ofstream<unsigned char> u(freq_mask_path);
    io::obitwrap8 m(u);
    return _spl_freq_mask_calc(num_freqs, freqs, sp, m, window_error);
}


//
// Pitch calculations
//

static inline size_t _spl_pitch_calc(int num_freqs, const freq_t *freqs, 
    io::istream<mask_t>& m, io::ostream<freq_t>& p, 
    freq_t min_pitch, freq_t max_pitch, double window_error)
{
    spl::freq_scale_t scale(const_cast<freq_t *>(freqs), num_freqs);

    spl::mask_params_t pm = spl::spl_params_t::DEFAULT.freq_mask;
    pm.ksi = window_error;


    spl::pitch_params_t pp = spl::spl_params_t::DEFAULT.pitch;
    pp.F1 = min_pitch;
    pp.F2 = max_pitch;

    spl::pitch_calculator calc(scale, pm, pp);
    spl::freq_translator trans(p, freqs);
    return calc.execute(m, trans);
}

size_t C_CALL spl_pitch_calc_mem(int num_freqs, const freq_t *freqs, int num_samples, const mask_t *freq_mask, freq_t *pitch, freq_t min_pitch, freq_t max_pitch, double window_error)
{
    io::imstream<mask_t> m(freq_mask, num_samples * num_freqs);
    io::omstream<freq_t> p(pitch, num_samples * num_freqs);
    return _spl_pitch_calc(num_freqs, freqs, m, p, min_pitch, max_pitch, window_error);
}

size_t C_CALL spl_pitch_calc_bin_file(int num_freqs, const freq_t *freqs, const char *freq_mask_path, const char *pitch_path, freq_t min_pitch, freq_t max_pitch, double window_error)
{
    io::ifstream<mask_t> m(freq_mask_path);
    io::ofstream<freq_t> p(pitch_path);
    return _spl_pitch_calc(num_freqs, freqs, m, p, min_pitch, max_pitch, window_error);
}

size_t C_CALL spl_pitch_calc_bit_file(int num_freqs, const freq_t *freqs, const char *freq_mask_path, const char *pitch_path, freq_t min_pitch, freq_t max_pitch, double window_error)
{
    io::ifstream<unsigned char> u(freq_mask_path);
    io::ibitwrap8 m(u);
    io::ofstream<freq_t> p(pitch_path);
    return _spl_pitch_calc(num_freqs, freqs, m, p, min_pitch, max_pitch, window_error);
}


static inline size_t C_CALL _spl_vocal_calc(int num_freqs, const freq_t* freqs, const char* pitch_chan_test, const char* vocal_chan_test, freq_t minV, freq_t minNV, double orgF)
{
    io::ifstream<short> p(pitch_chan_test);
    io::ofstream<short> v(vocal_chan_test);

    spl::freq_scale_t scale(const_cast<freq_t*>(freqs), num_freqs);

    spl::vocal_params_t par;
    par.minV = minV;
    par.minNV = minNV;

    freq_t F = orgF;

    return spl::vocal_segment(p, v, F, par);
}

size_t C_CALL spl_vocal_calc_bin_file(int num_freqs, const freq_t* freqs, const char* pitch_chan_test, const char* vocal_chan_test, freq_t minV, freq_t minNV, double orgF)
{
    return _spl_vocal_calc(num_freqs, freqs, pitch_chan_test, vocal_chan_test, minV, minNV, orgF);
}
