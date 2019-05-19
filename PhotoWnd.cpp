// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "Capture.h"

extern CaptureManager *g_pEngine;

// Implements the window procedure for the video preview window.

namespace PhotoWnd//даем имя функции
{
	HINSTANCE hInst = NULL;//код оконной процедуры, по которой ос будет отличать ее от других
	HBITMAP hPhotoBitmap;//место, куда мы сохраним картинку

	HBRUSH hBackgroundBrush = 0;//кисть

	// высококонтрастный пиксель
	int hcPixelX = 0;
	int hcPixelY = 0;
	int BytesPerPixel = 0;

	BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)//HWND -уникальный номер для окна,
		//LPCREATESTRUCT - используется в обработке соответствующего WM_CREATE, указатель приходит в IPAraum
	{
		hBackgroundBrush = CreateSolidBrush(RGB(128, 128, 128));
		return (hBackgroundBrush != NULL);
	}

	void OnDestroy(HWND hwnd)
	{
		DeleteObject(hBackgroundBrush);
		if (hPhotoBitmap)
		{
			DeleteObject(hPhotoBitmap);
			hPhotoBitmap = NULL;
		}
	}

	void OnPaint(HWND hwnd)
	{
		PAINTSTRUCT ps;// информация для приложения, используется для рисования клиентской области окна
		HDC hdc = BeginPaint(hwnd, &ps);//DC для рисования

		FillRect(hdc, &ps.rcPaint, hBackgroundBrush);

		if (hPhotoBitmap)//получение фотографии
		{
			BITMAP          bitmap;//сохраняем картинку
			HDC             hdcMem;//для рисования
			HGDIOBJ         oldBitmap;//тип 
			RECT rc;

			int x1, y1, cx, cy;
			int delta = 0, deltax = 0, deltay = 0;
			LONG new_width, new_height;

			TCHAR szMessage[80];				// To print message 
			int strLen;

			GetClientRect(hwnd, &rc);//извлекает координаты рабочей области окна 
			cx = rc.right - rc.left;//подсчет ширины
			cy = rc.bottom - rc.top;//посчет длины

			hdcMem = CreateCompatibleDC(hdc);//CreateCompatibleDC-расстровое изображение

			if (hdcMem) {

				// draw photo
				oldBitmap = SelectObject(hdcMem, hPhotoBitmap);// SelectObject-выделяет объект в указанном контексте устройства.
				//новый объект заменяет предыдущий того же типа

				GetObject(hPhotoBitmap, sizeof(bitmap), &bitmap);//получение фото

				// рассчитать, как размер изображения превышает окно
				if (cx < bitmap.bmWidth)//если ширина картинки меньше ширины окна 
				{
					deltax = bitmap.bmWidth - cx;
				}
				if (cy < bitmap.bmHeight)//если длина картинки меньше длины окна 
				{
					deltay = bitmap.bmHeight - cy;
				}
				delta = max(deltax, deltay);

				new_width = bitmap.bmWidth - delta;
				new_height = bitmap.bmHeight - delta;

				// center of the window
				// поиск центра окна и расположение картинки по центру
				x1 = cx / 2;
				y1 = cy / 2;

				//поиск серединного пикселя
				x1 = x1 - new_width / 2;
				y1 = y1 - new_height / 2;

				int nOldMode = SetStretchBltMode(hdc, COLORONCOLOR);// устанавливает режим расстрового изображения
				//в указательном контексте устройства
				StretchBlt(//изменяет размер изображения
					hdc,
					x1, y1,
					new_width,
					new_height,
					hdcMem,
					0, 0,
					bitmap.bmWidth,
					bitmap.bmHeight,
					SRCCOPY
				);
				SetStretchBltMode(hdc, nOldMode);
				SelectObject(hdcMem, oldBitmap);

				// write pixel coordinates
				strLen = wsprintf(szMessage, TEXT("High contrast pixel: x=%i y=%i"), hcPixelX, hcPixelY);

				SetTextColor(hdc, RGB(255, 0, 0));//рисовка символов
				SetBkMode(hdc, TRANSPARENT);// устанавливает фоновый режим смешивания контекста заданного устройства. 
				//Фоновый режим смешивания используется текстом, кистями для штриховки и стилями пера, которые не являются
				//сплошными линиями. делает окна такого же цвета, что и фон



				TextOut(hdc, 5, 5, szMessage, strLen);//записывает строку символов в заданном месте, используя текущий выбранный 
				//шрифт, цвет фона и цвет текста,5 и 5-количество пикселей, которые мы отступаем 


				DeleteDC(hdcMem);// удаление вспомогательных инструментов из памяти
			}
		}
		EndPaint(hwnd, &ps);
	}


	void OnSize(HWND hwnd, UINT state, int /*cx*/, int /*cy*/)//для дочерних окон
	{
		if (state == SIZE_RESTORED)//окно измененно
		{
			InvalidateRect(hwnd, NULL, FALSE);//добавляем прямоугольник к обновляемому региону заданного окна
		}
	}


	BOOL OnEraseBkgnd(HWND hwnd, HDC hDC)
	{
		RECT rc;
		GetClientRect(hwnd, &rc);
		FillRect(hDC, &rc, hBackgroundBrush);
		return TRUE;
	}

	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		switch (uMsg)
		{
			HANDLE_MSG(hwnd, WM_CREATE, OnCreate);
			HANDLE_MSG(hwnd, WM_PAINT, OnPaint);
			HANDLE_MSG(hwnd, WM_SIZE, OnSize);
			HANDLE_MSG(hwnd, WM_DESTROY, OnDestroy);
			HANDLE_MSG(hwnd, WM_ERASEBKGND, OnEraseBkgnd);
		}
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}

	// procedure draws the cross to show where the pint is located
