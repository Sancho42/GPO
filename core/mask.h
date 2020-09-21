#ifndef _SPL_MASK_
#define _SPL_MASK_

///
/// \file  mask.h
/// \brief Типы данных, структуры и функции, относящиеся к маскировке.
///

#include "config.h"
#include "../io/io.h"

NAMESPACE_SPL_BEGIN;

class freq_mask_calculator :
    public io::filter<spectrum_t, mask_t>
{
public:
    freq_mask_calculator(const char *filepath);
    freq_mask_calculator(const freq_scale_t& s, double ksi);
    freq_mask_calculator(const freq_scale_t& s, const mask_params_t& p);
    ~freq_mask_calculator();

    /// Сохранить коэффициенты в файл.
    bool save(const char *filepath);

    size_t execute(io::istream<spectrum_t>& spectrum, io::ostream<mask_t>& mask) const override;

private:

    int K, Ws;
    double *H;

    bool init(const freq_scale_t& s, const mask_params_t& p);
};

class freq_mask_calculator_fast :
    public io::filter<spectrum_t, mask_t>
{
public:
    freq_mask_calculator_fast(const char *filepath);
    freq_mask_calculator_fast(const freq_scale_t& s, double ksi);
    freq_mask_calculator_fast(const freq_scale_t& s, const mask_params_t& p);
    ~freq_mask_calculator_fast();

    /// Сохранить коэффициенты в файл.
    bool save(const char *filepath);

    size_t execute(io::istream<spectrum_t>& spectrum, io::ostream<mask_t>& mask) const override;

private:
    int K, Ws;
    double *H;

    bool init(const freq_scale_t& s, const mask_params_t& p);
};

size_t mask_memory(const freq_scale_t& scale, size_t N, const spectrum_t *spectrum, mask_t *mask, const mask_params_t& p);

NAMESPACE_SPL_END;

#endif//_SPL_MASK_
