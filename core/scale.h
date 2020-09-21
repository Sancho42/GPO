#ifndef _SPL_SCALE
#define _SPL_SCALE

///
/// \file  scale.h
/// \brief Структуры и функции, относящиеся к частотным шкалам.
///

#include "spl_types.h"
#include "config.h"

NAMESPACE_SPL_BEGIN;

/// Стандартное количество точек, по которому определяется тип шкалы.
const int SCALE_ESTIMATE_DEFAULT = 5;

/// Стандартное количество точек, по которому определяется тип шкалы.
const int SCALE_ESTIMATE_ACCURATE = 0;

/// Максимальная ошибка функции scale_form_estimate
const double SCALE_ESTIMATE_ERROR = 1E-10;

///
/// Шкала резонансных частот нерекурсивных фильтров.
///
/// Шкала резонансных частот (шкала) - это основной параметр системы фильтров, 
///  который и передается в библиотеку для фильтрации сигнала.
/// 
/// Шкала представляет из себя обычный массив частот.
/// Частоты в шкале возрастают: чем больше номер канала, тем выше частота.
/// 

struct freq_scale_t {

    /// Массив частот.
    const freq_t *frequences() const { return F; }

    /// Количество частот в шкале.
    int size() const { return K; }
    
    //
    // Frequency scale generation
    //

    /// Функция шкалы - для любой заданной частоты возвращает значение некоторой метрики.
    typedef double(*f2x_func)(freq_t);

    /// Обратная функция шкалы - для значения метрики возвращает частоту.
    typedef freq_t(*x2f_func)(double);

    /// Сгенерировать шкалу частот по прямой и обратной функциям.
    bool generate(f2x_func f2x, x2f_func x2f, const scale_point_t& point1, const scale_point_t& point2);

    /// Сгенерировать шкалу частот заданной формы.
    bool generate(scale_form_t form, const scale_point_t& point1, const scale_point_t& point2);

    /// Сгенерировать шкалу частот заданной формы.
    bool generate(scale_form_t form, freq_t freq1, freq_t freq2);


    //
    // Frequency scale form 
    //

    /// Получение (или определение) формы шкалы частот.
    scale_form_t get_form(bool estimate = false, int accuracy = SCALE_ESTIMATE_DEFAULT) const;

    /// Название формы шкалы частот.
    static const char *scale_form_name(scale_form_t form);

    /// Определение формы шкалы частот.
    scale_form_t estimate_form(int accuracy = SCALE_ESTIMATE_DEFAULT) const;

    /// Проверка формы шкалы частот.
    bool is_of_form(scale_form_t, int accuracy = SCALE_ESTIMATE_DEFAULT) const;

    
    //
    // Frequency searching & interpolation
    //

    /// Получение частоты по номеру канала.
    freq_t operator[](int index) const { return F[index]; }

    /// Получение частоты по номеру канала.
    freq_t get_freq(int index) const { return F[index]; }

    /// Получение номера канала по частоте.
    int get_index(freq_t freq) const;

    /// Интерполяция шкалы по номеру канала.
    freq_t interpolate_freq(double index) const;
    
    /// Интерполяция шкалы по частоте.
    double interpolate_index(freq_t freq) const;


    //
    // Construction & Destruction
    //

    /// construct from explicit array of frequences and copy them
    freq_scale_t(freq_t *freqs, int size);

    /// allocate new scale of specified size
    freq_scale_t(int size);

    /// move constructor
    freq_scale_t(freq_scale_t&);

    /// Копировать шкалу из указанного массива.
    static freq_scale_t copy(const freq_t *freqs, int size);

    /// Копировать шкалу из указанного массива.
    static freq_scale_t copy(const freq_scale_t& source);

    /// Загрузка шкалы частот из файла.
    static freq_scale_t load(const char *filepath);

    /// Сгенерировать шкалу частот по прямой и обратной функциям.
    static freq_scale_t generate(int size, f2x_func f2x, x2f_func x2f, const scale_point_t& point1, const scale_point_t& point2);

    /// Сгенерировать шкалу частот заданной формы.
    static freq_scale_t generate(int size, scale_form_t form, const scale_point_t& point1, const scale_point_t& point2);

    /// Сгенерировать шкалу частот заданной формы.
    static freq_scale_t generate(int size, scale_form_t form, freq_t freq1, freq_t freq2);

    /// Сгенерировать шкалу частот по параметрам.
    static freq_scale_t generate(const scale_params_t& params);

    /// Сохранение шкалы частот в файл.
    bool save(const char *filepath) const;

    ~freq_scale_t();

private:
    const bool allocated;
    
    freq_t * const F;

    const int K;

    mutable scale_form_t form;
    mutable int form_accuracy;

    void init();

    freq_scale_t(freq_t *freqs, int size, bool maintain) :
        F(freqs), K(size), allocated(maintain) 
    {
        init();
    }

};

NAMESPACE_SPL_END;

#endif//_SPL_SCALE
