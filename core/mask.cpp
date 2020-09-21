///
/// \file  mask.cpp
/// \brief Функции для расчета одновременной маскировки (\ref mask).
///
/// Модуль содержит функционал для генерации фильтров маскировки (функция mask_filters_generate())
///  и собственно для маскировки (свертки) - функции mask_stream(), mask_memory(), mask_binary_file().
///

#include "mask.h"
#include "matrix.h"
#include "model.h"
#include "scale.h"
#include "conv.h"
#include <math.h>
#include <stdio.h>

#include <algorithm>

#include "../io/iobit.h"
#include "../io/iomem.h"
#include "../io/iofile.h"
using io::istream;
using io::ostream;

NAMESPACE_SPL_BEGIN;

///
/// Препроцессинг потока: расширение блоков данных.
///
/// Это позволяет реализовывать потоковую одновременную маскировку
///  за счет расширения области спектра в обе стороны на половину размера окна.
///

template<typename T>
class istream_block_extend : public io::iwrap<T>
{
public:
    /// Конструктор.
    /// Принимает оборачиваемый поток, размер блока и размер расширений.
    istream_block_extend(io::istream<T>& str, int _K, int _W) :
        io::iwrap<T>(str), K(_K), W(_W), _pos(0), _maxpos(0) {
        // ставя _maxpos = _pos = 0, мы заставляем загрузить первый блок
        _buf = spl_alloc<T>(W + K + W);
    }

    /// Деструктор.
    /// Освобождает выделенную память
    ~istream_block_extend() { spl_free(_buf); }

    //@{
    /// Стандартные функции abstract_stream.
    virtual bool eos() const { return _pos >= _maxpos && _understream->eos(); }
    virtual size_t pos() const { return _understream->pos() / K * (W + K + W) + _pos; }
    virtual size_t pos(size_t newsize) { return 0; } // пока не реализован за ненадобностью
                                                        //@}

                                                        /// Получение элемента.
    virtual bool get(T& x) {
        if (_pos >= _maxpos) {
            if (!read_block()) return false;
        }
        x = _buf[_pos++];
        return true;
    }
    //*
    /// Получение строки элементов.
    /// Реализация функции istream::read()
    virtual size_t read(T *buf, size_t count) {
        size_t i = 0;
        // пока есть куда читать
        while (i < count) {
            // если элементы кончились - грузим следующий блок
            if (_pos >= _maxpos) {
                if (!read_block()) break; // если элементы и там кончились - выходим
            }
            // копируем минимальное из: оставшегося в _buf и оставшегося в buf
            size_t N = std::min(_maxpos - _pos, count - i);
            size_t r = std::copy(_buf + _pos, _buf + _pos + N, buf + i) - buf - i;
            _pos += r; i += r;
        }
        return i; // возвращаем количество прочтенных символов
    }
    //*/
private:

    bool read_block() {
        // читаем блок: K элементов - запоминаем, сколько прочиталось
        _maxpos = _understream->read(_buf + W, K);
        // если ничего не прочиталось - конец файла
        if (_maxpos == 0) return false;
        // прочиталось _maxpos элементов, делаем расширение области:
        std::fill(_buf, _buf + W, _buf[W]);
        T *buf2 = _buf + W + _maxpos;
        std::fill(buf2, buf2 + W, buf2[-1]);
        _pos = 0;
        _maxpos = W + _maxpos + W;
        return true;
    }

    /// Размер блока.
    const int K;
    /// Размер расширения.
    const int W;
    /// Внутренний буфер с расширенной областью.
    T *_buf;
    /// Позиция в буфере.
    size_t _pos;
    /// Максимальная позиция в буфере.
    size_t _maxpos;
};

///
/// Постпроцессинг потока: отбрасывание отрезков.
/// Используется в быстром алгоритме одновременной маскировки
///  для перехода от расширенной шкалы частот к обычной.
///

template<typename T>
class ostream_block_reduce : public io::owrap<T>
{
public:
    /// Конструктор.
    /// Принимает выходной поток, размер блока и размер расширений.
    ostream_block_reduce(io::ostream<T>& str, size_t _K, size_t _W) :
        io::owrap<T>(str), K(_K), W(_W), rel_pos(0) {}

