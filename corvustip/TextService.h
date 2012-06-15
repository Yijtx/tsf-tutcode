﻿
#ifndef TEXTSERVICE_H
#define TEXTSERVICE_H

#include "corvustip.h"
#include "convtype.h"

class CLangBarItemButton;
class CCandidateList;

class CTextService :
	public ITfTextInputProcessorEx,
	public ITfThreadMgrEventSink,
	public ITfThreadFocusSink,
	public ITfCompartmentEventSink,
	public ITfTextEditSink,
	public ITfKeyEventSink,
	public ITfCompositionSink,
	public ITfDisplayAttributeProvider,
	public ITfFunctionProvider,
	public ITfFnConfigure,
	public ITfFnShowHelp
{
public:
	CTextService();
	~CTextService();
	
	// IUnknown
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	// ITfTextInputProcessor
	STDMETHODIMP Activate(ITfThreadMgr *ptim, TfClientId tid);
	STDMETHODIMP Deactivate();

	// ITfTextInputProcessorEx
	STDMETHODIMP ActivateEx(ITfThreadMgr *ptim, TfClientId tid, DWORD dwFlags);

	// ITfThreadMgrEventSink
	STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr *pdim);
	STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr *pdim);
	STDMETHODIMP OnSetFocus(ITfDocumentMgr *pdimFocus, ITfDocumentMgr *pdimPrevFocus);
	STDMETHODIMP OnPushContext(ITfContext *pic);
	STDMETHODIMP OnPopContext(ITfContext *pic);

	// ITfTextEditSink
	STDMETHODIMP OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord);

	// ITfThreadFocusSink
	STDMETHODIMP OnSetThreadFocus();
	STDMETHODIMP OnKillThreadFocus();

	// ItfCompartmentEventSink
	STDMETHODIMP OnChange(REFGUID rguid);

	// ITfKeyEventSink
	STDMETHODIMP OnSetFocus(BOOL fForeground);
	STDMETHODIMP OnTestKeyDown(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
	STDMETHODIMP OnKeyDown(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
	STDMETHODIMP OnTestKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
	STDMETHODIMP OnKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten);
	STDMETHODIMP OnPreservedKey(ITfContext *pic, REFGUID rguid, BOOL *pfEaten);

	// ITfCompositionSink
	STDMETHODIMP OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition);

	// ITfDisplayAttributeProvider
	STDMETHODIMP EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum);
	STDMETHODIMP GetDisplayAttributeInfo(REFGUID guid, ITfDisplayAttributeInfo **ppInfo);

	// ITfFunctionProvider
    STDMETHODIMP GetType(GUID *pguid);
    STDMETHODIMP GetDescription(BSTR *pbstrDesc);
    STDMETHODIMP GetFunction(REFGUID rguid, REFIID riid, IUnknown **ppunk);

	// ITfFunction
	STDMETHODIMP GetDisplayName(BSTR *pbstrName);

	// ITfFnConfigure
	STDMETHODIMP Show(HWND hwndParent, LANGID langid, REFGUID rguidProfile);

	// ITfFnShowHelp
	STDMETHODIMP Show(HWND hwndParent);

	ITfThreadMgr *_GetThreadMgr()
	{
		return _pThreadMgr;
	}
	TfClientId _GetClientId()
	{
		return _ClientId;
	}
	ITfComposition *_GetComposition()
	{
		return _pComposition;
	}
	CCandidateList *_GetCandidateList()
	{
		return _pCandidateList;
	}

	// Compartment
	HRESULT _SetCompartment(REFGUID rguid, const VARIANT *pvar);
	BOOL _IsKeyboardDisabled();
	BOOL _IsKeyboardOpen();
	HRESULT _SetKeyboardOpen(BOOL fOpen);

	// Composition
	BOOL _IsComposing();
	void _SetComposition(ITfComposition *pComposition);
	BOOL _IsRangeCovered(TfEditCookie ec, ITfRange *pRangeTest, ITfRange *pRangeCover);
	void _StartComposition(ITfContext *pContext);
	void _TerminateComposition(TfEditCookie ec, ITfContext *pContext);
	void _EndComposition(ITfContext *pContext);
	void _CancelComposition(TfEditCookie ec, ITfContext *pContext);
	void _ClearComposition();

	// LanguageBar
	void _UpdateLanguageBar();
	
	// DisplayAttribureProvider
	void _ClearCompositionDisplayAttributes(TfEditCookie ec, ITfContext *pContext);
	BOOL _SetCompositionDisplayAttributes(TfEditCookie ec, ITfContext *pContext, TfGuidAtom gaDisplayAttribute);

	// KeyHandler
	HRESULT _InvokeKeyHandler(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BYTE bSf);
	HRESULT _HandleKey(TfEditCookie ec, ITfContext *pContext, WPARAM wParam, BYTE bSf);
	void _KeyboardChanged();
	void _ResetStatus();
	BOOL _IsSurrogatePair(WCHAR first, WCHAR second);

	// KeyHandlerChar
	HRESULT _HandleChar(TfEditCookie ec, ITfContext *pContext, WCHAR ch, WCHAR chO);
	HRESULT _HandleCharReturn(TfEditCookie ec, ITfContext *pContext, BOOL back = FALSE);

	// KeyHandlerCompostion
	HRESULT _Update(TfEditCookie ec, ITfContext *pContext, BOOL fixed = FALSE, BOOL back = FALSE);
	HRESULT _SetText(TfEditCookie ec, ITfContext *pContext, const std::wstring &text, BOOL fixed);
	HRESULT _ShowCandidateList(TfEditCookie ec, ITfContext *pContext, BOOL reg);

	// KeyHandlerControl
	HRESULT _HandleControl(TfEditCookie ec, ITfContext *pContext, BYTE sf, WCHAR &ch);

	// KeyHandlerConv
	WCHAR _GetCh(WPARAM wParam);
	BYTE _GetSf(WPARAM wParam, WCHAR ch);
	HRESULT _ConvRomanKana(ROMAN_KANA_CONV *pconv);
	HRESULT _ConvAsciiJLatin(ASCII_JLATIN_CONV *pconv);
	void _StartConv();
	void _NextConv();
	void _PrevConv();
	void _NextComp();
	void _PrevComp();
	void _SetComp(const std::wstring &candidate);
	BOOL _ConvN(WCHAR ch);
	void _ConvKanaToKana(std::wstring &dst, int dstmode, const std::wstring &src, int srcmode);

	// KeyHandlerDictionary
	void _ConvDic(WCHAR command);
	void _ConnectDic();
	void _DisconnectDic();
	void _AddUserDic(const std::wstring &key, const std::wstring &candidate, const std::wstring &annotation, WCHAR command);
	void _DelUserDic(const std::wstring &key, const std::wstring &candidate);
	void _SaveUserDic();
	void _ConvDicNum();
	void _ConvNum(std::wstring &convnum, const std::wstring &key, const std::wstring &candidate);
	void _StartDicSrv();
	void _StartConfigure();
	void _StartProcess(LPCWSTR fname);

	// FnConfigure
	void _CreateConfigPath();
	void _LoadBehavior();
	void _LoadSelKey();
	void _LoadKeyMap();
	void _LoadConvPoint();
	void _LoadKana();
	void _LoadJLatin();

