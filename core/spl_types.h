#ifndef _SPL_TYPES_
#define _SPL_TYPES_

///
/// \file  spl_types.h
/// \brief Типы данных и структуры библиотеки SPL.
///

#include "common.h"

NAMESPACE_SPL_BEGIN;

/// Тип чисел, представляющих частоту (в т.ч. частоту дискретизации).
typedef double freq_t;

/// Тип числа в сигнале. 
/// По умолчанию вещественные числа двойной точности (double).
typedef double signal_t;

/// Тип числа в спектре. 
/// По умолчанию вещественные числа двойной точности (double).
typedef double spectrum_t;

/// Тип числа в маске.
/// По умолчанию булево.
typedef bool mask_t;


/// Форма шкалы частот
enum class scale_form_t {
    unknown = -1, ///< неизвестная форма шкалы
    linear = 0,   ///< линейная шкала
    log,          ///< логарифмическая шкала
    mel,          ///< шкала мел
    bark,         ///< шкала барк
    model,        ///< шкала модели слуха
};


/// Шкала частот.
struct freq_scale_t;

/// Калькулятор спектра.
class spectrum_calculator;

/// Калькулятор одновременной маскировки.
class freq_mask_calculator;
class freq_mask_calculator_fast;

/// Калькулятор последовательной маскировки.
class temp_mask_calculator;

/// Калькулятор частоты основного тона (ЧОТ).
class pitch_calculator;

/// Калькулятор признака вокализованности.
class vocal_calculator;

NAMESPACE_SPL_END;

#endif//_SPL_TYPES_
