// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "Capture.h"

extern CaptureManager *g_pEngine;

// Implements the window procedure for the video preview window.

namespace PhotoWnd//���� ��� �������
{
	HINSTANCE hInst = NULL;//��� ������� ���������, �� ������� �� ����� �������� �� �� ������
	HBITMAP hPhotoBitmap;//�����, ���� �� �������� ��������

	HBRUSH hBackgroundBrush = 0;//�����

	// ����������������� �������
	int hcPixelX = 0;
	int hcPixelY = 0;
	int BytesPerPixel = 0;

	BOOL OnCreate(HWND hwnd, LPCREATESTRUCT lpCreateStruct)//HWND -���������� ����� ��� ����,
		//LPCREATESTRUCT - ������������ � ��������� ���������������� WM_CREATE, ��������� �������� � IPAraum
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
		PAINTSTRUCT ps;// ���������� ��� ����������, ������������ ��� ��������� ���������� ������� ����
		HDC hdc = BeginPaint(hwnd, &ps);//DC ��� ���������

		FillRect(hdc, &ps.rcPaint, hBackgroundBrush);

		if (hPhotoBitmap)//��������� ����������
		{
			BITMAP          bitmap;//��������� ��������
			HDC             hdcMem;//��� ���������
			HGDIOBJ         oldBitmap;//��� 
			RECT rc;

			int x1, y1, cx, cy;
			int delta = 0, deltax = 0, deltay = 0;
			LONG new_width, new_height;

			TCHAR szMessage[80];				// To print message 
			int strLen;

			GetClientRect(hwnd, &rc);//��������� ���������� ������� ������� ���� 
			cx = rc.right - rc.left;//������� ������
			cy = rc.bottom - rc.top;//������ �����

			hdcMem = CreateCompatibleDC(hdc);//CreateCompatibleDC-���������� �����������

			if (hdcMem) {

				// draw photo
				oldBitmap = SelectObject(hdcMem, hPhotoBitmap);// SelectObject-�������� ������ � ��������� ��������� ����������.
				//����� ������ �������� ���������� ���� �� ����

				GetObject(hPhotoBitmap, sizeof(bitmap), &bitmap);//��������� ����

				// ����������, ��� ������ ����������� ��������� ����
				if (cx < bitmap.bmWidth)//���� ������ �������� ������ ������ ���� 
				{
					deltax = bitmap.bmWidth - cx;
				}
				if (cy < bitmap.bmHeight)//���� ����� �������� ������ ����� ���� 
				{
					deltay = bitmap.bmHeight - cy;
				}
				delta = max(deltax, deltay);

				new_width = bitmap.bmWidth - delta;
				new_height = bitmap.bmHeight - delta;

				// center of the window
				// ����� ������ ���� � ������������ �������� �� ������
				x1 = cx / 2;
				y1 = cy / 2;

				//����� ����������� �������
				x1 = x1 - new_width / 2;
				y1 = y1 - new_height / 2;

				int nOldMode = SetStretchBltMode(hdc, COLORONCOLOR);// ������������� ����� ����������� �����������
				//� ������������ ��������� ����������
				StretchBlt(//�������� ������ �����������
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

				SetTextColor(hdc, RGB(255, 0, 0));//������� ��������
				SetBkMode(hdc, TRANSPARENT);// ������������� ������� ����� ���������� ��������� ��������� ����������. 
				//������� ����� ���������� ������������ �������, ������� ��� ��������� � ������� ����, ������� �� ��������
				//��������� �������. ������ ���� ������ �� �����, ��� � ���



				TextOut(hdc, 5, 5, szMessage, strLen);//���������� ������ �������� � �������� �����, ��������� ������� ��������� 
				//�����, ���� ���� � ���� ������,5 � 5-���������� ��������, ������� �� ��������� 


				DeleteDC(hdcMem);// �������� ��������������� ������������ �� ������
			}
		}
		EndPaint(hwnd, &ps);
	}


