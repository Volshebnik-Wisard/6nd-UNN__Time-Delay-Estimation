#include "pch.h"
#define _USE_MATH_DEFINES // Для использования констант, таких как M_PI
#include "signalProccesser.h"
using namespace std;

// Генерация случайной битовой последовательности
std::vector<bool> generate_bits(int nbits, int sample)
{
	vector<bool> result(sample); // Вектор результата (биты для каждого отсчёта)
	vector<bool> bits(nbits); // Вектор исходных битов
	std::random_device rd; // Источник энтропии
	std::mt19937 gen(rd()); // Генератор Mersenne Twister
	std::bernoulli_distribution dist(0.5); // Бернуллиевское распределение (вероятность 0.5)

	// Генерация последовательности исходных битов
	for (int i = 0; i < nbits; i++) {
		bits[i] = dist(gen); // Генерация случайного бита (true/false)
	}
	int sam_bit = ceil((double)sample / nbits); // Количество отсчётов на один бит
	for (int i = 0; i < sample; i++) {
		result[i] = bits[floor((double)i / sam_bit)]; // Распространение бита на несколько отсчётов
	}
	return result;
}

// Рекурсивный алгоритм быстрого преобразования Фурье (FFT)
void fft(std::vector<base>& a, bool invert)
{
	int n = (int)a.size(); // Длина входного вектора
	if (n == 1)  return; // Базовый случай рекурсии

	vector<base> a0(n / 2), a1(n / 2); // Разделение на чётные и нечётные компоненты
	for (int i = 0, j = 0; i < n; i += 2, ++j) {
		a0[j] = a[i]; // Чётные
		a1[j] = a[i + 1]; // Нечётные
	}
	fft(a0, invert); // Рекурсивный вызов для чётных
	fft(a1, invert); // Рекурсивный вызов для нечётных

	double ang = 2 * M_PI / n * (invert ? -1 : 1); // Угловой шаг (обратное или прямое преобразование)
	base w(1), wn(cos(ang), sin(ang)); // Текущий и начальный поворотные множители
	for (int i = 0; i < n / 2; ++i) {
		// Сборка результата по формуле "бабочки"
		a[i] = a0[i] + w * a1[i];
		a[i + n / 2] = a0[i] - w * a1[i];
		if (invert) // Нормализация при обратном преобразовании
			a[i] /= 2, a[i + n / 2] /= 2;
		w *= wn; // Обновление поворотного множителя
	}
}

// Вычисление корреляции через FFT
void correlation(signal s1, signal s2, double step, std::vector<double>& corr, std::vector<double>& t)
{
	fft(s1.s, true); // Прямое преобразование Фурье первого сигнала (инвертированный порядок?)
	fft(s2.s, true); // Прямое преобразование Фурье второго сигнала
	std::vector<base> result(s1.size()); // Вектор для результата перемножения спектров
	for (int i = 0; i < result.size(); i++) {
		result[i] = s1.s[i] * std::conj(s2.s[i]); // Умножение спектра первого на сопряжённый спектр второго
	}
	fft(result, false); // Обратное преобразование Фурье (получаем корреляцию)
	corr.resize(s1.size()); // Изменение размера вектора корреляции
	t.resize(s1.size()); // Изменение размера вектора времени
	int center = corr.size() / 2; // Центральный индекс (нулевая задержка)
	for (int i = 0; i < corr.size(); i++) {
		if (i >= center) {
			// Переупорядочивание для получения корреляции от -max до +max
			corr[i - center] = (result[i] * std::conj(result[i])).real(); // Модуль результата
		}
		else {
			corr[i + center] = (result[i] * std::conj(result[i])).real();
		}
		t[i] = ((double)i - (double)center) * step; // Временной сдвиг в секундах
	}
}

