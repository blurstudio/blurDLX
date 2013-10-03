
#include "imports.h"
#include "blurString.h"

#include <algorithm>
#include <sstream>
#include <cctype>
#include <string>
#include <vector>

#define BLURSTRING_VERSION 1000

#include "Dictionary.h"

#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
#include "maxscript\macros\define_external_functions.h"				// external name definitions
#include "maxscript\macros\define_instantiation_functions.h"		// internal name definitions
#else
#include "defextfn.h"			// external name definitions
#include "definsfn.h"			// internal name definitions
#endif

#define n_lowercase 	Name::intern( _T( "lowercase" ) )
#define n_allowchars	Name::intern( _T( "allowchars" ) )
#define n_quoteChar 	Name::intern( _T( "quoteChar" ) )

// ------------------------------------------------------------------------------------------------------
//											C++ METHODS
// ------------------------------------------------------------------------------------------------------

void operator+=(TSTR & str, TCHAR chr)
{
	const int len = str.Length();
	TCHAR * data = str.dataForWrite(len+2);
	data[len] = chr;
	data[len+1] = 0;
}

TSTR & operator+(TSTR & str, TCHAR chr)
{
	str += chr;
	return str;
}

int find( const TSTR & source, TCHAR ch, int i )
{
	if( i < 0 ) i = 0;
	for ( int end = source.Length(); i < end; ++i )
		if( source[i] == ch ) {
			//mprintf( _T("find \"%s\" '%c'returning %i"), source.data(), ch, i );
			return i;
		}
	//mprintf( _T("find \"%s\" '%c'returning -1"), source.data(), ch );
	return -1;
}

int find( const TSTR & source, const TSTR & match, int i )
{
	if( source.Length() == 0 || match.Length() == 0 || match.Length() > source.Length() - i ) return -1;
	if( i < 0 ) i = 0;
	TCHAR mf = match[0];
	for( int end = source.Length() - match.Length() + 1; i < end; ++i ) {
		if( source[i] != mf ) continue;
		for( int j = 0, jend = match.Length(); j < jend; ++j )
			if( source[i] != match[j] ) continue;
		//mprintf( _T("find \"%s\"\"%s\"returning %i"), source.data(), match.data(), i );
		return i;
	}
	return -1;
}

bool endswith( const TSTR & source, const TSTR & search )
{
	if( search.Length() > source.Length() ) return false;
	const int start = source.Length() - search.Length();
	for( int i = start, end = source.Length(); i < end; i++ )
		if( source[i] != search[i-start] ) return false;
	return true;
}

TSTR join( Array* parts, const TSTR & separator )
{
	TSTR out;
	for ( int i = 0; i < parts->size; i++ ) {
		if ( i > 0 )
			out += separator;
		out += TSTR(parts->data[i]->to_string());
	}
	return out;
}

bool isNum( const TCHAR & ch )
{ return ch >= '0' && ch <= '9'; }

bool isTrue( const TSTR &source )
{
	TSTR lower(source);
	lower.toLower();
	return !lower.isNull() && ((isNum(source[0]) && source != _T("0")) || (source == _T("true")) || (source == _T("t")));
}

bool matches( const TSTR & source, const TSTR & compare, int sourcePos )
{
	for( int i = 0, end = compare.Length(); i < end; i++ ) {
		if( sourcePos + i >= source.Length() || source[sourcePos + i] != compare[i] )
			return false;
	}
	return true;
}

void replace( TSTR &source, const TSTR & _searchString, const TSTR & replaceString, bool ignoreCase, bool replaceAll )
{
	// we create an uppercase version of the Strings
	TSTR strTmp( source );
	TSTR searchString(_searchString);
	
	if( ignoreCase ) {
		strTmp.toUpper();
		searchString.toUpper();
	}

    assert( searchString != replaceString );

	for( int i = 0, end = strTmp.Length(); i < end; ) {
		if( matches( strTmp, searchString, i ) ) {
			int end = i + searchString.Length();
			source = source.Substr( 0, i ) + replaceString + source.Substr( end, source.Length() - end );
			if( !replaceAll )
				break;
			i += searchString.Length();
		} else
			++i;
	}
}

