#ifndef _SPL_COMMON_
#define _SPL_COMMON_

///
/// \file  common.h
/// \brief Common definitions for SPL.
///

#define _USE_MATH_DEFINES // M_PI, etc
#define _CRT_SECURE_NO_WARNINGS

#define NAMESPACE_SPL_BEGIN namespace spl {
#define NAMESPACE_SPL_END   } // namespace spl

/// 
/// Namespace for all functions and classes of SPL.
/// It is used for all functions and classes, that are used 
///  for calculation of speech parameters 
///  included in SPL (Speech Parameterization Library).
///

NAMESPACE_SPL_BEGIN;

/// Allocate memory.
void *spl_alloc_low(size_t siz);

/// Free memory.
void spl_free(void *addr);

/// Allocate memory.
template<typename T>
T *spl_alloc(size_t siz) {
	return (T*) spl_alloc_low(siz * sizeof(T));
}
//@}

NAMESPACE_SPL_END;

#endif//_SPL_COMMON_
