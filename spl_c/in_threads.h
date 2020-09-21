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
		int num_freqs;				 ///< ������ ����� ������
		const freq_t *freqs;         ///< ����� ������
		io::istream<signal_t> *s;    ///< ������� ����� (������)
		io::ostream<spectrum_t> *sp; ///< �������� ����� (������)
		freq_t sampling_freq;        ///< ������� �������������
		double window_error;         ///< ��������, ������������ �������� ���������� (������ ����)
	};


	struct mask_params {
		int num_freqs;				 ///< ������ ����� ������
		const freq_t *freqs;               ///< ����� ������
		io::istream<spectrum_t> *sp; ///< ������� �����  (������)
		io::ostream<mask_t> *m;		 ///< �������� ����� (�����)
		double window_error;         ///< ��������, ������������ �������� ���������� (������ ����)
	};


	struct pitch_params {
		int num_freqs;				 ///< ������ ����� ������
		const freq_t *freqs;               ///< ����� ������
		io::istream<mask_t> *m;		 ///< ������� ����� (�����)
		io::ostream<freq_t> *p;	     ///< �������� ����� (������ ������� ���)
		freq_t min_pitch;			 ///< ����������� ������� ����������� ���
		freq_t max_pitch;		     ///< ������������ ������� ����������� ���
		double window_error;         ///< ��������, ������������ �������� ���������� (������ ����)
	};
}
#endif//_SPL_THREADS_