void lstrip( TSTR &source, const TSTR & separator )
{
	bool ss = separator.Length() == 0;
	int i = 0;
	for( int end = source.Length(); i < end; ++i ) {
		if( ss ) {
			if( !_istspace(source[i]) )
				break;
		} else if( find( separator, source[i] ) == -1 )
			break;
	}
	if( i > 0 ) {
		if( i == source.Length() )
			source = TSTR();
		else
			source.remove(0,i);
	}
}

void rstrip( TSTR &source, const TSTR & separator )
{
	bool ss = separator.Length() == 0;
	int i = source.Length() - 1;
	for( ; i >= 0; --i ) {
		if( ss ) {
			if( !_istspace(source[i]) )
				break;
		} else if( find( separator, source[i] ) == -1 )
			break;
	}
	if( i < source.Length() - 1 ) {
		if( i < 0 )
			source = TSTR();
		else
			source.remove(i+1,source.Length()-i+1);
	}
}

Array * split( const TSTR & source, const TSTR & separator )
{
	one_typed_value_local( Array* out );
	vl.out = new Array(0);

	int iPos = 0;
	int newPos = -1;
	int sizeS2 = (int)separator.Length();
	int isize = (int)source.Length();

	if ( isize != 0 && sizeS2 != 0 ) {
		std::vector<int> positions;
		newPos = find( source, separator, 0 );
		if ( newPos >= 0 ) {
			int numFound = 0;

			// Collect the positions of the separator string
			while ( newPos >= iPos ) {
				numFound++;
				positions.push_back(newPos);
				iPos = newPos;
				newPos = find( source, separator, iPos + sizeS2 );
			}

			if ( numFound != 0 ) {
				// Spit the string based on the positions and size of the deliminator
				for ( int i = 0; i <= (int)positions.size(); ++i ) {
					TSTR s;
					if ( i == 0 )
						s = source.Substr( i, positions[i] );
					else {
						int offset = positions[i-1] + sizeS2;
						if ( offset < isize ) {
							if ( i == positions.size() )
								s = source.Substr( offset, source.Length() - offset );

							else if ( i > 0 )
								s = source.Substr( positions[i-1] + sizeS2, positions[i] - positions[i-1] - sizeS2 );
						}
					}
					
					if ( s.Length() > 0 )
						vl.out->append( new String( s ) );
				}
			}
		}
		else
			vl.out->append( new String( source ) );
	}

	return_value( vl.out );
}

bool startswith( const TSTR & source, const TSTR & search )
{
	return matches( source, search, 0 );
}

// ------------------------------------------------------------------------------------------------------
//										BLURSTRING STRUCT METHODS
// ------------------------------------------------------------------------------------------------------
def_struct_primitive( endswith,	blurString, "endswith" );
def_struct_primitive( format,	blurString, "format" );
def_struct_primitive( join,		blurString, "join" );
def_struct_primitive( lstrip,	blurString, "lstrip" );
def_struct_primitive( replace,	blurString, "replace" );
def_struct_primitive( rstrip,	blurString, "rstrip" );
def_struct_primitive( split,	blurString, "split" );
def_struct_primitive( startswith,blurString, "startswith" );
def_struct_primitive( strip,	blurString, "strip" );
def_struct_primitive( toBool,	blurString, "toBool" );
def_struct_primitive( toId,		blurString, "toId" );
def_struct_primitive( toKey,	blurString, "toKey" );
def_struct_primitive( toLower,	blurString, "toLower" );
def_struct_primitive( toUpper,	blurString, "toUpper" );

// ------------------------------------------------------------------------------------------------------

