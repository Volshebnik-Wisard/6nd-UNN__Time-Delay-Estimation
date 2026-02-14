#pragma once 
#include <vector>
#include <fstream>
#include <cmath>
#include <complex>
#include <algorithm>
#include <random>
#include <numeric>

typedef std::complex<double> base; // Определение псевдонима для комплексного числа

// Перечисление типов модуляции
enum type_modulation {
    AM, // Амплитудная модуляция
    PM, // Фазовая модуляция
    FM  // Частотная модуляция
};

// Структура для представления сигнала
struct signal {
    int N; // Количество отсчётов
    std::vector<double> I; // In-phase компонента (действительная часть)
    std::vector<double> Q; // Quadrature компонента (мнимая часть)
    std::vector<base> s; // Комплексный сигнал

    void resize(int n) { // Метод изменения размера всех векторов сигнала
        I.resize(n);
        Q.resize(n);
        s.resize(n);
    }

    void erase(int delay, int sample) { // Метод для извлечения части сигнала (базового сигнала)
        I = std::vector<double>(I.begin() + delay, I.begin() + delay + sample);
        Q = std::vector<double>(Q.begin() + delay, Q.begin() + delay + sample);
        s = std::vector<base>(s.begin() + delay, s.begin() + delay + sample);
    }

    int size() { // Метод получения размера сигнала
        return I.size();
    }

    std::vector<double> getSreal() { // Метод получения действительной части комплексного сигнала
        std::vector<double> res(s.size());
        for (int i = 0; i < res.size(); i++) {
            res[i] = s[i].real();
        }
        return res;
    }

    std::vector<double> getSimag() { // Метод получения мнимой части комплексного сигнала
        std::vector<double> res(s.size());
        for (int i = 0; i < res.size(); i++) {
            res[i] = s[i].imag();
        }
        return res;
    }
};

// Объявления функций обработки сигналов:

// Генерация случайной битовой последовательности
std::vector<bool> generate_bits(int nbits, int sample);

// Быстрое преобразование Фурье (FFT)
void fft(std::vector<base>& a, bool invert);

// Корреляция двух сигналов через FFT
void correlation(signal s1, signal s2, double step, std::vector<double>& corr, std::vector<double>& t);

// Корреляция через прямое суммирование (для небольших сдвигов)
std::pair<std::vector<double>, std::vector<double>> correlationSumma(signal s1, signal s2);

// Нормализация размеров двух сигналов до степени двойки (для FFT)
void normalizeSize(signal& s1, signal& s2);

// Поиск максимума корреляционной функции и вычисление ошибки оценки задержки
std::pair<double, double> findMax(std::vector<double> corr, std::vector<double> t, double delay);

// Добавление белого гауссовского шума к сигналу с заданным SNR
void addNoise(signal s, double snr, signal& s_n);

// Класс модуляции
class modulation {
    std::vector<double> t; // Вектор времени
    signal s; // Сигнал
    std::vector<bool> bits; // Биты исходного сообщения
    double fd; // Частота дискретизации (в МГц)
    int nbits; // Количество бит в последовательности
    int sample_base; // Количество отсчётов базового сигнала
    double bitrate; // Битовая скорость (в бит/с)
    double fc; // Несущая частота (в Гц)
    int delay; // Задержка в отсчётах
    double snr; // SNR (не используется?)
    type_modulation type; // Тип модуляции
    double duration; // Длительность сигнала (в секундах)
    int sample; // Общее количество отсчётов сигнала

public:
    // Установка параметров модуляции
    void setParam(double _fd, int _nbits, double _bitrate, double _fc, double _delay, double _sample_base, type_modulation _type);

    // Генерация вектора времени
    void generate_t();

    // Амплитудная модуляция
    void modAM();

    // Фазовая модуляция (двоичная)
    void modPM2();

    // Частотная модуляция (двоичная)
    void modFM2();

    // Получение сигнала
    signal getS();

    // Получение вектора времени
    std::vector<double> getT();

    // Получение длительности сигнала
    double getDuration();

    // Получение битовой последовательности в виде вектора double
    std::vector<double> getBits();

    // Выполнение модуляции (генерация битов, модуляция, перенос на несущую)
    void manipulation();

    // Установка типа модуляции
    void setType(type_modulation _type);

    // Создание базового сигнала (вырезание части из полного сигнала)
    signal createBaseSignal();
};