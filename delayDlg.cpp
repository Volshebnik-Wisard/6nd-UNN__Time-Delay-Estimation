#include "pch.h"
#include "framework.h"
#include "delay.h"
#include "delayDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif



CdelayDlg::CdelayDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CHECK_DELAY_DIALOG, pParent) // Вызов конструктора базового класса
	, fd(1.1) // Инициализация частоты дискретизации значением по умолчанию
	, nbits(150) // Инициализация количества бит
	, bitrate(150) // Инициализация битовой скорости
	, fc(0.5) // Инициализация несущей частоты
	, delay(100) // Инициализация задержки
	, snr(3) // Инициализация SNR
	, result_delay(_T("")) // Инициализация строки результата
	, sample_base(400) // Инициализация длительности базового сигнала
	, snr_fully(10) // Инициализация SNR для полного сигнала
	, min_snr(-30) // Инициализация минимального SNR для экспериментов
	, max_snr(10) // Инициализация максимального SNR
	, step_snr(1) // Инициализация шага SNR
	, N_generate(300) // Инициализация количества генераций
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME); // Загрузка иконки приложения
}

// Функция обмена данными между элементами управления и переменными (DDX/DDV)
void CdelayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX); // Вызов функции базового класса
	// Связывание переменных с элементами управления:
	DDX_Text(pDX, IDC_EDIT_FD, fd);
	DDX_Text(pDX, IDC_EDIT_NBITS, nbits);
	DDX_Text(pDX, IDC_EDIT_BITRATE, bitrate);
	DDX_Text(pDX, IDC_EDIT_FC, fc);
	DDX_Text(pDX, IDC_EDIT_DELAY, delay);
	DDX_Text(pDX, IDC_EDIT_SNR, snr);
	DDX_Text(pDX, IDC_STATIC_EST_DELAY, result_delay);
	DDX_Text(pDX, IDC_EDIT_SAMPLE_BASE, sample_base);
	DDX_Text(pDX, IDC_EDIT_SNR2, snr_fully);
	DDX_Text(pDX, IDC_EDIT_DELAY2, min_snr);
	DDX_Text(pDX, IDC_EDIT_SAMPLE_BASE2, max_snr);
	DDX_Text(pDX, IDC_EDIT_SNR3, step_snr);
	DDX_Text(pDX, IDC_EDIT_SNR4, N_generate);
	DDX_Control(pDX, IDC_PROGRESS1, progress_experement); // Связывание элемента управления прогресс-баром
	DDX_Control(pDX, IDC_GRAPH3, m_picCtrlBaseSignal);
	DDX_Control(pDX, IDC_GRAPH2, m_picCtrlFullSignal);
	DDX_Control(pDX, IDC_GRAPH1, m_picCtrlCorrelation);
	DDX_Control(pDX, IDC_GRAPH4, m_picCtrlExperiment);
	DDX_Control(pDX, IDC_GRAPH5, m_picCodeSequence);
}

// Макрос для объявления карты сообщений MFC
BEGIN_MESSAGE_MAP(CdelayDlg, CDialogEx)
	ON_WM_PAINT() // Обработчик сообщения WM_PAINT (рисование)
	ON_WM_QUERYDRAGICON() // Обработчик запроса иконки для перетаскивания
	ON_BN_CLICKED(IDC_BUTCREATE, &CdelayDlg::OnBnClickedButcreate) // Обработчик нажатия кнопки создания
	ON_BN_CLICKED(IDC_RADIO_AM, &CdelayDlg::OnBnClickedRadioAm) // Обработчик выбора AM
	ON_BN_CLICKED(IDC_RADIO_PM, &CdelayDlg::OnBnClickedRadioPm) // Обработчик выбора PM
	ON_BN_CLICKED(IDC_RADIO_FRM, &CdelayDlg::OnBnClickedRadioFrm) // Обработчик выбора FM
	ON_BN_CLICKED(IDC_BUTTON_ESTIMATE, &CdelayDlg::OnBnClickedButtonEstimate) // Обработчик кнопки оценки
