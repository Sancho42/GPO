///
/// \file  spectrum.cpp
/// \brief Модуль выделения спектра звукового сигнала ("фильтрации").
///
/// Выделение спектра реализовано через систему нерекурсивных фильтров. 
/// Модуль содержит функционал для генерации параметров данных фильтров и собственно для фильтрации (свертки).
///

#include "matrix.h"
#include "model.h"
#include "spl_types.h"
#include "spectrum.h"
#include "conv.h"
#include "scale.h"

#include "../io/iofile.h"
#include "../io/iowrap.h"

#define _USE_MATH_DEFINES // M_PI, etc
#include <math.h>
#include <algorithm>

#define _SCL_SECURE_NO_WARNINGS

NAMESPACE_SPL_BEGIN;

using namespace io;

///
/// Обертка, продлевающая поток сигнала на \a N нулевых отсчетов
///

template<typename T>
class iwstream_extend : public iwrap<T> {
public:

    iwstream_extend(istream<T>& str, size_t N) :
        iwrap<T>(str), _N(N), _pos(0) {}

    //@{
    /// Стандартные функции abstract_stream.
    virtual bool eos() const { return _pos >= _N; }
    virtual size_t pos() const { return _understream->pos() + _pos; }
    virtual size_t pos(size_t newpos) { return 0; } // пока не реализован за ненадобностью
    //@}

    /// Получение элемента 
    virtual bool get(T& x) {
        if (_understream->eos()) {
            if (!eos()) {
                x = 0;
                _pos++;
            }
            else {
                return false;
            }
        }
        else {
            _understream->get(x);
        }
        return true;
    }

    /// Получение строки элементов
    virtual size_t read(T *buf, size_t count) {
        size_t r1 = _understream->read(buf, count);
        size_t r2 = 0;
        if (r1 < count) {
            r2 = std::min(count - r1, _N - _pos);
            std::fill(buf + r1, buf + r1 + r2, 0);
            _pos += r2;
        }
        return r1 + r2;
    }

private:
    size_t _pos, _N;

};

/// 
/// Генерация коэффициентов фильтрации по шкале резонансных частот фильтров.
/// В структуре фильтров \a f уже должно быть выделено место для помещения коэффициентов фильтрации.
/// 

bool spectrum_calculator::init(
	const freq_scale_t& s,        ///< шкала резонансных частот фильтров
    freq_t F,                     ///< частота дискретизации сигнала
    double ksi                    ///< допустимая ошибка вычислений (определяет размер окна фильтров)
) {
	double array[CONV_WIN_SIZ * 2 + 2];
	double *Hc = SPL_MEMORY_ALIGN(array);
	double *Hs = Hc + CONV_WIN_SIZ;

	if(K != s.size()) return false;

	// вычисляем Ws - размер реального окна фильтров
	// выбирается как максимальное из вычисленных для каждого канала
	int Ws = 0;
	for(int k = 0; k < K; k++) {
		const double std = model::filter_std(s[k], F);
		int Wsk = (int) model::gauss_border(std, ksi);
		if(Wsk > Ws) Ws = Wsk; // на каком канале это произошло, нам даже не важно
	}
	this->Ws = 2*Ws+1;
	
	// матрица - для удобного доступа к коэффициентам фильтрации
	Matrix<double, 3> HM = matrix_ptr(H, 2, K, CONV_WIN_SIZ);

	// вычисляем собственно коэффициенты фильтра
	// для размера окна Ws
	for(int k = 0; k < K; k++) {
		const double std = model::filter_std(s[k], F);
		const double Wf = 2 * M_PI * s[k] / F;
        int j = 0;
		for(int n = -Ws; n <= Ws; n++) {
			double norm = model::gauss_win<double>(n, 0.0, std); // окно Гаусса нормирует синусоиды
			Hc[j] = norm * cos(Wf * n); // действительная часть
			Hs[j] = norm * sin(Wf * n); // мнимая часть
            j++;
		}
		// заполняем оставшиеся коэффициенты нулями
		std::fill(Hc + j, Hc + CONV_WIN_SIZ, 0);
		std::fill(Hs + j, Hs + CONV_WIN_SIZ, 0);
		// предварительное вычисление вектора BC
		cconv_calc_BC(Hc, Hs, &HM(0,k,0), &HM(1,k,0));
	}
	// нормализация коэффициентов фильтрации:
	cconv_normalize(H, HM.size());
	return true;
}

spectrum_calculator::spectrum_calculator(const freq_scale_t& s, freq_t F, double ksi)
{
    K = s.size();
    H = conv_alloc<double>(K * 2 * CONV_WIN_SIZ);
    if (H == 0)
        throw "Can't allocate memory for spectrum filters coefficients";
    if (!init(s, F, ksi))
        throw "Error while generating spectrum filters";
}

spectrum_calculator::~spectrum_calculator() {
    conv_free(H);
}

bool spectrum_calculator::save(const char *file) {
    return array_to_file(H, K * CONV_WIN_SIZ * 2, file);
}

