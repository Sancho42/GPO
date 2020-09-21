#ifndef _SPL_VOCAL_
#define _SPL_VOCAL_

///
/// \file  vocal.h
/// \brief Функции для обработки звуков по признаку вокализованности.
///
/// Включает в себя функции для определения частоты основного тона (ЧОТ) 
///  и автоматической сегментации сигналов на вокализованные/невокализованные.
///

#include "common.h"
#include "spl_types.h"
#include "mask.h"
#include "scale.h"
#include "../io/io.h"
#include "../io/iowrap.h"
#include "mpir.h"


NAMESPACE_SPL_BEGIN;

typedef mp_limb_t limb_t;

class pitch_calculator :
    public io::filter<mask_t, short>
{
public:
    pitch_calculator(const freq_scale_t& scale, const mask_params_t& pm, const pitch_params_t& pp);
    ~pitch_calculator();

    size_t execute(io::istream<mask_t>& mask, io::ostream<short>& pitch) const override;

private:
    freq_scale_t scale;
    int K, Nt;
    int k1, k2;
    const int max_diff;
    void *memory;

    bool init(const mask_params_t& pm, const pitch_params_t& pp);    
};


/// Стандартное значение максимального отклонения маски вокализованного звука от шаблона.
const int DEFAULT_PITCH_MAX_DIFF = 6;


class freq_translator : public io::owrapelem<short, freq_t>
{
public:
    freq_translator(io::ostream<freq_t>& output, const freq_t *F) :
        io::owrapelem<short, freq_t>(output), freqs(F) {}

    void convert(const short& i, freq_t& f) const override
    {
        f = i < 0 ? 0.0 : freqs[i];
    }

    const freq_t *freqs;
};


/// Функция сегментации сигнала по признаку вокализованности
size_t vocal_segment(
	io::istream<short>& in_str,  ///< входной поток (номера каналов ЧОТ)
	io::ostream<short>& out_str, ///< выходной поток (номера отсчетов-границ сегментов)
	const freq_t F,                 ///< частота дискретизации сигнала
	const vocal_params_t& p       ///< параметры сегментации
);

NAMESPACE_SPL_END;

#endif//_SPL_VOCAL_