#ifndef _SPL_THREADS_API_
#define _SPL_THREADS_API_

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

SPL_C_API void C_CALL spl_pitch_signal(
	int K,
	int N,
	double sample_rate,
	freq_t F1,
	freq_t F2,
	signal_t* signal,
	freq_t* pitch,
	double window_error);

#endif//_SPL_THREADS_API_