private:
	LONG _cRef;

	BOOL _InitThreadMgrEventSink();
	void _UninitThreadMgrEventSink();

	BOOL _InitThreadFocusSink();
	void _UninitThreadFocusSink();

	BOOL _InitCompartmentEventSink();
	void _UninitCompartmentEventSink();

	BOOL _InitTextEditSink(ITfDocumentMgr *pDocumentMgr);

	BOOL _InitKeyEventSink();
	void _UninitKeyEventSink();

	BOOL _InitPreservedKey();
	void _UninitPreservedKey();

	BOOL _InitLanguageBar();
	void _UninitLanguageBar();

	BOOL _InitDisplayAttributeGuidAtom();

	BOOL _InitFunctionProvider();
	void _UninitFunctionProvider();

	BOOL _IsKeyEaten(ITfContext *pContext, WPARAM wParam);

	ITfThreadMgr *_pThreadMgr;
	TfClientId _ClientId;

	DWORD _dwThreadMgrEventSinkCookie;
	DWORD _dwThreadFocusSinkCookie;
	DWORD _dwCompartmentEventSinkCookie;

	ITfContext *_pTextEditSinkContext;
	DWORD _dwTextEditSinkCookie;

	ITfComposition *_pComposition;

	CLangBarItemButton *_pLangBarItem;

	CCandidateList *_pCandidateList;

	TfGuidAtom _gaDisplayAttributeInput;
	TfGuidAtom _gaDisplayAttributeConverted;