END_MESSAGE_MAP()

// Обработчики сообщений CdelayDlg

// Обработчик инициализации диалогового окна
BOOL CdelayDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog(); // Вызов базового класса

	// Задает значок для этого диалогового окна. Среда делает это автоматически,
	// если главное окно приложения не является диалоговым
	SetIcon(m_hIcon, TRUE); // Установка большой иконки
	SetIcon(m_hIcon, FALSE); // Установка маленькой иконки
	drv1.Create(GetDlgItem(IDC_GRAPH1)->GetSafeHwnd());
	drv2.Create(GetDlgItem(IDC_GRAPH2)->GetSafeHwnd());
	drv3.Create(GetDlgItem(IDC_GRAPH3)->GetSafeHwnd());
	drv4.Create(GetDlgItem(IDC_GRAPH4)->GetSafeHwnd());
	drv5.Create(GetDlgItem(IDC_GRAPH5)->GetSafeHwnd());
	// TODO: добавьте дополнительную инициализацию
	progress_experement.SetRange(0, 100); // Установка диапазона прогресс-бара от 0 до 100
	return TRUE; // Возврат TRUE, если фокус не передан элементу управления
}

// Обработчик сообщения WM_PAINT (рисование окна)
void CdelayDlg::OnPaint()
{
	if (IsIconic()) // Если окно свёрнуто (отображается как иконка)
	{
		CPaintDC dc(this); // Контекст устройства для рисования

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0); // Сообщение для затирания фона иконки

		// Выравнивание значка по центру клиентского прямоугольника
		int cxIcon = GetSystemMetrics(SM_CXICON); // Ширина системной иконки
		int cyIcon = GetSystemMetrics(SM_CYICON); // Высота системной иконки
		CRect rect; // Прямоугольник клиентской области
		GetClientRect(&rect); // Получение размеров клиентской области
		int x = (rect.Width() - cxIcon + 1) / 2; // Координата X для рисования иконки
		int y = (rect.Height() - cyIcon + 1) / 2; // Координата Y для рисования иконки

		// Нарисуйте значок
		dc.DrawIcon(x, y, m_hIcon); // Рисование иконки
	}
	else // Если окно не свёрнуто
	{
		CDialogEx::OnPaint(); // Вызов стандартной обработки WM_PAINT
	}
}

// Обработчик запроса курсора для перетаскивания окна
HCURSOR CdelayDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon); // Возврат иконки в виде курсора
}

// Функция, выполняемая в потоке для оценки задержки (многократная генерация)
int mainThread(double _fd, int _nbits, double _bitrate, double _fc, double _delay, double _snr, double _snr_fully, double duration_base, type_modulation _type, double& result, int N_generate)
{
	int N_exp = N_generate; // Количество экспериментов
	result = 0; // Инициализация результата (количество успешных оценок)
	for (int k = 0; k < N_exp; k++) { // Цикл по количеству генераций
		modulation manipulated; // Создание объекта модуляции
		manipulated.setParam(_fd, _nbits, _bitrate, _fc, _delay, duration_base, _type); // Установка параметров
		manipulated.manipulation(); // Генерация и модуляция сигнала
		auto base_signal = manipulated.createBaseSignal(); // Создание базового сигнала
		auto fully_signal = manipulated.getS(); // Получение полного сигнала

		signal base_signal_noise; // Базовый сигнал с шумом
		signal fully_signal_noise; // Полный сигнал с шумом
		addNoise(base_signal, _snr, base_signal_noise); // Добавление шума к базовому сигналу
		addNoise(fully_signal, _snr_fully, fully_signal_noise); // Добавление шума к полному сигналу

		std::vector<double> corr; // Вектор корреляции
		std::vector<double> t_corr; // Вектор времён корреляции
		correlation(fully_signal_noise, base_signal_noise, 1. / (_fd * 1000), corr, t_corr); // Вычисление корреляции
		auto a = findMax(corr, t_corr, _delay); // Поиск максимума корреляции и оценка задержки
		if (abs(a.first - _delay / 1000) <= 0.5 / _bitrate) { // Проверка, что оценка в пределах одного бита
			result += 1; // Увеличение счётчика успешных оценок
		}
	}
	result /= N_exp; // Вычисление доверительной вероятности (доля успешных оценок)
	return 0; // Возврат успешного завершения
}

