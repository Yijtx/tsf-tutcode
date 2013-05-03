﻿
#include "imcrvmgr.h"

typedef std::vector<std::wstring> _GPARAM;
typedef std::wstring (*_GFUNC)(const _GPARAM &param);

static const struct {
	int year;
	LPWSTR kana;
	LPWSTR gengo[2];
} gengo[] = {
	{1989, L"へいせい", {L"平成", L"H"}},
	{1926, L"しょうわ", {L"昭和", L"S"}},
	{1912, L"たいしょう", {L"大正", L"T"}},
	{1868, L"めいじ", {L"明治", L"M"}}
};
static const LPWSTR month_en[] = {L"Jan",L"Feb",L"Mar",L"Apr",L"May",L"Jun",L"Jul",L"Aug",L"Sep",L"Oct",L"Nov",L"Dec"};
static const LPWSTR week_en[] = {L"Sun",L"Mon",L"Tue",L"Wed",L"Thu",L"Fri",L"Sat"};
static const LPWSTR week_jp[] = {L"日",L"月",L"火",L"水",L"木",L"金",L"土"};

static const struct {
	LPWSTR name;
	struct {
		LPWSTR name;
		double value;
	} u[2];
} units[] = {
	{L"mile", {{L"km", 1.6093}, {L"yard", 1760}}},
	{L"yard", {{L"cm", 91.44}, {L"feet", 3}}},
	{L"feet", {{L"cm", 30.48}, {L"inch", 12}}},
	{L"inch", {{L"cm", 2.54}, {L"feet", (1.0/12.0)}}}
};

static const LPWSTR omikuji[] = {L"大吉",L"中吉",L"小吉",L"吉",L"末吉",L"凶",L"大凶"};

static const LPWSTR s_window_width = L"80";
static const LPWSTR s_fill_column = L"70";
static const LPWSTR s_comment_start =L"//";
static const struct {
	LPWSTR name;
	LPWSTR value;
} values[] = {
	{L"window-width", s_window_width},
	{L"fill-column", s_fill_column},
	{L"comment-start", s_comment_start}
};

static std::wstring gadgetkey;
static std::vector<std::wstring> skk_num_list;
static time_t gadgettime;



static std::wstring xf(const _GPARAM &param)
{
	return L"";
}

static std::wstring substring(const _GPARAM &param)
{
	if(param.size() < 3) return L"";

	std::wstring s = param[0];
	std::wstring si = param[1];
	std::wstring ei = param[2];
	size_t is = _wtoi(si.c_str());
	size_t ie = _wtoi(ei.c_str());

	return s.substr(is, ie - is);
}

static std::wstring concat(const _GPARAM &param)
{
	if(param.size() < 1) return L"";

	std::wstring ret;
	std::wstring s = param[0];
	std::wstring numstr, substr = s;
	std::wregex renum(L"\\\\[0-7]{3}");
	std::wsmatch res;
	unsigned long u;

	while(std::regex_search(substr, res, renum))
	{
		ret += res.prefix();
		numstr = res.str();
		numstr[0] = L'0';
		u = wcstoul(numstr.c_str(), NULL, 0);
		if(u >= 0x20 && u <= 0x7E)
		{
			ret.append(1, (wchar_t)u);
		}
		else
		{
			ret += res.str();
		}
		substr = res.suffix();
	}
	ret += substr;

	return ret;
}

static std::wstring make_string(const _GPARAM &param)
{
	if(param.size() < 2) return L"";
	if(param[1].size() < 1) return L"";

	int count = _wtoi(param[0].c_str());
	if(count < 0) count = 0;
	else if(count >= 256) count = 255;
	WCHAR ch = param[1][1];
	if(param[1].size() == 1)
	{
		ch = param[1][0];
	}
	else
	{
		ch = param[1][1];
	}
	WCHAR str[256];
	wmemset(str, ch, count);
	str[count] = L'\0';

	return str;
}

