﻿
#ifndef IMCRVTIP_H
#define IMCRVTIP_H

#include "common.h"

//入力モード
enum InputMode
{
	im_disable = -1,	//無効
	im_default,			//デフォルト
	im_hiragana,		//ひらがな
	im_katakana,		//カタカナ
	im_katakana_ank,	//半角ｶﾀｶﾅ
	im_jlatin,			//全英
	im_ascii			//ASCII
};

#define CKEYMAPNUM		0x80	// 0x00-0x7F
#define VKEYMAPNUM		0x100	// 0x00-0xFF

//skk function code
#define SKK_NULL		0x00	// NUL

#define SKK_KANA		0x71	// かな／カナ	q
#define SKK_CONV_CHAR	0x11	// ｶﾅ全英変換	c-q
#define SKK_JLATIN		0x4C	// 全英			L
#define SKK_ASCII		0x6C	// アスキー		l
#define SKK_JMODE		0x0A	// ひらがな		c-j(LF)	(c-q)	(ASCII/全英モード)
#define SKK_ABBREV		0x2F	// abbrev		/
#define SKK_AFFIX		0x3E	// 接辞			> <
#define SKK_NEXT_CAND	0x20	// 次候補		SP	c-n
#define SKK_PREV_CAND	0x78	// 前候補		x	c-p
#define SKK_PURGE_DIC	0x58	// 辞書削除		X
#define SKK_NEXT_COMP	0x09	// 次補完		c-i(HT)
#define SKK_PREV_COMP	0x15	// 前補完		c-u
#define SKK_HINT		0x3B	// 絞り込み		;

#define SKK_CONV_POINT	0x51	// 変換位置		Q ;
#define SKK_DIRECT		0x30	// 直接入力		0-9
#define SKK_ENTER		0x0D	// 確定			c-m(CR)	c-j(LF)
#define SKK_CANCEL		0x07	// 取消			c-g	(c-[)
#define SKK_BACK		0x08	// 後退			c-h(BS)	VK_BACK
#define SKK_DELETE		0x7F	// 削除			DEL	VK_DELETE
#define SKK_VOID		0xFF	// 無効
#define SKK_LEFT		0x02	// 左移動		c-b	VK_LEFT
#define SKK_UP			0x01	// 先頭移動		c-a	VK_UP
#define SKK_RIGHT		0x06	// 右移動		c-f	VK_RIGHT
#define SKK_DOWN		0x05	// 末尾移動		c-e	VK_DOWN
#define SKK_PASTE		0x19	// 貼付			c-y	(c-v)
#define SKK_OTHERIME	0xF1	// 他IME切替
#define SKK_VIESC		0x1B	// Vi Esc
#define SKK_AFTER_DELETER		0xFE	// Deleterによる直前文字列削除後

typedef struct {	//キー設定(文字)
	BYTE keylatin[CKEYMAPNUM];	//全英/アスキー
	BYTE keyjmode[CKEYMAPNUM];	//ひらがな/カタカナ
	BYTE keyvoid[CKEYMAPNUM];	//無効
} CKEYMAP;

typedef struct {	//キー設定(仮想キー)
	BYTE keylatin[VKEYMAPNUM];	//全英/アスキー
	BYTE keyjmode[VKEYMAPNUM];	//ひらがな/カタカナ
	BYTE keyvoid[VKEYMAPNUM];	//無効
} VKEYMAP;

#define CHAR_SKK_HINT	L'\x20'

#define TKB_NEXT_PAGE	L'\uF003'	//next page key on touch-optimized keyboard
#define TKB_PREV_PAGE	L'\uF004'	//previous page key on touch-optimized keyboard

//候補一覧選択キー数
#define MAX_SELKEY		7

#define CL_COLOR_BG		0
#define CL_COLOR_FR		1
#define CL_COLOR_SE		2
#define CL_COLOR_CO		3
#define CL_COLOR_CA		4
#define CL_COLOR_SC		5
#define CL_COLOR_AN		6
#define CL_COLOR_NO		7

extern LPCWSTR TextServiceDesc;
extern LPCWSTR LangbarItemDesc;

extern HINSTANCE g_hInst;

extern const CLSID c_clsidTextService;
extern const GUID c_guidProfile;
extern const GUID c_guidPreservedKeyOn;
extern const GUID c_guidPreservedKeyOff;
extern const GUID c_guidPreservedKeyOnOff;
extern const GUID c_guidLangBarItemButton;
extern const GUID c_guidCandidateListUIElement;

extern const GUID c_guidDisplayAttributeInputMark;
extern const GUID c_guidDisplayAttributeInputText;
extern const GUID c_guidDisplayAttributeInputOkuri;
extern const GUID c_guidDisplayAttributeConvMark;
extern const GUID c_guidDisplayAttributeConvText;
extern const GUID c_guidDisplayAttributeConvOkuri;
extern const GUID c_guidDisplayAttributeConvAnnot;

typedef struct {
	LPCWSTR key;
	const GUID guid;
	const BOOL se;
	const TF_DISPLAYATTRIBUTE da;
} DISPLAYATTRIBUTE_INFO;

extern const DISPLAYATTRIBUTE_INFO c_gdDisplayAttributeInfo[DISPLAYATTRIBUTE_INFO_NUM];

LONG DllAddRef();
LONG DllRelease();

#define IID_IUNK_ARGS(pType) __uuidof(*(pType)), (IUnknown *)pType

// added in Windows 8 SDK
#if (_WIN32_WINNT < 0x0602)

#define EVENT_OBJECT_IME_SHOW               0x8027
#define EVENT_OBJECT_IME_HIDE               0x8028
#define EVENT_OBJECT_IME_CHANGE             0x8029

#define TF_TMF_IMMERSIVEMODE          0x40000000

#define TF_IPP_CAPS_IMMERSIVESUPPORT            0x00010000
#define TF_IPP_CAPS_SYSTRAYSUPPORT              0x00020000

extern const GUID GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT;
extern const GUID GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT;
extern const GUID GUID_LBI_INPUTMODE;

typedef DECLSPEC_UUID("E9967127-FB3C-4978-9008-FB3060D92730")
enum __MIDL_ITfFnGetPreferredTouchKeyboardLayout_0001
{
	TKBLT_UNDEFINED = 0,
	TKBLT_CLASSIC = 1,
	TKBLT_OPTIMIZED = 2
} TKBLayoutType;

MIDL_INTERFACE("5F309A41-590A-4ACC-A97F-D8EFFF13FDFC")
ITfFnGetPreferredTouchKeyboardLayout : public ITfFunction
{
public:
	virtual HRESULT STDMETHODCALLTYPE GetLayout(
		TKBLayoutType *pTKBLayoutType,
		WORD *pwPreferredLayoutId) = 0;
};

#define TKBL_UNDEFINED                 0
#define TKBL_OPT_JAPANESE_ABC                       0x0411

extern const IID IID_ITfFnGetPreferredTouchKeyboardLayout;

#endif //(_WIN32_WINNT < 0x0602)

#endif //IMCRVTIP_H
