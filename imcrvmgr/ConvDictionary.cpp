﻿
#include "configxml.h"
#include "imcrvmgr.h"
#include "eucjis2004.h"

#define BUFSIZE 0x2000

void ConvUserDic(const std::wstring &searchkey, CANDIDATES &candidates);
void ConvSKKDic(const std::wstring &searchkey, CANDIDATES &candidates);
void AddKeyOrder(const std::wstring &searchkey, KEYORDER &keyorder);
void DelKeyOrder(const std::wstring &searchkey, KEYORDER &keyorder);
void LoadKeyOrder(LPCWSTR section, KEYORDER &keyorder);
void WriteSKKUserDicEntry(FILE *fp, const USERDIC::iterator &userdic_itr);

//ユーザ辞書
USERDIC userdic;
//補完あり
KEYORDER complements;
//補完なし
KEYORDER accompaniments;

void ConvDictionary(const std::wstring &searchkey, CANDIDATES &candidates, WCHAR command)
{
	CANDIDATES::iterator candidates_itrf;
	CANDIDATES::iterator candidates_itrb;

	candidates.clear();

	//ユーザ辞書
	ConvUserDic(searchkey, candidates);

	//Unicodeコードポイント
	ConvUnicode(searchkey, candidates);

	//JIS X 0213 面区点番号
	ConvJISX0213(searchkey, candidates);

	//SKK辞書
	ConvSKKDic(searchkey, candidates);

	//SKK辞書サーバ
	if(serv)
	{
		ConvSKKServer(searchkey, candidates);
	}

	//重複候補を削除
	if(candidates.size() > 1)
	{
		for(candidates_itrf = candidates.begin(); candidates_itrf != candidates.end(); candidates_itrf++)
		{
			for(candidates_itrb = candidates_itrf + 1; candidates_itrb != candidates.end(); )
			{
				if(candidates_itrf->first == candidates_itrb->first)
				{
					candidates_itrb = candidates.erase(candidates_itrb);
				}
				else
				{
					candidates_itrb++;
				}
			}
		}
	}
}

void ConvUserDic(const std::wstring &searchkey, CANDIDATES &candidates)
{
	USERDIC::iterator userdic_itr;
	CANDIDATES::iterator candidates_itr;

	userdic_itr = userdic.find(searchkey);
	if(userdic_itr != userdic.end())
	{
		for(candidates_itr = userdic_itr->second.begin(); candidates_itr != userdic_itr->second.end(); candidates_itr++)
		{
			if(!candidates_itr->first.empty())
			{
				candidates.push_back(*candidates_itr);
			}
		}
	}
}

void ConvSKKDic(const std::wstring &searchkey, CANDIDATES &candidates)
{
	FILE *fpidx;
	ULONGLONG pos;
	long left, mid, right;
	int comp;
	APPDATAXMLDIC xmldic;
	APPDATAXMLDIC::iterator d_itr;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;
	std::wstring candidate;
	std::wstring annotation;
	std::wregex re(L"\\t|\\r|\\n");
	std::wstring fmt(L"");

	_wfopen_s(&fpidx, pathskkcvdicidx, L"rb");
	if(fpidx == NULL)
	{
		return;
	}

	left = 0;
	fseek(fpidx, 0, SEEK_END);
	right = (ftell(fpidx) / sizeof(pos)) - 1;

	while(left <= right)
	{
		mid = (left + right) / 2;
		fseek(fpidx, mid*sizeof(pos), SEEK_SET);
		fread(&pos, sizeof(pos), 1, fpidx);

		if(ReadDicList(pathskkcvdicxml, SectionDictionary, xmldic, pos) != S_OK)
		{
			break;
		}

		d_itr = xmldic.begin();
		if(d_itr != xmldic.end())
		{
			comp = searchkey.compare(d_itr->first);
		}
		else
		{
			break;
		}

		if(comp == 0)
		{
			for(l_itr = d_itr->second.begin(); l_itr != d_itr->second.end(); l_itr++)
			{
				candidate.clear();
				annotation.clear();

				for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
				{
					if(r_itr->first == AttributeCandidate)
					{
						candidate = std::regex_replace(r_itr->second, re, fmt);
					}
					else if(r_itr->first == AttributeAnnotation)
					{
						annotation = std::regex_replace(r_itr->second, re, fmt);
					}
				}

				if(candidate.empty())
				{
					continue;
				}

				candidates.push_back(CANDIDATE(candidate, annotation));
			}
			break;
		}
		else if(comp > 0)
		{
			left = mid + 1;
		}
		else
		{
			right = mid - 1;
		}
	}

	fclose(fpidx);
}

