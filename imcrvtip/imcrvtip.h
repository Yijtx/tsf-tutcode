﻿
#ifndef IMCRVTIP_H
#define IMCRVTIP_H

#include "common.h"

//入力モード
enum
{
	im_disable = -1,	//無効
	im_default,			//デフォルト
	im_hiragana,		//ひらがな
	im_katakana,		//カタカナ
	im_katakana_ank,	//半角ｶﾀｶﾅ
	im_jlatin,			//全英
	im_ascii			//ASCII
};

//候補   pair< candidate, annotation >
typedef std::pair< std::wstring, std::wstring > CANDIDATEBASE;
//		pair< CANDIDATEBASE(表示用), CANDIDATEBASE(辞書登録用) >
//		例）数値変換の場合 < < "明治四五年", "年号(1868-1912)" >, < "明治#2年", "年号(1868-1912)" > >
typedef std::pair< CANDIDATEBASE, CANDIDATEBASE > CANDIDATE;
typedef std::vector< CANDIDATE > CANDIDATES;

#define KEYMAPNUM		0x80

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
#define SKK_AFTER_DELETER		0xFE	// Deleterによる直前文字列削除後

typedef struct {
	BYTE keylatin[KEYMAPNUM];	//全英/アスキー
	BYTE keyjmode[KEYMAPNUM];	//ひらがな/カタカナ
	BYTE keyvoid[KEYMAPNUM];	//無効
} KEYMAP;

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

extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeInput;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeCandidate;
extern const TF_DISPLAYATTRIBUTE c_daDisplayAttributeAnnotation;

extern const CLSID c_clsidTextService;
extern const GUID c_guidProfile;
extern const GUID c_guidPreservedKeyOn;
extern const GUID c_guidPreservedKeyOff;
extern const GUID c_guidPreservedKeyOnOff;
extern const GUID c_guidLangBarItemButton;
extern const GUID c_guidDisplayAttributeInput;
extern const GUID c_guidDisplayAttributeCandidate;
extern const GUID c_guidDisplayAttributeAnnotation;
extern const GUID c_guidCandidateListUIElement;

LONG DllAddRef();
LONG DllRelease();

#define IID_IUNK_ARGS(pType) __uuidof(*(pType)), (IUnknown *)pType

// for Windows 8
#if 1
#define EVENT_OBJECT_IME_SHOW               0x8027
#define EVENT_OBJECT_IME_HIDE               0x8028
#define EVENT_OBJECT_IME_CHANGE             0x8029

#define TF_TMF_IMMERSIVEMODE          0x40000000

#define TF_IPP_CAPS_IMMERSIVESUPPORT            0x00010000
#define TF_IPP_CAPS_SYSTRAYSUPPORT              0x00020000

extern const GUID GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT;
extern const GUID GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT;
extern const GUID GUID_LBI_INPUTMODE;

// from mozc/win32/tip/tip_text_service.cc
// ITfFnGetPreferredTouchKeyboardLayout is available on Windows 8 SDK and later.
#ifndef TKBL_UNDEFINED
#define TKBL_UNDEFINED                             0x0000
#define TKBL_CLASSIC_TRADITIONAL_CHINESE_PHONETIC  0x0404
#define TKBL_CLASSIC_TRADITIONAL_CHINESE_CHANGJIE  0xF042
#define TKBL_CLASSIC_TRADITIONAL_CHINESE_DAYI      0xF043
#define TKBL_OPT_JAPANESE_ABC                      0x0411
#define TKBL_OPT_KOREAN_HANGUL_2_BULSIK            0x0412
#define TKBL_OPT_SIMPLIFIED_CHINESE_PINYIN         0x0804
#define TKBL_OPT_TRADITIONAL_CHINESE_PHONETIC      0x0404

enum TKBLayoutType {
  TKBLT_UNDEFINED = 0,
  TKBLT_CLASSIC = 1,
  TKBLT_OPTIMIZED = 2
};

extern const IID IID_ITfFnGetPreferredTouchKeyboardLayout;

// Note: "5F309A41-590A-4ACC-A97F-D8EFFF13FDFC" is equivalent to
// IID_ITfFnGetPreferredTouchKeyboardLayout
struct __declspec(uuid("5F309A41-590A-4ACC-A97F-D8EFFF13FDFC"))
ITfFnGetPreferredTouchKeyboardLayout : public ITfFunction {
 public:
  virtual HRESULT STDMETHODCALLTYPE GetLayout(TKBLayoutType *layout_type,
                                              WORD *preferred_layout_id) = 0;
};
#endif // TKBL_UNDEFINED
#endif

#endif //IMCRVTIP_H
