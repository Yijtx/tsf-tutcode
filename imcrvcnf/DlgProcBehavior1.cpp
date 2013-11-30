﻿
#include "configxml.h"
#include "imcrvcnf.h"
#include "resource.h"

static struct {
	int id;
	LPCWSTR value;
	COLORREF color;
} colors[8] = {
	{IDC_COL_BG, ValueColorBG, RGB(0xFF,0xFF,0xFF)},
	{IDC_COL_FR, ValueColorFR, RGB(0x00,0x00,0x00)},
	{IDC_COL_SE, ValueColorSE, RGB(0x00,0x00,0xFF)},
	{IDC_COL_CO, ValueColorCO, RGB(0x80,0x80,0x80)},
	{IDC_COL_CA, ValueColorCA, RGB(0x00,0x00,0x00)},
	{IDC_COL_SC, ValueColorSC, RGB(0x80,0x80,0x80)},
	{IDC_COL_AN, ValueColorAN, RGB(0x80,0x80,0x80)},
	{IDC_COL_NO, ValueColorNO, RGB(0x00,0x00,0x00)}
};

void DrawColor(HWND hwnd, COLORREF col);

INT_PTR CALLBACK DlgProcBehavior1(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND hwnd;
	size_t i;
	WCHAR num[16];
	WCHAR fontname[LF_FACESIZE];
	int fontpoint, fontweight, x, y;
	BOOL fontitalic;
	CHOOSEFONTW cf;
	LOGFONT lf;
	HDC hdc;
	HFONT hFont;
	RECT rect;
	POINT pt;
	LONG w;
	FILE *fp;
	std::wstring strxmlval;
	CHOOSECOLORW cc;
	static COLORREF colCust[16];
	PAINTSTRUCT ps;

	switch(message)
	{
	case WM_INITDIALOG:
		ReadValue(pathconfigxml, SectionFont, ValueFontName, strxmlval);
		wcsncpy_s(fontname, strxmlval.c_str(), _TRUNCATE);

		ReadValue(pathconfigxml, SectionFont, ValueFontSize, strxmlval);
		fontpoint = _wtoi(strxmlval.c_str());
		ReadValue(pathconfigxml, SectionFont, ValueFontWeight, strxmlval);
		fontweight = _wtoi(strxmlval.c_str());
		ReadValue(pathconfigxml, SectionFont, ValueFontItalic, strxmlval);
		fontitalic = _wtoi(strxmlval.c_str());

		if(fontpoint < 8 || fontpoint > 72)
		{
			fontpoint = 12;
		}
		if(fontweight < 0 || fontweight > 1000)
		{
			fontweight = FW_NORMAL;
		}
		if(fontitalic != TRUE && fontitalic != FALSE)
		{
			fontitalic = FALSE;
		}

		SetDlgItemTextW(hDlg, IDC_EDIT_FONTNAME, fontname);
		hdc = GetDC(hDlg);
		hFont = CreateFontW(-MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72), 0, 0, 0,
			fontweight, fontitalic, FALSE, FALSE, SHIFTJIS_CHARSET,
			OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, fontname);
		SendMessage(GetDlgItem(hDlg, IDC_EDIT_FONTNAME), WM_SETFONT, (WPARAM)hFont, 0);
		ReleaseDC(hDlg, hdc);

		SetDlgItemInt(hDlg, IDC_EDIT_FONTPOINT, fontpoint, FALSE);

		ReadValue(pathconfigxml, SectionBehavior, ValueMaxWidth, strxmlval);
		w = strxmlval.empty() ? -1 : _wtol(strxmlval.c_str());
		if(w < 0)
		{
			w = MAX_WIDTH_DEFAULT;
		}
		_snwprintf_s(num, _TRUNCATE, L"%d", w);
		SetDlgItemTextW(hDlg, IDC_EDIT_MAXWIDTH, num);

		ZeroMemory(&colCust, sizeof(colCust));

		for(i = 0; i < _countof(colors); i++)
		{
			ReadValue(pathconfigxml, SectionBehavior, colors[i].value, strxmlval);
			if(!strxmlval.empty())
			{
				colors[i].color = wcstoul(strxmlval.c_str(), NULL, 0);
			}
		}

		hwnd = GetDlgItem(hDlg, IDC_COMBO_UNTILCANDLIST);
		num[1] = L'\0';
		for(i = 0; i <= 9; i++)
		{
			num[0] = L'0' + (WCHAR)i;
			SendMessage(hwnd, CB_ADDSTRING, 0, (LPARAM)num);
		}
		ReadValue(pathconfigxml, SectionBehavior, ValueUntilCandList, strxmlval);
		i = strxmlval.empty() ? 5 : _wtoi(strxmlval.c_str());
		if(i > 9)
		{
			i = 5;
		}
		SendMessage(hwnd, CB_SETCURSEL, (WPARAM)i, 0);

		LoadCheckButton(hDlg, IDC_CHECKBOX_DISPCANDNO, SectionBehavior, ValueDispCandNo);

		LoadCheckButton(hDlg, IDC_CHECKBOX_ANNOTATION, SectionBehavior, ValueAnnotation, L"1");
		LoadCheckButton(hDlg, IDC_RADIO_ANNOTATLST, SectionBehavior, ValueAnnotatLst);
		if(!IsDlgButtonChecked(hDlg, IDC_RADIO_ANNOTATLST))
		{
			CheckDlgButton(hDlg, IDC_RADIO_ANNOTATALL, BST_CHECKED);
		}

		LoadCheckButton(hDlg, IDC_CHECKBOX_SHOWMODEINL, SectionBehavior, ValueShowModeInl);
		LoadCheckButton(hDlg, IDC_RADIO_SHOWMODEIMM, SectionBehavior, ValueShowModeImm, L"1");
		if(!IsDlgButtonChecked(hDlg, IDC_RADIO_SHOWMODEIMM))
		{
			CheckDlgButton(hDlg, IDC_RADIO_SHOWMODEALL, BST_CHECKED);
		}

		LoadCheckButton(hDlg, IDC_CHECKBOX_SHOWMODEMARK, SectionBehavior, ValueShowModeMark, L"1");
		LoadCheckButton(hDlg, IDC_CHECKBOX_SHOWROMANCOMP, SectionBehavior, ValueShowRomanComp);

		return TRUE;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_CHOOSEFONT:
			hdc = GetDC(hDlg);

			hFont = (HFONT)SendMessage(GetDlgItem(hDlg, IDC_EDIT_FONTNAME), WM_GETFONT, 0, 0);
			GetObject(hFont, sizeof(LOGFONT), &lf);
			lf.lfHeight = -MulDiv(GetDlgItemInt(hDlg, IDC_EDIT_FONTPOINT, NULL, FALSE), GetDeviceCaps(hdc, LOGPIXELSY), 72);
			lf.lfCharSet = SHIFTJIS_CHARSET;

			ZeroMemory(&cf, sizeof(cf));
			cf.lStructSize = sizeof(CHOOSEFONTW);
			cf.hwndOwner = hDlg;
			cf.lpLogFont = &lf;
			cf.Flags = CF_INITTOLOGFONTSTRUCT | CF_NOVERTFONTS | CF_SCREENFONTS | CF_SELECTSCRIPT;

			if(ChooseFontW(&cf) == TRUE)
			{
				PropSheet_Changed(GetParent(hDlg), hDlg);

				SetDlgItemText(hDlg, IDC_EDIT_FONTNAME, lf.lfFaceName);
				lf.lfHeight = -MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72);
				hFont = CreateFontIndirect(&lf);
				SendMessage(GetDlgItem(hDlg, IDC_EDIT_FONTNAME), WM_SETFONT, (WPARAM)hFont, 0);
				SetDlgItemInt(hDlg, IDC_EDIT_FONTPOINT, cf.iPointSize / 10, FALSE);
			}

			ReleaseDC(hDlg, hdc);
			return TRUE;

		case IDC_EDIT_MAXWIDTH:
			switch(HIWORD(wParam))
			{
			case EN_CHANGE:
				PropSheet_Changed(GetParent(hDlg), hDlg);
				return TRUE;
			default:
				break;
			}
			break;

		case IDC_COMBO_UNTILCANDLIST:
			switch(HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				PropSheet_Changed(GetParent(hDlg), hDlg);
				return TRUE;
			default:
				break;
			}
			break;

		case IDC_CHECKBOX_DISPCANDNO:
		case IDC_CHECKBOX_ANNOTATION:
		case IDC_RADIO_ANNOTATALL:
		case IDC_RADIO_ANNOTATLST:
		case IDC_CHECKBOX_SHOWMODEINL:
		case IDC_RADIO_SHOWMODEALL:
		case IDC_RADIO_SHOWMODEIMM:
		case IDC_CHECKBOX_SHOWMODEMARK:
		case IDC_CHECKBOX_SHOWROMANCOMP:
			PropSheet_Changed(GetParent(hDlg), hDlg);
			return TRUE;

		default:
			break;
		}
		break;

	case WM_LBUTTONDOWN:
		for(i = 0; i < _countof(colors); i++)
		{
			hwnd = GetDlgItem(hDlg, colors[i].id);
			GetWindowRect(hwnd, &rect);
			pt.x = x = GET_X_LPARAM(lParam);
			pt.y = y = GET_Y_LPARAM(lParam);
			ClientToScreen(hDlg, &pt);

			if(rect.left <= pt.x && pt.x <= rect.right &&
				rect.top <= pt.y && pt.y <= rect.bottom)
			{
				cc.lStructSize = sizeof(cc);
				cc.hwndOwner = hDlg;
				cc.hInstance = NULL;
				cc.rgbResult = colors[i].color;
				cc.lpCustColors = colCust;
				cc.Flags = CC_FULLOPEN | CC_RGBINIT;
				cc.lCustData = NULL;
				cc.lpfnHook = NULL;
				cc.lpTemplateName = NULL;
				if(ChooseColorW(&cc))
				{
					DrawColor(hwnd, cc.rgbResult);
					colors[i].color = cc.rgbResult;
					PropSheet_Changed(GetParent(hDlg), hDlg);
					return TRUE;
				}
				break;
			}
		}
		break;
		
	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
		for(i = 0; i < _countof(colors); i++)
		{
			DrawColor(GetDlgItem(hDlg, colors[i].id), colors[i].color);
		}
		EndPaint(hDlg, &ps);
		return TRUE;
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			_wfopen_s(&fp, pathconfigxml, L"ab");
			if(fp != NULL)
			{
				fclose(fp);
			}
			SetFileDacl(pathconfigxml);

			WriterInit(pathconfigxml, &pXmlWriter, &pXmlFileStream);

			WriterStartElement(pXmlWriter, TagRoot);

			WriterStartSection(pXmlWriter, SectionFont);		//Start of SectionFont

			GetDlgItemTextW(hDlg, IDC_EDIT_FONTNAME, fontname, _countof(fontname));
			WriterKey(pXmlWriter, ValueFontName, fontname);

			hFont = (HFONT)SendMessage(GetDlgItem(hDlg, IDC_EDIT_FONTNAME), WM_GETFONT, 0, 0);
			GetObject(hFont, sizeof(LOGFONT), &lf);
			GetDlgItemTextW(hDlg, IDC_EDIT_FONTPOINT, num, _countof(num));
			WriterKey(pXmlWriter, ValueFontSize, num);
			_snwprintf_s(num, _TRUNCATE, L"%d", lf.lfWeight);
			WriterKey(pXmlWriter, ValueFontWeight, num);
			_snwprintf_s(num, _TRUNCATE, L"%d", lf.lfItalic);
			WriterKey(pXmlWriter, ValueFontItalic, num);

			WriterEndSection(pXmlWriter);						//End of SectionFont

			WriterStartSection(pXmlWriter, SectionBehavior);	//Start of SectionBehavior -> End at DlgProcBehavior2

			GetDlgItemTextW(hDlg, IDC_EDIT_MAXWIDTH, num, _countof(num));
			w = _wtol(num);
			if(w < 0)
			{
				w = MAX_WIDTH_DEFAULT;
			}
			_snwprintf_s(num, _TRUNCATE, L"%d", w);
			SetDlgItemTextW(hDlg, IDC_EDIT_MAXWIDTH, num);
			WriterKey(pXmlWriter, ValueMaxWidth, num);

			for(i = 0; i < _countof(colors); i++)
			{
				_snwprintf_s(num, _TRUNCATE, L"0x%06X", colors[i].color);
				WriterKey(pXmlWriter, colors[i].value, num);
			}

			hwnd = GetDlgItem(hDlg, IDC_COMBO_UNTILCANDLIST);
			num[0] = L'0' + (WCHAR)SendMessage(hwnd, CB_GETCURSEL, 0, 0);
			num[1] = L'\0';
			WriterKey(pXmlWriter, ValueUntilCandList, num);

			SaveCheckButton(hDlg, IDC_CHECKBOX_DISPCANDNO, ValueDispCandNo);
			SaveCheckButton(hDlg, IDC_CHECKBOX_ANNOTATION, ValueAnnotation);
			SaveCheckButton(hDlg, IDC_RADIO_ANNOTATLST, ValueAnnotatLst);
			SaveCheckButton(hDlg, IDC_CHECKBOX_SHOWMODEINL, ValueShowModeInl);
			SaveCheckButton(hDlg, IDC_RADIO_SHOWMODEIMM, ValueShowModeImm);
			SaveCheckButton(hDlg, IDC_CHECKBOX_SHOWMODEMARK, ValueShowModeMark);
			SaveCheckButton(hDlg, IDC_CHECKBOX_SHOWROMANCOMP, ValueShowRomanComp);

			//WriterEndSection(pXmlWriter);						//-> DlgProcBehavior2

			return TRUE;

		default:
			break;
		}
		break;

	default:
		break;
	}

	return FALSE;
}

void DrawColor(HWND hwnd, COLORREF col)
{
	RECT rect;
	HDC hdc;

	hdc = GetDC(hwnd);
	SelectObject(hdc, GetStockObject(BLACK_PEN));
	SetDCBrushColor(hdc, col);
	SelectObject(hdc, GetStockObject(DC_BRUSH));
	GetClientRect(hwnd, &rect);
	Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
	ReleaseDC(hwnd, hdc);
}
