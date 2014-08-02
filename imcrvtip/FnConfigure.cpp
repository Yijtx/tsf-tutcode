﻿
#include "configxml.h"
#include "imcrvtip.h"
#include "TextService.h"
#include "convtype.h"

static const struct {
	BYTE skkfunc;
	LPCWSTR keyname;
} configkeymap[] =
{
	{SKK_KANA,		ValueKeyMapKana},
	{SKK_CONV_CHAR,	ValueKeyMapConvChar},
	{SKK_JLATIN,	ValueKeyMapJLatin},
	{SKK_ASCII,		ValueKeyMapAscii},
	{SKK_JMODE,		ValueKeyMapJMode},
	{SKK_ABBREV,	ValueKeyMapAbbrev},
	{SKK_AFFIX,		ValueKeyMapAffix},
	{SKK_NEXT_CAND,	ValueKeyMapNextCand},
	{SKK_PREV_CAND,	ValueKeyMapPrevCand},
	{SKK_PURGE_DIC,	ValueKeyMapPurgeDic},
	{SKK_NEXT_COMP,	ValueKeyMapNextComp},
	{SKK_PREV_COMP,	ValueKeyMapPrevComp},
	{SKK_HINT,		ValueKeyMapHint},
	{SKK_CONV_POINT,ValueKeyMapConvPoint},
	{SKK_DIRECT,	ValueKeyMapDirect},
	{SKK_ENTER,		ValueKeyMapEnter},
	{SKK_CANCEL,	ValueKeyMapCancel},
	{SKK_BACK,		ValueKeyMapBack},
	{SKK_DELETE,	ValueKeyMapDelete},
	{SKK_VOID,		ValueKeyMapVoid},
	{SKK_LEFT,		ValueKeyMapLeft},
	{SKK_UP,		ValueKeyMapUp},
	{SKK_RIGHT,		ValueKeyMapRight},
	{SKK_DOWN,		ValueKeyMapDown},
	{SKK_PASTE,		ValueKeyMapPaste},
	{SKK_OTHERIME,	ValueKeyMapOtherIme},
	{SKK_VIESC,		ValueKeyMapViEsc},
	{SKK_NULL,		L""}
};

static const TF_PRESERVEDKEY configpreservedkey[] =
{
	{VK_OEM_3		/*0xC0*/, TF_MOD_ALT},
	{VK_KANJI		/*0x19*/, TF_MOD_IGNORE_ALL_MODIFIER},
	{VK_OEM_AUTO	/*0xF3*/, TF_MOD_IGNORE_ALL_MODIFIER},
	{VK_OEM_ENLW	/*0xF4*/, TF_MOD_IGNORE_ALL_MODIFIER}
};

static const struct {
	LPCWSTR value;
	COLORREF color;
} colorsxmlvalue[8] =
{
	{ValueColorBG, RGB(0xFF,0xFF,0xFF)},
	{ValueColorFR, RGB(0x00,0x00,0x00)},
	{ValueColorSE, RGB(0x00,0x00,0xFF)},
	{ValueColorCO, RGB(0x80,0x80,0x80)},
	{ValueColorCA, RGB(0x00,0x00,0x00)},
	{ValueColorSC, RGB(0x80,0x80,0x80)},
	{ValueColorAN, RGB(0x80,0x80,0x80)},
	{ValueColorNO, RGB(0x00,0x00,0x00)}
};

void CTextService::_CreateConfigPath()
{
	WCHAR appdata[MAX_PATH];

	pathconfigxml[0] = L'\0';

	if(SHGetFolderPathW(NULL, CSIDL_APPDATA | CSIDL_FLAG_DONT_VERIFY, NULL, SHGFP_TYPE_CURRENT, appdata) != S_OK)
	{
		appdata[0] = L'\0';
		return;
	}

	wcsncat_s(appdata, L"\\", _TRUNCATE);
	wcsncat_s(appdata, TextServiceDesc, _TRUNCATE);
	wcsncat_s(appdata, L"\\", _TRUNCATE);

	_snwprintf_s(pathconfigxml, _TRUNCATE, L"%s%s", appdata, fnconfigxml);

	ZeroMemory(mgrpipename, sizeof(mgrpipename));
	ZeroMemory(mgrmutexname, sizeof(mgrmutexname));
	ZeroMemory(cnfmutexname, sizeof(cnfmutexname));

	LPWSTR pszDigest = NULL;

	if(GetSidMD5Digest(&pszDigest))
	{
		_snwprintf_s(mgrpipename, _TRUNCATE, L"%s%s", CORVUSMGRPIPE, pszDigest);
		_snwprintf_s(mgrmutexname, _TRUNCATE, L"%s%s", CORVUSMGRMUTEX, pszDigest);
		_snwprintf_s(cnfmutexname, _TRUNCATE, L"%s%s", CORVUSCNFMUTEX, pszDigest);

		LocalFree(pszDigest);
	}
}

