///
/// \file  vocal.cpp
/// \brief Функции для обработки звуков по признаку вокализованности.
///

#include "vocal.h"
#include "scale.h"
#include "model.h"
#include "mask.h"
#include "matrix.h"
#include "../io/iomem.h"
#include "../io/iofile.h"
#include "../io/iobit.h"

#include "mpir.h"
#include <algorithm>
#include <queue>
using std::fill_n;

#define CEIL_MODULUS(x,y) ( ( (x) + (y) - 1 ) / (y) )

NAMESPACE_SPL_BEGIN;

#define vec_add_limb mpn_add_n
#define vec_sub_limb mpn_sub_n
#define popcount_limb mpn_hamdist

const int num_part_bits = 8;
const int num_part_variants = 1 << num_part_bits;
#define GET_PART get_part<num_part_bits>

template<typename Elem>
struct bit_vector {
    struct ref {
        ref(Elem& p, Elem m) : ptr(p), mask(m) {}
        bool operator=(bool v) {
            if (v)
                ptr |= mask;
            else
                ptr &= ~mask;
            return v;
        }
        operator bool() const {
            return (ptr & mask) != 0;
        }
    private:
        Elem& ptr;
        Elem mask;
    };

    bit_vector(Elem *p) : ptr(p) {}

    bool operator[](int i) const {
        return (ptr[i / bits_per_elem] & get_mask(i)) != 0;
    }
    ref operator[](int i) {
        return ref(ptr[i / bits_per_elem], get_mask(i));
    }

private:
    Elem *ptr;
    static const int bits_per_elem = sizeof(Elem) * 8;
    static Elem get_mask(int i) {
        return Elem(1) << (i % bits_per_elem);
    }
};

bool pitch_calculator::init(const mask_params_t& pm, const pitch_params_t& p)
{
	bool r;

	// 1. вычисление k1, k2
	k1 = scale.get_index(p.F1);
	k2 = scale.get_index(p.F2);
    K = scale.size();
    Nt = 2 * p.Nh + 2;
    int nk = k2 - k1 + 1;

	// 2. Формирование тестовых сигналов

	freq_t Fs = 12000; // частота тестового сигнала в принципе не имеет значения
	double Sm = floor(double(Fs) / p.F1); // длительность тестовых сигналов
	int s = (int) floor(Sm / 2); // точка, в которой рассчитывается интенсивность

	// выделяем место: 
	spectrum_t *I1 = spl_alloc<spectrum_t>(nk * K);
	Matrix<spectrum_t, 2> I = matrix_ptr(I1, nk, K);

	// собственно вычисления:
	for(int kt = k1; kt <= k2; kt++) {
		for(int k = 0; k < K; k++) {
			double yc = 0.0, ys = 0.0;
			for(int n = 1; n <= p.Nh; n++) {
				double tmp = model::f_C24 / 2 * model::filter_quality(scale[k]) * (1 - n * scale[kt]/scale[k]);
				double H = exp( - tmp * tmp );
				yc += H * cos(2 * M_PI * n * scale[kt] / Fs * s);
				ys += H * sin(2 * M_PI * n * scale[kt] / Fs * s);
			}
			I(kt - k1, k) = yc * yc + ys * ys;
		}
	}

    // 3. Одновременная маскировка и вычисление шаблона

    // количество каналов = количество бит в сэмпле
    const int num_sample_bits = K;

    // количество (неполных) байт в сэмпле
    const int num_sample_bytes = CEIL_MODULUS(num_sample_bits, 8);

    // количество (неполных) чисел в сэмпле
    const int num_sample_limbs = CEIL_MODULUS(num_sample_bytes, sizeof(limb_t));

    memory = spl_alloc_low(2 * num_sample_limbs * nk * sizeof(limb_t));
    limb_t *tpl_values = (limb_t *) memory;
    limb_t *tpl_masks = (limb_t *)(memory) + num_sample_limbs * nk;

    fill_n(tpl_masks, num_sample_limbs * nk, 0);

    freq_mask_calculator mask_calc(scale, pm);

    // для каждого канала ЧОТ:
    for (int kt = 0; kt < nk; kt++) {

        limb_t *tpl = tpl_values + kt * num_sample_limbs;
        bit_vector<limb_t> tpl_bits(tpl);

        // одновременная маскировка
        io::imstream<spectrum_t> input(&I(kt, 0), K);
        io::omstream<limb_t> output_u(tpl, num_sample_limbs);
        io::obitwrap<limb_t> output_b(output_u);

        if (mask_calc.execute(input, output_b) != K) {
            // обработка ошибки  
            spl_free(memory);
            r = false;
            goto out;
        }

        // ищем единицу перед первой гармоникой:
        // вначале смотрим на первую гармонику (ЧОТ)
        // там должна быть единица
        if (tpl_bits[k1 + kt] == false) {
            spl_free(memory);
            r = false;
            goto out;
        }

		// ищем первую границу слева:
        int k;
        int borders[100];
		for(k = k1 + kt - 1; k >= 0 && tpl_bits[k]; k--);
		borders[1] = k + 1;

		// определяем границы единичных областей маскировки
		int n = 2;
		for(int k = k1 + kt + 1; k < K; k++) {
			if(tpl_bits[k] == tpl_bits[k-1])
				continue;
			borders[n++] = k;
			if(n > 2 * p.Nh) 
				break;
		}

		// добавляем границы:
        borders[0] = borders[1] - (borders[3] - borders[2] + 2) / 3;
        borders[n] = borders[n-1] + (borders[n-2] - borders[n-3] + 2) / 3;
        borders[0] = borders[0] < 0 ? 0 : borders[0];
        borders[n] = borders[n] >= K ? K - 1 : borders[n];

        limb_t *mask = tpl_masks + kt * num_sample_limbs;
        bit_vector<limb_t> mask_bits(mask);
        for (int b = borders[0]; b < borders[n]; b++) {
            mask_bits[b] = true;
        }
    }

	r = true;

out:

	spl_free(I1);

    return r;
}

