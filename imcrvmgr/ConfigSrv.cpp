﻿
#include "common.h"
#include "configxml.h"
#include "corvussrv.h"

LPCWSTR TextServiceDesc = TEXTSERVICE_DESC;

// ファイルパス
WCHAR pathconfigxml[MAX_PATH];		//設定
WCHAR pathuserdicxml[MAX_PATH];		//ユーザ辞書
WCHAR pathskkcvdicxml[MAX_PATH];	//取込SKK辞書
WCHAR pathskkcvdicidx[MAX_PATH];	//取込SKK辞書インデックス

WCHAR krnlobjsddl[MAX_KRNLOBJNAME];		//SDDL
WCHAR pipename[MAX_KRNLOBJNAME];		//名前付きパイプ
WCHAR srvmutexname[MAX_KRNLOBJNAME];	//ミューテックス

// 辞書サーバ設定
BOOL serv;		//SKK辞書サーバを使用する
WCHAR host[MAX_SKKSERVER_HOST] = {L'\0'};	//ホスト
WCHAR port[MAX_SKKSERVER_PORT] = {L'\0'};	//ポート
DWORD timeout;	//タイムアウト

void CreateConfigPath()
{
	WCHAR appdata[MAX_PATH];

	pathconfigxml[0] = L'\0';
	pathuserdicxml[0] = L'\0';
	pathskkcvdicxml[0] = L'\0';
	pathskkcvdicidx[0] = L'\0';

	if(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, NULL, appdata) != S_OK)
	{
		appdata[0] = L'\0';
		return;
	}

	wcsncat_s(appdata, L"\\", _TRUNCATE);
	wcsncat_s(appdata, TextServiceDesc, _TRUNCATE);
	wcsncat_s(appdata, L"\\", _TRUNCATE);

	_wmkdir(appdata);

	wcsncpy_s(pathconfigxml, appdata, _TRUNCATE);
	wcsncat_s(pathconfigxml, fnconfigxml, _TRUNCATE);

	wcsncpy_s(pathuserdicxml, appdata, _TRUNCATE);
	wcsncat_s(pathuserdicxml, fnuserdicxml, _TRUNCATE);

	wcsncpy_s(pathskkcvdicxml, appdata, _TRUNCATE);
	wcsncat_s(pathskkcvdicxml, fnskkcvdicxml, _TRUNCATE);

	wcsncpy_s(pathskkcvdicidx, appdata, _TRUNCATE);
	wcsncat_s(pathskkcvdicidx, fnskkcvdicidx, _TRUNCATE);

	HANDLE hToken;
	PTOKEN_USER pTokenUser;
	DWORD dwLength;
	LPWSTR pszUserSid = L"";
	WCHAR szDigest[32+1];
	MD5_DIGEST digest;

	ZeroMemory(krnlobjsddl, sizeof(krnlobjsddl));
	ZeroMemory(pipename, sizeof(pipename));
	ZeroMemory(szDigest, sizeof(szDigest));

	if(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
	{
		GetTokenInformation(hToken, TokenUser, NULL, 0, &dwLength);
		pTokenUser = (PTOKEN_USER)LocalAlloc(LPTR, dwLength);

		if(GetTokenInformation(hToken, TokenUser, pTokenUser, dwLength, &dwLength))
		{
			ConvertSidToStringSidW(pTokenUser->User.Sid, &pszUserSid);
		}

		LocalFree(pTokenUser);
		CloseHandle(hToken);
	}

	_snwprintf_s(krnlobjsddl, _TRUNCATE, L"D:(A;;GA;;;RC)(A;;GA;;;SY)(A;;GA;;;BA)(A;;GA;;;%s)", pszUserSid);

	if(IsVersion62AndOver(ovi))
	{
		// for Windows 8 SDDL_ALL_APP_PACKAGES
		wcsncat_s(krnlobjsddl, L"(A;;GA;;;AC)", _TRUNCATE);
	}
	if(IsVersion6AndOver(ovi))
	{
		// (SDDL_MANDATORY_LABEL, SDDL_NO_WRITE_UP, SDDL_ML_LOW)
		wcsncat_s(krnlobjsddl, L"S:(ML;;NW;;;LW)", _TRUNCATE);
	}

	if(GetMD5(&digest, (const BYTE *)pszUserSid, (DWORD)wcslen(pszUserSid)*sizeof(WCHAR)))
	{
		for(int i=0; i<_countof(digest.digest); i++)
		{
			_snwprintf_s(&szDigest[i*2], _countof(szDigest)-i*2, _TRUNCATE, L"%02x", digest.digest[i]);
		}
	}

	_snwprintf_s(pipename, _TRUNCATE, L"%s%s", CORVUSSRVPIPE, szDigest);
	_snwprintf_s(srvmutexname, _TRUNCATE, L"%s%s", CORVUSSRVMUTEX, szDigest);

	LocalFree(pszUserSid);
}

void LoadConfig()
{
	WCHAR hosttmp[MAX_SKKSERVER_HOST];	//ホスト
	WCHAR porttmp[MAX_SKKSERVER_PORT];	//ポート
	std::wstring strxmlval;

	ReadValue(pathconfigxml, SectionServer, Serv, strxmlval);
	serv = _wtoi(strxmlval.c_str());
	if(serv != TRUE && serv != FALSE)
	{
		serv = FALSE;
	}

	//使用しないとき切断する
	if(!serv)
	{
		DisconnectSKKServer();
	}

	ReadValue(pathconfigxml, SectionServer, Host, strxmlval);
	wcsncpy_s(hosttmp, strxmlval.c_str(), _TRUNCATE);

	ReadValue(pathconfigxml, SectionServer, Port, strxmlval);
	wcsncpy_s(porttmp, strxmlval.c_str(), _TRUNCATE);

	ReadValue(pathconfigxml, SectionServer, TimeOut, strxmlval);
	timeout = _wtoi(strxmlval.c_str());
	if(timeout > 60000) timeout = 1000;

	//変更があったら接続し直す
	if(wcscmp(hosttmp, host) != 0 || wcscmp(porttmp, port) != 0)
	{
		wcsncpy_s(host, hosttmp, _TRUNCATE);
		wcsncpy_s(port, porttmp, _TRUNCATE);

		DisconnectSKKServer();
		if(serv)
		{
			ConnectSKKServer();
			GetSKKServerVersion();
		}
	}
}
