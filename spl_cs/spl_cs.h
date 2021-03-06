#pragma once

#include "../spl_c/spl_cpp.h"

using namespace System;
using namespace spl;

namespace SPL {
	public ref class ScaleParams {
	public:
		int K = 256;
		double F1 = 50;
		double F2 = 4000;
	};

	public ref class PitchParams {
	public:
		int numHarm = 2;
		double F1 = 75;
		double F2 = 400;
	};

	public ref class SPLParams {
	public:
		ScaleParams ^scale = gcnew ScaleParams();
		PitchParams ^pitch = gcnew PitchParams();
		double ksi = 0.001;
	};

	public ref class SPLCalc
	{
    public:
        SPLCalc() { impl = new spl::spl_calc_t(); }
        ~SPLCalc() { delete impl; }

        SPLCalc^ WithParams(String^ params_path);
		SPLCalc^ WithParams(SPLParams^ params) {
			impl->with_spectrum(params->ksi);
			impl->with_freq_mask(params->ksi);
			WithScale(params->scale);
			WithPitch(params->pitch);
			return this;
		};
        SPLCalc^ WithScale(int K, double F1, double F2) { impl->with_scale(K, F1, F2); return this; }
		SPLCalc^ WithScale(ScaleParams ^params) { impl->with_scale(params->K, params->F1, params->F2); return this; };
        SPLCalc^ WithSpectrum(double ksi) { impl->with_spectrum(ksi); return this; }
        SPLCalc^ WithFreqMask(double ksi) { impl->with_freq_mask(ksi); return this; }
        SPLCalc^ WithPitch(double F1, double F2) { impl->with_pitch(F1, F2); return this; }
        SPLCalc^ WithPitch(double F1, double F2, int numHarm) { impl->with_pitch(F1, F2, numHarm); return this; }
		SPLCalc^ WithPitch(PitchParams ^params) { impl->with_pitch(params->F1, params->F2, params->numHarm); return this; }

        array<spectrum_t, 2>^ CalcSpectrum(freq_t sample_freq, array<signal_t>^ signal);
        size_t CalcSpectrumBin(freq_t sample_freq, String^ signal_path, String^ spectrum_path);
        size_t CalcSpectrumWav(String^ signal_path, String^ spectrum_path);

        array<mask_t, 2>^ CalcFreqMask(array<spectrum_t, 2>^ spectrum);
        size_t CalcFreqMaskBin(String^ spectrum_path, String^ freq_mask_path);
        size_t CalcFreqMaskBit(String^ spectrum_path, String^ freq_mask_path);

        array<freq_t>^ CalcPitch(array<mask_t, 2>^ freq_mask);
        size_t CalcPitchBin(String^ freq_mask_path, String^ pitch_path);
        size_t CalcPitchBit(String^ freq_mask_path, String^ pitch_path);

		array<freq_t>^ SPLCalc::CalcAllParallel(freq_t sample_freq, array<signal_t>^ signal);
		
    private:
        spl::spl_calc_t *impl;
	};

} // namespace SPL
