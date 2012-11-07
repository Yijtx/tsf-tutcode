﻿
#include "eucjis2004.h"
#include "parseskkdic.h"

#define BUFSIZE 0x2000

LPCWSTR EntriesAri = L";; okuri-ari entries.\n";
LPCWSTR EntriesNasi = L";; okuri-nasi entries.\n";

int ReadSKKDicLine(FILE *fp, WCHAR bom, int &okuri, std::wstring &key, SKKDICCANDIDATES &c)
{
	CHAR buf[BUFSIZE*2];
	WCHAR wbuf[BUFSIZE];
	size_t size, is;
	void *rp;
	std::wstring s;
	std::wregex re;
	std::wstring fmt;

	c.clear();

	switch(bom)
	{
	case 0xFEFF:
		rp = fgetws(wbuf, _countof(wbuf), fp);
		break;
	default:
		rp = fgets(buf, _countof(buf), fp);
		break;
	}
	if(rp == NULL)
	{
		return -1;
	}

	switch(bom)
	{
	case 0xFEFF:
		break;
	default:
		size = _countof(wbuf);
		if(!EucJis2004ToWideChar(buf, NULL, wbuf, &size))
		{
			return 1;
		}
		break;
	}

	if(wcscmp(wbuf, EntriesAri) == 0)
	{
		okuri = 1;
		return 1;
	}
	else if(wcscmp(wbuf, EntriesNasi) == 0)
	{
		okuri = 0;
		return 1;
	}
	else
	{
		switch(wbuf[0])
		{
		case L'\0':
		case L'\r':
		case L'\n':
		case L';':
		case L'\x20':
			return 1;
			break;
		default:
			break;
		}
	}

	if(okuri == -1)
	{
		return 1;
	}

	s.assign(wbuf);
	re.assign(L"\\t|\\r|\\n");
	fmt.assign(L"");
	s = std::regex_replace(s, re, fmt);
	//送りありエントリのブロック形式を除去
	re.assign(L"\\[.+?/.+?/\\]/");
	fmt.assign(L"");
	s = std::regex_replace(s, re, fmt);

	is = s.find_first_of(L'\x20');
	if(is == std::wstring::npos)
	{
		return 1;
	}

	key = s.substr(0, is);
	s = s.substr(is + 1);

	ParseSKKDicCandiate(s, c);

	return 0;
}

void ParseSKKDicCandiate(const std::wstring &s, SKKDICCANDIDATES &d)
{
	size_t i, is, ie, ia;
	std::wstring candidate;
	std::wstring annotation;

	i = 0;
	while(i < s.size())
	{
		is = s.find_first_of(L'/', i);
		ie = s.find_first_of(L'/', is + 1);
		if(ie == std::wstring::npos)
		{
			break;
		}

		candidate = s.substr(i + 1, ie - is - 1);
		i = ie;

		ia = candidate.find_first_of(L';');

		if(ia == std::wstring::npos)
		{
			annotation.clear();
		}
		else
		{
			annotation = candidate.substr(ia + 1);
			candidate = candidate.substr(0, ia);
		}

		ParseLisp(candidate);
		ParseLisp(annotation);

		d.push_back(SKKDICCANDIDATE(candidate, annotation));
	}
}

void ParseLisp(std::wstring &s)
{
	std::wregex re;
	std::wstring fmt;

	if(s.find(L"concat") != std::wstring::npos)
	{
		re.assign(L".*\\(concat \".*(\\\\057|\\\\073).*\"\\).*");
		if(std::regex_match(s, re))
		{
			re.assign(L"(.*)\\(concat \"(.*)\"\\)(.*)");
			fmt.assign(L"$1$2$3");
			s = std::regex_replace(s, re, fmt);

			re.assign(L"\\\\057");
			fmt.assign(L"/");
			s = std::regex_replace(s, re, fmt);

			re.assign(L"\\\\073");
			fmt.assign(L";");
			s = std::regex_replace(s, re, fmt);
		}
	}
}