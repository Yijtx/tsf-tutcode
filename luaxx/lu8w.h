/*
  UTF-8 Wrapper for Windows
*/

#ifndef U8W_H
#define U8W_H

#include <stdio.h>
#include <windows.h>

/* from utf-8 to utf-16 */
wchar_t *u8stows(const char *s);	/* call free function to deallocate */
/* from utf-16 to utf-8 */
char *u8wstos(const wchar_t *s);	/* call free function to deallocate */

DWORD u8GetModuleFileName(HMODULE hModule, LPSTR lpFilename, DWORD nSize);
DWORD u8FormatMessage(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId,
	LPSTR lpBuffer, DWORD nSize, va_list *Arguments);
HMODULE u8LoadLibraryEx(LPCSTR lpLibFileName, HANDLE hFile, DWORD dwFlags);

FILE *u8fopen(const char *fname, const char *mode);
FILE *u8freopen(const char *fname, const char *mode, FILE *oldfp);
FILE *u8popen(const char *command, const char *mode);
int u8fprintf(FILE *file, const char *format, ...);
int u8printf(const char *format, ...);
char *u8fgets(char *buf, int len, FILE *file);
int u8fputs(const char *buf, FILE *file);
char *u8getenv(const char *varname);	/* call free function to deallocate */
char *u8tmpnam(char *str);
int u8system(const char *command);
int u8remove(const char *fname);
int u8rename(const char *oldfname, const char *newfname);

#if !defined(lu8w_c)
#if defined(LUA_U8W) || defined(LUA_CORE) || defined(LUA_LIB) || defined(lua_c) || defined(luac_c)
#undef LoadString

#define GetModuleFileNameA u8GetModuleFileName
#define FormatMessageA u8FormatMessage
#define LoadLibraryExA u8LoadLibraryEx

#define fopen u8fopen
#define freopen u8freopen
#define _popen u8popen
#define fprintf u8fprintf
#define printf u8printf
#define fgets u8fgets
#define fputs u8fputs
#define getenv u8getenv
#define tmpnam u8tmpnam
#define system u8system
#define remove u8remove
#define rename u8rename
#endif /* LUA_CORE or LUA_LIB or lua_c or luac_c */
#endif /* not lu8w_c */

#endif /* U8W_H */