static std::wstring string_to_char(const _GPARAM &param)
{
	if(param.size() < 1) return L"";
	if(param[0].size() < 1) return L"";

	std::wstring ret;
	ret.push_back(param[0][0]);

	return ret;
}

static std::wstring string_to_number(const _GPARAM &param)
{
	if(param.size() < 1) return L"";

	return param[0];
}

static std::wstring current_time_string(const _GPARAM &param)
{
	struct tm d;
	localtime_s(&d, &gadgettime);
	WCHAR st[32];
	//_wasctime_s(st, &d);
	//st[24] = L'\0';
	_snwprintf_s(st, _TRUNCATE, L"%s %s %2d %02d:%02d:%02d %04d",
		week_en[d.tm_wday], month_en[d.tm_mon], d.tm_mday,
		d.tm_hour, d.tm_min, d.tm_sec, d.tm_year + 1900);

	return st;
}

static std::wstring car(const _GPARAM &param)
{
	if(param.size() < 1) return L"";

	std::wstring list = param[0];
	std::wstring ret;
	if(list.compare(L"skk-num-list") == 0 && !skk_num_list.empty())
	{
		ret = skk_num_list[0];
	}

	return ret;
}

static std::wstring lambda(const _GPARAM &param)
{
	if(param.size() < 2) return L"";

	return param[1];
}

#if 0
static std::wstring plus1(const _GPARAM &param)
{
	if(param.size() < 1) return L"";

	WCHAR num[32];
	_snwprintf_s(num, _TRUNCATE, L"%d", _wtoi(param[0].c_str()) + 1);

	return num;
}

static std::wstring minus1(const _GPARAM &param)
{
	if(param.size() < 1) return L"";

	WCHAR num[32];
	_snwprintf_s(num, _TRUNCATE, L"%d", _wtoi(param[0].c_str()) - 1);

	return num;
}
#endif

static std::wstring plus(const _GPARAM &param)
{
	int n = 0;
	for(size_t i = 0; i<param.size(); i++)
	{
		n += _wtoi(param[i].c_str());
	}

	WCHAR num[32];
	_snwprintf_s(num, _TRUNCATE, L"%d", n);

	return num;
}

static std::wstring minus(const _GPARAM &param)
{
	int n = 0;

	if(param.size() == 1)
	{
		n -= _wtoi(param[0].c_str());
	}
	else
	{
		n = _wtoi(param[0].c_str());
		for(size_t i = 1; i<param.size(); i++)
		{
			n -= _wtoi(param[i].c_str());
		}
	}

	WCHAR num[32];
	_snwprintf_s(num, _TRUNCATE, L"%d", n);

	return num;
}

#if 0
static std::wstring mul(const _GPARAM &param)
{
	int n = 1;
	for(size_t i = 0; i<param.size(); i++)
	{
		n *= _wtoi(param[i].c_str());
	}

	WCHAR num[32];
	_snwprintf_s(num, _TRUNCATE, L"%d", n);

	return num;
}

static std::wstring div(const _GPARAM &param)
{
	int n = 1;
	for(size_t i = 0; i<param.size(); i++)
	{
		if(_wtoi(param[i].c_str()) == 0)
		{
			n = 0;
			break;
		}
		n /= _wtoi(param[i].c_str());
	}

	WCHAR num[32];
	_snwprintf_s(num, _TRUNCATE, L"%d", n);

	return num;
}

static std::wstring mod(const _GPARAM &param)
{
	if(param.size() < 2) return L"";
	
	int n1 = _wtoi(param[0].c_str());
	int n2 = _wtoi(param[1].c_str());
	if(n2 == 0)
	{
		n1 = 0;
		n2 = 1;
	}

	WCHAR num[32];
	_snwprintf_s(num, _TRUNCATE, L"%d", n1 % n2);

	return num;
}
#endif



static std::wstring window_width(const _GPARAM &param)
{
	return s_window_width;
}

static std::wstring fill_column(const _GPARAM &param)
{
	return s_fill_column;
}