/*bmWidth
Задает ширину растрового изображения в пикселях. Ширина должна быть больше 0.
bmHeight
Задает высоту растрового изображения в линиях растра. Высота должна быть больше 0.
bmWidthBytes
Указывает число байтов в каждой линии растра. Это значение должно быть четным числом, поскольку приборный интерфейс (GDI) 
графики высказывать значения в формы растрового изображения массив значений целого числа 2 (byte). 
Другими словами, bmWidthBytes * 8 должны быть следующей несколько условных определений более 16 больше или равно 
значению получена после выбора bmWidth умножен членом bmBitsPixel.
bmPlanes
Указывает число самолетов цвета в различных форматах.
bmBitsPixel
Указывает количество соседних бит цветов в каждом плоскости, необходимом для определения пиксель.
bmBits
Указывает на расположение значений бита для растрового изображения. Элемент bmBits должен быть указателем длинным к массиву значений 1 байта.
*/
	void DrawCross(BYTE* pBits, int BytesPerPixel, LONG bmWidth, LONG bmHeight, int PixelX, int PixelY)
	{
		int stride = bmWidth + (bmWidth * BytesPerPixel) % 4;

		int StartX, EndX;
		int StartY, EndY;
		StartX = PixelX - 10;
		if (StartX < 0) {
			StartX = 0;
		}
		EndX = PixelX + 10;
		if (EndX > bmWidth) {
			EndX = bmWidth;
		}
		StartY = PixelY - 10;
		if (StartY < 0) {
			StartY = 0;
		}
		EndY = PixelY + 10;
		if (EndY > bmHeight) {
			EndY = bmHeight;
		}

		for (int x = StartX; x < EndX; x++)
		{
			int i = (x + PixelY * stride) * BytesPerPixel;
			pBits[i + 0] = 0;
			pBits[i + 1] = 0;
			pBits[i + 2] = 255;
			if (BytesPerPixel == 4) {
				pBits[i + 3] = 0;
			}
		}

		for (int y = StartY; y < EndY; y++)
		{
			int i = (PixelX + y * stride) * BytesPerPixel;
			pBits[i + 0] = 0;
			pBits[i + 1] = 0;
			pBits[i + 2] = 255;
			if (BytesPerPixel == 4) {
				pBits[i + 3] = 0;
			}
		}
	}

	BOOL GrayscaleBitmap(HBITMAP hBitmap)//Назначение цветовых моделей
	{
		BOOL Result = FALSE;
		BITMAP bitmap;
		BYTE*  pBits;//1байт

		int hX = 0, hY = 0; // highest contrast pixel
		BYTE hValue = 0;

		// coping bitmap bits to buffer for modification
		GetObject(hBitmap, sizeof(bitmap), &bitmap);

		// allocate buffer for bitmap bits, dword=4байта без знака
		DWORD BitmapSizeInBytes = (bitmap.bmWidth * bitmap.bmBitsPixel + 31) / 8 * bitmap.bmHeight;
		//количество байт в массиве цветов, ширина должна быть выровнена по DWORD
		pBits = new BYTE[BitmapSizeInBytes];
		if (pBits)
		{
			if (GetBitmapBits(hBitmap, BitmapSizeInBytes, pBits))
			{
				int BytesPerPixel = bitmap.bmBitsPixel / 8;// подсчет количества пикселей в байтах
				int stride = bitmap.bmWidth + (bitmap.bmWidth * BytesPerPixel) % 4;
				// после палитры записывается  растр в виде байтового массива. 
				//В битовом массиве последовательно записываются байты строк растра. Количество байт в строке должно быть кратно четырем 
				for (int y = 0; y < bitmap.bmHeight; y++)
				{
					for (int x = 0; x < stride; x++)
					{
						int i = (x + y * stride) * BytesPerPixel;
						BYTE gray = BYTE(0.1 * pBits[i + 0] + 0.6 * pBits[i + 1] + 0.3 * pBits[i + 2]);
						pBits[i + 0] = pBits[i + 1] = pBits[i + 2] = gray;
						if (BytesPerPixel == 4) {
							pBits[i + 3] = gray;
						}
						if (gray > hValue)
						{
							hValue = gray;
							hX = x;
							hY = y;
						}
					}
				}
				// mark the highest contrast pixel with red
				int i = (hX + hY * stride) * BytesPerPixel;
				pBits[i + 0] = 0;
				pBits[i + 1] = 0;
				pBits[i + 2] = 255;

				// draw the cross
				DrawCross(pBits, BytesPerPixel, bitmap.bmWidth, bitmap.bmHeight, hX, hY);
				
				SetBitmapBits(hBitmap, BitmapSizeInBytes, pBits);

				PhotoWnd::hcPixelX = hX;
				PhotoWnd::hcPixelY = hY;
				PhotoWnd::BytesPerPixel = BytesPerPixel;
			}

			delete pBits;
			Result = TRUE;
		}
		return Result;
	}
};