Value * endswith_cf( Value** arg_list, int count )
{
	/*!------------------------------------------------------------
		\remarks
			Checks to see if the inputed string ends with a second

		\param		rootStr		<string>
		\param		endStr		<string>

		\usage		blurString.endswith "Test" "est"	-> true

		\return
			<boolean>
	-------------------------------------------------------------*/

	check_arg_count( endswith, 2, count );
	return ( endswith( TSTR( arg_list[0]->eval()->to_string() ), TSTR( arg_list[1]->eval()->to_string() ) ) ) ? &true_value : &false_value;
}

Value * format_cf( Value** arg_list, int count )
{
	/*!------------------------------------------------------------
		\remarks
			Formats a given string with the inputed values, replacing the
			given quote character with a " (easier formatting), by filtering
			the rootStr for the wildcard character %.  The possible options
			following the wildcard char are - f (for float), i (for int), s (for string),
			% (to place an actual '%' in the string).  When using a float, one can define
			the floating point number, total amount of padding, and padding char, e.g. a
			format string of "S%07.2f" given a 10.5 will format to S0010.50.  Similarly, for an
			integer conversion one can define the amount of padding so that "Sc%03i" given a 40 will format to
			Sc040.  The possible variable types inputed as the format characters need to sync up to the
			rootStr, so that anything being converted to a %s needs to have a valid string
			conversion.  If there is only 1 input format char, then a single value can be passed,
			2+ format characters expect an array of values, and all values can work with the
			Dictionary class value by passing in the key of the value in the dictionary.  e.g.

			blurString.format "Sc(sequence)%03i_S(shot)%07.2f" (dictionary keys:#( "sequence", "shot" ) values:#( 10, 15.5 ))
			results in "Sc010_S0015.50"

		\param		rootStr			<string>								String to replace
		\param		repalceChars	<variant> | <array> | <dictionary>		Values to format
		\param		quoteChar		<string>								[OPTIONAL] Replaces this char with a single quote ("), default '

		\usage		blurString.format "%04i"				10																													->	"0010"
					blurString.format "Sc%03i_S%07.2f"		#( 1, 10.5 )																										->	"Sc001_S0010.50"
					blurString.format "%(project)s_%(sequence)s_%(shot)s.max" (dictionary keys:#( "project", "sequence", "shot" ) values:#( "Hellgate", "Sc001", "S0010.50" )	->	"Hellgate_Sc001_S0010.50.max"

		\return
			<string>	Resulting formated string
	-------------------------------------------------------------*/
	check_arg_count_with_keys( format, 2, count );

	TSTR outString;
	TSTR inString( arg_list[0]->to_string() );
	TSTR formatChar( _T("%") );
	TSTR typeChar( _T("fis") );
	TSTR openKeyChar( _T("(") );
	TSTR closeKeyChar( _T(")") );
	TSTR optionChars( _T(" 01234567890.") );
	TSTR padChar( _T("0") );
	TSTR quoteChar( _T("'") );

	Value * quoteCharVal				= key_arg(quoteChar);
	if ( quoteCharVal != &unsupplied )
		quoteChar					= TSTR( quoteCharVal->to_string() );

	TSTR key;
	TSTR options;
	BOOL amFormating = false;
	BOOL amKeyChar = false;
	BOOL isDictionary = is_dictionary( arg_list[1] );
	BOOL isArray = is_array( arg_list[1] );
	int inputIndex = 1;

	for( int i = 0, end = inString.Length(); i < end; ++i ) {
		TCHAR c = inString[i];
		if ( find(formatChar,c) != -1 ) {
			if ( !amFormating )
				amFormating = true;
			else {
				amFormating = false;
				outString += formatChar;
				key = options = TSTR();
			}
		}
		else if ( amFormating && find(openKeyChar,c) != -1 )
			amKeyChar = true;
		else if ( amKeyChar && find(closeKeyChar,c) != -1 )
			amKeyChar = false;
		else if ( amFormating && amKeyChar )
			key += c;
		else if ( amFormating && find(optionChars,c) != -1 )
			options += c;
		else if ( amFormating && find(typeChar,c) != -1 ) {
			if ( !( key.Length() == 0 || isDictionary ) )
				throw RuntimeError( _T( "Invalid options input: format needs a Dictionary class when using keys." ) );
			else if ( key.Length() == 0 && isDictionary )
				throw RuntimeError( _T( "Invalid format string: Dictionary options need format keys." ) );
			else if ( !isDictionary && !isArray && inputIndex > 1 )
				throw RuntimeError( _T( "Invalid options input: multiple format values need an Array or Dictionary." ) );
			else if ( isArray && inputIndex > ( (Array*) arg_list[1] )->size )
				throw RuntimeError( _T( "Invalid format string: Too many format variables." ) );
			else {
				Value* val;
				if ( isDictionary ) {
					val = ( (Dictionary*) arg_list[1] )->get( key );
					if ( val == NULL ) {
						TSTR errorMsg = _T( "Invalid format string: Dictionary key #" );
						errorMsg += key;
						errorMsg += _T(" not found.");
						throw RuntimeError( errorMsg );
					}
				}
				else if ( isArray )
					val = ( (Array*) arg_list[1] )->get(inputIndex);
				else
					val = arg_list[1];

				if ( c == 'f' ) {
					TSTR tmp,tmp2;
					tmp.printf(_T("%%%sf"), options.data() );
					tmp2.printf(tmp.data(),val->to_float());
					outString += tmp2;
				}
				else if ( c == 'i' ) {
					TSTR tmp,tmp2;
					tmp.printf(_T("%%%si"), options.data() );
					tmp2.printf(tmp.data(),val->to_int());
					outString += tmp2;
				}
				else {
					if ( is_string(val) )
						outString += TSTR( val->to_string() );
					else {
						StringStream * s = new StringStream();
						val->sprin1( (CharStream*) s );
						outString += TSTR( s->to_string() );
					}
				}
				
				options = key = TSTR();
				inputIndex += 1;
				amFormating = false;
			}
		}
		else if ( amFormating ) {
			TSTR errorMsg = _T("Cannot format character: ");
			errorMsg += c;
			throw RuntimeError( errorMsg );
		}
		else if ( find(quoteChar,c) != -1 )
			outString += '\"';
		else
			outString += c;

	}

	one_value_local( result );
	vl.result = new String( outString );
	return_value( vl.result );
}

