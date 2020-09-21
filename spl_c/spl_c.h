#ifndef _SPL_C_API_
#define _SPL_C_API_

#include <stdbool.h>
#include <stddef.h>

typedef double signal_t;
typedef double freq_t;
typedef double spectrum_t;
typedef bool mask_t;

#ifdef _MSC_VER
#   define EXPORT __declspec(dllexport)
#   define C_CALL 
#else
#   define EXPORT
#   define C_CALL
#endif

#ifdef __cplusplus
#   define EXTERN extern "C"
#else
#   define EXTERN
#endif

#define SPL_C_API EXTERN EXPORT

typedef enum {
    freq_scale_unknown = -1,
    freq_scale_linear = 0,
    freq_scale_log,
    freq_scale_mel,
    freq_scale_bark,
    freq_scale_model,
} scale_form_t;

SPL_C_API void C_CALL spl_freq_scale_generate(int num_freqs, freq_t *&freqs, scale_form_t form, freq_t freq_first, freq_t freq_last);
SPL_C_API bool C_CALL spl_freq_scale_load(const char *freq_scale_path, freq_t **freqs, int *num_freqs);
SPL_C_API void C_CALL spl_freq_scale_save(const char *freq_scale_path, const freq_t *freqs, int num_freqs);
SPL_C_API void C_CALL spl_freq_scale_free(const freq_t *freqs);
          
SPL_C_API size_t C_CALL spl_spectrum_calc_mem(int num_freqs, const freq_t *freqs, int num_samples, const signal_t *signal, freq_t sampling_freq, spectrum_t *spectrum, double window_error);
SPL_C_API size_t C_CALL spl_spectrum_calc_bin_file(int num_freqs, const freq_t *freqs, const char *signal_path, freq_t sampling_freq, const char *spectrum_path, double window_error);
SPL_C_API size_t C_CALL spl_spectrum_calc_wav_file(int num_freqs, const freq_t *freqs, const char *signal_path, const char *spectrum_path, double window_error);

SPL_C_API size_t C_CALL spl_freq_mask_calc_mem(int num_freqs, const freq_t *freqs, int num_samples, const spectrum_t *spectrum, mask_t *freq_mask, double window_error);
SPL_C_API size_t C_CALL spl_freq_mask_calc_bin_file(int num_freqs, const freq_t *freqs, const char *spectrum_path, const char *freq_mask_path, double window_error);
SPL_C_API size_t C_CALL spl_freq_mask_calc_bit_file(int num_freqs, const freq_t *freqs, const char *spectrum_path, const char *freq_mask_path, double window_error);

SPL_C_API size_t C_CALL spl_pitch_calc_mem(int num_freqs, const freq_t *freqs, int num_samples, const mask_t *freq_mask, freq_t *pitch, freq_t min_pitch, freq_t max_pitch, double window_error);
SPL_C_API size_t C_CALL spl_pitch_calc_bin_file(int num_freqs, const freq_t *freqs, const char *freq_mask_path, const char *pitch_path, freq_t min_pitch, freq_t max_pitch, double window_error);
SPL_C_API size_t C_CALL spl_pitch_calc_bit_file(int num_freqs, const freq_t *freqs, const char *freq_mask_path, const char *pitch_path, freq_t min_pitch, freq_t max_pitch, double window_error);

SPL_C_API size_t C_CALL spl_vocal_calc_bin_file(int num_freqs, const freq_t* freqs, const char* pitch_chan_test, const char* vocal_chan_test, freq_t minV, freq_t minNV, double orgF);
#endif//_SPL_C_API_