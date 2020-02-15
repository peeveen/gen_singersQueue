#pragma once
#include "pch.h"

bool CreateWindows();
void DestroyWindows();

extern HDC g_hQueueWindowDC;
extern HWND g_hQueueWindow;