pitch_calculator::pitch_calculator(const freq_scale_t& sc, const mask_params_t& pm, const pitch_params_t& pp) :
    scale(freq_scale_t::copy(sc)), max_diff(DEFAULT_PITCH_MAX_DIFF)
{
    if (!init(pm, pp)) {
        throw "Failed to create pitch_calculator";
    }
}

pitch_calculator::~pitch_calculator() {
    spl_free(memory);
}

template<int NUM_PART_BITS>
inline limb_t get_part(limb_t *limbs, int num_part) {
    throw "Such num_part_bits value is not supported";
}

template<>
inline limb_t get_part<16>(limb_t *s1p, int num_part) {
    uint16_t *x = (uint16_t *)s1p;
    return x[num_part];
}

template<>
inline limb_t get_part<8>(limb_t *s1p, int num_part) {
    uint8_t *x = (uint8_t *)s1p;
    return x[num_part];
}

template<>
inline limb_t get_part<4>(limb_t *s1p, int num_part) {
    uint8_t *x = (uint8_t *)s1p;
    uint8_t two_parts = x[num_part / 2];
    return num_part & 1 ? two_parts >> 4 : two_parts & 0x0F;
}


size_t pitch_calculator::execute(io::istream<mask_t>& in_str, io::ostream<short>& out_str) const
{
	// тип для хранения количества отличий (diff count) между сэмплом и маской
	// для этого достаточно одного байта
	typedef unsigned char diff_t;

	// параметры шаблонов
	const int num_templates = k2 - k1 + 1;  // количество шаблонов

	// количество каналов = количество бит в сэмпле
	const int num_sample_bits = K;

	// количество (неполных) байт в сэмпле
	const int num_sample_bytes = CEIL_MODULUS(num_sample_bits, 8);

	// количество (неполных) чисел в сэмпле
	const int num_sample_limbs = CEIL_MODULUS(num_sample_bytes, sizeof(limb_t));

	// количество кусков в сэмпле
	const int num_sample_parts = CEIL_MODULUS(num_sample_bits, num_part_bits);

	// количество чисел в строке с отличиями
	const int num_diff_limbs = CEIL_MODULUS(num_templates * sizeof(diff_t), sizeof(limb_t));

	// количество отличий в одном числе (их должно быть ровное количество)
	const int num_limb_diffs = sizeof(limb_t) / sizeof(diff_t);

	// diff count в строке (вместе с неиспользуемыми)
	const int num_diffs_all = num_diff_limbs * num_limb_diffs;

	// инициализация таблиц поиска
	// выделяем место под таблицы
	limb_t *mem_tables = spl_alloc<limb_t>(num_sample_parts * num_part_variants * num_diff_limbs
        + num_sample_parts * 2 + num_diff_limbs + num_sample_limbs * 2 + sizeof(mask_t) * K);
    limb_t *buffer = mem_tables;

	// ссылка для быстрого сложения
	Matrix<limb_t,3> sum_tables = matrix_ptr(buffer, num_sample_parts, num_part_variants, num_diff_limbs);
	buffer += sum_tables.size();

	// ссылка для доступа к diff count
	Matrix<diff_t,3> diff_tables = matrix_ptr((diff_t *)sum_tables.ptr(), num_sample_parts, num_part_variants, num_diffs_all);

	// индексы ненулевых элементов для быстрого суммирования
	Matrix<limb_t,2> sum_indices = matrix_ptr(buffer, num_sample_parts, 2);
    buffer += sum_indices.size();

    limb_t *tpl_values = (limb_t *)memory;
    limb_t *tpl_masks = (limb_t *)(memory) + num_sample_limbs * num_templates;


	// заполняем таблицы разностей
	// заполняем индексы суммирования
	for(int i = 0; i < num_sample_parts; i++) {
		limb_t k1 = 0, k2 = 0;
		bool found = false;
		for(int k = 0; k < num_templates; k++) {
			limb_t x = GET_PART(tpl_values + k * num_sample_limbs, i);
			limb_t y = GET_PART(tpl_masks + k * num_sample_limbs, i);

			// таблица разностей:
			for(limb_t j = 0; j < num_part_variants; j++) {
				// нам нужно расстояние хэмминга между i-той частью k-того шаблона и j-тым вариантом i-той части
                limb_t z = j & y;
                diff_tables(i, j, k) = (diff_t)popcount_limb(&x, &z, 1);
            }

			// индексы суммирования
			if(!found) {
				if(y != 0) {
					k1 = k2 = k;
					found = true;
				}
			} else {
				if(y != 0) {
					k2 = k;
				}
			}
		}
		sum_indices(i, 0) = found ? k1 / num_limb_diffs : 0;
		sum_indices(i, 1) = found ? k2 / num_limb_diffs + 1: 0;
	}

	// процедура сравнения с шаблонами
	limb_t *diff_limbs = buffer; buffer += num_diff_limbs;
	limb_t *input_limbs = buffer; buffer += num_sample_limbs;
	limb_t *input2_limbs = buffer; buffer += num_sample_limbs;
	mask_t *input = (mask_t *) buffer;
	diff_t *diffs = (diff_t *) diff_limbs;
	
	const int limb_bits = sizeof(limb_t) * 8;
	bool first = true;
	while(in_str.read(input, K) == K) {

		// конвертируем в биты
		fill_n(input_limbs, num_sample_limbs, 0);
		for(size_t i = 0; i < K; i++) {
			size_t n_byte = i / limb_bits;
			size_t n_bit  = i % limb_bits;
			if(input[i]) {
				input_limbs[n_byte] |= limb_t(1) << n_bit;
			}
		}

        // считаем разницы для всех шаблонов
		if(first) {
			// если все происходит в первый раз,
			// нужно посчитать разницу полностью
			fill_n(diff_limbs, num_diff_limbs, 0);
			for(int i = 0; i < num_sample_parts; i++) {
				limb_t part_value = GET_PART(input_limbs, i);
				limb_t *tpl_diffs = &sum_tables(i, part_value, 0);
				limb_t k1 = sum_indices(i, 0);
				limb_t k2 = sum_indices(i, 1);
				if(k2 > k1) {
                    vec_add_limb(diff_limbs + k1, diff_limbs + k1, tpl_diffs + k1, k2 - k1);
				}
			}

            first = false;

        } else {
			// если все происходит не в первый раз,
			// нужно обновлять только если часть изменилась
			for(int i = 0; i < num_sample_parts; i++) {
				limb_t current_value = GET_PART(input_limbs, i);
				limb_t previous_value = GET_PART(input2_limbs, i);
				limb_t k1 = sum_indices(i, 0);
				limb_t k2 = sum_indices(i, 1);
				if(current_value != previous_value && k2 > k1) {
					// вычитаем предыдущие значения
					{
						const limb_t part_value = previous_value;
						limb_t *tpl_diffs = &sum_tables(i, part_value, 0);
                        vec_sub_limb(diff_limbs + k1, diff_limbs + k1, tpl_diffs + k1, k2 - k1);
					}
					// добавляем текущие значения
					{
						const limb_t part_value = current_value;
						limb_t *tpl_diffs = &sum_tables(i, part_value, 0);
                        vec_add_limb(diff_limbs + k1, diff_limbs + k1, tpl_diffs + k1, k2 - k1);
					}
				}
			}
		}

        // ищем наименьшее отличие от шаблона
		diff_t min_index = -1, min_diff = -1;
		for(int k = 0; k < num_templates; k++) {
			if(diffs[k] < min_diff) {
				min_index = k;
				min_diff = diffs[k];
			}
		}

		// если нас устраивает это отличие, мы выводим номер канала, иначе - 0
		if(min_diff < max_diff) {
			int k = k1 + min_index;
			out_str.put(k);
		} else {
			out_str.put(-1);
		}

		// save old input
		limb_t *tmp = input_limbs;
		input_limbs = input2_limbs;
		input2_limbs = tmp;
	}

	out_str.close(); // закрыть поток (с) Осипов
	spl_free(mem_tables);

	return 0;
}


