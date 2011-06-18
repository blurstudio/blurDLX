#ifdef __MAXSCRIPT_2012__
#include "maxscript\maxscript.h"
#include "maxscript\foundation\numbers.h"
#include "maxscript\foundation\arrays.h"
#include "maxscript\compiler\thunks.h"
#include "maxscript\foundation\structs.h"
#else
#include "MAXScrpt.h"
#include "Numbers.h"
#include "arrays.h"
#include "thunks.h"
#include "Structs.h"
#endif
#include <sys/stat.h>
#include <io.h>

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "BlurString.h"

#ifdef __MAXSCRIPT_2012__
#include "maxscript\macros\define_external_functions.h"				// external name definitions
#include "maxscript\macros\define_instantiation_functions.h"		// internal name definitions
#else
#include "defextfn.h"			// external name definitions
#include "definsfn.h"			// internal name definitions
#endif

#define			n_includeFolder			(Name::intern(_T("includeFolder")))
#define			n_recursive				(Name::intern(_T("recursive")))

// ------------------------------------------------------------------------------------------------------
//											C++ METHODS
// ------------------------------------------------------------------------------------------------------

bool			isUNCPath( std::string path ) {
	return startswith( path, "\\\\" );
}
bool			isExtension( std::string source ) {
	std::string numeral( "0123456789" );
	std::string	chars( "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789" );
	std::string::iterator i;

	bool is_number	= true;
	bool is_valid	= true;
	for (i = source.begin(); i != source.end(); i++ ) {
		if ( chars.find(*i) != -1 ) {
			if ( numeral.find(*i) == -1 )
				is_number = false;
		}
		else {
			is_valid = false;
			break;
		}
	}

	return ( !is_number && is_valid );
}
bool			isFileName( std::string source ) {
	Array* pointList	= split( source, "." );
	if ( pointList->size > 1 )
		return isExtension( std::string( pointList->data[ pointList->size-1 ]->to_string() ) );
	return false;
}
std::string		normpath( std::string source, std::string separator = "/" ) {
	std::string out("");
	if ( source.length() > 0 ) {
		bool isUNC = isUNCPath( source );
		replace( source, "\\", "/", true, true );
		Array* pathList		= split( source, "/" );

		out					= join( pathList, separator );
		if ( !isFileName( std::string( pathList->data[ pathList->size-1 ]->to_string() ) ) )
			out				+= separator;

		if ( isUNC )
			out				= "\\\\" + out;
	}
	return out;
}
bool			exists( std::string path ) {
	if ( path.length() > 0 ) {
		struct stat St;
		std::string norm = normpath( path, "\\" );
		rstrip( norm, "\\" );
		if ( endswith( norm, ":" ) )
			norm += "\\";

		return ( stat( norm.c_str(), &St ) == 0 );
	}
	return false;
}
bool			makedir( std::string path, bool recursive = false ) {
	if ( path.length() > 0 ) {
		std::string norm = normpath( path, "\\" );
		if ( !recursive )
			CreateDirectory( norm.c_str(), NULL );
		else {
			std::string::iterator i;
			int pos = 0;
			while ( (pos = norm.find( "\\", pos + 1 )) != -1 ) {
				std::string newpath = norm.substr(0,pos+1);
				CreateDirectory( newpath.c_str(), NULL );
			}
		}
		return ( exists( path ) );
	}
	return false;
}
bool			rmdir( std::string path, bool recursive = false ) {
	HANDLE hFind;
	WIN32_FIND_DATA fileData;
	
	TCHAR dirPath[ MAX_PATH ];
	TCHAR fileName[ MAX_PATH ];

	_tcscpy( dirPath, path.c_str() );
	_tcscat( dirPath, "*" );								// Searching all files
	_tcscpy( fileName, path.c_str() );
	_tcscat( fileName, "" );

	hFind = FindFirstFile( dirPath, &fileData );			// Find the first file
	if ( hFind == INVALID_HANDLE_VALUE ) return false;
	
	_tcscpy( dirPath, fileName );

	bool bSearch = true;
	while ( bSearch ) {										// Until we find an entry
		if ( FindNextFile( hFind, &fileData ) ) {
			if ( !(_tcscmp( fileData.cFileName, "." ) && _tcscmp( fileData.cFileName, ".." )) ) continue;
			if ( recursive ) {								// Delete all files & folders
				_tcscat( fileName, fileData.cFileName );
				if ( fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
					// we have found a directory, recurse if desired
					if ( !rmdir( std::string( fileName ) + "\\", recursive ) ) {
						FindClose( hFind );
						return false;
					}
					RemoveDirectory( fileName );			// Remove the empty directory
					_tcscpy( fileName, dirPath );
				}
				else {
					if ( fileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY )
						_chmod( fileName, _S_IWRITE );			// Change the read only attribute
					if ( !DeleteFile( fileName ) ) {			// Delete the file
						FindClose( hFind );
						return false;
					}
					_tcscpy( fileName, dirPath );
				}
			}
			else {
				FindClose( hFind );
				return false;
			}
		}
		else {
			if ( GetLastError() == ERROR_NO_MORE_FILES )	// no more files there
				bSearch = false;
			else {
				// some error occurred, close the handle and return false
				FindClose( hFind );
				return false;
			}
		}
	}
	FindClose( hFind ); // closing the file handle
	return (RemoveDirectory(path.c_str())) ? true : false;		// Remove the empty directory
}
Array*			splitext( std::string source ) {
	one_typed_value_local( Array* out );
	vl.out = new Array(0);
	std::string norm	= normpath( source );
	Array* pointList	= split( norm, "." );
	if ( pointList->size > 0 && isExtension( std::string( pointList->data[ pointList->size-1 ]->to_string() ) ) ) {
		Array* temp		= new Array(0);
		for ( int i = 0; i < pointList->size - 1; i++ )
			temp->append( pointList->data[i] );

		vl.out->append( new String( join( temp, "." ).c_str() ) );
		vl.out->append( pointList->data[ pointList->size-1 ] );
	}
	else {
		vl.out->append( new String( source.c_str() ) );
		vl.out->append( new String( "" ) );
	}
	return_value( vl.out );
}
Array*			splitpath( std::string source ) {
	one_typed_value_local( Array* out );
	vl.out = new Array(0);

	std::string norm	= normpath( source );
	Array* pathList		= split( norm, "/" );
	if ( pathList->size > 0 ) {
		std::string	last( pathList->data[ pathList->size-1 ]->to_string() );
		if ( !isFileName( last ) ) {
			vl.out->append( new String( norm.c_str() ) );
			vl.out->append( new String( "" ) );
		}
		else {
			Array* temp	= new Array(0);
			for ( int i = 0; i < pathList->size - 1; i++ )
				temp->append( pathList->data[i] );
			vl.out->append( new String( normpath( join( temp, "/" ) ).c_str() ) );
			vl.out->append( new String( last.c_str() ) );
		}
	}

	return_value( vl.out );
}
std::string		metapath() {
	TSTR filepath		= GetCOREInterface()->GetCurFilePath();
	std::string out("");
	if ( filepath.length() > 0 ) {
		Array* pathList		= splitpath( std::string( filepath ) );
		out					= std::string( pathList->data[0]->to_string() ) + ".meta/";
	}
	return out;
}

