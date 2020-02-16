#include "pch.h"
#include <stdio.h>
#include <malloc.h>
#include "QueueDefs.h"
#include "QueueWindows.h"
#include "QueueGraphics.h"

Singer** g_ppSingers=NULL;
int g_nSingers = 0;

HANDLE g_hFileScannerThread=NULL;
HANDLE g_hStopFileScannerEvent = NULL;

const WCHAR* g_pszSingersFilename = L"C:\\Users\\steve\\Documents\\Rainmeter\\Skins\\KaraokeManager\\KaraokeManager.singers.txt";
FILETIME g_lastModifiedTime = { 0,0 };

void ClearSingers() {
	for (int f = 0; f < g_nSingers; ++f)
		free(g_ppSingers[f]);
	g_nSingers = 0;
	free(g_ppSingers);
	g_ppSingers = NULL;
}

void AddSinger(const WCHAR* pszName, bool songs) {
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
		}
	}
}

void ReadList() {
	ClearSingers();
	FILE* pFile = NULL;
	errno_t error = _wfopen_s(&pFile, g_pszSingersFilename, L"rt");
	if (pFile && !error) {
		WCHAR szBuffer[256];
		while (fgetws(szBuffer, 256, pFile)) {
			bool noSongs = szBuffer[0] == '\t';
			AddSinger(szBuffer + (noSongs ? 1 : 0), !noSongs);
		}
		fclose(pFile);
	}
}

DWORD WINAPI ScanFile(LPVOID pParams) {
	while (::WaitForSingleObject(g_hStopFileScannerEvent, 1000)==WAIT_TIMEOUT) {
		HANDLE hFile=::CreateFile(g_pszSingersFilename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hFile != INVALID_HANDLE_VALUE) {
			FILETIME lastModified;
			if (::GetFileTime(hFile, NULL, NULL, &lastModified)) {
				if ((g_lastModifiedTime.dwHighDateTime < lastModified.dwHighDateTime) || ((g_lastModifiedTime.dwHighDateTime == lastModified.dwHighDateTime) && (g_lastModifiedTime.dwLowDateTime < lastModified.dwLowDateTime))) {
					ReadList();
					DrawList(true);
				}
				g_lastModifiedTime = lastModified;
			}
			::CloseHandle(hFile);
		}
	}
	return 0;
}

void StartFileScanner() {
	g_hStopFileScannerEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	g_hFileScannerThread = ::CreateThread(NULL, 0, ScanFile, NULL, 0, NULL);
}

void StopFileScanner() {
	::SetEvent(g_hStopFileScannerEvent);
	::WaitForSingleObject(g_hFileScannerThread, INFINITE);
	::CloseHandle(g_hStopFileScannerEvent);
	ClearSingers();
}