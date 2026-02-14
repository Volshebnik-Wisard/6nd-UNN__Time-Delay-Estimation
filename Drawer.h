#pragma once

#include <afxwin.h>
#include <vector>
#define pi 3.1415926535897932384626433832795

using namespace std;

class Drawer
{
	// Прямоугольник области рисования.
	CRect frame;
	// Указатель на объект окна, области рисования.
	CWnd* wnd;
	// Контекст рисования, привязанный к области рисования.
	CDC* dc;
	// Контекст рисования, привязанный к памяти.
	CDC memDC;
	// Память для контекста рисования memDC.
	CBitmap bmp;
	// Флаг для отслеживания состояния инициализации класса.
	bool init;
public:


	// Проинициализировать объект класса на основе HWND.
	void Create(HWND hWnd)
	{
		// Получаем указатель на окно.
		wnd = CWnd::FromHandle(hWnd);
		// Получаем прямоугольник окна.
		wnd->GetClientRect(frame);
		// Получаем контекст для рисования в этом окне.
		dc = wnd->GetDC();

		// Создаем буфер-контекст.
		memDC.CreateCompatibleDC(dc);
		// Создаем растр для контекста рисования.
		bmp.CreateCompatibleBitmap(dc, frame.Width(), frame.Height());
		// Выбираем растр для использования буфер-контекстом.
		memDC.SelectObject(&bmp);
		init = true;
	}

	// Нарисовать график по переданным данным.
	void Draw(vector<double>& data, double min_data, double max_data,
		vector<double>& keys, double min_keys, double max_keys,
		char color, CString name_x, CString name_y)
	{
		if (!init) return;
		CPen subgrid_pen(PS_DOT, 1, RGB(200, 200, 200));
		CPen grid_pen(PS_SOLID, 1, RGB(0, 0, 0));
		CPen data_pen(PS_SOLID, 2, RGB(255, 0, 0));
		CPen data_pen2(PS_SOLID, 2, RGB(38, 0, 255));
		CPen pen_red(PS_SOLID, 2, RGB(178, 34, 34));
		CPen pen_green(PS_SOLID, 2, RGB(0, 128, 0));
		CFont font;
		font.CreateFontW(18, 0, 0, 0,
			FW_DONTCARE,
			FALSE,				// Курсив
			FALSE,				// Подчеркнутый
			FALSE,				// Перечеркнутый
			DEFAULT_CHARSET,	// Набор символов
			OUT_OUTLINE_PRECIS,	// Точность соответствия.	
			CLIP_DEFAULT_PRECIS,//  
			CLEARTYPE_QUALITY,	// Качество
			VARIABLE_PITCH,		//
			TEXT("Times New Roman")		//
		);

		int padding = 20;
		int left_keys_padding = 20;
		int bottom_keys_padding = 10;

		int actual_width = frame.Width() - 2 * padding - left_keys_padding;
		int actual_height = frame.Height() - 2 * padding - bottom_keys_padding;

		int actual_top = padding;
		int actual_bottom = actual_top + actual_height;
		int actual_left = padding + left_keys_padding;
		int actual_right = actual_left + actual_width;

		// Белый фон.
		memDC.FillSolidRect(frame, RGB(255, 255, 255));

		// Рисуем сетку и подсетку.
		unsigned int grid_size = 10;

		memDC.SelectObject(&subgrid_pen);

		for (double i = 0.5; i < grid_size; i += 1.0)
		{
			memDC.MoveTo(actual_left + i * actual_width / grid_size, actual_top);
			memDC.LineTo(actual_left + i * actual_width / grid_size, actual_bottom);
			memDC.MoveTo(actual_left, actual_top + i * actual_height / grid_size);
			memDC.LineTo(actual_right, actual_top + i * actual_height / grid_size);
		}

		memDC.SelectObject(&grid_pen);

		for (double i = 0.0; i < grid_size + 1; i += 1.0)
		{
			memDC.MoveTo(actual_left + i * actual_width / grid_size, actual_top);
			memDC.LineTo(actual_left + i * actual_width / grid_size, actual_bottom);
			memDC.MoveTo(actual_left, actual_top + i * actual_height / grid_size);
			memDC.LineTo(actual_right, actual_top + i * actual_height / grid_size);
		}

		// Рисуем график.
		if (data.empty()) return;

		if (keys.size() != data.size())
		{
			keys.resize(data.size());
			for (int i = 0; i < keys.size(); i++)
			{
				keys[i] = i;
			}
		}

		if (color == 'R')memDC.SelectObject(&pen_red);
		else if (color == 'G')memDC.SelectObject(&pen_green);
		else memDC.SelectObject(&data_pen);

		double data_y_max(max_data), data_y_min(min_data);
		double data_x_max(max_keys), data_x_min(min_keys);

		vector<double> y = convert_range(data, actual_top, actual_bottom, data_y_max, data_y_min);
		vector<double> x = convert_range(keys, actual_right, actual_left, data_x_max, data_x_min);

		memDC.MoveTo(x[0], y[0]);
		for (unsigned int i = 0; i < y.size(); i++)
		{
			memDC.LineTo(x[i], y[i]);
		}

		memDC.SelectObject(&font);
		memDC.SetTextColor(RGB(0, 0, 0));
		for (int i = 0; i < grid_size; i++)
		{
			CString str;
			str.Format(L"%.1f", data_x_min + i * (data_x_max - data_x_min) / (grid_size / 1));
			memDC.TextOutW(actual_left + (double)i * actual_width / (grid_size / 1) - bottom_keys_padding, actual_bottom + bottom_keys_padding / 2, str);
		}

		for (int i = 0; i < grid_size / 2; i++)
		{
			CString str;
			str.Format(L"%.1f", data_y_min + i * (data_y_max - data_y_min) / (grid_size / 2));
			memDC.TextOutW(actual_left - 1.5 * left_keys_padding, actual_bottom - (double)i * actual_height / (grid_size / 2) - bottom_keys_padding, str);
		}

		CString str;
		str = name_x;
		memDC.TextOutW(actual_left - 10 + (grid_size / 2) * actual_width / (grid_size / 2) - bottom_keys_padding, actual_bottom + bottom_keys_padding / 2, str);


		str = name_y;
		memDC.TextOutW(actual_left - 1.5 * left_keys_padding, actual_bottom - grid_size / 2 * actual_height / (grid_size / 2) - bottom_keys_padding, str);

		dc->BitBlt(0, 0, frame.Width(), frame.Height(), &memDC, 0, 0, SRCCOPY);
	}