void CTextService::_ReadBoolValue(LPCWSTR section, LPCWSTR key, BOOL &value, BOOL defval)
{
	std::wstring strxmlval;
	ReadValue(pathconfigxml, section, key, strxmlval, (defval ? L"1" : L"0"));
	value = _wtoi(strxmlval.c_str());
	if(value != TRUE && value != FALSE)
	{
		value = defval;
	}
}

void CTextService::_LoadBehavior()
{
	std::wstring strxmlval;
	int i;

	//Behavior

	_ReadBoolValue(SectionBehavior, ValueBeginCvOkuri, cx_begincvokuri, TRUE);
	_ReadBoolValue(SectionBehavior, ValueDelCvPosCncl, cx_delcvposcncl, TRUE);
	_ReadBoolValue(SectionBehavior, ValueDelOkuriCncl, cx_delokuricncl, FALSE);
	_ReadBoolValue(SectionBehavior, ValueBackIncEnter, cx_backincenter, TRUE);
	_ReadBoolValue(SectionBehavior, ValueAddCandKtkn, cx_addcandktkn, FALSE);
	_ReadBoolValue(SectionBehavior, ValueShiftNNOkuri, cx_shiftnnokuri, TRUE);

	//Font

	ReadValue(pathconfigxml, SectionFont, ValueFontName, strxmlval);
	wcsncpy_s(cx_fontname, strxmlval.c_str(), _TRUNCATE);

	ReadValue(pathconfigxml, SectionFont, ValueFontSize, strxmlval);
	cx_fontpoint = _wtoi(strxmlval.c_str());
	ReadValue(pathconfigxml, SectionFont, ValueFontWeight, strxmlval);
	cx_fontweight = _wtoi(strxmlval.c_str());
	ReadValue(pathconfigxml, SectionFont, ValueFontItalic, strxmlval);
	cx_fontitalic = _wtoi(strxmlval.c_str());

	if(cx_fontpoint < 8 || cx_fontpoint > 72)
	{
		cx_fontpoint = 12;
	}
	if(cx_fontweight < 0 || cx_fontweight > 1000)
	{
		cx_fontweight = FW_NORMAL;
	}
	if(cx_fontitalic != TRUE && cx_fontitalic != FALSE)
	{
		cx_fontitalic = FALSE;
	}

	//Display

	ReadValue(pathconfigxml, SectionDisplay, ValueMaxWidth, strxmlval);
	cx_maxwidth = strxmlval.empty() ? -1 : _wtol(strxmlval.c_str());
	if(cx_maxwidth < 0)
	{
		cx_maxwidth = MAX_WIDTH_DEFAULT;
	}

	for(i = 0; i < _countof(cx_colors); i++)
	{
		cx_colors[i] = colorsxmlvalue[i].color;
		ReadValue(pathconfigxml, SectionDisplay, colorsxmlvalue[i].value, strxmlval);
		if(!strxmlval.empty())
		{
			cx_colors[i] = wcstoul(strxmlval.c_str(), NULL, 0);
		}
	}

	_ReadBoolValue(SectionDisplay, ValueDrawAPI, cx_drawapi, FALSE);
	_ReadBoolValue(SectionDisplay, ValueColorFont, cx_colorfont, FALSE);

	ReadValue(pathconfigxml, SectionDisplay, ValueUntilCandList, strxmlval);
	cx_untilcandlist = _wtoi(strxmlval.c_str());
	if(cx_untilcandlist > 9 || strxmlval.empty())
	{
		cx_untilcandlist = 5;
	}

	_ReadBoolValue(SectionDisplay, ValueDispCandNo, cx_dispcandnum, FALSE);
	_ReadBoolValue(SectionDisplay, ValueVerticalCand, cx_verticalcand, FALSE);
	_ReadBoolValue(SectionDisplay, ValueAnnotation, cx_annotation, TRUE);
	_ReadBoolValue(SectionDisplay, ValueAnnotatLst, cx_annotatlst, FALSE);
	_ReadBoolValue(SectionDisplay, ValueShowModeInl, cx_showmodeinl, FALSE);
	_ReadBoolValue(SectionDisplay, ValueShowModeImm, cx_showmodeimm, TRUE);
	_ReadBoolValue(SectionDisplay, ValueShowModeMark, cx_showmodemark, TRUE);
	_ReadBoolValue(SectionDisplay, ValueShowRoman, cx_showroman, TRUE);
	_ReadBoolValue(SectionDisplay, ValueShowRomanComp, cx_showromancomp, FALSE);
}

