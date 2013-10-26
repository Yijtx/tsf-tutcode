﻿
#include "imcrvtip.h"
#include "TextService.h"
#include "CandidateList.h"
#include "LanguageBar.h"

static LPCWSTR c_PreservedKeyDesc = L"OnOff";

BOOL CTextService::_IsKeyEaten(ITfContext *pContext, WPARAM wParam)
{
	if(_IsKeyboardDisabled())
	{
		return FALSE;
	}

	if(!_IsKeyboardOpen())
	{
		return FALSE;
	}

	if(_pCandidateList && _pCandidateList->_IsContextCandidateWindow(pContext))
	{
		return FALSE;
	}

	if(_IsComposing())
	{
		if(inputmode != im_ascii)
		{
			return TRUE;
		}
	}

	SHORT vk_ctrl = GetKeyState(VK_CONTROL) & 0x8000;
	SHORT vk_kana = GetKeyState(VK_KANA) & 0x0001;

	WCHAR ch = _GetCh((BYTE)wParam);
	BYTE sf = _GetSf((BYTE)wParam, ch);

	//確定状態で処理する機能
	switch(inputmode)
	{
	case im_jlatin:
	case im_ascii:
		switch(sf)
		{
		case SKK_JMODE:
			return TRUE;
			break;
		default:
			break;
		}
		break;
	case im_hiragana:
	case im_katakana:
		switch(sf)
		{
		case SKK_CONV_POINT:
		case SKK_KANA:
		case SKK_CONV_CHAR:
		case SKK_JLATIN:
		case SKK_ASCII:
		case SKK_ABBREV:
			return TRUE;
			break;
		default:
			break;
		}
		break;
	case im_katakana_ank:
		switch(sf)
		{
		case SKK_KANA:
		case SKK_CONV_CHAR:
		case SKK_JLATIN:
		case SKK_ASCII:
			return TRUE;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	//無効
	if(_IsKeyVoid(ch, (BYTE)wParam))
	{
		return TRUE;
	}
	//処理しないCtrlキー
	if(vk_ctrl)
	{
		return FALSE;
	}
	//ASCIIモード、かなキーロック
	if(inputmode == im_ascii && !vk_kana)
	{
		return FALSE;
	}

	if(ch >= L'\x20')
	{
		return TRUE;
	}

	return FALSE;
}

STDAPI CTextService::OnSetFocus(BOOL fForeground)
{
	return S_OK;
}

STDAPI CTextService::OnTestKeyDown(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	*pfEaten = _IsKeyEaten(pic, wParam);

	if(_pCandidateList == NULL || !_pCandidateList->_IsShowCandidateWindow())
	{
		//ASCIIモード
		if(inputmode == im_ascii)
		{
			WCHAR ch = _GetCh((BYTE)wParam);
			//無効
			if(_IsKeyVoid(ch, (BYTE)wParam))
			{
				_UpdateLanguageBar();
			}
			else if(ch != L'\0')
			{
				_ClearComposition();
			}
		}
	}
	return S_OK;
}

STDAPI CTextService::OnKeyDown(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	*pfEaten = _IsKeyEaten(pic, wParam);

	if(*pfEaten)
	{
		_InvokeKeyHandler(pic, wParam, lParam, SKK_NULL);
	}
	return S_OK;
}

STDAPI CTextService::OnTestKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	*pfEaten = _IsKeyEaten(pic, wParam);
	return S_OK;
}

STDAPI CTextService::OnKeyUp(ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
	*pfEaten = _IsKeyEaten(pic, wParam);
	return S_OK;
}

STDAPI CTextService::OnPreservedKey(ITfContext *pic, REFGUID rguid, BOOL *pfEaten)
{
	if(IsEqualGUID(rguid, c_guidPreservedKeyOnOff))
	{
		BOOL fOpen = _IsKeyboardOpen();
		if(!fOpen)
		{
			inputmode = im_disable;
		}
		else
		{
			_ClearComposition();
		}
		_SetKeyboardOpen(fOpen ? FALSE : TRUE);
		*pfEaten = TRUE;
	}
	else
	{
		*pfEaten = FALSE;
	}

	return S_OK;
}

BOOL CTextService::_InitKeyEventSink()
{
	ITfKeystrokeMgr *pKeystrokeMgr;
	HRESULT hr = E_FAIL;

	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pKeystrokeMgr)) == S_OK)
	{
		hr = pKeystrokeMgr->AdviseKeyEventSink(_ClientId, (ITfKeyEventSink *)this, TRUE);
		pKeystrokeMgr->Release();
	}

	return (hr == S_OK);
}

void CTextService::_UninitKeyEventSink()
{
	ITfKeystrokeMgr *pKeystrokeMgr;

	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pKeystrokeMgr)) == S_OK)
	{
		pKeystrokeMgr->UnadviseKeyEventSink(_ClientId);
		pKeystrokeMgr->Release();
	}
}

BOOL CTextService::_InitPreservedKey()
{
	ITfKeystrokeMgr *pKeystrokeMgr;
	HRESULT hr = E_FAIL;
	int i;

	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pKeystrokeMgr)) == S_OK)
	{
		for(i=0; i<MAX_PRESERVEDKEY; i++)
		{
			if(preservedkey[i].uVKey == 0 && preservedkey[i].uModifiers == 0)
			{
				break;
			}
			hr = pKeystrokeMgr->PreserveKey(_ClientId, c_guidPreservedKeyOnOff,
				&preservedkey[i], c_PreservedKeyDesc, (ULONG)wcslen(c_PreservedKeyDesc));
		}

		pKeystrokeMgr->Release();
	}

	return (hr == S_OK);
}

void CTextService::_UninitPreservedKey()
{
	ITfKeystrokeMgr *pKeystrokeMgr;
	int i;

	if(_pThreadMgr->QueryInterface(IID_PPV_ARGS(&pKeystrokeMgr)) == S_OK)
	{
		for(i=0; i<MAX_PRESERVEDKEY; i++)
		{
			if(preservedkey[i].uVKey == 0 && preservedkey[i].uModifiers == 0)
			{
				break;
			}
			pKeystrokeMgr->UnpreserveKey(c_guidPreservedKeyOnOff, &preservedkey[i]);
		}

		pKeystrokeMgr->Release();
	}
}
