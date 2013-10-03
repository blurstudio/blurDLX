#ifndef		__BLURFILE_H__
#define		__BLURFILE_H__

#include	"BlurString.h"

extern bool			createMetapath();
extern bool			exists( const TSTR & path );
extern bool			isExtension( const TSTR & source );
extern bool			isFileName( const TSTR & source );
extern bool			makedir( const TSTR & path, bool recursive = false );
extern TSTR			metapath();
extern TSTR 		normpath( const TSTR & source, TCHAR separator = '/' );
extern Array*		splitext( const TSTR & source );
extern Array*		splitpath( const TSTR & source );

#endif		__BLURFILE_H__