static std::wstring skk_version(const _GPARAM &param)
{
	return TEXTSERVICE_NAME L" " TEXTSERVICE_VER;
}

static std::wstring skk_omikuji(const _GPARAM &param)
{
	srand(clock());
	return omikuji[rand() % _countof(omikuji)];
}

static std::wstring skk_gadget_units_conversion(const _GPARAM &param)
{
	if(param.size() < 3) return L"";

	std::wstring ut1 = param[0];
	std::wstring num = param[1];
	std::wstring ut2 = param[2];
	std::wstring ret;
	size_t i, j;

	for(i=0; i<_countof(units); i++)
	{
		if(ut1.compare(units[i].name) == 0)
		{
			for(j=0; j<_countof(units[i].u); j++)
			{
				if(ut2.compare(units[i].u[j].name) == 0)
				{
					double dbl = _wtof(num.c_str()) * units[i].u[j].value;
					WCHAR sdbl[32];
					_snwprintf_s(sdbl, _TRUNCATE, L"%g", dbl);
					ret = sdbl + ut2;
					break;
				}
			}
			break;
		}
	}

	return ret;
}



static std::wstring conv_ad_to_gengo(const std::wstring &num, const std::wstring &gengotype, const std::wstring &type, const std::wstring &div)
{
	std::wstring ret;
	WCHAR ystr[32];
	int year = _wtoi(num.c_str());

	for(int i=0; i<_countof(gengo); i++)
	{
		if(year >= gengo[i].year)
		{
			ret = gengo[i].gengo[_wtoi(gengotype.c_str())] + div;
			if(year - gengo[i].year == 0)
			{
				ret += L"元";
			}
			else
			{
				_snwprintf_s(ystr, _TRUNCATE, L"%d", year - gengo[i].year + 1);
				ret += ConvNum(ystr, type);
			}
			break;
		}
	}

	return ret;
}

static std::wstring skk_ad_to_gengo(const _GPARAM &param)
{
	if(param.size() < 3) return L"";

	std::wstring num = skk_num_list[0];
	std::wstring gengotype = param[0];
	std::wstring div = param[1];
	if(div.compare(L"nil") == 0) div.clear();
	std::wstring tail = param[2];
	if(tail.compare(L"nil") == 0) tail.clear();

	std::wstring ret = conv_ad_to_gengo(num, gengotype, L"#0", div);
	if(!ret.empty())
	{
		ret += tail;
	}

	return ret;
}

static std::wstring conv_gengo_to_ad(const std::wstring &num, const std::wstring &gengokana, const std::wstring &head, const std::wstring &tail)
{
	std::wstring ret;
	WCHAR ystr[32];
	int year = _wtoi(num.c_str());

	for(int i=0; i<_countof(gengo); i++)
	{
		if(gengokana.compare(gengo[i].kana) == 0)
		{
			_snwprintf_s(ystr, _TRUNCATE, L"%d", year + gengo[i].year - 1);
			ret = head + ystr + tail;
			break;
		}
	}

	return ret;
}

static std::wstring skk_gengo_to_ad(const _GPARAM &param)
{
	if(param.size() < 2) return L"";

	std::wstring num = skk_num_list[0];
	std::wstring head = param[0];
	std::wstring tail = param[1];

	std::wsmatch m;
	std::regex_search(gadgetkey, m, std::wregex(L"[0-9]+"));
	std::wstring ret = conv_gengo_to_ad(num, m.prefix(), head, tail);

	return ret;
}

static std::wstring skk_current_date(const _GPARAM &param)
{
	if(param.size() > 0) return param[0];

	struct tm d;
	localtime_s(&d, &gadgettime);
	WCHAR y[5];
	_snwprintf_s(y, _TRUNCATE, L"%d", d.tm_year + 1900);
	std::wstring gg = conv_ad_to_gengo(y, L"0", L"#1", L"");
	if (gg.empty())
	{
		return L"";
	}
	WCHAR st[32];
	WCHAR sm[4];
	WCHAR sd[4];
	_snwprintf_s(sm, _TRUNCATE, L"%d", d.tm_mon + 1);
	_snwprintf_s(sd, _TRUNCATE, L"%d", d.tm_mday);
	_snwprintf_s(st, _TRUNCATE, L"%s年%s月%s日(%s)",
		gg.c_str(), ConvNum(sm, L"#1").c_str(), ConvNum(sd, L"#1").c_str(), week_jp[d.tm_wday]);

	return st;
}