// В delayDlg.cpp (перед mainProcess) добавим:
struct ProcessResult {
	std::pair<double, double> delayEstimation; // Оценка задержки и ошибка
	std::vector<double> bitsY;                // Кодовая последовательность
	std::vector<double> bitsX;                // Время битов
	std::vector<double> fullSignalY;          // Полный сигнал
	std::vector<double> fullSignalX;          // Время полного сигнала
	std::vector<double> baseSignalY;          // Базовый сигнал
	std::vector<double> baseSignalX;          // Время базового сигнала
	std::vector<double> correlationY;         // Корреляция
	std::vector<double> correlationX;         // Временной сдвиг корреляции
	double signalDuration;                    // Длительность сигнала
};

// Изменяем mainProcess чтобы он возвращал ProcessResult
ProcessResult mainProcess(double _fd, int _nbits, double _bitrate, double _fc, double _delay,
	double _snr, double _snr_fully, double duration_base, type_modulation _type)
{
	ProcessResult result;

	modulation manipulated; // Создание объекта модуляции
	manipulated.setParam(_fd, _nbits, _bitrate, _fc, _delay, duration_base, _type); // Установка параметров
	manipulated.manipulation(); // Генерация и модуляция сигнала
	auto base_signal = manipulated.createBaseSignal(); // Создание базового сигнала
	auto fully_signal = manipulated.getS(); // Получение полного сигнала
	auto t = manipulated.getT(); // Получение вектора времени

	signal base_signal_noise; // Базовый сигнал с шумом
	signal fully_signal_noise; // Полный сигнал с шумом
	addNoise(base_signal, _snr, base_signal_noise); // Добавление шума к базовому сигналу
	addNoise(fully_signal, _snr_fully, fully_signal_noise); // Добавление шума к полному сигналу

	std::vector<double> corr; // Вектор корреляции
	std::vector<double> t_corr; // Вектор времён корреляции
	correlation(fully_signal_noise, base_signal_noise, 1. / (_fd * 1000), corr, t_corr); // Вычисление корреляции
	int sample = fully_signal_noise.N; // Количество отсчётов
	fully_signal_noise.resize(sample); // Изменение размера (избыточно)
	base_signal_noise.resize(sample); // Изменение размера (избыточно)

	// Сохраняем данные в структуру результата
	result.bitsY = manipulated.getBits();
	result.bitsX = t;
	result.fullSignalY = fully_signal_noise.getSreal();
	result.fullSignalX = t;
	result.baseSignalY = base_signal_noise.getSreal();
	result.baseSignalX = t;
	result.correlationY = corr;
	result.correlationX = t_corr;
	result.signalDuration = manipulated.getDuration();
	result.delayEstimation = findMax(corr, t_corr, _delay);

	return result;
}

