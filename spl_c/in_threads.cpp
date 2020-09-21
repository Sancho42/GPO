///
/// \file  spl_threads.cpp
/// \brief Модуль полной обработки звука в параллельных потоках
///


#include "../spl_c/spl_c.h"
#include "../core/spl_types.h"
#include "../core/scale.h"
#include "../core/spectrum.h"
#include "../core/mask.h"
#include "../core/vocal.h"
#include "../io/iobuf.h"
#include "../io/iomem.h"
#include "io.h"
#include <stdio.h>
#include <windows.h>
#include "in_threads.h"

using io::istream;
using io::ostream;
using io::memory_buffer;

namespace spl {

	static inline size_t _spl_spectrum_calc(int num_freqs, const freq_t *freqs,
		io::istream<signal_t>& s, io::ostream<spectrum_t>& sp, freq_t sample_freq, double window_error)
	{
		spl::freq_scale_t scale(const_cast<freq_t *>(freqs), num_freqs);
		spl::spectrum_calculator calc(scale, sample_freq, window_error);
		return calc.execute(s, sp);
	}

	static inline size_t _spl_freq_mask_calc(int num_freqs, const freq_t *freqs,
		io::istream<spectrum_t>& sp, io::ostream<mask_t>& m, double window_error)
	{
		spl::freq_scale_t scale(const_cast<freq_t *>(freqs), num_freqs);
		spl::freq_mask_calculator_fast calc(scale, window_error);
			return calc.execute(sp, m);
	}

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

	DWORD WINAPI specThreadProc(LPVOID lpParam) {
		spec_params *params = (spec_params *)lpParam;
		return (DWORD)_spl_spectrum_calc(params->num_freqs, const_cast<freq_t *>(params->freqs),
			*params->s, *params->sp, params->sampling_freq, params->window_error);
	}

	DWORD WINAPI maskThreadProc(LPVOID lpParam) {
		mask_params *params = (mask_params *)lpParam;
		return (DWORD)_spl_freq_mask_calc(params->num_freqs, const_cast<freq_t *>(params->freqs),
			*params->sp, *params->m, params->window_error);
	}

	DWORD WINAPI pitchThreadProc(LPVOID lpParam) {
		pitch_params *params = (pitch_params *)lpParam;
		return (DWORD)_spl_pitch_calc(params->num_freqs, const_cast<freq_t *>(params->freqs),
			*params->m, *params->p, params->min_pitch, params->max_pitch, params->window_error);
	}

	HANDLE createSimpleThread(LPTHREAD_START_ROUTINE startAddress, LPVOID lpParam) {
		return CreateThread(NULL, 1000000, startAddress, lpParam, NULL, NULL);
	}

	///
	/// Поиск вокализированных участков в аудиопотоке
	///
	void spl_pitch_in_threads(
		int K,
		double sample_rate,
		freq_t F1,
		freq_t F2,
		istream<signal_t>& in_str,
		ostream<freq_t>& out_str,
		double window_error)
	{
		// generate scale
		scale_params_t scale_p = scale_params_t::create(K, scale_form_t::model, F1, F2);
		freq_scale_t scale = freq_scale_t::generate(scale_p);
		const freq_t* scale_model = scale.frequences();

		// initiate pipe:
		memory_buffer<spectrum_t> spec_buf = memory_buffer<spectrum_t>();
		memory_buffer<mask_t> mask_buf = memory_buffer<mask_t>();

		// spectrum thread:
		spec_params spec_p;
		spec_p.num_freqs = K;
		spec_p.freqs = scale_model;
		spec_p.s = &in_str;
		spec_p.sp = &spec_buf.output();
		spec_p.sampling_freq = sample_rate;
		spec_p.window_error = window_error;
		HANDLE specThread = createSimpleThread(specThreadProc, &spec_p);

		// mask thread:
		mask_params mask_p;
		mask_p.num_freqs = K;
		mask_p.freqs = scale_model;
		mask_p.sp = &spec_buf.input();
		mask_p.m = &mask_buf.output();
		mask_p.window_error = window_error;
		HANDLE maskThread = createSimpleThread(maskThreadProc, &mask_p);

		// pitch thread:
		pitch_params pitch_p;
		pitch_p.num_freqs = K;
		pitch_p.freqs = scale_model;
		pitch_p.m = &mask_buf.input();
		pitch_p.p = &out_str;
		pitch_p.min_pitch = 70;
		pitch_p.max_pitch = 1000;
		pitch_p.window_error = window_error;
		HANDLE pitchThread = createSimpleThread(pitchThreadProc, &pitch_p);

		HANDLE threads[3] = {
			specThread,
			maskThread,
			pitchThread
		};

		WaitForMultipleObjects(sizeof(threads) / sizeof(*threads), threads, true, INFINITE);

		// destroy everything:
		CloseHandle(pitchThread);
		CloseHandle(maskThread);
		CloseHandle(specThread);
	}
}