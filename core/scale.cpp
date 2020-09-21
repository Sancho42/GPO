#include "scale.h"
#include "../io/iofile.h"
#include "model.h"
#include <math.h>
#include <limits>

NAMESPACE_SPL_BEGIN;

//
// Frequency scale generation
//

namespace {
    double lin_scale(freq_t x) { return (double)x; }
    freq_t rlin_scale(double x) { return (freq_t)x; }

    double log_scale(freq_t x) { return (double)log(x); }
    freq_t rlog_scale(double x) { return (freq_t)exp(x); }

    double mel(freq_t f) { return 1125 * log(1 + double(f) / 700); }
    freq_t rmel(double B) { return freq_t(700 * (exp(B / 1125) - 1)); }

    /// Прямая и обратная функция барк-шкалы частот.
    /// Используется апроксимация B(f) = 7* arcsinh(f/650)
    /// и arcsinh(x) = ln(x + sqrt(x^2 + 1))
    double bark(freq_t f) { return 7 * log(f / 650 + sqrt(1 + f*f / 422500)); }
    freq_t rbark(double B) { return freq_t(650 * sinh(B / 7)); }

    struct scale_form_info_t {
        const char *name;
        freq_scale_t::f2x_func f2x;
        freq_scale_t::x2f_func x2f;
    }
    scale_form_info[] = {
        { "linear",  lin_scale,  rlin_scale },
        { "log",     log_scale,  rlog_scale },
        { "mel",     mel,        rmel },
        { "bark",    bark,       rbark },
        { "model",   model::f2x, model::x2f },
    };

    const int scale_forms_count = sizeof(scale_form_info) / sizeof(scale_form_info_t);

    scale_form_info_t *get_info(scale_form_t form) {
        int i = int(form);
        if (i < 0 || i >= scale_forms_count)
            return nullptr;
        return &scale_form_info[i];
    }

    // Линейный поиск частоты в шкале.
    int search_freq_linear(const freq_t F[], int K, freq_t f) {
        freq_t min_error = std::numeric_limits<freq_t>::infinity();
        int min_index = K - 1;
        for (int k = 0; k < K; k++) {
            freq_t error = abs(F[k] - f);
            if (error < min_error) {
                min_error = error;
                min_index = k;
            }
        }
        return min_index;
    }

    // Двоичный поиск частоты в шкале.
    int search_freq_binary(const freq_t F[], int K, freq_t f) {
        int a = 0, b = K - 1;
        if (f < F[a]) return a;
        if (f > F[b]) return b;
        while (b - a > 1) {
            int c = (b + a) / 2;
            if (f < F[c]) b = c;
            else if (f > F[c]) a = c;
            else return c;
        }
        return (f - F[a] < F[b] - f) ? a : b;
    }

    void generate_freq_scale(freq_t F[], int K, 
        const freq_scale_t::f2x_func f2x, 
        const freq_scale_t::x2f_func x2f,
        const scale_point_t& p1,
        const scale_point_t& p2)
    {
        // расчет шага
        int i = p1.index, j = p2.index;
        if (i == 0 && j == 0) {
            i = 0;
            j = K - 1;
        }
        double Xi = f2x(p1.freq);
        double Xj = f2x(p2.freq);
        double dx = (Xi - Xj) / (i - j);

        // собственно вычисление частот	
        for (int k = 0; k < K; k++) {
            F[k] = x2f(Xi + dx * (k - i));
        }
    }
};

bool freq_scale_t::generate(f2x_func f2x, x2f_func x2f, const scale_point_t& point1, const scale_point_t& point2)
{
    this->form = scale_form_t::unknown;
    this->form_accuracy = 0;
    generate_freq_scale(F, K, f2x, x2f, point1, point2);
    return true;
}

bool freq_scale_t::generate(scale_form_t form, const scale_point_t& point1, const scale_point_t& point2)
{
    auto info = get_info(form);
    if (info == nullptr)
        return false;
    this->form = scale_form_t::unknown;
    this->form_accuracy = 0;
    generate_freq_scale(F, K, info->f2x, info->x2f, point1, point2);
    return true;
}

bool freq_scale_t::generate(scale_form_t form, freq_t freq1, freq_t freq2)
{
    return generate(form, scale_point_t(0, freq1), scale_point_t(0, freq2));
}


//
// Frequency scale form
//

scale_form_t freq_scale_t::get_form(bool estimate, int accuracy) const
{
    if (estimate && (form == scale_form_t::unknown || form_accuracy < accuracy)) {
        form = estimate_form(accuracy);
        form_accuracy = accuracy;
    }
    return form;
}

const char *freq_scale_t::scale_form_name(scale_form_t form) {
    auto info = get_info(form);
    return info ? info->name : "unknown";
}

scale_form_t freq_scale_t::estimate_form(int accuracy) const {

    // перебираем шкалы
    for (int i = 0; i < scale_forms_count; i++) {
        auto info = scale_form_info[i];
        scale_form_t form = static_cast<scale_form_t>(i);
        if (is_of_form(form, accuracy)) {
            return form;
        }
    }

    // ни одна из известных шкал не подошла
    return scale_form_t::unknown;
}