static std::wstring skk_default_current_date(const _GPARAM &param)
{
	if(param.size() < 7) return L"";

	std::wstring format = param[1];
	if(format == L"nil")
	{
		format = L"%s年%s月%s日(%s)";
		if(param.size() > 7 && param[7] != L"nil")
		{
			format += L" %s:%s:%s";
		}
	}
	std::wstring numtype = L"#" + param[2];

	struct tm d;
	localtime_s(&d, &gadgettime);

	std::wstring year;
	WCHAR yy[5];
	_snwprintf_s(yy, _TRUNCATE, L"%d", d.tm_year + 1900);
	if(param[3] == L"nil")
	{
		year = ConvNum(yy, numtype);
	}
	else
	{
		year = conv_ad_to_gengo(yy, param[4], numtype, L"");
		if (year == L"")
		{
			return L"";
		}
	}

	std::wstring month;
	WCHAR mm[4];
	_snwprintf_s(mm, _TRUNCATE, L"%d", d.tm_mon + 1);
	if(param[5] == L"nil")
	{
		month = month_en[d.tm_mon];
	}
	else
	{
		month = ConvNum(mm, numtype);
	}

	std::wstring day;
	WCHAR dd[4];
	_snwprintf_s(dd, _TRUNCATE, L"%d", d.tm_mday);
	day = ConvNum(dd, numtype);

	std::wstring dayofweek;
	if(param[6] == L"nil")
	{
		dayofweek = week_en[d.tm_wday];
	}
	else
	{
		dayofweek = week_jp[d.tm_wday];
	}

	WCHAR hour[4] = L"";
	WCHAR min[4] = L"";
	WCHAR sec[4] = L"";
	if(param.size() > 7 && param[7] != L"nil")
	{
		_snwprintf_s(hour, _TRUNCATE, L"%02d", d.tm_hour);
		_snwprintf_s(min, _TRUNCATE, L"%02d", d.tm_min);
		_snwprintf_s(sec, _TRUNCATE, L"%02d", d.tm_sec);
	}

	std::wstring ret;
	std::wstring substr = format;
	std::wregex renum(L"%s");
	std::wsmatch res;
	size_t count = 0;
	while(std::regex_search(substr, res, renum))
	{
		ret += res.prefix();
		switch(count)
		{
		case 0: ret += year; break;
		case 1: ret += month; break;
		case 2: ret += day; break;
		case 3: ret += dayofweek; break;
		case 4: ret += hour; break;
		case 5: ret += min; break;
		case 6: ret += sec; break;
		default: ret += res.str(); break;
		}
		substr = res.suffix();
		count++;
	}
	ret += substr;

	return ret;
}



struct {
	LPWSTR name;
	_GFUNC func;
} func[] ={
	{L"substring", substring},
	{L"concat", concat},
	{L"make-string", make_string},
	{L"string-to-char", string_to_char},
	{L"string-to-number", string_to_number},
	{L"current-time-string", current_time_string},
	{L"car", car},
	{L"lambda", lambda},
#if 0
	{L"1+", plus1},
	{L"1-", minus1},
#endif
	{L"+", plus},
	{L"-", minus},
#if 0
	{L"*", mul},
	{L"/", div},
	{L"%", mod},
	//{L"mod", moda},
#endif
#if 0
	{L"and", xf},
	{L"or", xf},
	{L"=", xf},
	{L"/=", xf},
	{L"<", xf},
	{L">", xf},
	{L"<=", xf},
	{L">=", xf},
	{L"max", xf},
	{L"min", xf},
	{L"abs", xf},
#endif
	{L"window-width", window_width},
	{L"fill-column", fill_column},

	{L"skk-version", skk_version},
	{L"skk-omikuji", skk_omikuji},
	{L"skk-gadget-units-conversion", skk_gadget_units_conversion},

	{L"skk-ad-to-gengo", skk_ad_to_gengo},
	{L"skk-gengo-to-ad", skk_gengo_to_ad},
	{L"skk-current-date", skk_current_date},
	{L"skk-default-current-date", skk_default_current_date},

	{L"", xf}
};

