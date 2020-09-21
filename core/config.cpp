#include "config.h"
#include "../io/iofile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>

NAMESPACE_SPL_BEGIN;

// default params
const signal_params_t signal_params_t::DEFAULT = { 12000 };
const scale_params_t scale_params_t::DEFAULT = { 256, scale_form_t::model, {0, 50}, {0, 4000} };
const spectrum_params_t spectrum_params_t::DEFAULT = { 0.001 };
const mask_params_t mask_params_t::DEFAULT = { 0.001, 1, 1, true };
const pitch_params_t pitch_params_t::DEFAULT = { 2, 75, 400 };
const vocal_params_t vocal_params_t::DEFAULT = { 0.030, 0.030 };
const spl_params_t spl_params_t::DEFAULT = {
    signal_params_t::DEFAULT,
    scale_params_t::DEFAULT,
    spectrum_params_t::DEFAULT,
    mask_params_t::DEFAULT,
    pitch_params_t::DEFAULT,
    vocal_params_t::DEFAULT
};

scale_params_t scale_params_t::create(int K, scale_form_t form, freq_t Fa, freq_t Fb) {
    scale_params_t result;
    result.K = K;
    result.form = form;
    result.point1.index = 0;
    result.point2.index = 0;
    result.point1.freq = Fa;
    result.point2.freq = Fb;
    return result;
}

scale_params_t scale_params_t::create(int K, scale_form_t form, const scale_point_t& point1, const scale_point_t& point2) {
    scale_params_t result;
    result.K = K;
    result.form = form;
    result.point1 = point1;
    result.point2 = point2;
    return result;
}

scale_params_t scale_params_t::create(int K, scale_form_t form, int i, freq_t Fi, int j, freq_t Fj) {
    scale_params_t result;
    result.K = K;
    result.form = form;
    result.point1.index = i;
    result.point2.index = j;
    result.point1.freq = Fi;
    result.point2.freq = Fj;
    return result;
}


namespace {


static const char *scale_form_labels[] = {
    "linear",
    "log",
    "mel",
    "bark",
    "model",
    nullptr
};

struct field_config_t {
    const char *name;
    enum type_t {
        int_field,
        bool_field,
        double_field,
        enum_field,
    } type;
    size_t offset;
    const char **labels;

    template<typename T>
    field_config_t(const char *n, const spl_params_t& params, const T& field, const char **label) :
        name(n), labels(label)
    {
        offset = (char *)(&field) - (char *)(&params);
        type = type_getter<T>::get_type();
    }

    field_config_t(const char *n) : name(n) {}

    void read(const char *text, spl_params_t *params) {
        switch (type) {
        case int_field:
            sscanf(text, "%d", field_ptr<int>(params));
            break;
        case bool_field:
            field_ref<bool>(params) =
                0 != strcmp(text, "false") &&
                0 != strcmp(text, "no") &&
                0 != strcmp(text, "off") &&
                0 != strcmp(text, "0");
            break;
        case double_field:
            sscanf(text, "%lg", field_ptr<double>(params));
            break;
        case enum_field:
            for (int i = 0; labels[i]; i++) {
                if (0 == strcmp(text, labels[i])) {
                    field_ref<int>(params) = i;
                    return;
                }
            }
            field_ref<int>(params) = -1;
        }
    }

    void write(char *text, const spl_params_t *params) {
        switch (type) {
        case int_field:
            sprintf(text, "%d", field_ref<int>(params));
            break;
        case bool_field:
            sprintf(text, "%s", field_ref<bool>(params) ? "true" : "false");
            break;
        case double_field:
            sprintf(text, "%lg", field_ref<double>(params));
            break;
        case enum_field:
            sprintf(text, "%s", labels[field_ref<int>(params)]);
            break;
        }
    }

    static int compare_names(const void *pa, const void *pb) {
        const field_config_t *a = reinterpret_cast<const field_config_t *>(pa);
        const field_config_t *b = reinterpret_cast<const field_config_t *>(pb);
        return strcmp(a->name, b->name);
    }

private:

