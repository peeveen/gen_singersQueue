#include "pch.h"
#include <stdio.h>
#include <wchar.h>
#include "QueueGlobals.h"

#define PREF_BUFFER_SIZE (MAX_PATH*2)

// Path to the INI file
WCHAR g_szINIPath[MAX_PATH + 1] = { '\0' };
// Path to the singers file.
WCHAR g_szSingersFilePath[PREF_BUFFER_SIZE] = { '\0' };
// Font to use.
WCHAR g_szFont[PREF_BUFFER_SIZE] = L"Segoe UI";
// Default font size.
int g_nDefaultFontSize = 24;

void TrimLeading(WCHAR* pszString) {
	WCHAR* pszPointer = pszString;
	while (*pszPointer == ' ' || *pszPointer == '\t')
		++pszPointer;
	wcscpy_s(pszString, PREF_BUFFER_SIZE, pszPointer);
}

void SetPref(WCHAR* pszPrefLine, const WCHAR* pszPrefName, void* pDestVal, void (*pFunc)(const WCHAR*, void*)) {
	int len = wcslen(pszPrefName);
	if (!wcsncmp(pszPrefLine, pszPrefName, len)) {
		wcscpy_s(pszPrefLine, PREF_BUFFER_SIZE, pszPrefLine + len);
		TrimLeading(pszPrefLine);
		while (wcslen(pszPrefLine) && pszPrefLine[0] == '=')
			++pszPrefLine;
		TrimLeading(pszPrefLine);
		WCHAR* pszNewLine = wcsrchr(pszPrefLine, '\n');
		if (pszNewLine)
			*pszNewLine = '\0';
		if (wcslen(pszPrefLine))
			pFunc(pszPrefLine, pDestVal);
	}
}

void SetBoolValue(const WCHAR* pszPrefValue, void* pbDestVal) {
	*(bool*)pbDestVal = wcslen(pszPrefValue) && (pszPrefValue[0] == 't' || pszPrefValue[0] == 'y');
}

void SetStringValue(const WCHAR* pszPrefValue, void* pszDestVal) {
	wcscpy_s((WCHAR*)pszDestVal, PREF_BUFFER_SIZE, pszPrefValue);
}

void SetIntValueRadix(const WCHAR* pszPrefValue, int* pnDestVal, int radix) {
	*(int*)pnDestVal = wcstol(pszPrefValue, NULL, radix);
}

void SetIntValue(const WCHAR* pszPrefValue, void* pnDestVal) {
	SetIntValueRadix(pszPrefValue, (int*)pnDestVal, 10);
}

void SetHexIntValue(const WCHAR* pszPrefValue, void* pnDestVal) {
	SetIntValueRadix(pszPrefValue, (int*)pnDestVal, 16);
}

void SetInt(WCHAR* pszPrefLine, const WCHAR* pszPrefName, int* pnDestVal, int nMin, int nMax, bool hex = false) {
	SetPref(pszPrefLine, pszPrefName, pnDestVal, hex ? SetHexIntValue : SetIntValue);
	if (*pnDestVal < nMin)
		*pnDestVal = nMin;
	else if (*pnDestVal > nMax)
		*pnDestVal = nMax;
}

void SetBool(WCHAR* pszPrefLine, const WCHAR* pszPrefName, bool* pbDestVal) {
	SetPref(pszPrefLine, pszPrefName, pbDestVal, SetBoolValue);
}

void SetString(WCHAR* pszPrefLine, const WCHAR* pszPrefName, WCHAR* pszDestVal) {
	SetPref(pszPrefLine, pszPrefName, pszDestVal, SetStringValue);
}

void SetDefaultFontSize(WCHAR* pszPrefLine) {
	SetInt(pszPrefLine, L"defaultfontsize", &g_nDefaultFontSize, 8, 100);
}

void SetSingersFilePath(WCHAR* pszPrefLine) {
	SetString(pszPrefLine, L"singersfilepath", g_szSingersFilePath);
}

void SetFont(WCHAR* pszPrefLine) {
	SetString(pszPrefLine, L"font", g_szFont);
}

bool SetINIPath() {
	g_szINIPath[0] = '\0';
	if (GetModuleFileName(g_hInstance, g_szINIPath, MAX_PATH)) {
		WCHAR* pszLastSlash = wcsrchr(g_szINIPath, '\\');
		if (pszLastSlash) {
			*(pszLastSlash + 1) = '\0';
			wcscat_s(g_szINIPath, L"gen_singersQueue.ini");
			return true;
		}
	}
	return false;
}

bool ReadPrefs() {
	if (SetINIPath()) {
		FILE* pFile = NULL;
		errno_t error = _wfopen_s(&pFile, g_szINIPath, L"rt, ccs=UTF-8");
		if (pFile && !error) {
			WCHAR szBuffer[PREF_BUFFER_SIZE];
			while (fgetws(szBuffer, PREF_BUFFER_SIZE, pFile)) {
				_wcslwr_s(szBuffer);
				TrimLeading(szBuffer);
				SetSingersFilePath(szBuffer);
				SetFont(szBuffer);
				SetDefaultFontSize(szBuffer);
			}
			fclose(pFile);
			return true;
		}
	}
	return false;
}