static std::wstring trimdq(const std::wstring s)
{
	if(s.size() >= 2 && s[0] == L'\"' && s[s.size() - 1] == L'\"')
	{
		return s.substr(1, s.size() - 2);
	}
	return s;
}

static std::wstring gadget_func(const std::wstring s)
{
	std::wstring ret;
	size_t i, j;
	size_t is = s.find_first_of(L'(');
	size_t ie = s.find_last_of(L')');
	std::wstring trim;
	std::wstring funcname;
	_GPARAM param;
	size_t iparam, ibracket, ibracketcnt, idqcnt;

	std::wstring ss = s.substr(is + 1, ie - is - 1);
	if(ss.empty())
	{
		return s;
	}

	i = ss.find_first_of(L'\x20');
	if(i == std::wstring::npos)
	{
		funcname = ss;
	}
	else
	{
		funcname = ss.substr(0, i);

		iparam = i + 1;
		ibracketcnt = 0;
		idqcnt = 0;
		for(i=i+1; i<ss.size(); i++)
		{
			if(ss[i] == L'\"' && idqcnt == 0)
			{
				idqcnt = 1;
			}
			else if(ss[i] == L'\"' && idqcnt == 1)
			{
				idqcnt = 0;
			}

			if(ss[i] == L'(' && idqcnt == 0)
			{
				if(ibracketcnt == 0) ibracket = i;
				ibracketcnt++;
			}
			else if(ss[i] == L')' && idqcnt == 0)
			{
				ibracketcnt--;
				if(ibracketcnt == 0)
				{
					param.push_back(gadget_func(trimdq(ss.substr(ibracket, i - ibracket + 1))));
					iparam = i + 2;
					i++;
				}
			}
			else if(ss[i] == L'\x20' && ibracketcnt == 0 && idqcnt == 0)
			{
				param.push_back(trimdq(ss.substr(iparam, i - iparam)));
				iparam = i + 1;
			}
		}
		if(ss.size() > iparam)
		{
			param.push_back(trimdq(ss.substr(iparam)));
		}
	}

	for(i=0; i<param.size(); i++)
	{
		for(j=0; j<_countof(values); j++)
		{
			if(param[i].compare(values[j].name) == 0)
			{
				param[i] = values[j].value;
				break;
			}
		}
	}

	for(i=0; i<_countof(func); i++)
	{
		if(funcname.compare(func[i].name) == 0)
		{
			ret = func[i].func(param);
			break;
		}
	}

	if(ret.empty())
	{
		ret = s;
	}

	return ret;
}

std::wstring ConvGaget(const std::wstring &key, const std::wstring &candidate)
{
	std::wstring ret;
	size_t i;
	size_t is = 0;
	size_t ie = 0;

	for(i=0; i<candidate.size(); i++)
	{
		if(candidate[i] == L'(') is++;
	}
	for(i=0; i<candidate.size(); i++)
	{
		if(candidate[i] == L')') ie++;
	}
	if(is != ie)
	{
		return candidate;
	}

	gadgetkey = key;
	gadgettime = time(NULL);

	skk_num_list.clear();
	std::wsmatch m;
	std::regex_search(key, m, std::wregex(L"\\d+"));
	for(i=0; i<m.size(); i++)
	{
		skk_num_list.push_back(m[i].str());
	}

	ret = gadget_func(candidate);

	if(ret.empty())
	{
		ret = candidate;
	}

	return ret;
}