    /// Вывести элемент.
    bool put(const T& x) {
        bool r = true;
        if (rel_pos >= W && rel_pos < W + K) {
            r = _understream->put(x);
        }
        rel_pos = (rel_pos + 1) % (W + K + W);
        return r;
    }
    //*
    /// Вывести строку элементов.
    size_t write(const T *buf, size_t count) {
        size_t i = 0, j = 0;
        while (i < count) {
            // если позиция не там, где нужно
            size_t N;
            if (rel_pos < W) {
                N = std::min(W - rel_pos, count - i);
            }
            else if (rel_pos >= K + W) {
                N = std::min(W + K + W + W - rel_pos, count - i);
            }
            else {
                N = 0;
            }
            i += N;
            rel_pos = (rel_pos + N) % (W + K + W);
            if (i >= count) break;
            N = std::min(W + K - rel_pos, count - i);
            size_t M = _understream->write(buf + i, N);
            i += M; j += M;
            // если вывелось меньше, чем нужно - выходим
            if (M < N) break;
            rel_pos += M;
        }
        return j; // вернуть число записанных на выход элементов.
    }
    //*/
private:
    size_t rel_pos;
    /// Размер блока.
    const size_t K;
    /// Размер расширения.
    const size_t W;
};




/// Функция расчета окна фильтров маскировки.
/// Применяется для предварительного расчета - перед выделением памяти.

static int mask_window_size(const freq_scale_t& s, const mask_params_t& p) {
	// выбираем наибольшее из окон
	int Ws = 0;

	double Nmin, Nmax;
	for(int k = 0; k < s.size(); k++) {
		double std = model::mask_std(s[k], p.delta);
		double dF = model::gauss_border(std, p.ksi);
		if(p.border_effect) { 
			Nmin = s.interpolate_index(s[k] - dF); // если краевой эффект учитывается, 
			Nmax = s.interpolate_index(s[k] + dF);	// то используется экстраполяция частот
		} else {
			Nmin = s.get_index(s[k] - dF); // если краевой эффект не учитывается, 
			Nmax = s.get_index(s[k] + dF); // то используется простой поиск по частотам
		}
		if(Nmax - Nmin + 1 > Ws) Ws = (int) ceil(Nmax - Nmin + 1);
	}
	return Ws % 2 ? Ws : Ws + 1; // должно быть обязательно нечетное число - чтобы был интервал [-Ws;+Ws]
}

/// Функция генерации маскирующей функции.
static void mask_win(
	const freq_scale_t& s, // массив резонансных частот фильтров
	double *H,           // выходной массив - коэффициенты маскирующей функции
	int k, 
	int Ws, 
	const mask_params_t& p
) {
	size_t K = s.size();
	double *H2 = H + Ws;

	// стандартное отклонение (СКО) функции Гаусса
	double std = model::mask_std(s[k], p.delta);
	// посчитать коэффициенты
	for(int m = -Ws; m <= Ws; m++) {
		double Hkm;
		if(k + m >= 0 && k + m < K) { // точка k + m есть в шкале частот?
			Hkm = model::gauss_win(s[k+m], s[k], std); // да - берем из шкалы частот
		} else if(p.border_effect) { // если учитывается краевой эффект, 
			double Fm = s.interpolate_freq(k+m); // то делаем экстраполяцию
			Hkm = model::gauss_win(Fm, s[k], std);
		} else { // если краевой эффект не учитывается,
			Hkm = 0; // то просто ставим 0 в маскирующую функцию
		}
		H2[m] = Hkm;
	}

	// нормирование
	double sum = 0;
	for(int m = 2*Ws; m >= 0; m--) // цикл от 0 до 2*Ws ВКЛЮЧИТЕЛЬНО, т.к. кол-во элементов = 2*Ws+1
		sum += H[m];
	sum *= p.rho;
	for(int m = 2*Ws; m >= 0; m--)
		H[m] /= sum;
}


/// 
/// Вычисление коэффициентов одновременной маскировки по шкале резонансных частот фильтров.
/// В структуре фильтров \a f уже должно быть выделено место (Ws*K) для помещения коэффициентов фильтрации.
///
/// Позволяет сгенерировать коэффициенты одновременной маскировки с учетом или без учета краевого эффекта.
/// Поскольку коэффициенты одновременной маскировки рассчитываются 
///  для каждого канала шкалы частот \a s с учетом частот соседних каналов, 
///  то рассчитать коэффициенты на крайних (первых и последних) каналах шкалы \a s 
///  не представляется возможным. 
/// Для расчета таких частот производится интерполяция шкалы частот \a s с помощью функции scale_interp_k2f().
/// Для того чтобы интерполяция была возможна, предварительно угадывается 
///  форма шкалы (\ref scale_form) с помощью функции scale_form_estimate().
/// Поэтому для возможности использовать одновременную маскировку с учетом краевого эффекта, 
///  необходимо использовать шкалы частот известной формы (см. \ref scale_form).
///
/// При использовании шкалы частот неизвестной формы 
///  можно генерировать только коэффициенты без учета краевого эффекта.
///
/// Учет краевого эффекта регулируется параметром \a p.border_effect.
///

bool freq_mask_calculator::init(const freq_scale_t& s, const mask_params_t& p) 
{
    K = s.size();
    Ws = mask_window_size(s, p);
    size_t N = K * Ws;
    H = conv_alloc<double>(N);
    if (H == 0) 
        throw "Can't allocate memory for mask filters coefficients";

	// посчитать коэффициенты
	for(int k = 0; k < K; k++) {
		mask_win(s, H + k * Ws, k, Ws/2, p);
	}

	return true;
}

