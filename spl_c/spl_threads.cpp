#include "../io/iomem.h"
#include "spl_threads.h"
#include "in_threads.h"

void C_CALL spl_pitch_signal(
	int K,
	int N,
	double sample_rate,
	freq_t F1,
	freq_t F2,
	signal_t* signal,
	freq_t* pitch,
	double window_error)
{
	io::imstream<signal_t> s(signal, N);
	io::omstream<freq_t> p(pitch, N);
	return spl::spl_pitch_in_threads(K, sample_rate, F1, F2, s, p, window_error);
}