	void Draw2(vector<double>& data, double min_data, double max_data,
		vector<double>& keys, double min_keys, double max_keys,
		char color,
		vector<double>& data2,
		vector<double>& keys2,
		char color2, CString name_x, CString name_y
	)
	{
		if (!init) return;

		CPen subgrid_pen(PS_DOT, 1, RGB(200, 200, 200));
		CPen grid_pen(PS_SOLID, 1, RGB(0, 0, 0));
		CPen data_pen(PS_SOLID, 2, RGB(255, 0, 0));
		CPen data_pen2(PS_SOLID, 2, RGB(38, 0, 255));
		CPen pen_red(PS_SOLID, 2, RGB(178, 34, 34));
		CPen pen_green(PS_SOLID, 2, RGB(0, 128, 0));
		CFont font;
		font.CreateFontW(18, 0, 0, 0,
			FW_DONTCARE,
			FALSE,				// Курсив
			FALSE,				// Подчеркнутый
			FALSE,				// Перечеркнутый
			DEFAULT_CHARSET,	// Набор символов
			OUT_OUTLINE_PRECIS,	// Точность соответствия.	
			CLIP_DEFAULT_PRECIS,//  
			CLEARTYPE_QUALITY,	// Качество
			VARIABLE_PITCH,		//
			TEXT("Times New Roman")		//
		);

		int padding = 20;
		int left_keys_padding = 20;
		int bottom_keys_padding = 10;

		int actual_width = frame.Width() - 2 * padding - left_keys_padding;
		int actual_height = frame.Height() - 2 * padding - bottom_keys_padding;

		int actual_top = padding;
		int actual_bottom = actual_top + actual_height;
		int actual_left = padding + left_keys_padding;
		int actual_right = actual_left + actual_width;

		// Белый фон.
		memDC.FillSolidRect(frame, RGB(255, 255, 255));

		// Рисуем сетку и подсетку.
		unsigned int grid_size = 10;

		memDC.SelectObject(&subgrid_pen);

		for (double i = 0.5; i < grid_size; i += 1.0)
		{
			memDC.MoveTo(actual_left + i * actual_width / grid_size, actual_top);
			memDC.LineTo(actual_left + i * actual_width / grid_size, actual_bottom);
			memDC.MoveTo(actual_left, actual_top + i * actual_height / grid_size);
			memDC.LineTo(actual_right, actual_top + i * actual_height / grid_size);
		}

		memDC.SelectObject(&grid_pen);

		for (double i = 0.0; i < grid_size + 1; i += 1.0)
		{
			memDC.MoveTo(actual_left + i * actual_width / grid_size, actual_top);
			memDC.LineTo(actual_left + i * actual_width / grid_size, actual_bottom);
			memDC.MoveTo(actual_left, actual_top + i * actual_height / grid_size);
			memDC.LineTo(actual_right, actual_top + i * actual_height / grid_size);
		}

		// Рисуем график.
		if (data.empty()) return;

		if (keys.size() != data.size())
		{
			keys.resize(data.size());
			for (int i = 0; i < keys.size(); i++)
			{
				keys[i] = i;
			}
		}

		/*РИСУЕМ ПЕРВЫЙ ГРАФИК*/
		if (color == 'R')memDC.SelectObject(&pen_red);
		else if (color == 'G')memDC.SelectObject(&pen_green);
		else memDC.SelectObject(&data_pen);

		double data_y_max(max_data), data_y_min(min_data);
		double data_x_max(max_keys), data_x_min(min_keys);

		vector<double> y = convert_range(data, actual_top, actual_bottom, data_y_max, data_y_min);
		vector<double> x = convert_range(keys, actual_right, actual_left, data_x_max, data_x_min);

		memDC.MoveTo(x[0], y[0]);
		for (unsigned int i = 0; i < y.size(); i++)
		{
			memDC.LineTo(x[i], y[i]);
		}
		/*РИСУЕМ ВТОРОЙ ГРАФИК*/
		if (color2 == 'R')memDC.SelectObject(&pen_red);
		else if (color2 == 'G')memDC.SelectObject(&pen_green);
		else memDC.SelectObject(&data_pen);

		y = convert_range(data2, actual_top, actual_bottom, data_y_max, data_y_min);
		x = convert_range(keys2, actual_right, actual_left, data_x_max, data_x_min);

		memDC.MoveTo(x[0], y[0]);
		for (unsigned int i = 0; i < y.size(); i++)
		{
			memDC.LineTo(x[i], y[i]);
		}
		/**/

		memDC.SelectObject(&font);
		memDC.SetTextColor(RGB(0, 0, 0));
		for (int i = 0; i < grid_size / 2 + 1; i++)
		{
			CString str;
			str.Format(L"%.1f", data_x_min + i * (data_x_max - data_x_min) / (grid_size / 2));
			memDC.TextOutW(actual_left + (double)i * actual_width / (grid_size / 2) - bottom_keys_padding, actual_bottom + bottom_keys_padding / 2, str);

			str.Format(L"%.1f", data_y_min + i * (data_y_max - data_y_min) / (grid_size / 2));
			memDC.TextOutW(actual_left - 1.5 * left_keys_padding, actual_bottom - (double)i * actual_height / (grid_size / 2) - bottom_keys_padding, str);
		}
		CString str;
		str = name_x;
		memDC.TextOutW(actual_left - 10 + (grid_size / 2) * actual_width / (grid_size / 2) - bottom_keys_padding, actual_bottom + bottom_keys_padding / 2, str);


		str = name_y;
		memDC.TextOutW(actual_left - 1.5 * left_keys_padding, actual_bottom - grid_size / 2 * actual_height / (grid_size / 2) - bottom_keys_padding, str);

		dc->BitBlt(0, 0, frame.Width(), frame.Height(), &memDC, 0, 0, SRCCOPY);
	}