void CdelayDlg::OnBnClickedButcreate()
{
	UpdateData(TRUE); // Обновление данных из элементов управления

	// Вызываем mainProcess один раз
	ProcessResult result = mainProcess(fd, nbits, bitrate, fc, delay, snr, snr_fully, sample_base, type);

	// Сохраняем данные для отрисовки
	m_bitsY = result.bitsY;
	m_bitsX = result.bitsX;
	m_fullSignalY = result.fullSignalY;
	m_fullSignalX = result.fullSignalX;
	m_baseSignalY = result.baseSignalY;
	m_baseSignalX = result.baseSignalX;
	m_correlationY = result.correlationY;
	m_correlationX = result.correlationX;

	// Отрисовываем графики
	// 1. Кодовая последовательность (IDC_GRAPH5)
	if (!m_bitsY.empty() && !m_bitsX.empty()) {
		double minY = 0.0;
		double maxY = 1.5;
		double minX = 0.0;
		double maxX = result.signalDuration;
		drv5.Draw(m_bitsY, minY, maxY, m_bitsX, minX, maxX, 'B', L"t, c.", L"Бит");
	}

	// 2. Опорный сигнал (IDC_GRAPH3)
	if (!m_baseSignalY.empty() && !m_baseSignalX.empty()) {
		double minY, maxY;
		if (type == AM) {
			minY = -1.5;
			maxY = 1.5;
		}
		else { // PM или FM
			minY = -1.5;
			maxY = 1.5;
		}

		double minX = 0.0;
		double maxX = result.signalDuration;
		drv3.Draw(m_baseSignalY, minY, maxY, m_baseSignalX, minX, maxX, 'B', L"t, c.", L"Бит");
	}

	// 3. Полный сигнал (IDC_GRAPH2)
	if (!m_fullSignalY.empty() && !m_fullSignalX.empty()) {
		double minY, maxY;
		if (type == AM) {
			minY = -1.5;
			maxY = 1.5;
		}
		else { // PM или FM
			minY = -1.5;
			maxY = 1.5;
		}

		double minX = 0.0;
		double maxX = result.signalDuration;
		drv2.Draw(m_fullSignalY, minY, maxY, m_fullSignalX, minX, maxX, 'R', L"t, c.", L"Бит");
	}

	// 4. Корреляция (IDC_GRAPH1)
	if (!m_correlationY.empty() && !m_correlationX.empty()) {
		double minY = *std::min_element(m_correlationY.begin(), m_correlationY.end());
		double maxY = *std::max_element(m_correlationY.begin(), m_correlationY.end());
		double minX = *std::min_element(m_correlationX.begin(), m_correlationX.end());
		double maxX = *std::max_element(m_correlationX.begin(), m_correlationX.end());
		drv1.Draw(m_correlationY, minY, maxY, m_correlationX, minX, maxX, 'G', L"τ, с", L"R(τ)");
	}

	// Отображаем результат оценки задержки
	if (abs(result.delayEstimation.first - delay / 1000) > 0.5 / bitrate) {
		result_delay.Format(_T("Оценка сдвига: %.4f мс;\nОтличие сигнала от заданного: %.3f %%;\nНекорректно: больше, чем один бит"),
			result.delayEstimation.first * 1000, result.delayEstimation.second);
	}
	else {
		result_delay.Format(_T("Оценка сдвига: %.4f мс;\nОтличие сигнала от заданного: %.3f %%;\nКорректно: в пределах одного символа"),
			result.delayEstimation.first * 1000, result.delayEstimation.second);
	}

	UpdateData(FALSE); // Обновление элементов управления данными
}

// Обработчик выбора AM модуляции
void CdelayDlg::OnBnClickedRadioAm()
{
	// TODO: добавьте свой код обработчика уведомлений
	type = AM; // Установка типа модуляции AM
}

// Обработчик выбора PM модуляции
void CdelayDlg::OnBnClickedRadioPm()
{
	// TODO: добавьте свой код обработчика уведомлений
	type = PM; // Установка типа модуляции PM
}

// Обработчик выбора FM модуляции (опечатка в названии FRM)
void CdelayDlg::OnBnClickedRadioFrm()
{
	// TODO: добавьте свой код обработчика уведомлений
	type = FM; // Установка типа модуляции FM
}