void CTextService::_LoadDisplayAttr()
{
	int i;
	std::wstring strxmlval;
	BOOL se;
	TF_DISPLAYATTRIBUTE da;

	for(i = 0; i < DISPLAYATTRIBUTE_INFO_NUM; i++)
	{
		display_attribute_series[i] = c_gdDisplayAttributeInfo[i].se;
		display_attribute_info[i] = c_gdDisplayAttributeInfo[i].da;

		ReadValue(pathconfigxml, SectionDisplayAttr, c_gdDisplayAttributeInfo[i].key, strxmlval);
		if(!strxmlval.empty())
		{
			if(swscanf_s(strxmlval.c_str(), L"%d,%d,0x%06X,%d,0x%06X,%d,%d,%d,0x%06X,%d",
				&se, &da.crText.type, &da.crText.cr, &da.crBk.type, &da.crBk.cr,
				&da.lsStyle, &da.fBoldLine, &da.crLine.type, &da.crLine.cr, &da.bAttr) == 10)
			{
				display_attribute_series[i] = se;
				display_attribute_info[i] = da;
			}
		}
	}
}

void CTextService::_LoadSelKey()
{
	WCHAR num[2];
	WCHAR key[4];
	int i;
	std::wstring strxmlval;

	ZeroMemory(selkey, sizeof(selkey));

	for(i = 0; i < MAX_SELKEY_C; i++)
	{
		num[0] = L'0' + i + 1;
		num[1] = L'\0';
		ReadValue(pathconfigxml, SectionSelKey, num, strxmlval);
		wcsncpy_s(key, strxmlval.c_str(), _TRUNCATE);
		selkey[i][0][0] = key[0];
		selkey[i][1][0] = key[1];
	}
}

static bool operator ==(const TF_PRESERVEDKEY &a, const TF_PRESERVEDKEY &b)
{
	return a.uVKey == b.uVKey && a.uModifiers == b.uModifiers;
}

void CTextService::_LoadPreservedKey()
{
	TF_PRESERVEDKEY on[MAX_PRESERVEDKEY];
	TF_PRESERVEDKEY off[MAX_PRESERVEDKEY];
	_LoadPreservedKeySub(SectionPreservedKeyOn, on);
	_LoadPreservedKeySub(SectionPreservedKeyOff, off);

	ZeroMemory(preservedkeyon, sizeof(TF_PRESERVEDKEY) * MAX_PRESERVEDKEY);
	ZeroMemory(preservedkeyoff, sizeof(TF_PRESERVEDKEY) * MAX_PRESERVEDKEY);
	ZeroMemory(preservedkeyonoff, sizeof(TF_PRESERVEDKEY) * MAX_PRESERVEDKEY);
	//OnとOff両方に同じ定義がある場合は、トグルとして扱う
	int i, j;
	int idxonoff = 0;
	int idxon = 0;
	for(i=0; i<MAX_PRESERVEDKEY; i++)
	{
		if(on[i].uVKey == 0 && on[i].uModifiers == 0)
		{
			break;
		}
		if(std::find(off, off + MAX_PRESERVEDKEY, on[i]) < off + MAX_PRESERVEDKEY)
		{
			preservedkeyonoff[idxonoff] = on[i];
			idxonoff++;
		}
		else
		{
			preservedkeyon[idxon] = on[i];
			idxon++;
		}
	}
	//Onに無くOffだけにある定義
	int idxoff = 0;
	for(j=0; j<MAX_PRESERVEDKEY; j++)
	{
		if(off[j].uVKey == 0 && off[j].uModifiers == 0)
		{
			break;
		}
		if(std::find(on, on + MAX_PRESERVEDKEY, off[j]) == on + MAX_PRESERVEDKEY)
		{
			preservedkeyoff[idxoff] = off[j];
			idxoff++;
		}
	}
}

