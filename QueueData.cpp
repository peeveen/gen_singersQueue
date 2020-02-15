#include "pch.h"
#include <stdio.h>
#include <malloc.h>
#include "QueueDefs.h"
#include "QueueWindows.h"
#include "QueueGraphics.h"

Singer** g_ppSingers=NULL;
int g_nSingers = 0;

void ClearSingers() {
	for (int f = 0; f < g_nSingers; ++f)
		free(g_ppSingers[f]);
	g_nSingers = 0;
	free(g_ppSingers);
	g_ppSingers = NULL;
}

void AddSinger(const WCHAR* pszName, bool songs) {
	RECT windowRect;
	::GetWindowRect(g_hQueueWindow, &windowRect);
	int width = windowRect.right - windowRect.left;

	Graphics g(g_hQueueDC);
	FontFamily fontFamily(L"Segoe UI");
	++g_nSingers;
	Singer** ppSingers = (Singer**)malloc(sizeof(Singer*) * g_nSingers);
	if (ppSingers) {
		memcpy(ppSingers, g_ppSingers, sizeof(Singer*) * (g_nSingers - 1));
		Singer* pSinger = ppSingers[g_nSingers - 1] = (Singer*)malloc(sizeof(Singer));
		if (pSinger) {
			free(g_ppSingers);
			g_ppSingers = ppSingers;
			wcscpy_s(pSinger->szName, pszName);
			int nLen = wcslen(pSinger->szName);
			if (pSinger->szName[nLen - 1] == '\n')
				pSinger->szName[nLen - 1] = '\0';
			pSinger->bSongs = songs;
			RectF r(0, 0, 0, 0);
			RectF textBounds;
			float fontSize = 24;
			do {
				Font font(&fontFamily, (REAL)fontSize, FontStyleBold, UnitPixel);
				g.MeasureString(pSinger->szName, -1, &font, r, &textBounds);
				pSinger->stringSize = { textBounds.Width,textBounds.Height };
				pSinger->fontSize = fontSize;
				--fontSize;
			} while (pSinger->stringSize.Width > width);
		}
	}
}

void ReadList() {
	ClearSingers();
	FILE* pFile = NULL;
	errno_t error = _wfopen_s(&pFile, L"C:\\Users\\steve\\Documents\\Rainmeter\\Skins\\KaraokeManager\\KaraokeManager.singers.txt", L"rt");
	if (pFile && !error) {
		WCHAR szBuffer[256];
		while (fgetws(szBuffer, 256, pFile)) {
			bool noSongs = szBuffer[0] == '\t';
			AddSinger(szBuffer + (noSongs ? 1 : 0), !noSongs);
		}
		fclose(pFile);
	}
	DrawList();
}