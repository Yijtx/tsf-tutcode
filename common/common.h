﻿
#ifndef COMMON_H
#define COMMON_H

#define TEXTSERVICE_NAME	L"tsf-tutcode"
#define TEXTSERVICE_VER		L"0.2.1"

#ifndef _DEBUG
#define TEXTSERVICE_DESC	TEXTSERVICE_NAME
#else
#define TEXTSERVICE_DESC	TEXTSERVICE_NAME L"_DEBUG"
#endif

//for resource
#define RC_AUTHOR			"KIHARA Hideto"
#define RC_PRODUCT			"tsf-tutcode"
#define RC_VERSION			"0.2.1"
#define RC_VERSION_D		0,2,1,0

#define DICBUFSIZE			0x2000
#define PIPEBUFSIZE			0x2000

#define MAX_KRNLOBJNAME		256
#define CONV_POINT_NUM		256
#define KEYRELEN			256
#define MAX_PRESERVEDKEY	8
#define MAX_SELKEY_C		9

#define MAX_SKKSERVER_HOST	(255+1)
#define MAX_SKKSERVER_PORT	(5+1)

#define MAX_WIDTH_DEFAULT	800

//request
#define REQ_SEARCH			L'1'	//辞書検索
#define REQ_COMPLEMENT		L'8'	//補完
#define REQ_CONVERSION		L'9'	//候補変換
#define REQ_USER_ADD_0		L'A'	//ユーザー辞書追加(送りあり、補完なし)
#define REQ_USER_ADD_1		L'B'	//ユーザー辞書追加(送りなし、補完あり)
#define REQ_USER_DEL_0		L'C'	//ユーザー辞書削除(送りあり、補完なし)
#define REQ_USER_DEL_1		L'D'	//ユーザー辞書削除(送りなし、補完あり)
#define REQ_USER_SAVE		L'S'	//ユーザー辞書保存
#define REQ_BUSHU			L'b'	//部首合成変換
//reply
#define REP_OK				L'1'	//hit
#define REP_FALSE			L'4'	//nothing

#define CORVUSMGREXE		L"imtutmgr.exe"
#define CORVUSCNFEXE		L"imtutcnf.exe"
#ifndef _DEBUG
#define CORVUSKRNLOBJ		L"tsf-tutcode-"
#else
#define CORVUSKRNLOBJ		L"tsf-tutcode-debug-"
#endif
#define CORVUSMGRMUTEX		CORVUSKRNLOBJ L"mgr-"
#define CORVUSCNFMUTEX		CORVUSKRNLOBJ L"cnf-"
#define CORVUSMGRPIPE		L"\\\\.\\pipe\\" CORVUSKRNLOBJ

typedef struct {
	BYTE digest[16];
} MD5_DIGEST;

extern LPCWSTR RccsUNICODE;
extern LPCWSTR WccsUNICODE;
extern LPCWSTR RB;
extern LPCWSTR WB;

extern LPCWSTR fnconfigxml;	//設定
extern LPCWSTR fnuserdic;	//ユーザー辞書
extern LPCWSTR fnskkdic;	//取込SKK辞書
extern LPCWSTR fnskkidx;	//取込SKK辞書インデックス

#define DISPLAYATTRIBUTE_INFO_NUM	7

extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeInputMark;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeInputText;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeInputOkuri;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeConvMark;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeConvText;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeConvOkuri;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeConvAnnot;
extern const BOOL c_daDisplayAttributeSeries[DISPLAYATTRIBUTE_INFO_NUM];

BOOL IsVersion62AndOver();
BOOL GetSidMD5Digest(LPWSTR *ppszDigest);
BOOL GetMD5(MD5_DIGEST *digest, CONST BYTE *data, DWORD datalen);
BOOL GetUserSid(LPWSTR *ppszUserSid);
BOOL GetLogonSid(LPWSTR *ppszLogonSid);

#endif //COMMON_H
