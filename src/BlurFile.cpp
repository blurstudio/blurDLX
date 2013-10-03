
#include "imports.h"

#include <sys/stat.h>
#include <io.h>


#include "BlurString.h"

#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
#include "macros/define_external_functions.h"
#include "macros/define_instantiation_functions.h"
#else
#include "defextfn.h"
#include "definsfn.h"
#endif

#define n_includeFolder (Name::intern(_T("includeFolder")))
#define n_recursive (Name::intern(_T("recursive")))

// ------------------------------------------------------------------------------------------------------
//											C++ METHODS
// ------------------------------------------------------------------------------------------------------

static bool isUNCPath( const TSTR & path )
{
	return (path.Length() > 2) && (path[0] == '\\') && (path[1] == '\\');
}

static bool isAlpha( const TCHAR & ch )
{ return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'); }

static bool isAlnum( const TCHAR & ch )
{ return isAlpha(ch) || isNum(ch); }

static bool isSep( const TCHAR & ch )
{ return ch == '/' || ch == '\\'; }

bool isExtension( const TSTR & source )
{
	bool isNumber	= true;
	for ( int i = 0, end = source.Length(); i < end; ++i ) {
		TCHAR c = source[i];
		if( !isAlnum(c) )
			return false;
		if( !isNum(c) )
			isNumber = false;
	}
	return !isNumber;
}

bool isFileName( const TSTR & source )
{
	int pos = source.last(_T('.'));
	// Needs at least one dot and a valid extension
	if( pos < 0 || pos >= source.Length()-1 )
		return false;
	pos++;
	return isExtension(source.Substr(pos,source.Length()-pos));
}

TSTR normpath( const TSTR & source, TCHAR sep = '/' )
{
	TSTR out(source);
	TCHAR * outData = out.dataForWrite(out.Length());
	if( source.Length() > 0 ) {
		int d = 0, i = 0;
		if( isUNCPath(source) )
			d = i = 2;
		bool lastIsSep = false;
		for( int end = out.length(); i < end; ++i ) {
			TCHAR ch = source[i];
			if( isSep(ch) ) {
				if( !lastIsSep )
					outData[d++] = sep;
			} else {
				lastIsSep = false;
				outData[d++] = ch;
			}
		}
	}
	return out;
}

bool exists( const TSTR & path )
{
	if ( path.Length() > 0 ) {
		struct _stat St;
		TSTR norm = normpath( path, '\\' );
		while( norm.EndsWith('\\') )
			norm = norm.Substr(0,norm.Length()-1);
		if( norm.EndsWith(':') )
			norm += _T("\\");
		return (_tstat(norm.data(), &St) == 0);
	}
	return false;
}

bool makedir( const TSTR & path, bool recursive = false )
{
	if( path.Length() > 0 ) {
		TSTR norm = normpath( path, '\\' );
		if ( !recursive )
			CreateDirectory( norm.data(), NULL );
		else {
			int pos = 0;
			while ( (pos = find( norm, '\\', pos + 1 )) != -1 ) {
				TSTR newpath = norm.Substr(0,pos+1);
				CreateDirectory( newpath.data(), NULL );
			}
		}
		return exists(path);
	}
	return false;
}

bool rmdir( const TSTR & path, bool recursive = false )
{
	HANDLE hFind;
	WIN32_FIND_DATA fileData;
	
	TCHAR dirPath[ MAX_PATH ];
	TCHAR fileName[ MAX_PATH ];

	_tcscpy( dirPath, path.data() );
	_tcscat( dirPath, _T("*") );								// Searching all files
	_tcscpy( fileName, path.data() );
	_tcscat( fileName, _T("") );

	hFind = FindFirstFile( dirPath, &fileData );			// Find the first file
	if ( hFind == INVALID_HANDLE_VALUE ) return false;
	
	_tcscpy( dirPath, fileName );

	bool bSearch = true;
	while ( bSearch ) {										// Until we find an entry
		if ( FindNextFile( hFind, &fileData ) ) {
			if ( !(_tcscmp( fileData.cFileName, _T(".") ) && _tcscmp( fileData.cFileName, _T("..") )) ) continue;
			if ( recursive ) {								// Delete all files & folders
				_tcscat( fileName, fileData.cFileName );
				if ( fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {
					// we have found a directory, recurse if desired
					if ( !rmdir( TSTR( fileName ) + _T("\\"), recursive ) ) {
						FindClose( hFind );
						return false;
					}
					RemoveDirectory( fileName );			// Remove the empty directory
					_tcscpy( fileName, dirPath );
				}
				else {
					if ( fileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY )
						_tchmod( fileName, _S_IWRITE );			// Change the read only attribute
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
	return RemoveDirectory(path) ? true : false;		// Remove the empty directory
}

Array* splitext( const TSTR & source )
{
	one_typed_value_local( Array* out );
	vl.out = new Array(0);
	TSTR norm = normpath( source );
	Array* pointList = split( norm, _T(".") );
	if ( pointList->size > 0 && isExtension(pointList->data[pointList->size - 1]->to_string()) )
	{
		Array * temp = new Array(0);
		for ( int i = 0; i < pointList->size - 1; i++ )
			temp->append( pointList->data[i] );
		vl.out->append( new String( join( temp, _T(".") ) ) );
		vl.out->append( pointList->data[ pointList->size-1 ] );
	}
	else {
		vl.out->append( new String( source ) );
		vl.out->append( new String( _T("") ) );
	}
	return_value( vl.out );
}

Array * splitpath( const TSTR & source )
{
	one_typed_value_local( Array* out );
	vl.out = new Array(0);

	TSTR norm = normpath( source );
	Array* pathList = split( norm, _T("/") );
	if ( pathList->size > 0 ) {
		TSTR last( pathList->data[ pathList->size-1 ]->to_string() );
		if ( !isFileName( last ) ) {
			vl.out->append( new String( norm ) );
			vl.out->append( new String( _T("") ) );
		}
		else {
			Array* temp = new Array(0);
			for ( int i = 0; i < pathList->size - 1; i++ )
				temp->append( pathList->data[i] );
			vl.out->append( new String( normpath(join(temp, _T("/"))) ) );
			vl.out->append( new String( last ) );
		}
	}

	return_value( vl.out );
}

TSTR metapath()
{
	TSTR filepath = GetCOREInterface()->GetCurFilePath();
	TSTR out;
	if ( filepath.length() > 0 ) {
		Array* pathList		= splitpath( filepath );
		out = TSTR( pathList->data[0]->to_string() ) + _T(".meta/");
	}
	return out;
}

bool createMetapath()
{
	TSTR path = metapath();
	bool success = exists( path );
	if ( !success ) {
		if ( makedir( path ) ) {
			SetFileAttributes( path, FILE_ATTRIBUTE_HIDDEN );
			success = true;
		}
	}
	return success;
}
// ------------------------------------------------------------------------------------------------------
//											BLURFILE METHODS
// ------------------------------------------------------------------------------------------------------

def_struct_primitive( basename,		blurFile,		"basename" );
def_struct_primitive( createMetapath,blurFile,		"createMetapath" );
def_struct_primitive( exists,		blurFile,		"exists" );
def_struct_primitive( isUNCPath,	blurFile,		"isUNCPath" );
def_struct_primitive( fileJoin,		blurFile,		"join" );
def_struct_primitive( makedir,		blurFile,		"makedir" );
def_struct_primitive( makedirs,		blurFile,		"makedirs" );
def_struct_primitive( metapath,		blurFile,		"metapath" );
def_struct_primitive( normpath,		blurFile,		"normpath" );
def_struct_primitive( rmdir,		blurFile,		"rmdir" );
def_struct_primitive( fileSplit,	blurFile,		"split" );
def_struct_primitive( splitext,		blurFile,		"splitext" );
def_struct_primitive( splitpath,	blurFile,		"splitpath" );

//----------------------------------------------------------------------------------------------

Value * basename_cf( Value** arg_list, int count )
{
	check_arg_count_with_keys( blurFile.basename, 1, count );

	one_value_local( out );
	vl.out = new String( _T("") );

	Value* includeFolder = key_arg_or_default( includeFolder, &true_value );
	if ( includeFolder == &true_value ) {
		Array* pathList = split( normpath( TSTR( arg_list[0]->eval()->to_string() ) ), _T("/") );
		if ( pathList->size > 0 )
			vl.out = pathList->data[ pathList->size - 1 ];
	}
	else {
		Array * pathList = splitpath( TSTR( arg_list[0]->eval()->to_string() ) );
		vl.out = pathList->data[1];
	}
	return_value( vl.out );
}

Value * createMetapath_cf(	Value** arg_list, int count )
{
	check_arg_count( blurFile.createMetapath, 0, count );
	return createMetapath() ? &true_value : &false_value;
}

Value * exists_cf( Value** arg_list, int count )
{
	check_arg_count_with_keys( blurFile.exists, 1, count );
	return exists( TSTR(arg_list[0]->eval()->to_string()) ) ? &true_value : &false_value;
}

Value * isUNCPath_cf( Value** arg_list, int count )
{
	check_arg_count( blurFile.isUNCPath, 1, count );
	return isUNCPath( TSTR(arg_list[0]->eval()->to_string()) ) ? &true_value : &false_value;
}

Value * fileJoin_cf( Value** arg_list, int count )
{
	check_arg_count_with_keys( blurFile.buildpath, 1, count );

	Value* separator = key_arg(separator);
	if ( separator != &unsupplied ) {
		TSTR sepStr = separator->to_string();
		TCHAR sep = sepStr.Length() ? sepStr[0] : '/';
		return new String( normpath( join( (Array*) arg_list[0]->eval(), _T("/") ), sep ) );
	}
	return new String( normpath( join( (Array*) arg_list[0]->eval(), _T("/") ) ) );
}

Value * makedir_cf( Value** arg_list, int count )
{
	check_arg_count( blurFile.makedir, 1, count );
	return makedir( TSTR(arg_list[0]->eval()->to_string()) ) ? &true_value : &false_value;
}

Value * makedirs_cf( Value** arg_list, int count )
{
	check_arg_count( blurFile.makedirs, 1, count );
	return makedir( TSTR(arg_list[0]->eval()->to_string()), true ) ? &true_value : &false_value;
}

Value * metapath_cf( Value** arg_list, int count )
{
	check_arg_count( blurFile.metapath, 0, count );
	return new String( metapath() );
}

Value * normpath_cf(		Value** arg_list, int count )
{
	check_arg_count_with_keys( blurFile.normpath, 1, count );

	Value* separator = key_arg(separator);
	if ( separator != &unsupplied ) {
		TSTR sepStr = separator->eval()->to_string();
		TCHAR sep = sepStr.Length() ? sepStr[0] : '/';
		return new String( normpath( TSTR(arg_list[0]->eval()->to_string()), sep ) );
	}
	return new String( normpath( TSTR(arg_list[0]->eval()->to_string()) ) );
}

Value * fileSplit_cf( Value** arg_list, int count )
{
	check_arg_count( blurFile.split, 1, count );
	return split( normpath( TSTR(arg_list[0]->eval()->to_string()) ), _T("/") );
}

Value * rmdir_cf( Value** arg_list, int count )
{
	check_arg_count_with_keys( blurFile.rmdir, 1, count );
	bool recursive = key_arg( recursive ) == &true_value;
	return rmdir( normpath(arg_list[0]->eval()->to_string(), '\\'), recursive ) ? &true_value : &false_value;
}

Value * splitext_cf( Value** arg_list, int count )
{
	check_arg_count( blurFile.splitext, 1, count );
	return splitext( arg_list[0]->eval()->to_string() );
}

Value * splitpath_cf( Value** arg_list, int count )
{
	check_arg_count( blurFile.splitpath, 1, count );
	return splitpath( arg_list[0]->eval()->to_string() );
}

// ------------------------------------------------------------------------------------------------------

void BlurFileInit() {}
