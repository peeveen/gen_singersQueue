// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <objidl.h>
#include <shellapi.h>
#include <gdiplus.h>
#include "QueueGraphics.h"
#include "QueueWindows.h"
#include "QueuePrefs.h"
#include "QueueData.h"
#include "QueueDefs.h"
using namespace Gdiplus;

int init(void);
void config(void);
void quit(void);

// The instance handle of this DLL.
HINSTANCE g_hInstance = NULL;
// Handle to the Winamp window.
HWND g_hWinampWindow = NULL;

// Original WndProc that we have to swap back in at the end of proceedings.
WNDPROC g_pOriginalWndProc;
// This structure contains plugin information, version, name...
winampGeneralPurposePlugin plugin = { GPPHDR_VER,PLUGIN_NAME,init,config,quit,0,0 };
// GDI+ token
ULONG_PTR g_gdiPlusToken;

// This is an export function called by winamp which returns this plugin info.
// We wrap the code in 'extern "C"' to ensure the export isn't mangled if used in a CPP file.
extern "C" __declspec(dllexport) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin() {
  return &plugin;
}

LRESULT CALLBACK SingersQueueWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// Call Winamp Window Proc
	return ::CallWindowProc(g_pOriginalWndProc, hwnd, uMsg, wParam, lParam);
}

int init() {
	GdiplusStartupInput g_gdiPlusStartupInput;
	::GdiplusStartup(&g_gdiPlusToken, &g_gdiPlusStartupInput, NULL);

	g_hInstance = plugin.hDllInstance;
	g_hWinampWindow = plugin.hwndParent;
	g_pOriginalWndProc = (WNDPROC)::SetWindowLong(plugin.hwndParent, GWL_WNDPROC, (LONG)SingersQueueWndProc);

	ReadPrefs();
	CreateWindows();
	CreateGraphics();
	RecreateDisplay();
	StartFileScanner();

	return 0;
}

void config() {
	::ShellExecute(g_hWinampWindow, L"edit", g_szINIPath, NULL, NULL, SW_SHOWDEFAULT);
}

void quit() {
	DestroyWindows();

	::GdiplusShutdown(g_gdiPlusToken);
	::SetWindowLong(plugin.hwndParent, GWL_WNDPROC, (LONG)g_pOriginalWndProc);

	StopFileScanner();
	DestroyWindows();
	DestroyGraphics();
}