// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <objidl.h>
#include <gdiplus.h>
#include "QueueWindows.h"
using namespace Gdiplus;

// main structure with plugin information, version, name...
typedef struct {
	int version;             // version of the plugin structure
	const char* description; // name/title of the plugin 
	int(*init)();            // function which will be executed on init event
	void(*config)();         // function which will be executed on config event
	void(*quit)();           // function which will be executed on quit event
	HWND hwndParent;         // hwnd of the Winamp client main window (stored by Winamp when dll is loaded)
	HINSTANCE hDllInstance;  // hinstance of this plugin DLL. (stored by Winamp when dll is loaded) 
} winampGeneralPurposePlugin;

int init(void);
void config(void);
void quit(void);

// The instance handle of this DLL.
HINSTANCE g_hInstance = NULL;
// Handle to the Winamp window.
HWND g_hWinampWindow = NULL;

// Plugin version (don't touch this!)
#define GPPHDR_VER 0x10

// Plugin title
#define PLUGIN_NAME "Karaoke Singers Queue Display Plugin"

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

	CreateWindows();

	return 0;
}

void config() {
}

void quit() {
	DestroyWindows();

	::GdiplusShutdown(g_gdiPlusToken);
	::SetWindowLong(plugin.hwndParent, GWL_WNDPROC, (LONG)g_pOriginalWndProc);
}