///
/// Вычисление коэффициентов одновременной маскировки, 
///  оптимизированных для быстрого вычисления.
///
/// Быстрое вычисление одновременной маскировки возможно, 
///  если шкала частот имеет форму model (см. \ref scale_form_t).
/// В этом случае коэффициенты одновременной маскировки на всех частотных каналах одинаковы, 
///  что позволяет свести одновременную маскировку к цифровой фильтрации и 
///  вычислять ее по теореме о свертке.
///
/// В данной функции производится предварительное приготовление к быстрому вычислению:
///  вычисляется Фурье (вектор А) от маскирующей функции.
/// 
/// В структуре фильтров \a f уже должно быть выделено место (2*CONV_WIN_SIZ) 
///  для помещения коэффициентов фильтрации.
/// 

bool freq_mask_calculator_fast::init(
	const freq_scale_t& s, 
	const mask_params_t& p) 
{
    K = s.size();
    scale_form_t sc_form = s.get_form(true, K - 2);

    if (sc_form != scale_form_t::model)
        throw "Only model scale form is supported in freq_mask_calculator_fast";

    Ws = mask_window_size(s, p);
    size_t N = 2 * CONV_WIN_SIZ;
    H = conv_alloc<double>(N);
    if (H == 0)
        throw "Can't allocate memory for mask filters coefficients";

    double tmp[CONV_WIN_SIZ];
	// вычисляет маскирующую функцию для k = K/2
	// выбор конкретного k на самом деле неважен
	mask_win(s, tmp, K/2, Ws/2, p); // заполняет первые Ws/2 байт
	// меняем направление - т.к. свертка поменяет его еще раз ;)
	std::reverse(tmp, tmp + Ws);
	// заполняем остаток нулями
	std::fill(tmp + Ws, tmp + CONV_WIN_SIZ, 0.0);
	// предвычисление вектора A
	cconv_calc_A(tmp, H, H + CONV_WIN_SIZ);
	// нормировка вектора А
	cconv_normalize(H, 2 * CONV_WIN_SIZ);

	return true;
}


freq_mask_calculator::freq_mask_calculator(const freq_scale_t& s, double ksi)
{
    mask_params_t p = mask_params_t::DEFAULT;
    p.ksi = ksi;
    if (!init(s, p))
        throw "Error while generating mask filters";
}

freq_mask_calculator::freq_mask_calculator(const freq_scale_t& s, const mask_params_t& p)
{
    if (!init(s, p))
        throw "Error while generating mask filters";
}

freq_mask_calculator::~freq_mask_calculator()
{
    conv_free(H);
}

freq_mask_calculator_fast::freq_mask_calculator_fast(const freq_scale_t& s, double ksi)
{
    mask_params_t p = mask_params_t::DEFAULT;
    p.ksi = ksi;
    if (!init(s, p))
        throw "Error while generating mask filters";
}

freq_mask_calculator_fast::freq_mask_calculator_fast(const freq_scale_t& s, const mask_params_t& p)
{
    if (!init(s, p))
        throw "Error while generating mask filters";
}

freq_mask_calculator_fast::~freq_mask_calculator_fast()
{
    conv_free(H);
}


///
/// Потоковая одновременная маскировка.
/// Возвращает количество записанных на выход элементов.
///
/// Является наиболее общей функцией для вычисления одновременной маскировки, 
///  воспринимающей вход и выход как абстрактные потоки.
/// Более детальная реализация выбирает конкретную реализацию потоков 
///  (память, файлы, микрофон, колонки и т.п.),
///  инициализирует входные структуры фильтров и потоков 
///  и вызывает данную функцию.
/// Это обеспечивает единство вычислений, вне зависимости от используемых типов потоков.
/// 
/// Функция работает по наивному алгоритму, без оптимизаций.
///

size_t freq_mask_calculator::execute(istream<spectrum_t>& spectrum, ostream<mask_t>& mask) const 
{
    int Ws2 = Ws / 2;
	spectrum_t *spec_wide1 = spl_alloc<spectrum_t>(Ws2 + K + Ws2);
	spectrum_t *spec_input = spec_wide1 + Ws2;
	spectrum_t *spec_wide2 = spec_input + K;
	if(!spec_wide1) return 0;

	size_t written = 0;

	// основной цикл маскировки
	while(spectrum.read(spec_input, K) == K && !mask.eos()) {

		// расширение области частот спектра
		std::fill(spec_wide1, spec_wide1 + Ws2, spec_input[0]);
		std::fill(spec_wide2, spec_wide2 + Ws2, spec_input[K-1]);

		// цикл расчета маскировки:
		double *pH = H;
		for(size_t k = 0; k < K; k++) {
			double sum = 0;
			for(int m = -Ws2; m <= Ws2; m++) {
				sum += spec_input[k+m] * (*pH++);
			}
			written += mask.put(spec_input[k] > sum);
		}
	}

	spl_free(spec_wide1);
	return written;
}