void CTextService::_LoadPreservedKeySub(LPCWSTR SectionPreservedKey, TF_PRESERVEDKEY preservedkey[])
{
	APPDATAXMLLIST list;
	int i = 0;

	ZeroMemory(preservedkey, sizeof(TF_PRESERVEDKEY) * MAX_PRESERVEDKEY);

	if(ReadList(pathconfigxml, SectionPreservedKey, list) == S_OK && list.size() != 0)
	{
		FORWARD_ITERATION_I(l_itr, list)
		{
			if(i >= MAX_PRESERVEDKEY)
			{
				break;
			}

			FORWARD_ITERATION_I(r_itr, *l_itr)
			{
				if(r_itr->first == AttributeVKey)
				{
					preservedkey[i].uVKey = wcstoul(r_itr->second.c_str(), NULL, 0);
				}
				else if(r_itr->first == AttributeMKey)
				{
					preservedkey[i].uModifiers =
						wcstoul(r_itr->second.c_str(), NULL, 0) & (TF_MOD_ALT | TF_MOD_CONTROL | TF_MOD_SHIFT);
					if((preservedkey[i].uModifiers & (TF_MOD_ALT | TF_MOD_CONTROL | TF_MOD_SHIFT)) == 0)
					{
						preservedkey[i].uModifiers = TF_MOD_IGNORE_ALL_MODIFIER;
					}
				}
			}

			i++;
		}
	}
	else
	{
		for(i = 0; i < _countof(configpreservedkey); i++)
		{
			preservedkey[i] = configpreservedkey[i];
		}
	}
}

void CTextService::_LoadCKeyMap(LPCWSTR section)
{
	size_t i;
	WCHAR ch;
	WCHAR key[2];
	WCHAR keyre[KEYRELEN];
	std::wstring s;
	std::wregex re;
	std::wstring strxmlval;

	ZeroMemory(&ckeymap, sizeof(ckeymap));
	key[1] = L'\0';

	for(i = 0; i < _countof(configkeymap); i++)
	{
		if(configkeymap[i].skkfunc == SKK_NULL)
		{
			break;
		}
		ReadValue(pathconfigxml, section, configkeymap[i].keyname, strxmlval);
		wcsncpy_s(keyre, strxmlval.c_str(), _TRUNCATE);
		if(keyre[0] == L'\0')
		{
			continue;
		}

		//全英/アスキーモード
		switch(configkeymap[i].skkfunc)
		{
		case SKK_JMODE:
		case SKK_ENTER:
		case SKK_CANCEL:
		case SKK_BACK:
		case SKK_DELETE:
		case SKK_LEFT:
		case SKK_UP:
		case SKK_RIGHT:
		case SKK_DOWN:
		case SKK_PASTE:
		case SKK_OTHERIME:
		case SKK_VIESC:
			for(ch = 0x01; ch < CKEYMAPNUM; ch++)
			{
				key[0] = ch;
				s.assign(key);
				try
				{
					re.assign(keyre);
					if(std::regex_match(s, re))
					{
						if(ckeymap.keylatin[ch] != SKK_JMODE)	//「ひらがな」が優先
						{
							ckeymap.keylatin[ch] = configkeymap[i].skkfunc;
						}
					}
				}
				catch(...)
				{
					break;
				}
			}
			break;
		default:
			break;
		}

		//ひらがな/カタカナモード
		switch(configkeymap[i].skkfunc)
		{
		case SKK_JMODE:
		case SKK_VOID:
			break;
		default:
			for(ch = 0x01; ch < CKEYMAPNUM; ch++)
			{
				key[0] = ch;
				s.assign(key);
				try
				{
					re.assign(keyre);
					if(std::regex_match(s, re))
					{
						ckeymap.keyjmode[ch] = configkeymap[i].skkfunc;
					}
				}
				catch(...)
				{
					break;
				}
			}
			break;
		}

		//無効
		switch(configkeymap[i].skkfunc)
		{
		case SKK_VOID:
			for(ch = 0x01; ch < CKEYMAPNUM; ch++)
			{
				key[0] = ch;
				s.assign(key);
				try
				{
					re.assign(keyre);
					if(std::regex_match(s, re))
					{
						ckeymap.keyvoid[ch] = configkeymap[i].skkfunc;
					}
				}
				catch(...)
				{
					break;
				}
			}
			break;
		default:
			break;
		}
	}
}