	void OnSize(HWND hwnd, UINT state, int /*cx*/, int /*cy*/)//��� �������� ����
	{
		if (state == SIZE_RESTORED)//���� ���������
		{
			InvalidateRect(hwnd, NULL, FALSE);//��������� ������������� � ������������ ������� ��������� ����
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
������ ������ ���������� ����������� � ��������. ������ ������ ���� ������ 0.
bmHeight
������ ������ ���������� ����������� � ������ ������. ������ ������ ���� ������ 0.
bmWidthBytes
��������� ����� ������ � ������ ����� ������. ��� �������� ������ ���� ������ ������, ��������� ��������� ��������� (GDI) 
������� ����������� �������� � ����� ���������� ����������� ������ �������� ������ ����� 2 (byte). 
������� �������, bmWidthBytes * 8 ������ ���� ��������� ��������� �������� ����������� ����� 16 ������ ��� ����� 
�������� �������� ����� ������ bmWidth ������� ������ bmBitsPixel.
bmPlanes
��������� ����� ��������� ����� � ��������� ��������.
bmBitsPixel
��������� ���������� �������� ��� ������ � ������ ���������, ����������� ��� ����������� �������.
bmBits
��������� �� ������������ �������� ���� ��� ���������� �����������. ������� bmBits ������ ���� ���������� ������� � ������� �������� 1 �����.
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

	BOOL GrayscaleBitmap(HBITMAP hBitmap)//���������� �������� �������
	{
		BOOL Result = FALSE;
		BITMAP bitmap;
		BYTE*  pBits;//1����

		int hX = 0, hY = 0; // highest contrast pixel
		BYTE hValue = 0;

		// coping bitmap bits to buffer for modification
		GetObject(hBitmap, sizeof(bitmap), &bitmap);

		// allocate buffer for bitmap bits, dword=4����� ��� �����
		DWORD BitmapSizeInBytes = (bitmap.bmWidth * bitmap.bmBitsPixel + 31) / 8 * bitmap.bmHeight;
		//���������� ���� � ������� ������, ������ ������ ���� ��������� �� DWORD
		pBits = new BYTE[BitmapSizeInBytes];
		if (pBits)
		{
			if (GetBitmapBits(hBitmap, BitmapSizeInBytes, pBits))
			{
				int BytesPerPixel = bitmap.bmBitsPixel / 8;// ������� ���������� �������� � ������
				int stride = bitmap.bmWidth + (bitmap.bmWidth * BytesPerPixel) % 4;
				// ����� ������� ������������  ����� � ���� ��������� �������. 
				//� ������� ������� ��������������� ������������ ����� ����� ������. ���������� ���� � ������ ������ ���� ������ ������� 
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

DWORD LoadPhoto(LPCTSTR FileName)//32-������ ����������� �����; const char*
{
	DWORD Status = NO_ERROR;
	
	HBITMAP         hBitmap;

	if (PhotoWnd::hPhotoBitmap)
	{
		DeleteObject(PhotoWnd::hPhotoBitmap);
		PhotoWnd::hPhotoBitmap = NULL;
	}

	// �������� ���������� ����������� �� �����
	hBitmap =
		(HBITMAP)LoadImage(
			PhotoWnd::hInst,
			FileName,
			IMAGE_BITMAP, 0, 0, 
			LR_LOADFROMFILE//����
		);

	if (hBitmap && PhotoWnd::GrayscaleBitmap(hBitmap))
	{
		PhotoWnd::hPhotoBitmap = hBitmap;
	}
	return Status;
}


HWND CreatePhotoWindow(HINSTANCE hInstance, HWND hParent)//������������ ���������� ����
{
    // Register the window class.
    const wchar_t CLASS_NAME[]  = L"Capture Engine Photo Window Class";
    
    WNDCLASS wc = { };//�������� �������� ������ ����, ������� ���������������� �������� RegisterClass.

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
