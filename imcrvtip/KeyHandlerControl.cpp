﻿
#include "imcrvtip.h"
#include "TextService.h"
#include "LanguageBar.h"
#include "convtype.h"
#include "KeySender.h"

HRESULT CTextService::_HandleControl(TfEditCookie ec, ITfContext *pContext, BYTE sf, WCHAR ch)
{
	size_t i;
	BOOL skipupdate = FALSE;

	switch(sf)
	{
	case SKK_KANA:
		if(abbrevmode)
		{
			break;
		}
		switch(inputmode)
		{
		case im_hiragana:
		case im_katakana:
			_ConvRoman();
			if(inputkey && !showentry)
			{
				if(okuriidx != 0)
				{
					kana.erase(okuriidx, 1);
					okuriidx = 0;
				}
				//ひらがな/カタカナに変換
				_ConvKanaToKana(kana, (inputmode == im_hiragana ? im_katakana : im_hiragana),
					kana, inputmode);
				_HandleCharReturn(ec, pContext);
			}
			else
			{
				if(_ShowInputModeWindow)
				{
					_HandleCharShift(ec, pContext);
				}
				else
				{
					_HandleCharReturn(ec, pContext);
				}
				//ひらがな/カタカナモードへ
				inputmode = (inputmode == im_hiragana ? im_katakana : im_hiragana);
				_UpdateLanguageBar();
			}
			return S_OK;
			break;
		case im_katakana_ank:
			_ConvRoman();
			if(_ShowInputModeWindow)
			{
				_HandleCharShift(ec, pContext);
			}
			else
			{
				_HandleCharReturn(ec, pContext);
			}
			//ひらがなモードへ
			inputmode = im_hiragana;
			_UpdateLanguageBar();
			return S_OK;
			break;
		default:
			break;
		}
		break;

	case SKK_CONV_CHAR:
		if(abbrevmode)
		{
			//全英に変換
			ASCII_JLATIN_CONV ajc;
			ajc.ascii[1] = L'\0';
			roman = kana;
			kana.clear();
			cursoridx = 0;
			for(i = 0; i < roman.size(); i++)
			{
				ajc.ascii[0] = roman[i];
				if(_ConvAsciiJLatin(&ajc) == S_OK)
				{
					kana.append(ajc.jlatin);
				}
			}
			_HandleCharReturn(ec, pContext);
			return S_OK;
			break;
		}
		switch(inputmode)
		{
		case im_hiragana:
		case im_katakana:
			_ConvRoman();
			if(inputkey && !showentry)
			{
				if(okuriidx != 0)
				{
					kana.erase(okuriidx, 1);
					okuriidx = 0;
				}
				//半角ｶﾀｶﾅに変換
				_ConvKanaToKana(kana, im_katakana_ank, kana, inputmode);
				_HandleCharReturn(ec, pContext);
			}
			else
			{
				if(_ShowInputModeWindow)
				{
					_HandleCharShift(ec, pContext);
				}
				else
				{
					_HandleCharReturn(ec, pContext);
				}
				//半角ｶﾀｶﾅモードへ
				inputmode = im_katakana_ank;
				_UpdateLanguageBar();
			}
			return S_OK;
			break;
		case im_katakana_ank:
			_ConvRoman();
			if(_ShowInputModeWindow)
			{
				_HandleCharShift(ec, pContext);
			}
			else
			{
				_HandleCharReturn(ec, pContext);
			}
			//ひらがなモードへ
			inputmode = im_hiragana;
			_UpdateLanguageBar();
			return S_OK;
			break;
		default:
			break;
		}
		break;

	case SKK_JLATIN:
	case SKK_ASCII:
		if(abbrevmode)
		{
			break;
		}
		switch(inputmode)
		{
		case im_hiragana:
		case im_katakana:
		case im_katakana_ank:
			_ConvRoman();
			if(_ShowInputModeWindow)
			{
				_HandleCharShift(ec, pContext);
			}
			else
			{
				_HandleCharReturn(ec, pContext);
			}
			//アスキー/全英モードへ
			inputmode = (sf == SKK_ASCII ? im_ascii : im_jlatin);
			_UpdateLanguageBar();
			return S_OK;
			break;
		default:
			break;
		}
		break;

	case SKK_JMODE:
		switch(inputmode)
		{
		case im_jlatin:
		case im_ascii:
			//ひらがなモードへ
			inputmode = im_hiragana;
			_UpdateLanguageBar();
			return S_OK;
			break;
		default:
			break;
		}
		break;

	case SKK_ABBREV:
		if(abbrevmode)
		{
			break;
		}
		switch(inputmode)
		{
		case im_hiragana:
		case im_katakana:
			_ConvRoman();
			if(!inputkey || showentry)
			{
				_HandleCharShift(ec, pContext);
				//見出し入力開始(abbrev)
				inputkey = TRUE;
				abbrevmode = TRUE;
			}
			_Update(ec, pContext);
			return S_OK;
			break;
		default:
			break;
		}
		break;

	case SKK_AFFIX:
		if(!inputkey || (abbrevmode && !showentry))
		{
			break;
		}
		if(showentry || (inputkey && kana.empty() && roman.empty()))
		{
			if(showentry)
			{
				_HandleCharShift(ec, pContext);
			}
			//見出し入力開始(接尾辞)
			inputkey = TRUE;
			ch = L'>';
			kana.push_back(ch);
			cursoridx++;
			_Update(ec, pContext);
			return S_OK;
		}
		switch(inputmode)
		{
		case im_hiragana:
		case im_katakana:
			_ConvRoman();
			if(!kana.empty() && okuriidx == 0)
			{
				ch = L'>';
				roman.clear();
				kana.push_back(ch);
				cursoridx = kana.size();
				if(cx_begincvokuri && !hintmode)
				{
					//辞書検索開始(接頭辞)
					showentry = TRUE;
					_StartConv();
				}
			}
			_Update(ec, pContext);
			return S_OK;
			break;
		default:
			break;
		}
		break;

	case SKK_NEXT_CAND:
		if(showentry)
		{
			_NextConv();
			_Update(ec, pContext);
			return S_OK;
		}
		else if(inputkey)
		{
			_ConvRoman();
			if(!kana.empty())
			{
				//候補表示開始
				cursoridx = kana.size();
				showentry = TRUE;
				_StartConv();
			}
			_Update(ec, pContext);
			return S_OK;
		}
		break;

	case SKK_PREV_CAND:
		if(showentry)
		{
			_PrevConv();
			_Update(ec, pContext);
			return S_OK;
		}
		break;

	case SKK_PURGE_DIC:
		if(showentry)
		{
			if(purgedicmode)
			{
				purgedicmode = FALSE;
				_DelUserDic((okuriidx == 0 ? REQ_USER_DEL_1 : REQ_USER_DEL_0),
					((candorgcnt <= candidx) ? searchkey : searchkeyorg),
					candidates[candidx].second.first);
				showentry = FALSE;
				candidx = 0;
				kana.clear();
				okuriidx = 0;
				cursoridx = 0;
				_HandleCharReturn(ec, pContext);
			}
			else
			{
				purgedicmode = TRUE;
				_Update(ec, pContext);
			}
			return S_OK;
		}
		break;

	case SKK_NEXT_COMP:
		if(inputkey && !showentry)
		{
			_ConvRoman();
			_NextComp();
			_Update(ec, pContext);
			return S_OK;
		}
		break;

	case SKK_PREV_COMP:
		if(inputkey && !showentry)
		{
			_PrevComp();
			_Update(ec, pContext);
			return S_OK;
		}
		break;

	case SKK_HINT:
		if(!inputkey || abbrevmode)
		{
			break;
		}
		if(showentry)
		{
			candidx = 0;
			showentry = FALSE;
		}
		_ConvRoman();
		if(!kana.empty() &&
			kana.find_first_of(CHAR_SKK_HINT) == std::wstring::npos)
		{
			hintmode = TRUE;
			cursoridx = kana.size();
			kana.insert(cursoridx, 1, CHAR_SKK_HINT);
			cursoridx++;
		}
		_Update(ec, pContext);
		return S_OK;
		break;

	case SKK_CONV_POINT:
		if(_HandleConvPoint(ec, pContext, ch) == S_OK)
		{
			return S_OK;
		}
		break;

	case SKK_DIRECT:
		if(inputkey && !showentry && roman.empty() &&
			((okuriidx == 0) || ((okuriidx != 0) && (okuriidx + 1 != cursoridx))))
		{
			kana.insert(cursoridx, 1, ch);
			cursoridx++;
			_Update(ec, pContext);
			return S_OK;
		}
		break;

	case SKK_ENTER:
		_ConvRoman();
		_HandleCharReturn(ec, pContext, (_GetSf(0, ch) == SKK_BACK ? TRUE : FALSE));
		return S_OK;
		break;

	case SKK_CANCEL:
		if(showentry)
		{
			candidx = 0;
			showentry = FALSE;
			if(cx_delokuricncl && okuriidx != 0)
			{
				kana = kana.substr(0, okuriidx);
				okuriidx = 0;
				cursoridx = kana.size();
			}
			if(cx_delcvposcncl && okuriidx != 0)
			{
				kana.erase(okuriidx, 1);
				okuriidx = 0;
				cursoridx--;
			}
			_Update(ec, pContext);
		}
		else
		{
			kana.clear();
			okuriidx = 0;
			cursoridx = 0;
			_HandleCharReturn(ec, pContext);
		}
		return S_OK;
		break;

	case SKK_BACK:
		if(showentry)
		{
			if(_HandleControl(ec, pContext, (cx_backincenter ? SKK_ENTER : SKK_PREV_CAND), ch) == S_OK)
			{
				return S_OK;
			}
		}
		if(inputkey && roman.empty() && kana.empty())
		{
			_HandleCharReturn(ec, pContext);
			return S_OK;
		}
		if(!roman.empty())
		{
			roman.pop_back();
			if(!cx_showromancomp)
			{
				skipupdate = TRUE;
			}
			if(roman.empty())
			{
				if(okuriidx != 0 && okuriidx + 1 == cursoridx)
				{
					kana.erase(cursoridx - 1, 1);
					cursoridx--;
					okuriidx = 0;
					skipupdate = FALSE;
				}
			}
		}
		else if(okuriidx != 0 && okuriidx + 1 == cursoridx)
		{
			kana.erase(cursoridx - 1, 1);
			cursoridx--;
			okuriidx = 0;
		}
		else if(!kana.empty())
		{
			// surrogate pair
			if(cursoridx >= 2 &&
				IS_SURROGATE_PAIR(kana[cursoridx - 2], kana[cursoridx - 1]))
			{
				kana.erase(cursoridx - 2, 2);
				cursoridx -= 2;
				if(okuriidx != 0 && cursoridx < okuriidx)
				{
					okuriidx -= 2;
					if(okuriidx == 0)
					{
						kana.erase(0, 1);
					}
				}
			}
			else if(cursoridx >= 1)
			{
				kana.erase(cursoridx - 1, 1);
				cursoridx--;
				if(okuriidx != 0 && cursoridx < okuriidx)
				{
					okuriidx--;
					if(okuriidx == 0)
					{
						kana.erase(0, 1);
					}
				}
			}
			if(okuriidx != 0 && okuriidx + 1 == cursoridx)
			{
				kana.erase(cursoridx - 1, 1);
				cursoridx--;
				okuriidx = 0;
			}
		}
		if(!inputkey && roman.empty() && kana.empty())
		{
			_HandleCharReturn(ec, pContext);
		}
		else if(!skipupdate)
		{
			_Update(ec, pContext);
		}
		return S_OK;
		break;

	case SKK_DELETE:
		if(!inputkey || showentry || kana.empty())
		{
			break;
		}
		if(okuriidx != 0 && okuriidx == cursoridx)
		{
			kana.erase(cursoridx, 1);
			okuriidx = 0;
		}
		// surrogate pair
		if(kana.size() - cursoridx >= 2 &&
			IS_SURROGATE_PAIR(kana[cursoridx], kana[cursoridx + 1]))
		{
			kana.erase(cursoridx, 2);
			if(okuriidx >= 2 && cursoridx < okuriidx)
			{
				okuriidx -= 2;
				if(okuriidx == 0)
				{
					kana.erase(cursoridx, 1);
				}
			}
		}
		else
		{
			kana.erase(cursoridx, 1);
			if(okuriidx >= 1 && cursoridx < okuriidx)
			{
				okuriidx--;
				if(okuriidx == 0)
				{
					kana.erase(cursoridx, 1);
				}
			}
		}
		_Update(ec, pContext);
		return S_OK;
		break;

	case SKK_VOID:
		return S_OK;
		break;

	case SKK_LEFT:
		if(!inputkey || showentry)
		{
			break;
		}
		_ConvRoman();
		if(!kana.empty() && cursoridx > 0)
		{
			// surrogate pair
			if(cursoridx >= 2 &&
				IS_SURROGATE_PAIR(kana[cursoridx - 2], kana[cursoridx - 1]))
			{
				cursoridx -= 2;
			}
			else
			{
				cursoridx--;
			}
			if(okuriidx != 0 && okuriidx + 1 == cursoridx)
			{
				cursoridx--;
			}
		}
		_Update(ec, pContext);
		return S_OK;
		break;

	case SKK_UP:
		if(!inputkey || showentry)
		{
			break;
		}
		_ConvRoman();
		cursoridx = 0;
		_Update(ec, pContext);
		return S_OK;
		break;

	case SKK_RIGHT:
		if(!inputkey || showentry)
		{
			break;
		}
		_ConvRoman();
		if(!kana.empty() && cursoridx < kana.size())
		{
			// surrogate pair
			if(kana.size() - cursoridx >= 2 &&
				IS_SURROGATE_PAIR(kana[cursoridx], kana[cursoridx + 1]))
			{
				cursoridx += 2;
			}
			else
			{
				cursoridx++;
			}
			if(okuriidx != 0 && okuriidx + 1 == cursoridx)
			{
				cursoridx++;
			}
		}
		_Update(ec, pContext);
		return S_OK;
		break;

	case SKK_DOWN:
		if(!inputkey || showentry)
		{
			break;
		}
		_ConvRoman();
		cursoridx = kana.size();
		_Update(ec, pContext);
		return S_OK;
		break;

	case SKK_PASTE:
		if(!inputkey || showentry)
		{
			break;
		}
		if(IsClipboardFormatAvailable(CF_UNICODETEXT))
		{
			if(OpenClipboard(NULL))
			{
				HANDLE hCB = GetClipboardData(CF_UNICODETEXT);
				if(hCB != NULL)
				{
					PWCHAR pwCB = (PWCHAR)GlobalLock(hCB);
					if(pwCB != NULL)
					{
						_ConvRoman();
						std::wstring s = std::regex_replace(std::wstring(pwCB),
							std::wregex(L"[\\x00-\\x19]"), std::wstring(L""));
						kana.insert(cursoridx, s);
						if(okuriidx != 0 && cursoridx <= okuriidx)
						{
							okuriidx += s.size();
						}
						cursoridx += s.size();
						_Update(ec, pContext);
						GlobalUnlock(hCB);
					}
				}
				CloseClipboard();
			}
		}
		break;

	case SKK_OTHERIME:
		_ConvRoman();
		if(_ShowInputModeWindow)
		{
			_HandleCharShift(ec, pContext);
		}
		else
		{
			_HandleCharReturn(ec, pContext);
		}
		_ClearComposition();
		postbuf.clear();
		_SetKeyboardOpen(FALSE);

		KeySender::OtherIme();
		return S_OK;
		break;

	//DeleterがBS送り付けによりカーソル直前文字列を削除した後に実行される。
	//+ 削除した文字列を置換する文字列を確定する。
	//+ pending.mazeの場合、交ぜ書き変換の候補表示を開始する。
	//(TSFによるカーソル直前文字列削除ができなかった場合用。)
	case SKK_AFTER_DELETER:
		{
			mozc::commands::Output pending;
			pending.CopyFrom(deleter.pending_output());
			if(pending.maze)
			{
				_StartConvWithYomi(ec, pContext, pending.kana);
			}
			else
			{
				kana = pending.kana;
				cursoridx = kana.size();
				_HandleCharReturn(ec, pContext);
			}
		}
		return S_OK;
		break;

	default:
		break;
	}

	return E_PENDING;
}

