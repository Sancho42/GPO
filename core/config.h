#ifndef _SPL_CONFIG_
#define _SPL_CONFIG_

#include "spl_types.h"

NAMESPACE_SPL_BEGIN;

/// Параметры сигнала.
struct signal_params_t {
    freq_t F; ///< Частота дискретизации

    static const signal_params_t DEFAULT;
};

/// Параметры генерации шкалы частот.
struct scale_point_t {
    int index;
    freq_t freq;

    scale_point_t() = default;

    scale_point_t(int i, freq_t f) :
        index(i), freq(f) {}

};

struct scale_params_t {
    int K;
    scale_form_t form;
    scale_point_t point1, point2;

    static scale_params_t create(int K, scale_form_t form, freq_t Fa, freq_t Fb);

    static scale_params_t create(int K, scale_form_t form, const scale_point_t& point1, const scale_point_t& point2);

    static scale_params_t create(int K, scale_form_t form, int i, freq_t Fi, int j, freq_t Fj);

    static const scale_params_t DEFAULT;
};

/// Параметры генерации фильтров спектрограммы.
struct spectrum_params_t {
    double ksi;

    static const spectrum_params_t DEFAULT;
};

/// Параметры, используемые для генерации коэффициентов маскировки (\ref mask_filters).
struct mask_params_t {

    /// Величина, определяющая точность вычислений (размер окна фильтров).
    double ksi;

    /// Коэффициент, определяющий ширину маскирующей функции.
    double delta;

    /// Коэффициент, определяющий вес маскирующей функции.
    double rho;

    /// Триггер - учитывать краевой эффект или нет (см. mask_filters_generate()).
    bool border_effect;

    static const mask_params_t DEFAULT;
};

/// Параметры генерации шаблонов для выделения ЧОТ - частоты основного тона (см. \ref mask_templates).
struct pitch_params_t {

    /// Количество учитываемых гармоник (для определения ЧОТ: 2).
    int Nh;

    /// Нижняя граница определения ЧОТ.
    freq_t F1;

    /// Верхняя граница определения ЧОТ.
    freq_t F2;

    static const pitch_params_t DEFAULT;
};

/// Параметры сегментации по признаку вокализованности.
struct vocal_params_t {

    /// минимальная длительность вокализованного сегмента (в секундах)
    double minV;

    /// минимальная длительность невокализованного сегмента (в секундах)
    double minNV;

    static const vocal_params_t DEFAULT;
};

struct spl_params_t {
    signal_params_t signal;
    scale_params_t scale;
    spectrum_params_t spectrum;
    mask_params_t freq_mask;
    pitch_params_t pitch;
    vocal_params_t vocal;

    static const spl_params_t DEFAULT;
};

void load_params(spl_params_t *params, const char *filename);
void save_params(const spl_params_t *params, const char *filename);


NAMESPACE_SPL_END;

#endif//_SPL_CONFIG_