private:
	//ファイルパス
	WCHAR pathconfigxml[MAX_PATH];	//設定

	//corvussrv.exe との名前付きパイプ
	WCHAR pipename[MAX_KRNLOBJNAME];
	HANDLE hPipe;
	//ミューテックス
	WCHAR srvmutexname[MAX_KRNLOBJNAME];
	WCHAR cnfmutexname[MAX_KRNLOBJNAME];

	//キーマップ
	BYTE keymap_latin[KEYMAPNUM];	//全英/アスキー
	BYTE keymap_jmode[KEYMAPNUM];	//ひらがな/カタカナ
	BYTE keymap_void[KEYMAPNUM];	//無効

	//変換位置指定（開始,代替,送り）
	WCHAR conv_point[CONV_POINT_NUM][3];

	//変換テーブル
	ROMAN_KANA_CONV roman_kana_conv[ROMAN_KANA_TBL_NUM];
	ASCII_JLATIN_CONV ascii_jlatin_conv[ASCII_JLATIN_TBL_NUM];

public:
	//状態
	int inputmode;			//入力モード (無し/ひらがな/カタカナ/半角ｶﾀｶﾅ/全英/アスキー)
	BOOL inputkey;			//見出し入力▽モード
	BOOL abbrevmode;		//abbrevモード
	BOOL showentry;			//候補表示▼モード
	BOOL showcandlist;		//候補リスト表示
	BOOL complement;		//補完

	int exinputmode;		//入力モードの前回状態

	//動作設定
	WCHAR fontname[LF_FACESIZE];	//候補一覧のフォント設定
	int fontpoint;					//候補一覧のフォント設定
	int fontweight;					//候補一覧のフォント設定
	BOOL fontitalic;				//候補一覧のフォント設定
	LONG maxwidth;			//候補一覧の最大幅
	BOOL c_visualstyle;		//VisualStyleを使用する
	size_t c_untilcandlist;	//候補一覧表示に要する変換回数(0:表示なし/1:1回目)
	BOOL c_dispcandnum;		//候補一覧表示なしのとき候補数を表示する
	BOOL c_annotation;		//注釈を表示する
	BOOL c_annotatlst;		//（候補一覧のみ）
	BOOL c_nomodemark;		//▽▼*マークを表示しない
	BOOL c_nookuriconv;		//送り仮名が決定したとき変換を開始しない
	BOOL c_delokuricncl;	//取消のとき送り仮名を削除する
	BOOL c_backincenter;	//後退に確定を含める
	BOOL c_addcandktkn;		//候補に片仮名変換を追加する

	//ローマ字・仮名
	std::wstring roman;		//ローマ字
	std::wstring kana;		//仮名
	size_t accompidx;		//送り仮名インデックス

	//検索用見出し語
	std::wstring searchkey;		//数値変換で数値→#
	std::wstring searchkeyorg;	//オリジナル

	//候補
	CANDIDATES candidates;	//候補
	size_t candidx;			//候補インデックス

	//候補一覧選択キー
	WCHAR selkey[MAX_SELKEY_C][2][2];
};

#endif // TEXTSERVICE_H
