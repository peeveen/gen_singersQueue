#include "pch.h"
#include <objidl.h>
#include <gdiplus.h>
#include <gdipluscolor.h>
#include <gdiplusenums.h>
#include "QueueWindows.h"
#include "QueueData.h"
using namespace Gdiplus;

#define HEADER_MARGIN (10)

// The DC of the screen.
HDC g_hScreenDC = NULL;
// The DC of the queue graphics
HDC g_hQueueDC = NULL;
HBITMAP g_hQueueBitmap = NULL;
// Blend function params for queue window.
BLENDFUNCTION g_blendFn = { AC_SRC_OVER ,0,255,AC_SRC_ALPHA };

const WCHAR* g_pszHeaderText = L"SINGERS";
HANDLE g_hHeaderMovementThread = NULL;
HANDLE g_hStopHeaderMovementEvent = NULL;
int g_nHeaderXOffset = 0;
float g_nHeaderWidth = 0.0f;
bool g_bHeaderMovingRight = true;

void UpdateQueueWindow() {
	RECT windowRect;
	::GetWindowRect(g_hQueueWindow, &windowRect);
	int width = windowRect.right - windowRect.left;
	int height = windowRect.bottom - windowRect.top;
	POINT gfxSourcePoint = { 0,0 };
	SIZE s = { width,height };
	::UpdateLayeredWindow(g_hQueueWindow, g_hScreenDC, NULL, &s, g_hQueueDC, &gfxSourcePoint, 0, &g_blendFn, ULW_ALPHA);
}

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

void DrawHeader(bool updateWindow) {
	Graphics g(g_hQueueDC);
	g.SetSmoothingMode(SmoothingModeAntiAlias8x8);

	RECT windowRect;
	::GetWindowRect(g_hQueueWindow, &windowRect);
	int width = windowRect.right - windowRect.left;
	int height = windowRect.bottom - windowRect.top;

	Rect windowRect2(0, 0, width, height - 2);
	Rect headerRect(0, 0, width, 40);
	g.SetClip(headerRect);
	g.Clear(Color::MakeARGB(0, 0, 0, 0));
	SolidBrush headerBrush(Color::MakeARGB(192, 0, 0, 0));
	FillRoundRectangle(&g, &headerBrush, windowRect2, 4);

	FontFamily fontFamily(L"Segoe UI");
	Font font(&fontFamily, 24, FontStyleBold, UnitPixel);
	PointF p = { (REAL)HEADER_MARGIN+g_nHeaderXOffset,4 };
	RectF headerTextRect;
	if (g_nHeaderWidth == 0) {
		g.MeasureString(g_pszHeaderText, -1, &font, p, &headerTextRect);
		g_nHeaderWidth = headerTextRect.Width;
	}
	SolidBrush textBrush(Color::MakeARGB(254, 255, 255, 255));
	g.DrawString(g_pszHeaderText, -1, &font, p, &textBrush);
	if(updateWindow)
		UpdateQueueWindow();
}

