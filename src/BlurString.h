#ifndef		__BLURSTRING_H__
#define		__BLURSTRING_H__

#include <algorithm>
#include <sstream>
#include <cctype>
#include <string>
#include <vector>

#include "imports.h"

int find( const TSTR & source, TCHAR ch, int i = 0 );
int find( const TSTR & source, const TSTR & match, int i = 0 );
bool matches( const TSTR & source, const TSTR & compare, int sourcePos );
bool endswith( const TSTR & source, const TSTR & search );
void lstrip( TSTR &source, const TSTR & separator = _T("") );
TSTR join( Array* parts, const TSTR & separator = _T(" ") );
void replace( TSTR & source, const TSTR & searchString, const TSTR & replaceString, bool ignoreCase = true, bool replaceAll = true );
void rstrip( TSTR & source, const TSTR & separator = _T("") );
Array * split( const TSTR & source, const TSTR & separator = _T(" ") );
bool startswith( const TSTR & source, const TSTR & search );
bool isNum( const TCHAR & ch );
void operator+=(TSTR & str, TCHAR chr);
TSTR & operator+(TSTR & str, TCHAR chr);
#endif		__BLURSTRING_H__