#ifndef		__BLURFILE_H__
#define		__BLURFILE_H__

#include	"BlurString.h"

extern bool				createMetapath();
extern bool				exists( std::string path );
extern bool				isExtension( std::string source );
extern bool				isFileName( std::string source );
extern bool				makedir( std::string path, bool recursive = false );
extern std::string		metapath();
extern std::string		normpath( std::string source, std::string separator = "/" );
extern Array*			splitext( std::string source );
extern Array*			splitpath( std::string source );

#endif		__BLURFILE_H__