	void Draw3(vector<double>& data1, double min_data, double max_data,
		vector<double>& keys1, double min_keys, double max_keys,
		char color1,
		vector<double>& data2,
		vector<double>& keys2,
		char color2,
		vector<double>& data3,
		vector<double>& keys3,
		char color3, CString name_x, CString name_y)
	{
		if (!init) return;

		CPen subgrid_pen(PS_DOT, 1, RGB(200, 200, 200));
		CPen grid_pen(PS_SOLID, 1, RGB(0, 0, 0));
		CPen data_pen(PS_SOLID, 2, RGB(255, 0, 0));
		CPen data_pen2(PS_SOLID, 2, RGB(38, 0, 255));
		CPen pen_red(PS_SOLID, 2, RGB(178, 34, 34));
		CPen pen_green(PS_SOLID, 2, RGB(0, 128, 0));
		CFont font;
		font.CreateFontW(18, 0, 0, 0,
			FW_DONTCARE,
			FALSE,              // Курсив
			FALSE,              // Подчеркнутый
			FALSE,              // Перечеркнутый
			DEFAULT_CHARSET,    // Набор символов
			OUT_OUTLINE_PRECIS, // Точность соответствия.    
			CLIP_DEFAULT_PRECIS,//  
			CLEARTYPE_QUALITY,  // Качество
			VARIABLE_PITCH,     //
			TEXT("Times New Roman")     //
		);

		int padding = 20;
		int left_keys_padding = 20;
		int bottom_keys_padding = 10;

		int actual_width = frame.Width() - 2 * padding - left_keys_padding;
		int actual_height = frame.Height() - 2 * padding - bottom_keys_padding;

		int actual_top = padding;
		int actual_bottom = actual_top + actual_height;
		int actual_left = padding + left_keys_padding;
		int actual_right = actual_left + actual_width;

		// Белый фон.
		memDC.FillSolidRect(frame, RGB(255, 255, 255));

		// Рисуем сетку и подсетку.
		unsigned int grid_size = 10;

		memDC.SelectObject(&subgrid_pen);

		for (double i = 0.5; i < grid_size; i += 1.0)
		{
			memDC.MoveTo(actual_left + i * actual_width / grid_size, actual_top);
			memDC.LineTo(actual_left + i * actual_width / grid_size, actual_bottom);
			memDC.MoveTo(actual_left, actual_top + i * actual_height / grid_size);
			memDC.LineTo(actual_right, actual_top + i * actual_height / grid_size);
		}

		memDC.SelectObject(&grid_pen);

		for (double i = 0.0; i < grid_size + 1; i += 1.0)
		{
			memDC.MoveTo(actual_left + i * actual_width / grid_size, actual_top);
			memDC.LineTo(actual_left + i * actual_width / grid_size, actual_bottom);
			memDC.MoveTo(actual_left, actual_top + i * actual_height / grid_size);
			memDC.LineTo(actual_right, actual_top + i * actual_height / grid_size);
		}

		// Рисуем графики
		if (data1.empty() && data2.empty() && data3.empty()) return;

		/*РИСУЕМ ПЕРВЫЙ ГРАФИК*/
		if (color1 == 'R') memDC.SelectObject(&pen_red);
		else if (color1 == 'G') memDC.SelectObject(&pen_green);
		else if (color1 == 'B') memDC.SelectObject(&data_pen2); // Синий
		else memDC.SelectObject(&data_pen); // Красный по умолчанию

		if (!data1.empty() && !keys1.empty()) {
			// Проверяем размеры
			if (keys1.size() != data1.size()) {
				keys1.resize(data1.size());
				for (size_t i = 0; i < keys1.size(); i++) {
					keys1[i] = i;
				}
			}

			vector<double> y = convert_range(data1, actual_top, actual_bottom, max_data, min_data);
			vector<double> x = convert_range(keys1, actual_right, actual_left, max_keys, min_keys);

			if (!x.empty() && !y.empty()) {
				memDC.MoveTo(x[0], y[0]);
				for (size_t i = 0; i < y.size(); i++) {
					memDC.LineTo(x[i], y[i]);
				}
			}
		}

		/*РИСУЕМ ВТОРОЙ ГРАФИК*/
		if (color2 == 'R') memDC.SelectObject(&pen_red);
		else if (color2 == 'G') memDC.SelectObject(&pen_green);
		else if (color2 == 'B') memDC.SelectObject(&data_pen2);
		else memDC.SelectObject(&data_pen);

		if (!data2.empty() && !keys2.empty()) {
			// Проверяем размеры
			if (keys2.size() != data2.size()) {
				keys2.resize(data2.size());
				for (size_t i = 0; i < keys2.size(); i++) {
					keys2[i] = i;
				}
			}

			vector<double> y = convert_range(data2, actual_top, actual_bottom, max_data, min_data);
			vector<double> x = convert_range(keys2, actual_right, actual_left, max_keys, min_keys);

			if (!x.empty() && !y.empty()) {
				memDC.MoveTo(x[0], y[0]);
				for (size_t i = 0; i < y.size(); i++) {
					memDC.LineTo(x[i], y[i]);
				}
			}
		}

		/*РИСУЕМ ТРЕТИЙ ГРАФИК*/
		if (color3 == 'R') memDC.SelectObject(&pen_red);
		else if (color3 == 'G') memDC.SelectObject(&pen_green);
		else if (color3 == 'B') memDC.SelectObject(&data_pen2);
		else memDC.SelectObject(&data_pen);

		if (!data3.empty() && !keys3.empty()) {
			// Проверяем размеры
			if (keys3.size() != data3.size()) {
				keys3.resize(data3.size());
				for (size_t i = 0; i < keys3.size(); i++) {
					keys3[i] = i;
				}
			}

			vector<double> y = convert_range(data3, actual_top, actual_bottom, max_data, min_data);
			vector<double> x = convert_range(keys3, actual_right, actual_left, max_keys, min_keys);

			if (!x.empty() && !y.empty()) {
				memDC.MoveTo(x[0], y[0]);
				for (size_t i = 0; i < y.size(); i++) {
					memDC.LineTo(x[i], y[i]);
				}
			}
		}

		memDC.SelectObject(&font);
		memDC.SetTextColor(RGB(0, 0, 0));
		for (int i = 0; i < grid_size / 2 + 1; i++)
		{
			CString str;
			str.Format(L"%.1f", min_keys + i * (max_keys - min_keys) / (grid_size / 2));
			memDC.TextOutW(actual_left + (double)i * actual_width / (grid_size / 2) - bottom_keys_padding, actual_bottom + bottom_keys_padding / 2, str);

			str.Format(L"%.1f", min_data + i * (max_data - min_data) / (grid_size / 2));
			memDC.TextOutW(actual_left - 1.5 * left_keys_padding, actual_bottom - (double)i * actual_height / (grid_size / 2) - bottom_keys_padding, str);
		}
		CString str;
		str = name_x;
		memDC.TextOutW(actual_left - 10 + (grid_size / 2) * actual_width / (grid_size / 2) - bottom_keys_padding, actual_bottom + bottom_keys_padding / 2, str);


		str = name_y;
		memDC.TextOutW(actual_left - 1.5 * left_keys_padding, actual_bottom - grid_size / 2 * actual_height / (grid_size / 2) - bottom_keys_padding, str);

		dc->BitBlt(0, 0, frame.Width(), frame.Height(), &memDC, 0, 0, SRCCOPY);
	}

	vector<double> convert_range(vector <double>& data, double outmax, double outmin, double inmax, double inmin)
	{
		vector<double> output = data;
		double k = (outmax - outmin) / (inmax - inmin);
		for (auto& item : output)
		{
			item = (item - inmin) * k + outmin;
		}

		return output;
	}
};