void AddUserDic(WCHAR command, const std::wstring &searchkey, const std::wstring &candidate, const std::wstring &annotation)
{
	USERDIC::iterator userdic_itr;
	CANDIDATES::iterator candidates_itr;
	USERDICENTRY userdicentry;

	userdic_itr = userdic.find(searchkey);
	if(userdic_itr == userdic.end())
	{
		userdicentry.first = searchkey;
		userdicentry.second.push_back(CANDIDATE(candidate, annotation));
		userdic.insert(userdicentry);
	}
	else
	{
		for(candidates_itr = userdic_itr->second.begin(); candidates_itr != userdic_itr->second.end(); candidates_itr++)
		{
			if(candidates_itr->first == candidate)
			{
				userdic_itr->second.erase(candidates_itr);
				break;
			}
		}
		userdic_itr->second.insert(userdic_itr->second.begin(), CANDIDATE(candidate, annotation));
	}

	switch(command)
	{
	case REQ_USER_ADD_0:
		AddKeyOrder(searchkey, accompaniments);
		break;
	case REQ_USER_ADD_1:
		AddKeyOrder(searchkey, complements);
		break;
	default:
		break;
	}

	bUserDicChg = TRUE;
}

void DelUserDic(WCHAR command, const std::wstring &searchkey, const std::wstring &candidate)
{
	USERDIC::iterator userdic_itr;
	CANDIDATES::iterator candidates_itr;

	userdic_itr = userdic.find(searchkey);
	if(userdic_itr != userdic.end())
	{
		for(candidates_itr = userdic_itr->second.begin(); candidates_itr != userdic_itr->second.end(); candidates_itr++)
		{
			if(candidates_itr->first == candidate)
			{
				userdic_itr->second.erase(candidates_itr);
				break;
			}
		}

		if(userdic_itr->second.empty())
		{
			userdic.erase(userdic_itr);

			switch(command)
			{
			case REQ_USER_DEL_0:
				DelKeyOrder(searchkey, accompaniments);
				break;
			case REQ_USER_DEL_1:
				DelKeyOrder(searchkey, complements);
				break;
			default:
				break;
			}
		}

		bUserDicChg = TRUE;
	}
}

void LoadUserDic()
{
	HRESULT hr;
	USERDIC::iterator userdic_itr;
	CANDIDATES::iterator candidates_itrs;
	USERDICENTRY userdicentry;
	APPDATAXMLDIC xmldic;
	APPDATAXMLDIC::iterator d_itr;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;
	std::wstring candidate;
	std::wstring annotation;
	std::wregex re(L"\\t|\\r|\\n");
	std::wstring fmt(L"");

	userdic.clear();

	hr = ReadDicList(pathuserdicxml, SectionDictionary, xmldic);
	EXIT_NOT_S_OK(hr);

	for(d_itr = xmldic.begin(); d_itr != xmldic.end(); d_itr++)
	{
		userdicentry.first = d_itr->first;

		for(l_itr = d_itr->second.begin(); l_itr != d_itr->second.end(); l_itr++)
		{
			candidate.clear();
			annotation.clear();

			for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
			{
				if(r_itr->first == AttributeCandidate)
				{
					candidate = std::regex_replace(r_itr->second, re, fmt);
				}
				else if(r_itr->first == AttributeAnnotation)
				{
					annotation = std::regex_replace(r_itr->second, re, fmt);
				}
			}

			if(candidate.empty())
			{
				continue;
			}

			userdic_itr = userdic.find(userdicentry.first);
			if(userdic_itr == userdic.end())
			{
				userdicentry.second.clear();
				userdicentry.second.push_back(CANDIDATE(candidate, annotation));
				userdic.insert(userdicentry);
			}
			else
			{
				for(candidates_itrs = userdic_itr->second.begin(); candidates_itrs != userdic_itr->second.end(); candidates_itrs++)
				{
					if(candidates_itrs->first == candidate)
					{
						break;
					}
				}
				if(candidates_itrs == userdic_itr->second.end())
				{
					userdic_itr->second.push_back(CANDIDATE(candidate, annotation));
				}
			}
		}
	}
	
	LoadKeyOrder(SectionComplement, complements);
	LoadKeyOrder(SectionAccompaniment, accompaniments);

NOT_S_OK:
	;
}