DWORD LoadPhoto(LPCTSTR FileName)//32-битное беззнаковое целое; const char*
{
	DWORD Status = NO_ERROR;
	
	HBITMAP         hBitmap;

	if (PhotoWnd::hPhotoBitmap)
	{
		DeleteObject(PhotoWnd::hPhotoBitmap);
		PhotoWnd::hPhotoBitmap = NULL;
	}

	// загрузка растрового изображения из файла
	hBitmap =
		(HBITMAP)LoadImage(
			PhotoWnd::hInst,
			FileName,
			IMAGE_BITMAP, 0, 0, 
			LR_LOADFROMFILE//флаг
		);

	if (hBitmap && PhotoWnd::GrayscaleBitmap(hBitmap))
	{
		PhotoWnd::hPhotoBitmap = hBitmap;
	}
	return Status;
}


HWND CreatePhotoWindow(HINSTANCE hInstance, HWND hParent)//дескриптором экземпляра окна
{
    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"Capture Engine Photo Window Class";
    
    WNDCLASS wc = { };//содержит атрибуты класса окна, которые зарегистрированы функцией RegisterClass.

	PhotoWnd::hInst = hInstance;

    wc.lpfnWndProc   = PhotoWnd::WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    RECT rc;
    GetClientRect(hParent, &rc);

    // Create the window.
    return CreateWindowEx(0, CLASS_NAME, NULL, 
        WS_CHILD | WS_VISIBLE, 
		(rc.left - rc.right) / 2 + 4, //x
		0, //y
		(rc.left - rc.right) / 2 - 2, //cx
		(rc.bottom - rc.top), //cy
        hParent, NULL, hInstance, NULL);
};
