﻿
#ifndef IMCRVMGR_H
#define IMCRVMGR_H

#include "common.h"

//for resource
#define RC_FILE				"imcrvmgr"

//候補   pair< candidate, annotation >
typedef std::pair< std::wstring, std::wstring > CANDIDATE;
typedef std::vector< CANDIDATE > CANDIDATES;

//ユーザ辞書   pair< key , candidates >
typedef std::map< std::wstring, CANDIDATES > USERDIC;
typedef std::pair< std::wstring, CANDIDATES > USERDICENTRY;

//見出し語順序
typedef std::vector< std::wstring > KEYORDER;

typedef struct {
	USERDIC userdic;
	KEYORDER complements;
	KEYORDER accompaniments;
} USERDATA;

// ConfigSrv
void CreateConfigPath();
void LoadConfig();

// ConvCharacter
void ConvUnicode(const std::wstring &text, CANDIDATES &candidates);
void ConvJISX0213(const std::wstring &text, CANDIDATES &candidates);

// ConvDictionary
void ConvDictionary(const std::wstring &searchkey, CANDIDATES &candidates, WCHAR command);
void ConvComplement(const std::wstring &searchkey, CANDIDATES &candidates);
void AddUserDic(WCHAR command, const std::wstring &searchkey, const std::wstring &candidate, const std::wstring &annotation);
void DelUserDic(WCHAR command, const std::wstring &searchkey, const std::wstring &candidate);
void LoadUserDic();
HANDLE StartSaveUserDicEx();
void StartSaveUserDic();
BOOL LoadSKKUserDic(LPCWSTR path);
BOOL SaveSKKUserDic(LPCWSTR path);

// ConvSKKServer
void ConvSKKServer(const std::wstring &text, CANDIDATES &candidates);
void ConnectSKKServer();
void DisconnectSKKServer();
void GetSKKServerVersion();

extern LPCWSTR TextServiceDesc;

extern CRITICAL_SECTION csUserDataSave;
extern BOOL bUserDicChg;
extern OSVERSIONINFOW ovi;

extern USERDIC userdic;
extern KEYORDER complements;
extern KEYORDER accompaniments;

// ファイルパス
extern WCHAR pathconfigxml[MAX_PATH];
extern WCHAR pathuserdicxml[MAX_PATH];
extern WCHAR pathskkcvdicxml[MAX_PATH];
extern WCHAR pathskkcvdicidx[MAX_PATH];

extern WCHAR krnlobjsddl[MAX_KRNLOBJNAME];	//SDDL
extern WCHAR mgrpipename[MAX_KRNLOBJNAME];	//名前付きパイプ
extern WCHAR mgrmutexname[MAX_KRNLOBJNAME];	//ミューテックス

// 辞書サーバ設定
extern BOOL serv;		//SKK辞書サーバを使用する
extern WCHAR host[MAX_SKKSERVER_HOST];	//ホスト
extern WCHAR port[MAX_SKKSERVER_PORT];	//ポート
extern DWORD encoding;	//エンコーディング
extern DWORD timeout;	//タイムアウト

#endif //IMCRVMGR_H