Value * join_cf( Value** arg_list, int count )
{
	/*!------------------------------------------------------------
		\remarks
			Joins a list of strings together, given a desired separation character

		\param		stringList		<array> #( <string>, .. )
		\param		separator		<string>					[OPTIONAL] Given separator string to join the list together with, default ""

		\usage		blurString.join #( "Mesh", "Cloth" ) separator:"-"		-> "Mesh-Cloth"

		\return
			<string> Result
	-------------------------------------------------------------*/
	check_arg_count_with_keys( join, 1, count );

	Value* separatorVar		= key_arg( separator );
	if ( separatorVar != &unsupplied )
		return new String( join( (Array*) arg_list[0], TSTR( separatorVar->to_string() ) ) );
	else
		return new String( join( (Array*) arg_list[0] ) );
}

Value * lstrip_cf( Value** arg_list, int count )
{
	/*!------------------------------------------------------------
		\remarks
			Strips out all space characters from the beginning of the
			inputed string
		
		\param		rootStr		<string>

		\usage		blurString.lstrip "\t	Test"		-> "Test"

		\return
			<string> Result
	-------------------------------------------------------------*/
	check_arg_count_with_keys( lstrip, 1, count );
	TSTR source( arg_list[0]->to_string() );

	Value* separator = key_arg(separator);
	if ( separator != &unsupplied )
		lstrip( source, TSTR(separator->to_string()) );
	else
		lstrip( source );

	one_value_local( result );
	vl.result = new String( source );
	return_value( vl.result );
}