inline void keep(std::queue<short>& q, io::ostream<short>& o, short c) { q.push(c); }
inline void reset(std::queue<short>& q, io::ostream<short>& o, short c) {
	while(!q.empty()) q.pop(); 
}
inline void flush(std::queue<short>& q, io::ostream<short>& o, short c) {
	while(!q.empty()) {
		o.put(q.front());
		q.pop();
	}
	o.put(c);
}

// функция сегментации сигнала по признаку вокализованности
size_t vocal_segment(
	io::istream<short>& in_str,
	io::ostream<short>& out_str,
	const freq_t F,
	const vocal_params_t& p
) {
	// минимальная длительность вокализованного участка в отсчетах:
	int minV = int(p.minV * F);
	// минимальная длительность невокализованного участка в отсчетах:
	int minNV = int(p.minNV * F);

	bool VocP = false, Voc;
	short kffp = 0, kff;
	int Tnv = 0, Tkv = 0, T = 0;

	size_t written = 0;

	std::queue<short> q;

	enum { out_v, out_nv, keep } action = keep;

	while(in_str.get(kff)) {
		kff = kff + 1; // приводим к шкале 1:K
		Voc = kff != 0;

		// посреди вокализованного сегмента
		if(VocP && Voc) { 
			// должен быть плавный переход между каналами
			if( kff - kffp < 2 && kffp - kff < 2 ) {
				// достигли минимальной длительности вокализованного
				if(T - Tnv == minV 
				// и невокализованный сегмент достаточно большой длительности
				&& Tnv - Tkv > minNV
				){
					// фиксируем границу Tnv

					// нужно сбросить все, что запомнили
					action = out_v;
				}
			// если переход неплавный
			} else {
				// то звук невокализованный
				Voc = false;
			}
		}

		// был невокализованный, стал вокализованный
		if(!VocP && Voc) {
			Tnv = T;
			action = keep;
		}

		// был вокализованный, стал невокализованный
		if(VocP && !Voc) {
			// вокализованный сегмент достаточной длительности
			if(T - Tnv > minV) {
				Tkv = T;
				action = keep;
			// невокализованный сегмент достаточной длительности
			} else if(T - Tkv > minNV) {
				// невокализованный сегмент недостаточной длительности
				if(Tnv - Tkv <= minNV) {
					// фиксируем границу - Tkv
					action = out_nv;
				}
			}
		}

		// посреди невокализованного участка
		if(!VocP && !Voc) {
			// достигли минимальной длительности невокализованного участка
			if(T - Tkv == minNV) {
				// фиксируем границу - Tkv
				action = out_nv;
			}
		}

		switch(action) {
		case out_v: 
			while(!q.empty()) {
				if(out_str.put(q.front() - 1))
					written++;
				q.pop();
			}
			if(out_str.put(kff - 1)) 
				written++;
			break;
		case out_nv:
			while(!q.empty()) {
				if(out_str.put(-1))
					written++;
				q.pop();
			}
			if(out_str.put(-1)) 
				written++;
			break;
		case keep:
			q.push(kff);
			break;
		}

		VocP = Voc;
		kffp = kff;
		T++;
	}

	return written;
}

NAMESPACE_SPL_END;