bool			createMetapath() {
	std::string path	= metapath();
	bool success		= exists( path );
	if ( !success ) {
		if ( makedir( path ) ) {
			SetFileAttributes( path.c_str(), FILE_ATTRIBUTE_HIDDEN );
			success = true;
		}
	}
	return success;
}
// ------------------------------------------------------------------------------------------------------
//											BLURFILE METHODS
// ------------------------------------------------------------------------------------------------------

def_struct_primitive(	basename,		blurFile,		"basename" );
def_struct_primitive(	createMetapath,	blurFile,		"createMetapath" );
def_struct_primitive(	exists,			blurFile,		"exists" );
def_struct_primitive(	isUNCPath,		blurFile,		"isUNCPath" );
def_struct_primitive(	fileJoin,		blurFile,		"join" );
def_struct_primitive(	makedir,		blurFile,		"makedir" );
def_struct_primitive(	makedirs,		blurFile,		"makedirs" );
def_struct_primitive(	metapath,		blurFile,		"metapath" );
def_struct_primitive(	normpath,		blurFile,		"normpath" );
def_struct_primitive(	rmdir,			blurFile,		"rmdir" );
def_struct_primitive(	fileSplit,		blurFile,		"split" );
def_struct_primitive(	splitext,		blurFile,		"splitext" );
def_struct_primitive(	splitpath,		blurFile,		"splitpath" );

