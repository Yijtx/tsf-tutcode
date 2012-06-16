﻿
#include "corvussrv.h"

#define BUFSIZE 0x2000	// -> KeyHandlerDictionary.cpp

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HANDLE SrvStart();

CRITICAL_SECTION csUserDataSave;
BOOL bUserDicChg;
OSVERSIONINFOW ovi;
ULARGE_INTEGER ftConfig;
#ifdef _DEBUG
HWND hwndEdit;
HFONT hFont;
#endif
HINSTANCE hInst;
HANDLE hMutex;
HANDLE hThreadSrv;
BOOL bSrvThreadExit;

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASSEX wcex;
	HWND hWnd;
	WSADATA wsaData;
	PSECURITY_DESCRIPTOR psd;
	SECURITY_ATTRIBUTES sa;

	_wsetlocale(LC_ALL, L"japanese");

	ZeroMemory(&ovi, sizeof(ovi));
	ovi.dwOSVersionInfoSize = sizeof(ovi);
	GetVersionEx(&ovi);

	CreateConfigPath();

	ConvertStringSecurityDescriptorToSecurityDescriptorW(krnlobjsddl, SDDL_REVISION_1, &psd, NULL);
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = psd;
	sa.bInheritHandle = FALSE;

	hMutex = CreateMutexW(&sa, FALSE, srvmutexname);
	if(hMutex == NULL || GetLastError() == ERROR_ALREADY_EXISTS)
	{
		LocalFree(psd);
		return 0;
	}

	LocalFree(psd);

	ftConfig.QuadPart = 0;
	LoadConfig();

	WSAStartup(WINSOCK_VERSION, &wsaData);

	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.hInstance		= hInstance;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW);
	wcex.lpszClassName	= TextServiceDesc;
	RegisterClassEx(&wcex);

	hInst = hInstance;

#ifdef _DEBUG
	hWnd = CreateWindowW(TextServiceDesc, TextServiceDesc,
		WS_OVERLAPPED | WS_SYSMENU | WS_MINIMIZEBOX,
		0, 0, 400, 800, NULL, NULL, hInstance, NULL);
#else
	hWnd = CreateWindowW(TextServiceDesc, TextServiceDesc, WS_POPUP,
		0, 0, 0, 0, NULL, NULL, hInstance, NULL);
#endif

	if(!hWnd)
	{
		return 0;
	}

#ifdef _DEBUG
	//ShowWindow(hWnd, nCmdShow);
	ShowWindow(hWnd, SW_MINIMIZE);
	UpdateWindow(hWnd);
#endif

	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(!TranslateAccelerator(msg.hwnd, NULL, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	WSACleanup();

	return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HANDLE hThreadSave;
	HANDLE hPipe;
#ifdef _DEBUG
	RECT r;
	HDC hDC;
#endif

	switch (message)
	{
	case WM_CREATE:
#ifdef _DEBUG
		GetClientRect(hWnd, &r);
		hwndEdit = CreateWindowW(L"EDIT", L"",
			WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_READONLY,
			0, 0, r.right, r.bottom, hWnd, NULL, hInst, NULL);
		hDC = GetDC(hwndEdit);
		hFont = CreateFontW(-MulDiv(12, GetDeviceCaps(hDC, LOGPIXELSY), 72), 0, 0, 0,
			FW_NORMAL, FALSE, FALSE, FALSE, SHIFTJIS_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH,
			L"Meiryo");
		SendMessage(hwndEdit, WM_SETFONT, (WPARAM)hFont, 0);
		ReleaseDC(hwndEdit, hDC);
#endif
		InitializeCriticalSection(&csUserDataSave);

		bUserDicChg = FALSE;
		LoadUserDic();

		bSrvThreadExit = FALSE;
		hThreadSrv = SrvStart();
		if(hThreadSrv == NULL)
		{
			DestroyWindow(hWnd);
		}
		break;

	case WM_DESTROY:
	case WM_ENDSESSION:
#ifdef _DEBUG
		DeleteObject(hFont);
#endif
		hThreadSave = StartSaveUserDicEx();
		WaitForSingleObject(hThreadSave, INFINITE);
		CloseHandle(hThreadSave);

		bSrvThreadExit = TRUE;
		hPipe = CreateFileW(pipename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL, OPEN_EXISTING, SECURITY_SQOS_PRESENT | SECURITY_EFFECTIVE_ONLY | SECURITY_IDENTIFICATION, NULL);
		if(hPipe != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hPipe);
			WaitForSingleObject(hThreadSrv, INFINITE);
		}

		CloseHandle(hThreadSrv);

		DeleteCriticalSection(&csUserDataSave);

		ReleaseMutex(hMutex);
		CloseHandle(hMutex);

		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}
	return 0;
}

