#include "pch.h"
#include <objidl.h>
#include <gdiplus.h>
#include <gdipluscolor.h>
#include <gdiplusenums.h>
#include "QueueWindows.h"
#include "QueueData.h"
using namespace Gdiplus;

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

void DrawQueueWindow() {
	RECT windowRect;
	::GetWindowRect(g_hQueueWindow, &windowRect);
	int width = windowRect.right - windowRect.left;
	int height = windowRect.bottom - windowRect.top;

	Graphics g(g_hQueueDC);
	g.SetSmoothingMode(SmoothingModeAntiAlias8x8);
	Rect windowRect2(0, 0, width, height - 2);
	Rect headerRect(0, 0, width, 40);
	g.SetClip(headerRect);
	SolidBrush headerBrush(Color::MakeARGB(192, 0, 0, 0));
	FillRoundRectangle(&g, &headerBrush, windowRect2, 4);
	Rect listRect(0, 40, width, height);
	g.SetClip(listRect);
	SolidBrush listBrush(Color::MakeARGB(192, 24, 24, 24));
	FillRoundRectangle(&g, &listBrush, windowRect2, 8);
	g.ResetClip();
}

void DrawHeader() {
	Graphics g(g_hQueueDC);
	g.SetSmoothingMode(SmoothingModeAntiAlias8x8);
	FontFamily fontFamily(L"Segoe UI");
	Font font(&fontFamily, 24, FontStyleBold, UnitPixel);
	SolidBrush textBrush(Color::MakeARGB(254, 255, 255, 255));
	PointF p = { 10,4 };
	g.DrawString(L"SINGERS", -1, &font, p, &textBrush);
}

void DrawList() {
	Graphics g(g_hQueueDC);

	RECT windowRect;
	::GetWindowRect(g_hQueueWindow, &windowRect);
	int width = windowRect.right - windowRect.left;

	g.SetSmoothingMode(SmoothingModeAntiAlias8x4);
	FontFamily fontFamily(L"Segoe UI");
	PointF p = { 0,40 };// +(28 * f) + 
	for (int f = 0; f < g_nSingers; ++f) {
		p.X = (width - g_ppSingers[f]->stringSize.Width) / 2;
		int brighten = g_ppSingers[f]->bSongs ? 160 : 0;
		Font font(&fontFamily, g_ppSingers[f]->fontSize, FontStyleBold, UnitPixel);
		SolidBrush textBrush(Color::MakeARGB(254, 95 + brighten, 95 + brighten, 95 + brighten));
		g.DrawString(g_ppSingers[f]->szName, -1, &font, p, &textBrush);
		p.Y += g_ppSingers[f]->stringSize.Height;
		p.Y -= 6;
	}
}

void RecreateDisplay() {
	RECT r;
	::GetWindowRect(g_hQueueWindow, &r);
	int width = r.right - r.left;
	int height = r.bottom - r.top;
	g_hQueueBitmap = ::CreateCompatibleBitmap(g_hScreenDC, width, height);
	if (g_hQueueBitmap) {
		HBITMAP hOldBitmap = (HBITMAP)::SelectObject(g_hQueueDC, g_hQueueBitmap);
		if (hOldBitmap)
			::DeleteObject(hOldBitmap);
		DrawQueueWindow();
		DrawHeader();
		DrawList();
		POINT gfxSourcePoint = { 0,0 };
		SIZE s = { width,height };
		::UpdateLayeredWindow(g_hQueueWindow, g_hScreenDC, NULL, &s, g_hQueueDC, &gfxSourcePoint, 0, &g_blendFn, ULW_ALPHA);
	}
}

bool CreateQueueDC() {
	g_hQueueDC = ::CreateCompatibleDC(g_hScreenDC);
	if (g_hQueueDC)
		return true;
	return false;
}

void CreateGraphics() {
	g_hScreenDC = ::GetDC(NULL);
	CreateQueueDC();
}

void DeleteDCAndBitmap(HDC hDC, HBITMAP hBitmap) {
	if (hDC)
		::DeleteDC(hDC);
	if (hBitmap)
		::DeleteObject(hBitmap);
}

void DestroyGraphics() {
	::ReleaseDC(NULL, g_hScreenDC);
	DeleteDCAndBitmap(g_hQueueDC, g_hQueueBitmap);
}