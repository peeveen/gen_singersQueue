#include "pch.h"
#include <windowsx.h>
#include "QueueDefs.h"
#include "QueueGlobals.h"
#include "QueueGraphics.h"
#include "QueueData.h"
#include "resource.h"
#include <objidl.h>
#include <stdio.h>
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

bool CreateWindows() {
	if (RegisterQueueWindowClass() &&
		CreateQueueWindow()) {
		ReadList();
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
}