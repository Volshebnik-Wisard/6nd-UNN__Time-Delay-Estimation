#pragma once 
#include "signalProccesser.h"
#include <thread>
#include "Drawer.h"

class CdelayDlg : public CDialogEx // Определение класса диалогового окна, наследующего CDialogEx
{

public:
	CdelayDlg(CWnd* pParent = nullptr); // Стандартный конструктор

	// Данные диалогового окна
#ifdef AFX_DESIGN_TIME // Условная компиляция: если определено AFX_DESIGN_TIME (дизайнерское время MFC)
	enum {
		IDD = IDD_CHECK_DELAY_DIALOG
	}; // Enum, содержащий идентификатор диалогового окна
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX); // Функция для поддержки обмена данными (DDX/DDV)

	// Реализация
protected:
	HICON m_hIcon; // Иконка диалогового окна

	// Созданные функции схемы сообщений
	virtual BOOL OnInitDialog(); // Обработчик инициализации диалога
	afx_msg void OnPaint(); // Обработчик рисования окна
	afx_msg HCURSOR OnQueryDragIcon(); // Обработчик запроса иконки для перетаскивания
	DECLARE_MESSAGE_MAP() // Макрос для объявления карты сообщений MFC
public:
	// Данные для графиков
	std::vector<double> m_baseSignalY;      // Данные базового сигнала (Y)
	std::vector<double> m_baseSignalX;      // Время базового сигнала (X)
	std::vector<double> m_fullSignalY;      // Данные полного сигнала (Y) 
	std::vector<double> m_fullSignalX;      // Время полного сигнала (X)
	std::vector<double> m_correlationY;     // Данные корреляции (Y)
	std::vector<double> m_correlationX;     // Временной сдвиг корреляции (X)
	std::vector<double> m_bitsY;            // Биты (Y)
	std::vector<double> m_bitsX;            // Время битов (X)

	// Для экспериментов
	std::vector<double> m_experimentAM_Y;   // AM результаты
	std::vector<double> m_experimentPM_Y;   // PM результаты  
	std::vector<double> m_experimentFM_Y;   // FM результаты
	std::vector<double> m_experimentSNR_X;  // SNR значения (X)

	// Объекты Drawer для каждого Picture Control
	Drawer drv1;  // для IDC_GRAPH1 (Корреляция)
	Drawer drv2;  // для IDC_GRAPH2 (Полный сигнал)
	Drawer drv3;  // для IDC_GRAPH3 (Базовый сигнал)
	Drawer drv4;  // для IDC_GRAPH4 (Эксперименты)
	Drawer drv5;  // для IDC_GRAPH4 (Эксперименты)
	afx_msg void OnBnClickedButcreate(); // Обработчик нажатия кнопки создания
	double fd; // Частота дискретизации (в МГц)
	int nbits; // Количество бит в последовательности
	double bitrate; // Битовая скорость (в бит/с)
	double fc; // Несущая частота (в кГц)
	double delay; // Задержка сигнала (в мс)
	double snr; // Отношение сигнал/шум (в дБ)
	modulation modul_signal; // Объект модуляции (не используется?)
	CString result_delay; // Строка для отображения результата оценки задержки
	type_modulation type; // Тип модуляции (AM, PM, FM)
	afx_msg void OnBnClickedRadioAm(); // Обработчик выбора AM модуляции
	afx_msg void OnBnClickedRadioPm(); // Обработчик выбора PM модуляции
	afx_msg void OnBnClickedRadioFrm(); // Обработчик выбора FM модуляции (опечатка в названии)
	double sample_base; // Длительность базового сигнала (в мс)
	double snr_fully; // Отношение сигнал/шум для полного сигнала (в дБ)

	// Векторы для хранения результатов экспериментов:
	std::vector<std::vector<double>> vec_p = std::vector<std::vector<double>>(3); // vec_p[0] для AM, [1] для PM, [2] для FM
	std::vector<double> vec_snr; // Вектор значений SNR для экспериментов

	afx_msg void OnBnClickedButtonEstimate(); // Обработчик кнопки запуска оценки (экспериментов)
	double min_snr; // Минимальное значение SNR для экспериментов
	double max_snr; // Максимальное значение SNR для экспериментов
	double step_snr; // Шаг изменения SNR для экспериментов
	double N_generate; // Количество генераций сигнала на одну точку эксперимента
	CProgressCtrl progress_experement; // Элемент управления прогресс-баром
	afx_msg void OnBnClickedButtonDrawOne(); // Обработчик кнопки отрисовки одного графика
	afx_msg void OnBnClickedButtonDrawMany(); // Обработчик кнопки отрисовки нескольких графиков
	CStatic m_picCtrlBaseSignal;
	CStatic m_picCtrlFullSignal;
	CStatic m_picCtrlCorrelation;
	CStatic m_picCtrlExperiment;
	CStatic m_picCodeSequence;
	afx_msg void OnStnClickedGraph2();
	afx_msg void OnStnClickedGraph4();
	afx_msg void OnStnClickedGraph5();
};

// Объявление функции, выполняющейся в отдельном потоке для оценки задержки
int mainThread(double _fd, int _nbits, double _bitrate, double _fc, double _delay, double _snr, double _snr_fully, double duration_base, type_modulation _type, double& result, int N_generate);
struct ProcessResult;
// Объявление функции основного процесса обработки сигнала и оценки задержки
ProcessResult mainProcess(double _fd, int _nbits, double _bitrate, double _fc, double _delay, double _snr, double _snr_fully, double duration_base, type_modulation _type);

// Объявление функции запуска потока для экспериментов
void StartThread(CdelayDlg* dlg);