//search candidate
//	request	"1\n<key>\n"
//	reply	"1\n<candidate>\t<annotation>\n...\n":hit, "4":nothing
//search key for complement
//	request	"8\n<key>\n"
//	reply	"1\n<key>\t\n...\n":hit, "4":nothing
//add candidate (complement off)
//	request	"A\n<key>\t<candidate>\t<annotation>\n"
//	reply	"1"
//add candidate (complement on)
//	request	"B\n<key>\t<candidate>\t<annotation>\n"
//	reply	"1"
//delete candidate
//	request	"D\n<key>\t<candidate>\n"
//	reply	"1"
//save user dictionary
//	request	"S"
//	reply	"1"

void SrvProc(WCHAR *wbuf, size_t size)
{
	CANDIDATES candidates;
	CANDIDATES::iterator candidates_itr;
	const wchar_t seps[]   = L"\t\n";
	wchar_t *token = NULL;
	wchar_t *next_token = NULL;
	std::wstring key;
	std::wstring candidate;
	std::wstring annotation;

	switch(wbuf[0])
	{
	case REQ_SEARCH:
		wbuf[wcslen(wbuf) - 1] = L'\0';
		ConvDictionary(std::wstring(&wbuf[2]), candidates, wbuf[0]);
		if(!candidates.empty())
		{
			wbuf[0] = REP_OK;
			wbuf[1] = L'\n';
			wbuf[2] = L'\0';
			for(candidates_itr = candidates.begin(); candidates_itr != candidates.end(); candidates_itr++)
			{
				wcsncat_s(wbuf, size, candidates_itr->first.c_str(), _TRUNCATE);
				wcsncat_s(wbuf, size, L"\t", _TRUNCATE);
				wcsncat_s(wbuf, size, candidates_itr->second.c_str(), _TRUNCATE);
				wcsncat_s(wbuf, size, L"\n", _TRUNCATE);
			}
		}
		else
		{
			wbuf[0] = REP_FALSE;
			wbuf[1] = L'\0';
		}
		break;

	case REQ_COMPLEMENT:
		wbuf[wcslen(wbuf) - 1] = L'\0';
		ConvComplement(std::wstring(&wbuf[2]), candidates);
		if(!candidates.empty())
		{
			wbuf[0] = REP_OK;
			wbuf[1] = L'\n';
			wbuf[2] = L'\0';
			for(candidates_itr = candidates.begin(); candidates_itr != candidates.end(); candidates_itr++)
			{
				wcsncat_s(wbuf, size, candidates_itr->first.c_str(), _TRUNCATE);
				wcsncat_s(wbuf, size, L"\t\n", _TRUNCATE);
			}
		}
		else
		{
			wbuf[0] = REP_FALSE;
			wbuf[1] = L'\0';
		}
		break;

	case REQ_USER_ADD_0:
	case REQ_USER_ADD_1:
		token = wcstok_s(&wbuf[2], seps, &next_token);
		if(token != NULL)
		{
			key.assign(token);
			token = wcstok_s(NULL, seps, &next_token);
		}
		if(token != NULL)
		{
			candidate.assign(token);
			token = wcstok_s(NULL, seps, &next_token);
		}
		if(token != NULL)
		{
			annotation.assign(token);
		}

		AddUserDic(key, candidate, annotation, wbuf[0]);

		wbuf[0] = REP_OK;
		wbuf[1] = L'\0';
		break;

	case REQ_USER_DEL:
		token = wcstok_s(&wbuf[2], seps, &next_token);
		if(token != NULL)
		{
			key.assign(token);
			token = wcstok_s(NULL, seps, &next_token);
		}
		if(token != NULL)
		{
			candidate.assign(token);
		}

		DelUserDic(key, candidate);

		wbuf[0] = REP_OK;
		wbuf[1] = L'\0';
		break;

	case REQ_USER_SAVE:
		StartSaveUserDic();

		wbuf[0] = REP_OK;
		wbuf[1] = L'\0';
		break;

	default:
		wbuf[0] = L'0';
		wbuf[1] = L'\0';
		break;
	}
}