void CTextService::_LoadVKeyMap(LPCWSTR section)
{
	size_t i, j;
	WCHAR ch;
	WCHAR key[3];
	WCHAR keyre[KEYRELEN];
	std::wstring s;
	std::wregex re;
	std::wstring strxmlval;
	VKEYMAP *pkeymaps[] = {&vkeymap, &vkeymap_shift, &vkeymap_ctrl};

	for(j = 0; j < _countof(pkeymaps); j++)
	{
		ZeroMemory(pkeymaps[j], sizeof(*pkeymaps[j]));
	}

	for(i = 0; i < _countof(configkeymap); i++)
	{
		if(configkeymap[i].skkfunc == SKK_NULL)
		{
			break;
		}
		ReadValue(pathconfigxml, section, configkeymap[i].keyname, strxmlval);
		wcsncpy_s(keyre, strxmlval.c_str(), _TRUNCATE);
		if(keyre[0] == L'\0')
		{
			continue;
		}

		//全英/アスキーモード
		switch(configkeymap[i].skkfunc)
		{
		case SKK_JMODE:
		case SKK_ENTER:
		case SKK_CANCEL:
		case SKK_BACK:
		case SKK_DELETE:
		case SKK_LEFT:
		case SKK_UP:
		case SKK_RIGHT:
		case SKK_DOWN:
		case SKK_PASTE:
		case SKK_OTHERIME:
		case SKK_VIESC:
			for(j = 0; j < _countof(pkeymaps); j++)
			{
				for(ch = 0x01; ch < VKEYMAPNUM; ch++)
				{
					switch(j)
					{
					case 0:
						key[0] = ch;
						key[1] = L'\0';
						break;
					case 1:
						key[0] = L'S';
						key[1] = ch;
						key[2] = L'\0';
						break;
					case 2:
						key[0] = L'C';
						key[1] = ch;
						key[2] = L'\0';
						break;
					}
					s.assign(key);
					try
					{
						re.assign(keyre);
						if(std::regex_match(s, re))
						{
							if(pkeymaps[j]->keylatin[ch] != SKK_JMODE)	//「ひらがな」が優先
							{
								pkeymaps[j]->keylatin[ch] = configkeymap[i].skkfunc;
							}
						}
					}
					catch(...)
					{
						break;
					}
				}
			}
			break;
		default:
			break;
		}

		//ひらがな/カタカナモード
		switch(configkeymap[i].skkfunc)
		{
		case SKK_JMODE:
		case SKK_VOID:
			break;
		default:
			for(j = 0; j < _countof(pkeymaps); j++)
			{
				for(ch = 0x01; ch < VKEYMAPNUM; ch++)
				{
					switch(j)
					{
					case 0:
						key[0] = ch;
						key[1] = L'\0';
						break;
					case 1:
						key[0] = L'S';
						key[1] = ch;
						key[2] = L'\0';
						break;
					case 2:
						key[0] = L'C';
						key[1] = ch;
						key[2] = L'\0';
						break;
					}
					s.assign(key);
					try
					{
						re.assign(keyre);
						if(std::regex_match(s, re))
						{
							pkeymaps[j]->keyjmode[ch] = configkeymap[i].skkfunc;
						}
					}
					catch(...)
					{
						break;
					}
				}
			}
			break;
		}

		//無効
		switch(configkeymap[i].skkfunc)
		{
		case SKK_VOID:
			for(j = 0; j < _countof(pkeymaps); j++)
			{
				for(ch = 0x01; ch < VKEYMAPNUM; ch++)
				{
					switch(j)
					{
					case 0:
						key[0] = ch;
						key[1] = L'\0';
						break;
					case 1:
						key[0] = L'S';
						key[1] = ch;
						key[2] = L'\0';
						break;
					case 2:
						key[0] = L'C';
						key[1] = ch;
						key[2] = L'\0';
						break;
					}
					s.assign(key);
					try
					{
						re.assign(keyre);
						if(std::regex_match(s, re))
						{
							pkeymaps[j]->keyvoid[ch] = configkeymap[i].skkfunc;
						}
					}
					catch(...)
					{
						break;
					}
				}
			}
			break;
		default:
			break;
		}
	}
}

