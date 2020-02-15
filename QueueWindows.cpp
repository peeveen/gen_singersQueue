#include "pch.h"
#include <windowsx.h>
#include "QueueGlobals.h"
#include "resource.h"
#include <objidl.h>
#include <stdlib.h>
#include <gdiplus.h>
#include <gdipluscolor.h>
#include <gdiplusenums.h>
using namespace Gdiplus;

// Window class names.
const WCHAR* g_pszQueueWindowClassName = L"SingersQueue";
const WCHAR* g_pszWindowCaption = L"Singers Queue";

// Application icon.
HICON g_hIcon = NULL;
// The window (and it's DC) containing the foreground.
HDC g_hQueueWindowDC = NULL;
HWND g_hQueueWindow = NULL;
// The DC of the screen.
HDC g_hScreenDC = NULL;
// The DC of the queue graphics
HDC g_hQueueDC = NULL;
HBITMAP g_hQueueBitmap = NULL;
// Blend function params for queue window.
BLENDFUNCTION g_blendFn = { AC_SRC_OVER ,0,255,AC_SRC_ALPHA };

void FillRoundRectangle(Gdiplus::Graphics* g, Brush* p, Gdiplus::Rect& rect, UINT8 radius)
{
	if (g == NULL) return;
	GraphicsPath path;
	path.AddLine(rect.X + radius, rect.Y, rect.X + rect.Width - (radius * 2), rect.Y);
	path.AddArc(rect.X + rect.Width - (radius * 2), rect.Y, radius * 2, radius * 2, 270, 90);
	path.AddLine(rect.X + rect.Width, rect.Y + radius, rect.X + rect.Width, rect.Y + rect.Height - (radius * 2));
	path.AddArc(rect.X + rect.Width - (radius * 2), rect.Y + rect.Height - (radius * 2), radius * 2, radius * 2, 0, 90);
	path.AddLine(rect.X + rect.Width - (radius * 2), rect.Y + rect.Height, rect.X + radius, rect.Y + rect.Height);
	path.AddArc(rect.X, rect.Y + rect.Height - (radius * 2), radius * 2, radius * 2, 90, 90);
	path.AddLine(rect.X, rect.Y + rect.Height - (radius * 2), rect.X, rect.Y + radius);
	path.AddArc(rect.X, rect.Y, radius * 2, radius * 2, 180, 90);
	path.CloseFigure();
	g->FillPath(p, &path);
}

void DrawQueueWindow(Graphics *pGraphics) {
	RECT windowRect;
	::GetWindowRect(g_hQueueWindow, &windowRect);
	int width = windowRect.right - windowRect.left;
	int height = windowRect.bottom - windowRect.top;

	Rect windowRect2(0, 0, width, height-2);
	Rect headerRect(0, 0, width, 40);
	pGraphics->SetClip(headerRect);
	SolidBrush headerBrush(Color::MakeARGB(192, 0, 0, 0));
	FillRoundRectangle(pGraphics, &headerBrush, windowRect2, 4);
	Rect listRect(0, 40, width, height);
	pGraphics->SetClip(listRect);
	SolidBrush listBrush(Color::MakeARGB(192, 24, 24, 24));
	FillRoundRectangle(pGraphics, &listBrush, windowRect2, 8);
	pGraphics->ResetClip();
}

void DrawHeader(Graphics* pGraphics) {
	FontFamily fontFamily(L"Segoe UI");
	Font font(&fontFamily, 24, FontStyleBold, UnitPixel);
	SolidBrush textBrush(Color::MakeARGB(254,255,255,255));
	PointF p = { 10,4 };
	pGraphics->DrawString(L"SINGERS", -1, &font, p, &textBrush);
}

void DrawList(Graphics* pGraphics) {

}

void DeleteDCAndBitmap(HDC hDC, HBITMAP hBitmap) {
	if (hDC)
		::DeleteDC(hDC);
	if (hBitmap)
		::DeleteObject(hBitmap);
}