Value * replace_cf( Value** arg_list, int count )
{
	/*!-------------------------------------------------------------
		\remarks
			Takes an input string and replaces the instance of the provided
			second string with a third.  If the optional all parameter is set to
			true, then all instances of the replace string will be replaced.  If the
			optional ignoreCase flag parameter is flagged as true then the replace will
			be case insensitive.

		\param		rootStr			<string>		Full string to apply the replace to
		\param		searchStr		<string>		String to search for
		\param		replaceStr		<string>		String to replace with
		\param		all				<boolean>		[OPTIONAL] Tells the method to replace all instances,		default false
		\param		ignorecase		<boolean>		[OPTIONAL] Tells the method to ignore case when searching,	default true

		\usage		blurString.replace "mi mi mi mi" "mi" "test" all:true ignorecase:false

		\return
			<string>	Resulting string after replacement
	--------------------------------------------------------------*/
	check_arg_count_with_keys (SearchAndReplace, 3, count);

	TSTR str = arg_list[0]->to_string();
	TSTR searchString = arg_list[1]->to_string();
	TSTR replaceString = arg_list[2]->to_string();

	Value * val;
	BOOL ignoreCase = bool_key_arg(ignoreCase, val, TRUE);
	BOOL replaceAll = bool_key_arg(all, val, FALSE);

	replace( str, searchString, replaceString, (ignoreCase) ? true : false, (replaceAll) ? true : false );

	return new String( str );
}

Value * rstrip_cf( Value** arg_list, int count )
{
	/*!------------------------------------------------------------
		\remarks
			Strips out all space characters from the end of the
			inputed string
		
		\param		rootStr		<string>

		\usage		blurString.lstrip "Test\t	"		-> "Test"

		\return
			<string> Result
	-------------------------------------------------------------*/
	check_arg_count_with_keys( rstrip, 1, count );
	TSTR source( arg_list[0]->to_string() );

	Value* separator = key_arg(separator);
	if ( separator != &unsupplied )
		rstrip( source, TSTR( separator->to_string() ) );
	else
		rstrip( source );

	one_value_local( result );
	vl.result = new String( source );
	return_value( vl.result );
}

Value * split_cf( Value** arg_list, int count )
{
	check_arg_count_with_keys( blurString.split, 1, count );
	Value* separator = key_arg(separator);
	if ( separator != &unsupplied )
		return split( TSTR( arg_list[0]->to_string() ), TSTR( separator->to_string() ) );
	else
		return split( TSTR( arg_list[0]->to_string() ) );
}

Value * startswith_cf( Value** arg_list, int count )
{
	/*!------------------------------------------------------------
		\remarks
			Checks to see if the inputed string starts with a second

		\param		rootStr		<string>
		\param		startStr	<string>

		\usage		bluString.endswith "Test" "Te"	-> true

		\return
			<boolean>
	-------------------------------------------------------------*/
	check_arg_count( startswith, 2, count );
	return ( startswith( TSTR( arg_list[0]->eval()->to_string() ), TSTR( arg_list[1]->eval()->to_string() ) ) ) ? &true_value : &false_value;
}

Value * strip_cf( Value** arg_list, int count )
{
	/*!------------------------------------------------------------
		\remarks
			Strips out all space characters from the beginning and end of the
			inputed string
		
		\param		rootStr		<string>

		\usage		blurString.lstrip "\t	Test\t	"	-> "Test"

		\return
			<string> Result
	-------------------------------------------------------------*/
	check_arg_count_with_keys( strip, 1, count );
	TSTR source( arg_list[0]->to_string() );

	Value* separator = key_arg(separator);
	if ( separator != &unsupplied ) {
		lstrip( source, TSTR( separator->to_string() ) );
		rstrip( source, TSTR( separator->to_string() ) );
	}
	else {
		lstrip( source );
		rstrip( source );
	}

	one_value_local( result );
	vl.result = new String( source );
	return_value( vl.result );
}

Value * toBool_cf( Value** arg_list, int count )
{
	/*!------------------------------------------------------------
		\remarks
			Converts the inputed string to a boolean value
		
		\param		rootStr		<string>

		\usage		blurString.toBool "1"		-> true
					blurString.toBool "TRUE"	-> true

		\return
			<string> Result
	-------------------------------------------------------------*/
	check_arg_count( toBool, 1, count );
	return isTrue( TSTR(arg_list[0]->to_string()) ) ? &true_value : &false_value;
}