bool freq_scale_t::is_of_form(scale_form_t form, int accuracy) const {
    auto info = get_info(form);
    if (info == nullptr)
        return false;
    
    // функции шкалы:
    f2x_func f2x = info->f2x;
    x2f_func x2f = info->x2f;
    
    // расчет шага:
    double x1 = f2x(F[0]);
    double x2 = f2x(F[K - 1]);
    double dx = (x2 - x1) / (K - 1);

    // вычисление ошибки интерполяции
    double e = 0.0;
    for (int j = 1; j < accuracy; j++) {
        int k = j * K / (accuracy + 1);
        e += fabs(F[k] - x2f(x1 + k * dx));
    }

    // если суммарная ошибка по точкам меньше порога - то мы обнаружили шкалу
    return e < SCALE_ESTIMATE_ERROR * accuracy;
}


//
// Frequency searching & interpolation
//

int freq_scale_t::get_index(freq_t freq) const {
    if (form == scale_form_t::unknown) {
        return search_freq_linear(F, K, freq);
    }
    else {
        // TODO search frequencies by f2x and x2f
        return search_freq_binary(F, K, freq);
    }
}

freq_t freq_scale_t::interpolate_freq(double index) const {
    scale_form_t form = get_form(true);
    auto info = get_info(form);
    if (info == nullptr)
        throw "Can not interpolate without scale form";

    double x1 = info->f2x(F[0]);
    double x2 = info->f2x(F[K - 1]);
    double dx = (x2 - x1) / (K - 1);

    return info->x2f(x1 + index * dx);
}

double freq_scale_t::interpolate_index(freq_t freq) const {
    scale_form_t form = get_form(true);
    auto info = get_info(form);
    if (info == nullptr)
        throw "Can not interpolate without scale form";

    double x1 = info->f2x(F[0]);
    double x2 = info->f2x(F[K - 1]);
    double dx = (x2 - x1) / (K - 1);
    double x = info->f2x(freq);

    return (x - x1) / dx;
}


//
// Loading & saving
//

bool freq_scale_t::save(const char *filepath) const
{
	return io::array_to_file(F, K, filepath);
}

freq_scale_t freq_scale_t::load(const char *filepath)
{
    io::ifstream<freq_t> in(filepath);
    size_t file_size = in.size();
    if (file_size == 0) throw "Can't load scale from empty file";
    int K = int(file_size);
    if (K != file_size) throw "Scale file is too big";
    freq_t *freqs = spl_alloc<freq_t>(K);
    if (in.read(freqs, K) != K) {
        spl_free(freqs);
        throw "Can't read scale from file";
    }
    return freq_scale_t(freqs, K, true);
}

freq_scale_t freq_scale_t::copy(const freq_t *source, int size)
{
    freq_t *freqs = spl_alloc<freq_t>(size);
    memcpy(freqs, source, size * sizeof(freq_t));
    return freq_scale_t(freqs, size, true);
}

freq_scale_t freq_scale_t::copy(const freq_scale_t& source)
{
    freq_t *freqs = spl_alloc<freq_t>(source.K);
    memcpy(freqs, source.F, source.K * sizeof(freq_t));
    return freq_scale_t(freqs, source.K, true);
}

freq_scale_t freq_scale_t::generate(int size, scale_form_t form, const scale_point_t& point1, const scale_point_t& point2)
{
    freq_t *freqs = spl_alloc<freq_t>(size);
    freq_scale_t scale(freqs, size, true);
    scale.generate(form, point1, point2);
    return scale;
}

freq_scale_t freq_scale_t::generate(int size, scale_form_t form, freq_t freq1, freq_t freq2)
{
    return generate(size, form, scale_point_t(0, freq1), scale_point_t(0, freq2));
}

freq_scale_t freq_scale_t::generate(int size, f2x_func f2x, x2f_func x2f, const scale_point_t& point1, const scale_point_t& point2)
{
    freq_t *freqs = spl_alloc<freq_t>(size);
    freq_scale_t scale(freqs, size, true);
    scale.generate(f2x, x2f, point1, point2);
    return scale;
}

freq_scale_t freq_scale_t::generate(const scale_params_t& params) {
    freq_t *freqs = spl_alloc<freq_t>(params.K);
    freq_scale_t scale(freqs, params.K, true);
    scale.generate(params.form, params.point1, params.point2);
    return scale;
}


//
// Construction & Destruction
//

void freq_scale_t::init() {
    form = scale_form_t::unknown;
    form_accuracy = 0;
}

freq_scale_t::freq_scale_t(freq_t *freqs, int size) :
    allocated(false), F(freqs), K(size)
{
    init();
}

freq_scale_t::freq_scale_t(int size) :
    allocated(true), F(spl_alloc<freq_t>(size)), K(size)
{
    init();
}

freq_scale_t::freq_scale_t(freq_scale_t& that) :
    allocated(that.allocated), F(that.F), K(that.K)
{
    init();
    const_cast<bool&>(that.allocated) = false;
}

freq_scale_t::~freq_scale_t() {
    if (allocated) {
        spl_free(F);
    }
}

NAMESPACE_SPL_END;