// Прямой метод вычисления корреляции (через суммирование)
std::pair<std::vector<double>, std::vector<double>> correlationSumma(signal s1, signal s2)
{
	if (s1.size() > s2.size()) { // Обеспечение s1 короче или равен s2
		auto s = s1;
		s1 = s2;
		s2 = s;
	}
	int size_shift = abs(s1.size() - s2.size()) + 1; // Количество возможных сдвигов
	std::vector<double> shift(size_shift); // Вектор сдвигов
	base temp; // Временная переменная для комплексного результата
	std::vector<double> res(size_shift); // Вектор значений корреляции

	int size = s1.size(); // Длина более короткого сигнала
	for (int i = 0; i < size_shift; i++) {
		shift[i] = i; // Текущий сдвиг
		temp = base(0, 0); // Инициализация нулём
		for (int j = 0; j < size; j++) {
			temp += s1.s[j] * std::conj(s2.s[i + j]); // Суммирование произведений
		}
		res[i] = temp.real() * temp.real() + temp.imag() * temp.imag(); // Квадрат модуля
	}

	return std::pair<std::vector<double>, std::vector<double>>(res, shift);
}

// Нормализация размеров двух сигналов до ближайшей степени двойки
void normalizeSize(signal& s1, signal& s2)
{
	int size = 0;
	if (s1.size() >= s2.size()) {
		size = s1.size();
	}
	else {
		size = s2.size();
	}
	int pw2 = 1;
	while (size > pow(2, pw2)) { // Поиск степени двойки, большей или равной размеру
		pw2++;
	}
	size = pow(2, pw2); // Новая длина
	s1.resize(size); // Изменение размера первого сигнала
	s2.resize(size); // Изменение размера второго сигнала
}

// Поиск максимума корреляционной функции и вычисление относительной ошибки
std::pair<double, double> findMax(std::vector<double> corr, std::vector<double> t, double delay)
{
	delay /= 1000; // Перевод задержки из мс в секунды
	int indexMax = std::max_element(corr.begin(), corr.end()) - corr.begin(); // Индекс максимума
	double d_delay = 0.; // Относительная ошибка
	if (delay == 0) { // Если заданная задержка равна нулю
		if (t[indexMax] != 0) {
			d_delay = abs(delay - t[indexMax]) / t[indexMax]; // Относительная ошибка
		}
	}
	else {
		d_delay = abs(delay - t[indexMax]) * 100. / delay; // Процентная ошибка
	}
	return std::pair<double, double>(t[indexMax], d_delay); // Возврат оценки задержки и ошибки
}

// Добавление белого гауссовского шума к сигналу
void addNoise(signal s, double snr, signal& s_n)
{
	s_n = s; // Копирование исходного сигнала
	if (s_n.s.empty()) return; // Проверка на пустой сигнал

	std::random_device rd; // Источник энтропии
	std::mt19937 gen(rd()); // Генератор Mersenne Twister
	std::normal_distribution<double> dist(0.0, 1.0); // Нормальное распределение N(0,1)

	// Вычисление мощности сигнала
	double signal_power = 0.;
	for (auto sample : s_n.s) {
		signal_power += std::norm(sample); // Сумма квадратов модулей
	}
	signal_power /= s_n.N; // Средняя мощность

	double snr_linear = std::pow(10.0, snr / 10.0); // Перевод SNR из дБ в линейный масштаб
	double noise_power = signal_power / snr_linear; // Мощность шума
	double noise_stddev = std::sqrt(noise_power / 2.0); // Стандартное отклонение для каждой компоненты

	// Добавление шума
	for (int i = 0; i < s_n.N; i++) {
		s_n.s[i] += base(dist(gen) * noise_stddev,
			dist(gen) * noise_stddev); // Добавление шума к действительной и мнимой частям
	}
}

