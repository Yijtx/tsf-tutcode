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
	_ReadBoolValue(SectionBehavior, ValueKeepInputNoR, cx_keepinputnor, TRUE);
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

	ReadValue(pathconfigxml, SectionDisplay, ValueUntilCandList, strxmlval);
	cx_untilcandlist = _wtoi(strxmlval.c_str());
	if(cx_untilcandlist > 9 || strxmlval.empty())
	{
		cx_untilcandlist = 5;
	}

	_ReadBoolValue(SectionDisplay, ValueVerticalCand, cx_verticalcand, FALSE);
	_ReadBoolValue(SectionDisplay, ValueDispCandNo, cx_dispcandnum, FALSE);
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
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;
	int i = 0;

	ZeroMemory(preservedkey, sizeof(TF_PRESERVEDKEY) * MAX_PRESERVEDKEY);

	if(ReadList(pathconfigxml, SectionPreservedKey, list) == S_OK && list.size() != 0)
	{
		for(l_itr = list.begin(); l_itr != list.end() && i < MAX_PRESERVEDKEY; l_itr++)
		{
			for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
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

static void _LoadVKeyMapSub(BYTE vk, BYTE modifiers, BYTE skkfunc, VKEYMAP *vkeymap)
{
	WORD vkm = (modifiers<<8) | vk;
	//全英/アスキーモード
	switch(skkfunc)
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
		if(vkeymap->keylatin.find(vkm) != vkeymap->keylatin.end())
		{
			if(vkeymap->keylatin[vkm] != SKK_JMODE)	//「ひらがな」が優先
			{
				vkeymap->keylatin[vkm] = skkfunc;
			}
		}
		else
		{
			vkeymap->keylatin[vkm] = skkfunc;
		}
		break;
	default:
		break;
	}

	//ひらがな/カタカナモード
	switch(skkfunc)
	{
	case SKK_JMODE:
	case SKK_VOID:
		break;
	default:
		vkeymap->keyjmode[vkm] = skkfunc;
		break;
	}

	//無効
	if(skkfunc == SKK_VOID)
	{
		vkeymap->keyvoid[vkm] = skkfunc;
	}
}

void CTextService::_LoadVKeyMap()
{
	APPDATAXMLLIST list;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;

	if(ReadList(pathconfigxml, SectionVKeyMap, list) == S_OK)
	{
		for(l_itr = list.begin(); l_itr != list.end(); l_itr++)
		{
			BYTE vk = 0;
			BYTE m = 0;
			BYTE skkfunc = 0;
			for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
			{
				if(r_itr->first == AttributeVKey)
				{
					vk = wcstoul(r_itr->second.c_str(), NULL, 0);
				}
				else if(r_itr->first == AttributeMKey)
				{
					m = wcstoul(r_itr->second.c_str(), NULL, 0) & (TF_MOD_ALT | TF_MOD_CONTROL | TF_MOD_SHIFT);
				}
				else if(r_itr->first == AttributeName)
				{
					LPCWSTR name = r_itr->second.c_str();
					for(size_t i = 0; i < _countof(configkeymap); i++)
					{
						if(wcscmp(configkeymap[i].keyname, name) == 0)
						{
							skkfunc = configkeymap[i].skkfunc;
							break;
						}
					}
				}
			}

			if(vk != 0 && skkfunc != 0)
			{
				_LoadVKeyMapSub(vk, m, skkfunc, &vkeymap);
			}
		}
	}
	else
	{
		_LoadVKeyMapSub(VK_DELETE, 0, SKK_DELETE, &vkeymap);
		_LoadVKeyMapSub(VK_LEFT,   0, SKK_LEFT,   &vkeymap);
		_LoadVKeyMapSub(VK_UP,     0, SKK_UP,     &vkeymap);
		_LoadVKeyMapSub(VK_RIGHT,  0, SKK_RIGHT,  &vkeymap);
		_LoadVKeyMapSub(VK_DOWN,   0, SKK_DOWN,   &vkeymap);
	}
}

void CTextService::_LoadKeyMap(LPCWSTR section, KEYMAP &keymap)
{
	size_t i;
	WCHAR ch;
	WCHAR key[2];
	WCHAR keyre[KEYRELEN];
	std::wstring s;
	std::wregex re;
	std::wstring strxmlval;

	ZeroMemory(&keymap, sizeof(keymap));
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
			for(ch = 0x01; ch < KEYMAPNUM; ch++)
			{
				key[0] = ch;
				s.assign(key);
				try
				{
					re.assign(keyre);
					if(std::regex_match(s, re))
					{
						if(keymap.keylatin[ch] != SKK_JMODE)	//「ひらがな」が優先
						{
							keymap.keylatin[ch] = configkeymap[i].skkfunc;
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
			for(ch = 0x01; ch < KEYMAPNUM; ch++)
			{
				key[0] = ch;
				s.assign(key);
				try
				{
					re.assign(keyre);
					if(std::regex_match(s, re))
					{
						keymap.keyjmode[ch] = configkeymap[i].skkfunc;
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
			for(ch = 0x01; ch < KEYMAPNUM; ch++)
			{
				key[0] = ch;
				s.assign(key);
				try
				{
					re.assign(keyre);
					if(std::regex_match(s, re))
					{
						keymap.keyvoid[ch] = configkeymap[i].skkfunc;
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

void CTextService::_LoadConvPoint()
{
	APPDATAXMLLIST list;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;
	int i = 0;

	ZeroMemory(conv_point, sizeof(conv_point));

	if(ReadList(pathconfigxml, SectionConvPoint, list) == S_OK && list.size() != 0)
	{
		for(l_itr = list.begin(); l_itr != list.end() && i < CONV_POINT_NUM; l_itr++)
		{
			for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
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
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;
	int i = 0;
	ROMAN_KANA_CONV rkc;
	WCHAR *pszb;
	size_t blen = 0;
	std::wregex re(L"[\\x00-\\x19]");
	std::wstring fmt(L"");

	roman_kana_conv.clear();
	roman_kana_conv.shrink_to_fit();

	if(ReadList(pathconfigxml, SectionKana, list) == S_OK && list.size() != 0)
	{
		for(l_itr = list.begin(); l_itr != list.end() && i < ROMAN_KANA_TBL_MAX; l_itr++)
		{
			ZeroMemory(&rkc, sizeof(rkc));

			for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
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
			i++;
		}
	}
}

void CTextService::_LoadJLatin()
{
	APPDATAXMLLIST list;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;
	int i = 0;
	WCHAR *pszb;
	size_t blen = 0;
	std::wregex re(L"[\\x00-\\x19]");
	std::wstring fmt(L"");

	ZeroMemory(ascii_jlatin_conv, sizeof(ascii_jlatin_conv));

	if(ReadList(pathconfigxml, SectionJLatin, list) == S_OK && list.size() != 0)
	{
		for(l_itr = list.begin(); l_itr != list.end() && i < ASCII_JLATIN_TBL_NUM; l_itr++)
		{
			for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
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