void DrawList(bool updateWindow) {
	RECT windowRect;
	::GetWindowRect(g_hQueueWindow, &windowRect);
	int width = windowRect.right - windowRect.left;
	int height = windowRect.bottom - windowRect.top;	Graphics g(g_hQueueDC);

	g.SetSmoothingMode(SmoothingModeAntiAlias8x4);

	Rect windowRect2(0, 0, width, height - 2);

	Rect listRect(0, 40, width, height);
	g.SetClip(listRect);
	g.Clear(Color::MakeARGB(0, 0, 0, 0));
	SolidBrush listBrush(Color::MakeARGB(192, 24, 24, 24));
	FillRoundRectangle(&g, &listBrush, windowRect2, 8);

	FontFamily fontFamily(L"Segoe UI");
	PointF p = { 0,40 };
	Font moreFont(&fontFamily, 24, FontStyleBoldItalic, UnitPixel);
	RectF r(0, 0, 0, 0);
	RectF moreBounds;
	g.MeasureString(L"+ MORE", -1, &moreFont, r, &moreBounds);
	for (int f = 0; f < g_nSingers; ++f) {
		int remainingSpace = (int)(height - p.Y);
		int brighten = g_ppSingers[f]->bSongs ? 160 : 0;
		SolidBrush textBrush(Color::MakeARGB(254, 95 + brighten, 95 + brighten, 95 + brighten));
		if ((remainingSpace < 2 * moreBounds.Height)&&(f!=g_nSingers-1)) {
			int singersRemaining = g_nSingers - f;
			WCHAR szBuffer[48];
			wsprintf(szBuffer, L"+ %d MORE", singersRemaining);
			g.MeasureString(szBuffer, -1, &moreFont, r, &moreBounds);
			SolidBrush moreBrush(Color::MakeARGB(254, 200,255,255));
			p.X = (width - moreBounds.Width) / 2;
			g.DrawString(szBuffer, -1, &moreFont, p, &moreBrush);
			break;
		}
		else
		{
			p.X = (width - g_ppSingers[f]->stringSize.Width) / 2;
			RectF textBounds;
			float fontSize = 24;
			do {
				Font font(&fontFamily, fontSize, FontStyleBold, UnitPixel);
				g.MeasureString(g_ppSingers[f]->szName, -1, &font, r, &textBounds);
				g_ppSingers[f]->stringSize = { textBounds.Width,textBounds.Height };
				g_ppSingers[f]->fontSize = fontSize;
				--fontSize;
			} while (g_ppSingers[f]->stringSize.Width > width);
			Font font(&fontFamily, g_ppSingers[f]->fontSize, FontStyleBold, UnitPixel);
			SolidBrush textBrush(Color::MakeARGB(254, 95 + brighten, 95 + brighten, 95 + brighten));
			g.DrawString(g_ppSingers[f]->szName, -1, &font, p, &textBrush);
			p.Y += g_ppSingers[f]->stringSize.Height;
			p.Y -= 6;
		}
	}
	if(updateWindow)
		UpdateQueueWindow();
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
		DrawHeader(false);
		DrawList(false);
		UpdateQueueWindow();
	}
}

bool CreateQueueDC() {
	g_hQueueDC = ::CreateCompatibleDC(g_hScreenDC);
	if (g_hQueueDC)
		return true;
	return false;
}

void ResetHeaderPosition() {
	RECT r;
	::GetWindowRect(g_hQueueWindow, &r);
	int width = r.right - r.left;
	g_nHeaderXOffset = (int)((width - g_nHeaderWidth) / 2);
}

void MoveHeader() {
	RECT r;
	::GetWindowRect(g_hQueueWindow, &r);
	int width = r.right - r.left;
	if (g_bHeaderMovingRight)
		++g_nHeaderXOffset;
	else
		--g_nHeaderXOffset;
	g_bHeaderMovingRight = !g_nHeaderXOffset || (g_bHeaderMovingRight && (width - (g_nHeaderWidth + g_nHeaderXOffset) > HEADER_MARGIN*2));
	DrawHeader(true);
}

DWORD WINAPI MoveHeaderThread(LPVOID pParams) {
	while (::WaitForSingleObject(g_hStopHeaderMovementEvent, 200) == WAIT_TIMEOUT)
		MoveHeader();
	return 0;
}

void CreateGraphics() {
	g_hScreenDC = ::GetDC(NULL);
	CreateQueueDC();
	g_hStopHeaderMovementEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	g_hHeaderMovementThread = ::CreateThread(NULL, 0, MoveHeaderThread, NULL, 0, NULL);
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
	::SetEvent(g_hStopHeaderMovementEvent);
	::WaitForSingleObject(g_hHeaderMovementThread, INFINITE);
	::CloseHandle(g_hStopHeaderMovementEvent);
}