//----------------------------------------------------------------------------------------------

Value*			basename_cf(		Value** arg_list, int count ) {
	check_arg_count_with_keys( blurFile.basename, 1, count );

	one_value_local( out );
	vl.out = new String( "" );

	Value* includeFolder = key_arg_or_default( includeFolder, &true_value );
	if ( includeFolder == &true_value ) {
		Array* pathList = split( normpath( std::string( arg_list[0]->eval()->to_string() ) ), "/" );
		if ( pathList->size > 0 )
			vl.out = pathList->data[ pathList->size - 1 ];
	}
	else {
		Array* pathList = splitpath( std::string( arg_list[0]->eval()->to_string() ) );
		vl.out = pathList->data[1];
	}
	return_value( vl.out );
}
Value*			createMetapath_cf(	Value** arg_list, int count ) {
	check_arg_count( blurFile.createMetapath, 0, count );
	return ( createMetapath() ? &true_value : &false_value );
}
Value*			exists_cf(			Value** arg_list, int count ) {
	check_arg_count_with_keys( blurFile.exists, 1, count );
	return ( exists( std::string( arg_list[0]->eval()->to_string() ) ) ? &true_value : &false_value );
}
Value*			isUNCPath_cf(		Value** arg_list, int count ) {
	check_arg_count( blurFile.isUNCPath, 1, count );
	return ( isUNCPath( std::string( arg_list[0]->eval()->to_string() ) ) ) ? &true_value : &false_value;
}
Value*			fileJoin_cf(		Value** arg_list, int count ) {
	check_arg_count_with_keys( blurFile.buildpath, 1, count );

	Value* separator = key_arg(separator);
	if ( separator != &unsupplied )
		return new String( normpath( join( (Array*) arg_list[0]->eval(), "/" ), std::string( separator->to_string() ) ).c_str() );
	return new String( normpath( join( (Array*) arg_list[0]->eval(), "/" ) ).c_str() );

}
Value*			makedir_cf(			Value** arg_list, int count ) {
	check_arg_count( blurFile.makedir, 1, count );
	return ( makedir( std::string( arg_list[0]->eval()->to_string() ) ) ) ? &true_value : &false_value;
}
Value*			makedirs_cf(		Value** arg_list, int count ) {
	check_arg_count( blurFile.makedirs, 1, count );
	return ( makedir( std::string( arg_list[0]->eval()->to_string() ), true ) ) ? &true_value : &false_value;
}
Value*			metapath_cf(		Value** arg_list, int count ) {
	check_arg_count( blurFile.metapath, 0, count );
	return new String( metapath().c_str() );
}
Value*			normpath_cf(		Value** arg_list, int count ) {
	check_arg_count_with_keys( blurFile.normpath, 1, count );

	Value* separator = key_arg(separator);
	if ( separator != &unsupplied )
		return new String( normpath( std::string( arg_list[0]->eval()->to_string() ), std::string( separator->eval()->to_string() ) ).c_str() );
	return new String( normpath( std::string( arg_list[0]->eval()->to_string() ) ).c_str() );
}
Value*			fileSplit_cf(		Value** arg_list, int count ) {
	check_arg_count( blurFile.split, 1, count );
	return split( normpath( std::string( arg_list[0]->eval()->to_string() ) ), "/" );
}
Value*			rmdir_cf(			Value** arg_list, int count ) {
	check_arg_count_with_keys( blurFile.rmdir, 1, count );
	bool recursive = key_arg( recursive ) == &true_value;
	return (rmdir( normpath( arg_list[0]->eval()->to_string(), "\\" ), recursive )) ? &true_value : &false_value;
}
Value*			splitext_cf(		Value** arg_list, int count ) {
	check_arg_count( blurFile.splitext, 1, count );
	return splitext( std::string( arg_list[0]->eval()->to_string() ) );
}
Value*			splitpath_cf(		Value** arg_list, int count ) {
	check_arg_count( blurFile.splitpath, 1, count );
	return splitpath( std::string( arg_list[0]->eval()->to_string() ) );
}

// ------------------------------------------------------------------------------------------------------

void	BlurFileInit()		{}