unsigned int __stdcall SaveUserDicThreadEx(void *p)
{
	HRESULT hr;
	IXmlWriter *pWriter;
	IStream *pFileStream;
	std::wstring s;
	USERDIC::iterator userdic_itr;
	CANDIDATES::iterator candidates_itr;
	KEYORDER::iterator keyorder_itr;

	APPDATAXMLATTR attr;
	APPDATAXMLROW row;
	APPDATAXMLLIST list;
	USERDIC::iterator u_itr;

	USERDATA *userdata = (USERDATA *)p;

	EnterCriticalSection(&csUserDataSave);	// !

	hr = WriterInit(pathuserdicxml, &pWriter, &pFileStream, FALSE);
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

	hr = WriterStartElement(pWriter, TagRoot);
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

	//ユーザ辞書
	hr = WriterStartSection(pWriter, SectionDictionary);
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

	for(u_itr = userdata->userdic.begin(); u_itr != userdata->userdic.end(); u_itr++)
	{
		list.clear();

		for(candidates_itr = u_itr->second.begin(); candidates_itr != u_itr->second.end(); candidates_itr++)
		{
			row.clear();

			attr.first = AttributeCandidate;
			attr.second = candidates_itr->first;
			row.push_back(attr);
			attr.first = AttributeAnnotation;
			attr.second = candidates_itr->second;
			row.push_back(attr);

			list.push_back(row);
		}
		
		hr = WriterStartElement(pWriter, TagEntry);
		EXIT_NOT_S_OK(hr);

		hr = WriterAttribute(pWriter, TagKey, u_itr->first.c_str());	//見出し語
		EXIT_NOT_S_OK(hr);

		hr = WriterList(pWriter, list);
		EXIT_NOT_S_OK(hr);

		hr = WriterEndElement(pWriter);	//TagEntry
		EXIT_NOT_S_OK(hr);

		hr = WriterNewLine(pWriter);
		EXIT_NOT_S_OK(hr);
	}

	hr = WriterEndSection(pWriter);	//SectionDictionary
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

	//補完あり
	hr = WriterStartSection(pWriter, SectionComplement);
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

	list.clear();

	for(keyorder_itr = userdata->complements.begin(); keyorder_itr != userdata->complements.end(); keyorder_itr++)
	{
		row.clear();

		attr.first = AttributeKey;
		attr.second = *keyorder_itr;
		row.push_back(attr);

		list.push_back(row);
	}

	hr = WriterList(pWriter, list, TRUE);
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

	hr = WriterEndSection(pWriter);	//SectionComplement
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

	//補完なし
	hr = WriterStartSection(pWriter, SectionAccompaniment);
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

	list.clear();

	for(keyorder_itr = userdata->accompaniments.begin(); keyorder_itr != userdata->accompaniments.end(); keyorder_itr++)
	{
		row.clear();

		attr.first = AttributeKey;
		attr.second = *keyorder_itr;
		row.push_back(attr);

		list.push_back(row);
	}

	hr = WriterList(pWriter, list, TRUE);
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

	hr = WriterEndSection(pWriter);	//SectionAccompaniment
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

	hr = WriterEndElement(pWriter);	//TagRoot
	EXIT_NOT_S_OK(hr);

	hr = WriterNewLine(pWriter);
	EXIT_NOT_S_OK(hr);

NOT_S_OK:
	hr = WriterFinal(&pWriter, &pFileStream);

	LeaveCriticalSection(&csUserDataSave);	// !

	delete userdata;

	return 0;
}

