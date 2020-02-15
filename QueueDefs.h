#pragma once
#include "pch.h"
#include <objidl.h>
#include <gdiplus.h>
using namespace Gdiplus;

// Plugin version (don't touch this!)
#define GPPHDR_VER 0x10

// Plugin title
#define PLUGIN_NAME "Karaoke Singers Queue Display Plugin"

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

typedef struct Singer {
	WCHAR szName[256];
	bool bSongs;
	SizeF stringSize;
	float fontSize;
} Singer;