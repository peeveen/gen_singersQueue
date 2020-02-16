#pragma once
#include "pch.h"

void CreateGraphics();
void DestroyGraphics();
void RecreateDisplay();
void DrawList(bool updateWindow);
void ResetHeaderPosition();

extern HDC g_hQueueDC;