// Функция запуска потоков для экспериментов с разными SNR
// Функция запуска потоков для экспериментов с разными SNR
void StartThread(CdelayDlg* dlg)
{
	double min_snr = dlg->min_snr; // Минимальное SNR
	double max_snr = dlg->max_snr; // Максимальное SNR
	double step_snr = dlg->step_snr; // Шаг SNR
	int num_thread = 6; // Количество потоков (по 2 на каждый тип модуляции)
	int point = (max_snr - min_snr) / step_snr; // Количество точек эксперимента
	dlg->vec_p[0].resize(point); // Изменение размера вектора для AM
	dlg->vec_p[1].resize(point); // Изменение размера вектора для PM
	dlg->vec_p[2].resize(point); // Изменение размера вектора для FM
	dlg->vec_snr.resize(point); // Изменение размера вектора SNR

	// Сохраняем SNR значения для графика экспериментов
	dlg->m_experimentSNR_X.resize(point);
	for (int i = 0; i < point; i++) {
		dlg->m_experimentSNR_X[i] = min_snr + i * step_snr;
	}

	std::vector<std::thread> thr(num_thread); // Вектор потоков
	std::vector<type_modulation> types({ AM, PM, FM, AM, PM, FM }); // Типы модуляции для каждого потока

	for (int i = 0; i < point; i += 2) {
		dlg->progress_experement.SetPos((double)i / point * 100); // Обновление прогресс-бара
		dlg->vec_snr[i] = min_snr + step_snr * i;
		dlg->vec_snr[i + 1] = min_snr + step_snr * (i + 1);

		for (int j = 0; j < num_thread; j++) {
			if (i + j / 3 >= point) {
				break;
			}
			// Проверяем, можно ли присвоить новый поток
			if (thr[j].joinable()) {
				thr[j].join(); // Ждем завершения предыдущего потока
			}

			thr[j] = std::thread(
				mainThread,
				dlg->fd,
				dlg->nbits,
				dlg->bitrate,
				dlg->fc,
				dlg->delay,
				dlg->vec_snr[i + j / 3],
				dlg->snr_fully,
				dlg->sample_base,
				types[j],
				std::ref(dlg->vec_p[types[j]][i + j / 3]),
				dlg->N_generate);
		}

		// Ждем завершения всех потоков перед следующей итерацией
		for (int j = 0; j < num_thread; j++) {
			if (thr[j].joinable()) {
				thr[j].join();
			}
		}
	}

	dlg->progress_experement.SetPos(100); // Установка прогресс-бара в 100%

	// Сохраняем результаты экспериментов
	dlg->m_experimentAM_Y = dlg->vec_p[0];
	dlg->m_experimentPM_Y = dlg->vec_p[1];
	dlg->m_experimentFM_Y = dlg->vec_p[2];

	// Отрисовываем график экспериментов с тремя кривыми (IDC_GRAPH4)
	if (!dlg->m_experimentAM_Y.empty() && !dlg->m_experimentSNR_X.empty()) {
		double minY = 0.0; // Вероятность от 0 до 1
		double maxY = 1.0;
		double minX = *std::min_element(dlg->m_experimentSNR_X.begin(), dlg->m_experimentSNR_X.end());
		double maxX = *std::max_element(dlg->m_experimentSNR_X.begin(), dlg->m_experimentSNR_X.end());

		// Используем Draw3() для отрисовки всех трех кривых
		dlg->drv4.Draw3(
			dlg->m_experimentAM_Y, minY, maxY,
			dlg->m_experimentSNR_X, minX, maxX, 'R',  // AM - красный
			dlg->m_experimentPM_Y,
			dlg->m_experimentSNR_X,  // Для PM используем те же X
			'G',  // PM - зеленый
			dlg->m_experimentFM_Y,
			dlg->m_experimentSNR_X,  // Для FM используем те же X
			'B'   // FM - синий
			, L"SNR, дБ", L""
		);
	}
}

// Обработчик нажатия кнопки "Оценить" (запуск многопоточных экспериментов)
void CdelayDlg::OnBnClickedButtonEstimate()
{
	// TODO: добавьте свой код обработчика уведомлений
	UpdateData(); // Обновление данных из элементов управления
	std::thread t1(StartThread, this); // Запуск функции StartThread в отдельном потоке
	t1.detach(); // Отсоединение потока (позволяет ему работать независимо)
}