HRESULT CTextService::_HandleConvPoint(TfEditCookie ec, ITfContext *pContext, WCHAR ch)
{
	if(abbrevmode && !showentry)
	{
		return E_PENDING;
	}
	switch(inputmode)
	{
	case im_hiragana:
	case im_katakana:
		if(showentry)
		{
			_HandleCharShift(ec, pContext);
		}
		if(!inputkey)
		{
			if(_ConvN(ch))
			{
				if(!kana.empty())
				{
					_HandleCharShift(ec, pContext);
				}
				//見出し入力開始
				inputkey = TRUE;
				_Update(ec, pContext);
			}
		}
		else
		{
			if(_ConvN(ch) && (okuriidx == 0) && (cursoridx != 0))
			{
				//送り仮名入力開始
				kana.insert(cursoridx, 1, CHAR_SKK_OKURI);	//送りローマ字
				okuriidx = cursoridx;
				cursoridx++;
				_Update(ec, pContext);
			}
		}
		if(ch == L'\0')
		{
			return S_OK;
		}
		break;
	default:
		break;
	}

	return E_PENDING;
}

//入力途中のシーケンスを確定する
HRESULT CTextService::_CommitRoman(TfEditCookie ec, ITfContext *pContext)
{
	kana.insert(cursoridx, roman);
	cursoridx += roman.size();
	roman.clear();
	if(!inputkey)
	{
		_HandleCharReturn(ec, pContext);
	}
	else
	{
		_Update(ec, pContext);
	}
	return S_OK;
}