HANDLE StartSaveUserDicEx()
{
	HANDLE hThread = NULL;

	if(bUserDicChg)
	{
		bUserDicChg = FALSE;
		USERDATA *userdata = new USERDATA();
		userdata->userdic = userdic; 
		userdata->complements = complements;
		userdata->accompaniments = accompaniments;

		hThread = (HANDLE)_beginthreadex(NULL, 0, SaveUserDicThreadEx, userdata, 0, NULL);
	}

	return hThread;
}

void SaveUserDicThreadExClose(void *p)
{
	HANDLE hThread = (HANDLE)p;
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
}

void StartSaveUserDic()
{
	HANDLE hThread = StartSaveUserDicEx();
	if(hThread != NULL)
	{
		_beginthread(SaveUserDicThreadExClose, 0, hThread);
	}
}

void ConvComplement(const std::wstring &searchkey, CANDIDATES &candidates)
{
	KEYORDER::reverse_iterator keyorder_ritr;

	candidates.clear();

	if(!complements.empty())
	{
		for(keyorder_ritr = complements.rbegin(); keyorder_ritr != complements.rend(); keyorder_ritr++)
		{
			if(keyorder_ritr->compare(0, searchkey.size(), searchkey) == 0)
			{
				candidates.push_back(CANDIDATE(*keyorder_ritr, L""));
			}
		}
	}
}

void AddKeyOrder(const std::wstring &searchkey, KEYORDER &keyorder)
{
	DelKeyOrder(searchkey, keyorder);

	keyorder.push_back(searchkey);
}

void DelKeyOrder(const std::wstring &searchkey, KEYORDER &keyorder)
{
	KEYORDER::iterator keyorder_itr;

	if(!keyorder.empty())
	{
		for(keyorder_itr = keyorder.begin(); keyorder_itr != keyorder.end(); keyorder_itr++)
		{
			if(searchkey == *keyorder_itr)
			{
				keyorder.erase(keyorder_itr);
				break;
			}
		}
	}
}

void LoadKeyOrder(LPCWSTR section, KEYORDER &keyorder)
{
	HRESULT hr;
	KEYORDER::iterator keyorder_itr;
	APPDATAXMLLIST xmllist;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;
	std::wregex re(L"\\t|\\r|\\n");
	std::wstring fmt(L"");

	keyorder.clear();

	hr = ReadList(pathuserdicxml, section, xmllist);
	EXIT_NOT_S_OK(hr);

	for(l_itr = xmllist.begin(); l_itr != xmllist.end(); l_itr++)
	{
		for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
		{
			if(r_itr->first == AttributeKey)
			{
				for(keyorder_itr = keyorder.begin(); keyorder_itr != keyorder.end(); keyorder_itr++)
				{
					if(*keyorder_itr == r_itr->second)
					{
						keyorder.erase(keyorder_itr);
						keyorder_itr = keyorder.end();
						break;
					}
				}
				if(keyorder_itr == keyorder.end())
				{
					keyorder.push_back(std::regex_replace(r_itr->second, re, fmt));
				}
			}
		}
	}

NOT_S_OK:
	;
}