    template<typename T>
    struct type_getter { static type_t get_type() { return enum_field; } };

#define TYPE_GETTER_CLASS(type) template<> \
    struct type_getter<type> { static type_t get_type() { return type##_field; } }

    TYPE_GETTER_CLASS(int);
    TYPE_GETTER_CLASS(bool);
    TYPE_GETTER_CLASS(double);

    template<typename T>
    T *field_ptr(const spl_params_t *params) {
        return (T *)((char *)(params)+ offset);
    }

    template<typename T>
    T& field_ref(const spl_params_t *params) {
        return *field_ptr<T>(params);
    }
};

#define FIELD(text, name) { text, spl_params_t::DEFAULT, spl_params_t::DEFAULT . name, nullptr }
#define LABEL(text, name, labels) { text, spl_params_t::DEFAULT, spl_params_t::DEFAULT . name, labels }

// NOTE: keep it sorted for fast searching
field_config_t field_configurations[] = {
    FIELD("freq_mask_border", freq_mask.border_effect),
    FIELD("freq_mask_delta", freq_mask.delta),
    FIELD("freq_mask_ksi", freq_mask.ksi),
    FIELD("freq_mask_rho", freq_mask.rho),
    FIELD("pitch_freq_high", pitch.F2),
    FIELD("pitch_freq_low", pitch.F1),
    FIELD("pitch_num_harmonics", pitch.Nh),
    LABEL("scale_form", scale.form, scale_form_labels),
    FIELD("scale_freq_high", scale.point2.freq),
    FIELD("scale_freq_low", scale.point1.freq),
    FIELD("scale_num_channels", scale.K),
    FIELD("signal_sampling_freq", signal.F),
    FIELD("spectrum_ksi", spectrum.ksi),
    FIELD("vocal_min_interval", vocal.minV),
    FIELD("vocal_min_nonvocal", vocal.minNV),
};

const int num_fields = sizeof(field_configurations) / sizeof(field_config_t);

} // namespace {

bool is_config_index_sorted() {
    for (int i = 1; i < num_fields; i++) {
        if (strcmp(field_configurations[i - 1].name, field_configurations[i].name) >= 0)
            return false;
    }
    return true;
}

void load_params(spl_params_t *params, const char *filepath) 
{
    *params = spl_params_t::DEFAULT;
    FILE *file = fopen(filepath, "rt");
    const int BUFSIZE = 1000;
    char buffer[BUFSIZE];
    while (fgets(buffer, BUFSIZE, file) == buffer) {
        const char *name = buffer;
        const char *value = nullptr;
        int state = 0;
        for (char *c = buffer; c < buffer + BUFSIZE; c++) {
            switch (*c) {
            case '#':
            case '\n':
            case '\r':
                *c = '\0';
            case '\0':
                state = 4;
                break;
            case '=':
                state = 2;
                value = c + 1;
            case ' ':
            case '\t':
                *c = '\0';
                if (state == 0) name = c + 1;
                if (state == 2) value = c + 1;
                break;
            default:
                if (state == 0) state = 1;
                if (state == 2) state = 3;
            }
            if (state == 4) break;
        }

        if (value == nullptr) continue; // skip line

        field_config_t key(name);
        field_config_t *cfg = (field_config_t *) bsearch(&key, field_configurations, 
            num_fields, sizeof(key), field_config_t::compare_names);
        
        if (cfg != nullptr) {
            cfg->read(value, params);
        } else {
            printf("Warning: unknown setting '%s'\n", name);
        }

    }
    fclose(file);    
}

void save_params(const spl_params_t *params, const char *filepath)
{
    char value[1000];
    FILE *file = fopen(filepath, "wt");
    for (int i = 0; i < num_fields; i++) {
        field_configurations[i].write(value, params);
        fprintf(file, "%s = %s\n", field_configurations[i].name, value);
    }
    fclose(file);
}

NAMESPACE_SPL_END;