// Установка параметров модуляции
void modulation::setParam(double _fd, int _nbits, double _bitrate, double _fc, double _delay, double _sample_base, type_modulation _type)
{
	fd = _fd; // Частота дискретизации
	nbits = _nbits; // Количество бит
	bitrate = _bitrate; // Битовая скорость
	fc = _fc * 1000; // Перевод несущей частоты из кГц в Гц
	delay = ceil(_delay * fd); // Задержка в отсчётах (перевод из мс)
	sample_base = (double)_sample_base * fd; // Длительность базового сигнала в отсчётах
	type = _type; // Тип модуляции
	duration = nbits / bitrate; // Длительность сигнала в секундах
	if (sample != duration * fd * 1000) { // Если изменилось количество отсчётов
		sample = duration * fd * 1000; // Обновление количества отсчётов (перевод из МГц в кГц?)
		generate_t(); // Генерация вектора времени
		s.resize(sample); // Изменение размера сигнала
	}
}

// Генерация вектора времени
void modulation::generate_t()
{
	t.resize(sample); // Изменение размера вектора времени
	for (int i = 0; i < sample; i++) {
		t[i] = i / (fd * 1000); // Время в секундах (частота дискретизации в кГц)
	}
}

// Реализация амплитудной модуляции (двоичной)
void modulation::modAM()
{
	for (int i = 0; i < sample; i++) {
		s.I[i] = (1 + (bits[i] ? 1 : 0)) / 2.; // 1 для бита 1, 0.5 для бита 0
		s.Q[i] = 0.0; // Квадратурная компонента отсутствует
	}
}

// Реализация фазовой модуляции (двоичной)
void modulation::modPM2()
{
	for (int i = 0; i < sample; i++) {
		s.I[i] = (bits[i] == 1) ? 1.0 : -1.0; // +1 для бита 1, -1 для бита 0
		s.Q[i] = 0.0;
	}
}

// Реализация частотной модуляции (двоичной)
void modulation::modFM2()
{
	double accumulated_phase = 0.; // Накопленная фаза
	double df = bitrate / 4; // Девиация частоты
	for (int i = 0; i < sample; i++) {
		accumulated_phase += (bits[i] == 1 ? df : -df) / (fd * 1000); // Изменение фазы в зависимости от бита
		s.I[i] = cos(accumulated_phase); // In-phase компонента
		s.Q[i] = sin(accumulated_phase); // Quadrature компонента
	}
}

// Получение сигнала
signal modulation::getS()
{
	return s;
}

// Получение вектора времени
std::vector<double> modulation::getT()
{
	return t;
}

// Получение длительности сигнала
double modulation::getDuration()
{
	return duration;
}

// Преобразование битового вектора в вектор double (1.0 для true, 0.0 для false)
std::vector<double> modulation::getBits()
{
	std::vector<double> double_vec(bits.size());

	for (size_t i = 0; i < bits.size(); ++i) {
		double_vec[i] = bits[i] ? 1.0 : 0.0;
	}
	return double_vec;
}

// Выполнение полной процедуры модуляции
void modulation::manipulation()
{
	bits = generate_bits(nbits, sample); // Генерация битов
	switch (type) // Выбор типа модуляции
	{
	case AM:
		modAM();
		break;
	case PM:
		modPM2();
		break;
	case FM:
		modFM2();
		break;
	default:
		break;
	}
	base temp;
	double phase = 0;
	for (int i = 0; i < sample; i++) {
		phase += 2. * M_PI * fc / (fd * 1000); // Накопление фазы несущей
		temp = base(cos(phase), sin(phase)); // Комплексная экспонента
		s.s[i] = base(s.I[i], s.Q[i]) * temp; // Перенос на несущую частоту
	}
}

// Установка типа модуляции
void modulation::setType(type_modulation _type)
{
	type = _type;
}

// Создание базового сигнала (вырезание части из полного сигнала)
signal modulation::createBaseSignal()
{
	signal result = s; // Копирование полного сигнала
	result.erase(delay, sample_base); // Вырезание базового сигнала
	s.N = sample; // Установка количества отсчётов для полного сигнала
	result.N = sample_base; // Установка количества отсчётов для базового сигнала
	normalizeSize(result, s); // Нормализация размеров для FFT
	return result;
}