BOOL LoadSKKUserDic(LPCWSTR path)
{
	size_t size, i, is, ie;
	FILE *fpskkdic;
	WCHAR bom;
	CHAR buf[BUFSIZE*2];
	WCHAR wbuf[BUFSIZE];
	void *rp;
	std::wstring ca[2];
	std::wstring s;
	std::wregex re;
	std::wstring fmt;
	CANDIDATE entry;
	CANDIDATES entrys;
	CANDIDATES::iterator entrys_itr;
	CANDIDATES::reverse_iterator entrys_ritr;
	std::vector<std::wstring> es;
	CANDIDATE row;
	CANDIDATES list;
	CANDIDATES::reverse_iterator l_ritr;

	_wfopen_s(&fpskkdic, path, L"rb");
	if(fpskkdic == NULL)
	{
		return FALSE;
	}

	bom = L'\0';
	fread(&bom, 2, 1, fpskkdic);
	if(bom == 0xBBEF)
	{
		bom = L'\0';
		fread(&bom, 2, 1, fpskkdic);
		if((bom & 0xFF) == 0xBF)
		{
			bom = 0xFEFF;
		}
	}

	switch(bom)
	{
	case 0xFEFF:
		fclose(fpskkdic);
		_wfopen_s(&fpskkdic, path, RccsUNICODE);
		if(fpskkdic == NULL)
		{
			return FALSE;
		}
		break;
	default:
		fseek(fpskkdic, SEEK_SET, 0);
		break;
	}

	while(true)
	{
		switch(bom)
		{
		case 0xFEFF:
			rp = fgetws(wbuf, _countof(wbuf), fpskkdic);
			break;
		default:
			rp = fgets(buf, _countof(buf), fpskkdic);
			break;
		}
		if(rp == NULL)
		{
			break;
		}

		switch(bom)
		{
		case 0xFEFF:
			break;
		default:
			size = _countof(wbuf);
			if(!EucJis2004ToWideChar(buf, NULL, wbuf, &size))
			{
				continue;
			}
			break;
		}

		switch(wbuf[0])
		{
		case L'\0':
		case L'\r':
		case L'\n':
		case L';':
		case L'\x20':
			continue;
			break;
		default:
			break;
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
			continue;
		}

		entry.first = s.substr(0, is);
		entry.second = s.substr(is + 1);
		for(entrys_itr = entrys.begin(); entrys_itr != entrys.end(); entrys_itr++)
		{
			if(entrys_itr->first == entry.first)
			{
				entrys_itr->second += entry.second.substr(1);
				break;
			}
		}
		if(entrys_itr == entrys.end())
		{
			entrys.push_back(entry);
		}
	}

	fclose(fpskkdic);

	userdic.clear();
	complements.clear();
	accompaniments.clear();

	for(entrys_ritr = entrys.rbegin(); entrys_ritr != entrys.rend(); entrys_ritr++)
	{
		//エントリを「/」で分割
		es.clear();
		i = 0;
		s = entrys_ritr->second;
		while(i < s.size())
		{
			is = s.find_first_of(L'/', i);
			ie = s.find_first_of(L'/', is + 1);
			if(ie == std::wstring::npos)
			{
				break;
			}
			es.push_back(s.substr(i + 1, ie - is - 1));
			i = ie;
		}

		//候補と注釈を分割
		list.clear();
		for(i=0; i<es.size(); i++)
		{
			row.first.clear();
			row.second.clear();
			s = es[i];
			ie = s.find_first_of(L';');

			if(ie == std::wstring::npos)
			{
				row.first = s;
				row.second = L"";
			}
			else
			{
				row.first = s.substr(0, ie);
				row.second = s.substr(ie + 1);
			}

			list.push_back(row);
		}

		//concatを置換
		for(l_ritr = list.rbegin(); l_ritr != list.rend(); l_ritr++)
		{
			ca[0] = l_ritr->first;
			ca[1] = l_ritr->second;

			for(i=0; i<2; i++)
			{
				s = ca[i];
				re.assign(L".*\\(concat \".*\"\\).*");
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

					ca[i] = s;
				}
			}

			AddUserDic((std::regex_match(entrys_ritr->first,
				std::wregex(L"[^\\x00-\\x7F]+[a-z]")) ? REQ_USER_ADD_0 : REQ_USER_ADD_1),
				entrys_ritr->first, ca[0], ca[1]);
		}
	}

	return TRUE;
}

