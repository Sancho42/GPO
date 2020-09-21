#ifndef _SPL_THREADS_
#define _SPL_THREADS_

#include <windows.h>
#include "../core/spl_types.h"
#include "../io/iobuf.h"
#include "../io/iomem.h"
#include "io.h"

namespace spl {

	void spl_pitch_in_threads(
		int K,
		double sample_rate,
		freq_t F1,
		freq_t F2,
		io::istream<signal_t>& in_str,
		io::ostream<freq_t>& out_str,
		double window_error
	);

	struct spec_params {
		int num_freqs;				 ///< Размер шкалы частот
		const freq_t *freqs;         ///< Шкала частот
		io::istream<signal_t> *s;    ///< Входной поток (сигнал)
		io::ostream<spectrum_t> *sp; ///< Выходной поток (спектр)
		freq_t sampling_freq;        ///< Частота дискретизации
		double window_error;         ///< Величина, определяющая точность вычислений (размер окна)
	};


	struct mask_params {
		int num_freqs;				 ///< Размер шкалы частот
		const freq_t *freqs;               ///< Шкала частот
		io::istream<spectrum_t> *sp; ///< Входной поток  (спектр)
		io::ostream<mask_t> *m;		 ///< Выходной поток (маска)
		double window_error;         ///< Величина, определяющая точность вычислений (размер окна)
	};


	struct pitch_params {
		int num_freqs;				 ///< Размер шкалы частот
		const freq_t *freqs;               ///< Шкала частот
		io::istream<mask_t> *m;		 ///< входной поток (маска)
		io::ostream<freq_t> *p;	     ///< выходной поток (номера каналов ЧОТ)
		freq_t min_pitch;			 ///< минимальная частота определения ЧОТ
		freq_t max_pitch;		     ///< максимальная частота определения ЧОТ
		double window_error;         ///< Величина, определяющая точность вычислений (размер окна)
	};
}
#endif//_SPL_THREADS_