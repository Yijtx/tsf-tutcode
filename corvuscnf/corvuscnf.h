﻿
#ifndef CORVUSCNF_H
#define CORVUSCNF_H

#define TEXTSERVICE_NAME	L"CorvusSKK"
#define TEXTSERVICE_VER		L"0.6.0"

//for resource
#define RC_AUTHOR			"Corvus Solis"
#define RC_PRODUCT			"CorvusSKK"
#define RC_FILE				"corvuscnf"
#define RC_VERSION			"0.6.0"
#define RC_VERSION_D		0,6,0,0

#ifndef _DEBUG
#define TEXTSERVICE_DESC	TEXTSERVICE_NAME
#else
#define TEXTSERVICE_DESC	TEXTSERVICE_NAME L"_DEBUG"
#endif

#define CORVUSCNFMUTEX	TEXTSERVICE_DESC L"_CNF_Mutex"

#define MAX_SKKSERVER_HOST	(255+1)
#define MAX_SKKSERVER_PORT	(5+1)

#define CONV_POINT_NUM	32

#define KEYRELEN		256

// configure
void CreateConfigPath();
void CreateProperty(HINSTANCE hInst);

// configureDictionary
void LoadDictionary(HWND hwnd);
void SaveDictionary(HWND hwnd);
void MakeSKKDic(void);

// configureConv
void LoadCheckButton(HWND hDlg, int nIDDlgItem, LPCWSTR lpAppName, LPCWSTR lpKeyName);
void SaveCheckButton(HWND hDlg, int nIDDlgItem, LPCWSTR lpAppName, LPCWSTR lpKeyName);
void LoadKeyMap(HWND hDlg, int nIDDlgItem, LPCWSTR lpKeyName, LPCWSTR lpDefault);
void SaveKeyMap(HWND hDlg, int nIDDlgItem, LPCWSTR lpKeyName);
void LoadConvPoint(HWND hwnd);
void SaveConvPoint(HWND hwnd);
void LoadKana(HWND hwnd);
void SaveKana(HWND hwnd);
void LoadJLatin(HWND hwnd);
void SaveJLatin(HWND hwnd);

extern const WCHAR *TextServiceDesc;

extern const WCHAR *RccsUNICODE;
extern const WCHAR *WccsUNICODE;

// ファイルパス
extern WCHAR pathconfig[MAX_PATH];		//設定
extern WCHAR pathconfdic[MAX_PATH];		//SKK辞書リスト
extern WCHAR pathconfcvpt[MAX_PATH];	//変換位置指定
extern WCHAR pathconfkana[MAX_PATH];	//ローマ字仮名変換表
extern WCHAR pathconfjlat[MAX_PATH];	//ASCII全英変換表
extern WCHAR pathskkcvdic[MAX_PATH];	//取込SKK辞書
extern WCHAR pathskkcvidx[MAX_PATH];	//取込SKK辞書インデックス

//キー設定
typedef struct {
	int idd;
	WCHAR *keyName;
	WCHAR *defaultValue;
} KEYMAP;

#endif