Value * toId_cf( Value** arg_list, int count)
{
	/*!------------------------------------------------------------
		\remarks
			Takes an inputed string value and strips out all non-alphanumeric characters,
			converting any space characters to the separator variable char
		
		\param		rootStr		<string>
		\param		separator	<string>	[OPTIONAL] String by which all space characters should be converted to, default _
		\param		allowChars	<string>	[OPTIONAL] String to define what characters should be allowed beyond the default, default ""
		\param		lowercase	<boolean>	[OPTIONAL] Flag to set the output string to lowercase before returning, default false

		\usage		blurString.toId "DFSIU(*%#{{JF  D(${" -> "DFSIUJF__D"

		\return
			<string> Result
	-------------------------------------------------------------*/
	check_arg_count_with_keys( toId, 1, count );
	
	TSTR source( arg_list[0]->to_string() );
	TSTR outString;
	TSTR inString( arg_list[0]->to_string() );
	
	TSTR validCharacters(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"));
	TSTR spaceChars(_T("- _"));
	TSTR separator(_T("_"));
	if ( key_arg( separator ) != &unsupplied )
		separator = TSTR( key_arg( separator )->to_string() );

	TSTR allowChars;
	if ( key_arg( allowchars ) != &unsupplied )
		allowChars = TSTR( key_arg( allowchars )->to_string() );

	Value* separatorVal				= key_arg( separator );
	if ( separatorVal != &unsupplied )
		separator = separatorVal->to_string();

	for( int i = 0, end = inString.Length(); i < end; ++i ) {
		TCHAR c = inString[i];
		if( find(validCharacters,c) != -1 || find(allowChars,c) != -1 )
			outString += c;
		else if( find(spaceChars,c) != -1 )
			outString += separator;
	}

	Value* lower = key_arg_or_default( lowercase, &false_value );
	if ( lower == &true_value )
		outString.toLower();

	one_value_local( result );
	vl.result = new String( outString );
	return_value( vl.result );
}

Value * toKey_cf( Value** arg_list, int count )
{
	/*!------------------------------------------------------------
		\remarks
			Converts the inputed string to an Id and then converts the resulting
			id string into a Name key
		
		\see		toId_cf

		\usage		blurString.toKey "DFSIU(*%#{{JF  D(${" -> #DFSIUJF__D

		\return
			<name> Result
	-------------------------------------------------------------*/
	Value* result = toId_cf( arg_list, count );
	return ( Name::intern( result->to_string() ) );
}

Value * toLower_cf( Value** arg_list, int count )
{
	/*!------------------------------------------------------------
		\remarks
			Converts the inputed string to lowercase
		
		\param		rootStr		<string>

		\usage		blurString.toLower "TEST" -> "test"

		\return
			<string> Result
	-------------------------------------------------------------*/
	check_arg_count ( toLower, 1, count );
	TSTR converter( arg_list[0]->to_string() );
	converter.toLower();

	one_value_local( result );
	vl.result = new String( converter );
	return_value( vl.result );
}

Value * toUpper_cf( Value** arg_list, int count )
{
	/*!------------------------------------------------------------
		\remarks
			Converts the inputed string to uppercase
		
		\param		rootStr		<string>

		\usage		blurString.toUpper "test" -> "UPPER"

		\return
			<string> Result
	-------------------------------------------------------------*/
	check_arg_count ( toUpper, 1, count );
	TSTR converter( arg_list[0]->to_string() );
	converter.toUpper();
	one_value_local( result );
	vl.result = new String( converter );
	return_value( vl.result );
}

//----------------------------------------------	blurString Init		---------------------------------------------------------
void BlurStringInit() { mprintf( _T("------ blurString Loaded - Version:%.2f ------\n"), ((float) BLURSTRING_VERSION) / 1000.0f ); }