unsigned int __stdcall SrvThread(void *p)
{
	HANDLE hPipe = (HANDLE)p;
	WCHAR wbuf[BUFSIZE];
	DWORD bytesRead, bytesWrite;
	BOOL bRet;
	HANDLE hFile;
	ULARGE_INTEGER ft;
#ifdef _DEBUG
	std::wstring dedit;
	std::wregex re(L"\n");
	std::wstring fmt(L"\r\n");
#endif

	while(true)
	{
		if(ConnectNamedPipe(hPipe, NULL) == FALSE)
		{
			DisconnectNamedPipe(hPipe);
			break;
		}

		if(bSrvThreadExit)
		{
			DisconnectNamedPipe(hPipe);
			break;
		}

		ft.QuadPart = 0;
		hFile = CreateFileW(pathconfigxml, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			GetFileTime(hFile, NULL, NULL, (FILETIME*)&ft);
			CloseHandle(hFile);
		}
		if(ft.QuadPart != ftConfig.QuadPart)
		{
			ftConfig = ft;
			LoadConfig();
		}

		bytesRead = 0;
		ZeroMemory(wbuf, sizeof(wbuf));

		bRet = ReadFile(hPipe, wbuf, sizeof(wbuf), &bytesRead, NULL);

		if(bRet == FALSE || bytesRead == 0)
		{
			DisconnectNamedPipe(hPipe);
			continue;
		}

#ifdef _DEBUG
		dedit.assign(wbuf);
		dedit.append(L"\n");
		SetWindowTextW(hwndEdit, dedit.c_str());
#endif

		SrvProc(wbuf, _countof(wbuf));

#ifdef _DEBUG
		dedit.append(wbuf);
		dedit.append(L"\n");
		dedit = std::regex_replace(dedit, re, fmt);
		SetWindowTextW(hwndEdit, dedit.c_str());
#endif

		bRet = WriteFile(hPipe, wbuf, (DWORD)(wcslen(wbuf)*sizeof(WCHAR)), &bytesWrite, NULL);

		if(bRet)
		{
			FlushFileBuffers(hPipe);
		}

		DisconnectNamedPipe(hPipe);
	}

	CloseHandle(hPipe);
	
	return 0;
}

HANDLE SrvStart()
{
	PSECURITY_DESCRIPTOR psd;
	SECURITY_ATTRIBUTES sa;
	HANDLE hPipe;
	HANDLE hThread = NULL;

	ConvertStringSecurityDescriptorToSecurityDescriptorW(krnlobjsddl, SDDL_REVISION_1, &psd, NULL);
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = psd;
	sa.bInheritHandle = FALSE;

	hPipe = CreateNamedPipeW(pipename, PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE,
		PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT, 1,
		BUFSIZE*sizeof(WCHAR), BUFSIZE*sizeof(WCHAR), 0, &sa);

	LocalFree(psd);

	if(hPipe != INVALID_HANDLE_VALUE)
	{
		hThread = (HANDLE)_beginthreadex(NULL, 0, SrvThread, hPipe, 0, NULL);
	}

	if(hThread == NULL)
	{
		CloseHandle(hPipe);
	}

	return hThread;
}