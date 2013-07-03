﻿
#ifndef IMCRVCNF_H
#define IMCRVCNF_H

#include "common.h"

//for resource
#define RC_FILE				"imcrvcnf"

// imcrvcnf
void CreateProperty(HINSTANCE hInst);

// ConfigCnf
void CreateConfigPath();
BOOL SetFileDacl(LPCWSTR path);

// PropertyConfDictionary
void LoadDictionary(HWND hwnd);
void SaveDictionary(HWND hwnd);
void MakeSKKDic(HWND hwnd);

// PropertyConfConv
void LoadCheckButton(HWND hDlg, int nIDDlgItem, LPCWSTR lpAppName, LPCWSTR lpKeyName);
void SaveCheckButton(HWND hDlg, int nIDDlgItem, LPCWSTR lpAppName, LPCWSTR lpKeyName);
void LoadKeyMap(HWND hDlg, int nIDDlgItem, LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpDefault);
void SaveKeyMap(HWND hDlg, int nIDDlgItem, LPCWSTR lpKeyName);
void LoadPreservedKey(HWND hwnd);
void SavePreservedKey(HWND hwnd);
void LoadConvPoint(HWND hwnd);
void SaveConvPoint(HWND hwnd);
void LoadKana(HWND hwnd);
void SaveKana(HWND hwnd);
void LoadJLatin(HWND hwnd);
void SaveJLatin(HWND hwnd);
void LoadConfigKanaTxt(LPCWSTR path);
void LoadKanaTxt(HWND hwnd, LPCWSTR path);

extern LPCWSTR TextServiceDesc;

extern IXmlWriter *pXmlWriter;
extern IStream *pXmlFileStream;

extern HINSTANCE hInst;
extern OSVERSIONINFOW g_ovi;

extern WCHAR cnfmutexname[MAX_KRNLOBJNAME];	//ミューテックス

// ファイルパス
extern WCHAR pathconfigxml[MAX_PATH];	//設定
extern WCHAR pathskkdic[MAX_PATH];	//取込SKK辞書
extern WCHAR pathskkidx[MAX_PATH];	//取込SKK辞書インデックス

//キー設定
typedef struct {
	int idd;
	LPCWSTR keyName;
	LPCWSTR defaultValue;
} KEYMAPCNF;

#endif //IMCRVCNF_H