void RecreateDisplay() {
	RECT r;
	::GetWindowRect(g_hQueueWindow, &r);
	int width = r.right-r.left;
	int height = r.bottom - r.top;
	g_hQueueBitmap = ::CreateCompatibleBitmap(g_hScreenDC, width, height);
	if (g_hQueueBitmap) {
		HBITMAP hOldBitmap=(HBITMAP)::SelectObject(g_hQueueDC, g_hQueueBitmap);
		if (hOldBitmap)
			::DeleteObject(hOldBitmap);
		Graphics g(g_hQueueDC);
		g.SetSmoothingMode(SmoothingModeAntiAlias8x8);
		DrawQueueWindow(&g);
		DrawHeader(&g);
		DrawList(&g);
		POINT gfxSourcePoint = { 0,0 };
		SIZE s = { width,height};
		::UpdateLayeredWindow(g_hQueueWindow, g_hScreenDC,NULL, &s, g_hQueueDC, &gfxSourcePoint, 0, &g_blendFn, ULW_ALPHA);
	}
}

LRESULT CALLBACK QueueWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_NCHITTEST: {
		LRESULT hit = DefWindowProc(hwnd, uMsg, wParam, lParam);
		if (hit == HTCLIENT)
			hit = HTCAPTION;
		return hit;
	}
	case WM_CLOSE:
		return 1;
	case WM_SIZE: {
		RecreateDisplay();
		break;
	}
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

ATOM RegisterQueueWindowClass() {
	if (!g_hIcon)
		g_hIcon = ::LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_APPICON));
	if (g_hIcon) {
		WNDCLASSEX wndClass;
		::ZeroMemory(&wndClass, sizeof(WNDCLASSEX));
		wndClass.cbSize = sizeof(WNDCLASSEX);
		wndClass.style = CS_NOCLOSE | CS_PARENTDC | CS_HREDRAW | CS_VREDRAW;
		wndClass.lpfnWndProc = (WNDPROC)QueueWindowProc;
		wndClass.hInstance = g_hInstance;
		wndClass.hIcon = wndClass.hIconSm = g_hIcon;
		wndClass.lpszClassName = g_pszQueueWindowClassName;
		return ::RegisterClassEx(&wndClass);
	}
	return 0;
}

bool CreateQueueWindow() {
	int width = 200;
	int height = 600;
	g_hQueueWindow = CreateWindowEx(
		WS_EX_LAYERED|WS_EX_APPWINDOW,
		g_pszQueueWindowClassName,
		g_pszWindowCaption,
		WS_VISIBLE|WS_THICKFRAME,
		CW_USEDEFAULT, CW_USEDEFAULT,
		width, height,
		// We don't want this window to minimize/restore along with WinAmp, so we don't
		// tell Windows that this is a child of WinAmp.
		NULL,//g_hWinampWindow,
		NULL,
		g_hInstance,
		NULL);
	if (g_hQueueWindow) {
		g_hQueueWindowDC = ::GetDC(g_hQueueWindow);
		if (g_hQueueWindowDC)
			return true;
	}
	return false;
}

void UnregisterWindowClasses() {
	::UnregisterClass(g_pszQueueWindowClassName, g_hInstance);
	if (g_hIcon)
		::DeleteObject(g_hIcon);
}

bool CreateQueueDC() {
	g_hQueueDC = ::CreateCompatibleDC(g_hScreenDC);
	if (g_hQueueDC)
			return true;
	return false;
}

bool CreateWindows() {
	g_hScreenDC = ::GetDC(NULL);
	if (RegisterQueueWindowClass() &&
		CreateQueueWindow() &&
		CreateQueueDC()) {
		RecreateDisplay();
		return true;
	}
	return false;
}

void CloseWindow(HWND hWnd, HDC hDC) {
	if (hDC)
		::ReleaseDC(hWnd, hDC);
	if (hWnd) {
		::CloseWindow(hWnd);
		::DestroyWindow(hWnd);
	}
}

void DestroyWindows() {
	CloseWindow(g_hQueueWindow, g_hQueueWindowDC);
	UnregisterWindowClasses();
	::ReleaseDC(NULL, g_hScreenDC);
	DeleteDCAndBitmap(g_hQueueDC, g_hQueueBitmap);
}