#include "stdafx.h"
#include "spl_cs.h"
#include <stdlib.h>
#include <vcclr.h>
#include <memory>

namespace SPL {

array<spectrum_t, 2>^ SPLCalc::CalcSpectrum(freq_t sample_freq, array<signal_t>^ signal)
{
    int N = signal->Length;
    int K = impl->scale().size();

    array<spectrum_t, 2>^ spectrum = gcnew array<spectrum_t, 2>(K, N);
    pin_ptr<signal_t> signal_ptr = &signal[0];
    pin_ptr<spectrum_t> spectrum_ptr = &spectrum[0, 0];

    size_t NK = impl->calc_spectrum(signal->Length, sample_freq, signal_ptr, spectrum_ptr);
    if (NK != N * K) {
        throw gcnew Exception("Read wrong number of elements");
    }
    return spectrum;
}

static std::auto_ptr<char> convert_string(String^ string) 
{
    pin_ptr<const wchar_t> wcs = PtrToStringChars(string);
    std::auto_ptr<char> mbs(new char[string->Length * 2]);
    wcstombs(mbs.get(), wcs, string->Length * 2);
    return mbs;
}

size_t SPLCalc::CalcSpectrumBin(freq_t sample_freq, String^ signal_path, String^ spectrum_path)
{
    auto signal_path_c = convert_string(signal_path);
    auto spectrum_path_c = convert_string(spectrum_path);
    return impl->calc_spectrum_bin(sample_freq, signal_path_c.get(), spectrum_path_c.get());
}

size_t SPLCalc::CalcSpectrumWav(String^ signal_path, String^ spectrum_path)
{
    auto signal_path_c = convert_string(signal_path);
    auto spectrum_path_c = convert_string(spectrum_path);
    return impl->calc_spectrum_wav(signal_path_c.get(), spectrum_path_c.get());
}

array<mask_t, 2>^ SPLCalc::CalcFreqMask(array<spectrum_t, 2>^ spectrum)
{
    int K = spectrum->GetLength(1);
    int N = spectrum->GetLength(2);

    if (K != impl->scale().size()) {
        throw gcnew Exception("Spectrum has wrong size");
    }
    
    array<mask_t, 2>^ mask = gcnew array<mask_t, 2>(K, N);
    pin_ptr<spectrum_t> spectrum_ptr = &spectrum[0,0];
    pin_ptr<mask_t> mask_ptr = &mask[0,0];

    size_t NK = impl->calc_freq_mask(N, spectrum_ptr, mask_ptr);
    if (NK != N * K) {
        throw gcnew Exception("Read wrong number of elements");
    }
    return mask;
}

size_t SPLCalc::CalcFreqMaskBin(String^ spectrum_path, String^ freq_mask_path)
{
    auto spectrum_path_c = convert_string(spectrum_path);
    auto freq_mask_path_c = convert_string(freq_mask_path);
    return impl->calc_freq_mask_bin(spectrum_path_c.get(), freq_mask_path_c.get());
}

size_t SPLCalc::CalcFreqMaskBit(String^ spectrum_path, String^ freq_mask_path)
{
    auto spectrum_path_c = convert_string(spectrum_path);
    auto freq_mask_path_c = convert_string(freq_mask_path);
    return impl->calc_freq_mask_bit(spectrum_path_c.get(), freq_mask_path_c.get());
}

array<freq_t>^ SPLCalc::CalcPitch(array<mask_t, 2>^ freq_mask)
{
    int K = freq_mask->GetLength(1);
    int N = freq_mask->GetLength(2);

    if (K != impl->scale().size()) {
        throw gcnew Exception("Spectrum has wrong size");
    }

    array<freq_t>^ pitch = gcnew array<freq_t>(N);
    pin_ptr<mask_t> mask_ptr = &freq_mask[0, 0];
    pin_ptr<freq_t> pitch_ptr = &pitch[0];

    size_t N2 = impl->calc_pitch(N, mask_ptr, pitch_ptr);
    if (N2 != N) {
        throw gcnew Exception("Read wrong number of elements");
    }
    return pitch;
}

size_t SPLCalc::CalcPitchBin(String^ freq_mask_path, String^ pitch_path)
{
    auto freq_mask_path_c = convert_string(freq_mask_path);
    auto pitch_path_c = convert_string(pitch_path);
    return impl->calc_pitch_bin(freq_mask_path_c.get(), pitch_path_c.get());
}

size_t SPLCalc::CalcPitchBit(String^ freq_mask_path, String^ pitch_path)
{
    auto freq_mask_path_c = convert_string(freq_mask_path);
    auto pitch_path_c = convert_string(pitch_path);
    return impl->calc_pitch_bit(freq_mask_path_c.get(), pitch_path_c.get());
}

array<freq_t>^ SPLCalc::CalcAllParallel(freq_t sample_freq, array<signal_t>^ signal)
{
	int N = signal->Length;

	array<freq_t>^ pitch = gcnew array<freq_t>(N);
	pin_ptr<signal_t> signal_ptr = &signal[0];
	pin_ptr<freq_t> pitch_ptr = &pitch[0];
	impl->calc_all_parallel(signal->Length, sample_freq, signal_ptr, pitch_ptr);

	return pitch;
}

SPLCalc^ SPLCalc::WithParams(String^ params_path)
{
    auto freq_mask_path_c = convert_string(params_path);
    impl->with_params(freq_mask_path_c.get());
    return this;
}

} // namespace SPL