BOOL SaveSKKUserDic(LPCWSTR path)
{
	BOOL bRet = FALSE;
	LPCWSTR EntriesAri = L";; okuri-ari entries.\n";
	LPCWSTR EntriesNasi = L";; okuri-nasi entries.\n";
	FILE *fp;
	USERDIC userdic_tmp;
	USERDIC::iterator userdic_itr;
	KEYORDER::reverse_iterator keyorder_ritr;

	_wfopen_s(&fp, path, WccsUNICODE);
	if(fp == NULL)
	{
		goto NOT_S_OK;
	}

	userdic_tmp = userdic;

	//送りありエントリ
	fwprintf(fp, L"%s", EntriesAri);

	for(keyorder_ritr = accompaniments.rbegin(); keyorder_ritr != accompaniments.rend(); keyorder_ritr++)
	{
		userdic_itr = userdic_tmp.find(*keyorder_ritr);
		if(userdic_itr != userdic_tmp.end())
		{
			WriteSKKUserDicEntry(fp, userdic_itr);

			userdic_tmp.erase(userdic_itr);
		}
	}

	for(userdic_itr = userdic_tmp.begin(); userdic_itr != userdic_tmp.end(); )
	{
		if(std::regex_match(userdic_itr->first, std::wregex(L"[^\\x00-\\x7F]+[a-z]")))
		{
			WriteSKKUserDicEntry(fp, userdic_itr);

			userdic_itr = userdic_tmp.erase(userdic_itr);
		}
		else
		{
			userdic_itr++;
		}
	}

	//送りなしエントリ
	fwprintf(fp, L"%s", EntriesNasi);

	for(keyorder_ritr = complements.rbegin(); keyorder_ritr != complements.rend(); keyorder_ritr++)
	{
		userdic_itr = userdic_tmp.find(*keyorder_ritr);
		if(userdic_itr != userdic_tmp.end())
		{
			WriteSKKUserDicEntry(fp, userdic_itr);

			userdic_tmp.erase(userdic_itr);
		}
	}

	for(userdic_itr = userdic_tmp.begin(); userdic_itr != userdic_tmp.end(); )
	{
		WriteSKKUserDicEntry(fp, userdic_itr);

		userdic_itr = userdic_tmp.erase(userdic_itr);
	}

	fclose(fp);

	bRet = TRUE;

NOT_S_OK:

	return bRet;
}

void WriteSKKUserDicEntry(FILE *fp, const USERDIC::iterator &userdic_itr)
{
	CANDIDATES::iterator candidates_itr;
	std::wstring s;
	std::wregex re(L"\\t|\\r|\\n");
	std::wstring fmt(L"");
	std::wsmatch result;
	std::wstring ca[2];
	int i;

	fwprintf(fp, L"%s /", userdic_itr->first.c_str());

	for(candidates_itr = userdic_itr->second.begin(); candidates_itr != userdic_itr->second.end(); candidates_itr++)
	{
		ca[0] = candidates_itr->first;
		ca[1] = candidates_itr->second;

		for(i=0; i<2; i++)
		{
			s = ca[i];
			if(std::regex_search(s, result, std::wregex(L"[/;]")))
			{
				re.assign(L"/");
				fmt.assign(L"\\057");
				s = std::regex_replace(s, re, fmt);

				re.assign(L";");
				fmt.assign(L"\\073");
				s = std::regex_replace(s, re, fmt);

				s = L"(concat \"" + s + L"\")";

				ca[i] = s;
			}
		}

		fwprintf(fp, L"%s", ca[0].c_str());
		if(ca[1].empty())
		{
			fwprintf(fp, L"/");
		}
		else
		{
			fwprintf(fp, L";%s/", ca[1].c_str());
		}
	}

	fwprintf(fp, L"\n");
}
