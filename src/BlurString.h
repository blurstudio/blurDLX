#ifndef		__BLURSTRING_H__
#define		__BLURSTRING_H__

#ifdef __MAXSCRIPT_2012__
#include "maxscript\foundation\strings.h"
#else
#include "strings.h"
#endif

#include <algorithm>
#include <sstream>
#include <cctype>
#include <string>
#include <vector>

extern bool				endswith( std::string source, std::string search );
extern void				lstrip( std::string &source, std::string separator = "" );
extern std::string		join( Array* parts, std::string separator = " " );
extern void				replace( std::string &source, std::string searchString, std::string replaceString, bool ignoreCase = true, bool replaceAll = true );
extern void				rstrip( std::string &source, std::string separator = "" );
extern Array*			split( std::string source, std::string separator = " " );
extern bool				startswith( std::string source, std::string search );

#endif		__BLURSTRING_H__