///
/// Потоковая одновременная маскировка - быстрый вариант.
/// Вызывается только для модельной шкалы частот.
///

size_t freq_mask_calculator_fast::execute(istream<spectrum_t>& spectrum, ostream<mask_t>& mask) const
{
    int Ws2 = Ws/2;
	size_t Os = CONV_WIN_SIZ - Ws + 1;

	// Два буфера для чтения спектра
	spectrum_t *input_buf1 = conv_alloc<spectrum_t>(CONV_WIN_SIZ * 6 + 2);
	spectrum_t *input_buf2 = input_buf1 + CONV_WIN_SIZ;
	// Четыре временных буфера - B, C, abcr, abci
	spectrum_t *tmp_buf1 = input_buf2 + CONV_WIN_SIZ;
	spectrum_t *tmp_buf2 = tmp_buf1 + CONV_WIN_SIZ;
	spectrum_t *tmp_buf3 = tmp_buf2 + CONV_WIN_SIZ;
	spectrum_t *tmp_buf4 = tmp_buf3 + CONV_WIN_SIZ;
	// один выходной буфер
	mask_t *out_buf = spl_alloc<mask_t>(CONV_WIN_SIZ * 2);

	// i/o wrappers для расширения/сужения шкалы частот:
    istream_block_extend<spectrum_t> spectrum_ext(spectrum, K, Ws);
    ostream_block_reduce<mask_t> mask_ext(mask, K, Ws);

	// выходная величина
	size_t written = 0;

	// заполняем конец второго буфера, чтобы потом оттуда перенеслось в начало первого
	std::fill(input_buf2 + Os, input_buf2 + Os + Ws2, 0);
	spectrum_ext.read(input_buf2 + Os + Ws2, Ws2);

	size_t N1 = 0, N2 = 0;

	// основной цикл маскировки
	while(!spectrum_ext.eos() && !mask_ext.eos() || N2 > Os - Ws2) { // последний блок обрабатывается только на следующей итерации

		// копируем из конца второго буфера в начало первого
		std::copy(input_buf2 + Os, input_buf2 + CONV_WIN_SIZ, input_buf1);
		// читаем первый буфер 
		N1 = spectrum_ext.read(input_buf1 + 2*Ws2, Os);
		// если прочиталось меньше, чем нужно - остаток заполняем нулями
		if(N1 < Os) {
			std::fill(input_buf1 + 2*Ws2 + N1, input_buf1 + CONV_WIN_SIZ, 0);
		}

		// копируем из конца первого буфера в начало второго
		std::copy(input_buf1 + Os, input_buf1 + CONV_WIN_SIZ, input_buf2);
		// читаем второй буфер
		N2 = spectrum_ext.read(input_buf2 + 2*Ws2, Os);
		// если прочиталось меньше, чем нужно - остаток заполняем нулями
		if(N2 < Os) {
			std::fill(input_buf2 + 2*Ws2 + N2, input_buf2 + CONV_WIN_SIZ, 0);
		}

		// считаем B, C
		cconv_calc_BC(input_buf1, input_buf2, tmp_buf1, tmp_buf2);

		// свертка
		cconv(H, H + CONV_WIN_SIZ, tmp_buf1, tmp_buf2, tmp_buf3, tmp_buf4);

		int j = 0;
		// вычисляем результат маскировки для обоих буферов:
		for(size_t i = Ws2; i < Ws2 + N1; i++) {
			out_buf[j++] = (input_buf1[i] > tmp_buf3[i + Ws2]);
		}
		for(size_t i = Ws2; i < Ws2 + N2; i++) {
			out_buf[j++] = (input_buf2[i] > tmp_buf4[i + Ws2]);
		}

		// выводим результат
		written += mask_ext.write(out_buf, j);
	}
	// закрываем поток
	mask.close(); 
	// удаляем выделенные буферы
	conv_free(input_buf1);
	spl_free(out_buf);

	return written;
}

size_t mask_memory(const freq_scale_t& scale, size_t N, const spectrum_t *spectrum, mask_t *mask, const mask_params_t& p)
{
    size_t K = scale.size();
    io::imstream<spectrum_t> input(spectrum, N * K);
    io::omstream<mask_t> output(mask, N * K);
    if (scale.get_form(true) == scale_form_t::model) {
        freq_mask_calculator_fast calc(scale, p);
        return calc.execute(input, output);
    }
    else {
        freq_mask_calculator calc(scale, p);
        return calc.execute(input, output);
    }
}

NAMESPACE_SPL_END;