void CTextService::_LoadConvPoint()
{
	APPDATAXMLLIST list;
	int i = 0;

	ZeroMemory(conv_point, sizeof(conv_point));

	if(ReadList(pathconfigxml, SectionConvPoint, list) == S_OK && list.size() != 0)
	{
		FORWARD_ITERATION_I(l_itr, list)
		{
			if(i >= CONV_POINT_NUM)
			{
				break;
			}

			FORWARD_ITERATION_I(r_itr, *l_itr)
			{
				if(r_itr->first == AttributeCPStart)
				{
					conv_point[i][0] = r_itr->second.c_str()[0];
				}
				else if(r_itr->first == AttributeCPAlter)
				{
					conv_point[i][1] = r_itr->second.c_str()[0];
				}
				else if(r_itr->first == AttributeCPOkuri)
				{
					conv_point[i][2] = r_itr->second.c_str()[0];
				}
			}

			i++;
		}
	}
}

void CTextService::_LoadKana()
{
	APPDATAXMLLIST list;
	int i = 0;
	ROMAN_KANA_CONV rkc;
	WCHAR *pszb;
	size_t blen = 0;
	std::wregex re(L"[\\x00-\\x19]");
	std::wstring fmt(L"");

	roman_kana_conv.clear();
	roman_kana_conv.shrink_to_fit();
	ZeroMemory(isroman_tbl, sizeof(isroman_tbl));

	if(ReadList(pathconfigxml, SectionKana, list) == S_OK && list.size() != 0)
	{
		FORWARD_ITERATION_I(l_itr, list)
		{
			if(i >= ROMAN_KANA_TBL_MAX)
			{
				break;
			}

			ZeroMemory(&rkc, sizeof(rkc));

			FORWARD_ITERATION_I(r_itr, *l_itr)
			{
				pszb = NULL;

				if(r_itr->first == AttributeRoman)
				{
					pszb = rkc.roman;
					blen = _countof(rkc.roman);
				}
				else if(r_itr->first == AttributeHiragana)
				{
					pszb = rkc.hiragana;
					blen = _countof(rkc.hiragana);
				}
				else if(r_itr->first == AttributeKatakana)
				{
					pszb = rkc.katakana;
					blen = _countof(rkc.katakana);
				}
				else if(r_itr->first == AttributeKatakanaAnk)
				{
					pszb = rkc.katakana_ank;
					blen = _countof(rkc.katakana_ank);
				}
				else if(r_itr->first == AttributeSpOp)
				{
					rkc.soku = (_wtoi(r_itr->second.c_str()) & 0x1) ? TRUE : FALSE;
					rkc.wait = (_wtoi(r_itr->second.c_str()) & 0x2) ? TRUE : FALSE;
					rkc.func = (_wtoi(r_itr->second.c_str()) & 0x4) ? TRUE : FALSE;
				}

				if(pszb != NULL)
				{
					wcsncpy_s(pszb, blen, std::regex_replace(r_itr->second, re, fmt).c_str(), _TRUNCATE);
				}
			}

			roman_kana_conv.push_back(rkc);
			for(int j = 0; rkc.roman[j] != '\0'; j++) {
				if(rkc.roman[j] <= ISROMAN_TBL_SIZE) {
					isroman_tbl[rkc.roman[j]] = TRUE;
				}
			}
			i++;
		}
	}
}

void CTextService::_LoadJLatin()
{
	APPDATAXMLLIST list;
	int i = 0;
	WCHAR *pszb;
	size_t blen = 0;
	std::wregex re(L"[\\x00-\\x19]");
	std::wstring fmt(L"");

	ZeroMemory(ascii_jlatin_conv, sizeof(ascii_jlatin_conv));

	if(ReadList(pathconfigxml, SectionJLatin, list) == S_OK && list.size() != 0)
	{
		FORWARD_ITERATION_I(l_itr, list)
		{
			if(i >= ASCII_JLATIN_TBL_NUM)
			{
				break;
			}

			FORWARD_ITERATION_I(r_itr, *l_itr)
			{
				pszb = NULL;

				if(r_itr->first == AttributeLatin)
				{
					pszb = ascii_jlatin_conv[i].ascii;
					blen = _countof(ascii_jlatin_conv[i].ascii);
				}
				else if(r_itr->first == AttributeJLatin)
				{
					pszb = ascii_jlatin_conv[i].jlatin;
					blen = _countof(ascii_jlatin_conv[i].jlatin);
				}

				if(pszb != NULL)
				{
					wcsncpy_s(pszb, blen, std::regex_replace(r_itr->second, re, fmt).c_str(), _TRUNCATE);
				}
			}

			i++;
		}
	}
}