size_t spectrum_calculator::execute(istream<signal_t>& signal, ostream<spectrum_t>& spectrum) const 
{
	int Ws = this->Ws - 1; // можно брать на 1 меньше, чем окно - результат не меняется
	int Os = CONV_WIN_SIZ - Ws;
	spectrum_t *out_buf = NULL;

	// обеспечиваем отсутствие смещения в начале сигнала
	iwstream_extend<signal_t> signal_ext(signal, Ws/2);

	// сколько записано - выходная величина
	size_t written = 0;

	// буфер входного сигнала - состоит из двух частей:
	// CONV_WIN_SIZ = Ws + Os, 
	// где Ws - Window size - размер реального окна фильтра, 
	//     CONV_WIN_SIZ - размер вычисляемой циклической свертки
	//     Os - Output size - размер полезного выхода свертки
	// также используется как входной буфер свертки
	double *conv_in_buf = conv_alloc<double>(5 * CONV_WIN_SIZ + 2);

	// выходной буфер свертки
	// имеет такую же структуру как и входной буфер (2*Ws+1) + Os
	// только полезный выход - последние Os элементов - идут на выход
	double *tmp_buf1 = conv_in_buf + CONV_WIN_SIZ;
	double *tmp_buf2 = tmp_buf1 + CONV_WIN_SIZ;
	double *tmp_buf3 = tmp_buf2 + CONV_WIN_SIZ;
	double *tmp_buf4 = tmp_buf3 + CONV_WIN_SIZ;

	// буфер выходного сигнала
	// матрицы размера K x Os - для помещения результата свертки
	// и Os x K - для вывода наружу
	out_buf = spl_alloc<spectrum_t>(2 * K * Os);

	Matrix<spectrum_t,2> out_mtx1 = matrix_ptr(out_buf, K, Os);
	Matrix<spectrum_t,2> out_mtx2 = matrix_ptr(out_buf+K*Os, Os, K);

	// матрица - для удобного доступа к коэффициентам фильтрации
	Matrix<double, 3> H = matrix_ptr(this->H, 2, K, CONV_WIN_SIZ);

	if(!out_buf) 
		goto end; 

	//
	// подготовка структур данных
	//

	// очищаем последние Ws элементов входного буфера
	std::fill(conv_in_buf+CONV_WIN_SIZ-Ws, conv_in_buf+CONV_WIN_SIZ-Ws/2, 0);

	// обеспечиваем отсутствие смещения в начале сигнала
	signal_ext.read(conv_in_buf+CONV_WIN_SIZ-Ws/2, Ws/2);

	// основной цикл фильтрации
	while(!signal_ext.eos() && !spectrum.eos()) {

		// копируем последние Ws элементов сигнала в начало
		std::copy(conv_in_buf+CONV_WIN_SIZ-Ws, conv_in_buf+CONV_WIN_SIZ, conv_in_buf);

		// вводим Os новых отсчетов сигнала
		// этот буфер будет использоваться неизменно для каждого канала
		// получаем rOs - real output size - количество считанных элементов
		// в общем случае rOs == Os, отличия могут быть только в конце сигнала
		size_t rOs = signal_ext.read(conv_in_buf + Ws, Os);

		// если считано меньше, чем Os элементов,
		//  то будет последний виток цикла
		// очищаем последние отсчеты сигнала, 
		// и заменяем матрицы на матрицы меньшего размера (K x rOs)
		if(rOs != Os) {
			std::fill(conv_in_buf + Ws + rOs, conv_in_buf + CONV_WIN_SIZ, 0);
			out_mtx1 = matrix_ptr(out_buf, K, rOs);
			out_mtx2 = matrix_ptr(out_buf+K*rOs, rOs, K);
		}

		spectrum_t *out_ptr = out_mtx1;

		cconv_calc_A(conv_in_buf, tmp_buf1, tmp_buf2);

		// цикл по каналам
		for(int k = 0; k < K; k++) {

			// свертка
			cconv(tmp_buf1, tmp_buf2, &H(0,k,0), &H(1,k,0), tmp_buf3, tmp_buf4);

			// вычисление модуля комплексных чисел
			complex_abs_split(rOs, tmp_buf3 + Ws, tmp_buf4 + Ws, tmp_buf4 + Ws);

			// пишем выход в выходную матрицу - только из интервала [Ws, Ws+rOs]
			out_ptr = std::copy(tmp_buf4 + Ws, tmp_buf4 + Ws + rOs, out_ptr);

		}

		// транспонируем выходную матрицу
		// из K x rOs в rOs x K
		transpose(out_mtx1, out_mtx2);

		// выводим матрицу out_mtx2
		written += spectrum.write(out_mtx2, out_mtx2.size());

	}

end:
	// закрываем поток
	spectrum.close();
	// удаляем выделенные буферы
	conv_free(conv_in_buf);
	spl_free(out_buf);

	return written;
}


NAMESPACE_SPL_END;
