#ifndef _SPL_SPECTRUM_
#define _SPL_SPECTRUM_

///
/// \file  spectrum.h
/// \brief Типы данных, структуры и функции, относящиеся к спектрам.
///

#include "common.h"
#include "spl_types.h"
#include "../io/io.h"

NAMESPACE_SPL_BEGIN;

///
/// Вычисление спектрограммы.
/// Использует оптимизацию вычисления свертки через FFT.
///

class spectrum_calculator :
    public io::filter<signal_t, spectrum_t>
{
public:

    spectrum_calculator(const freq_scale_t& s, freq_t F, double ksi);
    ~spectrum_calculator();

    size_t execute(io::istream<signal_t>& signal, io::ostream<spectrum_t>& spectrum) const override;

    /// Сохранить параметры в файл.
    bool save(const char *filepath);

    // TODO: загрузка из файла, параметр Ws вычислять с помощью обратного Фурье.
    spectrum_calculator(const char *filepath);

private:
    int K, Ws;
    double *H;

    bool init(const freq_scale_t& scale, freq_t F, double ksi);
};

NAMESPACE_SPL_END;

#endif//_SPL_SPECTRUM_
