#include "MAXScrpt.h"
#include "Numbers.h"
#include "arrays.h"
#include "thunks.h"
#include "Structs.h"
#include "strings.h"

#include <algorithm>
#include <sstream>
#include <cctype>
#include <string>
#include <vector>

#define				BLURSTRING_VERSION			1000

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "Dictionary.h"

#include "defextfn.h"			// external name definitions
#include "definsfn.h"			// internal name definitions

#define		n_lowercase			Name::intern( _T( "lowercase" ) )
#define		n_allowchars		Name::intern( _T( "allowchars" ) )
#define		n_quoteChar			Name::intern( _T( "quoteChar" ) )

// ------------------------------------------------------------------------------------------------------
//											C++ METHODS
// ------------------------------------------------------------------------------------------------------
bool			endswith( std::string source, std::string search ) {
	return ( source.rfind( search ) == source.length() - search.length() );
}
std::string		join( Array* parts, std::string separator = " " ) {
	std::string out				= "";
	bool start = true;
	for ( int i = 0; i < parts->size; i++ ) {
		if ( !start )
			out					+= separator;

		out						+= parts->data[i]->to_string();
		start					= false;
	}
	return out.c_str();
}
void			lstrip( std::string &source, std::string separator = "" ) {
	std::string::iterator i;
	for (i = source.begin(); i != source.end(); i++) {
		if ( separator.length() == 0 ) {
			if ( !isspace(*i) )
				break;
		}
		else if ( separator.find(*i) == -1 ) {
			break;
		}
	}
	if ( i == source.end() ) {
		source.clear();
	}
	else {
		source.erase( source.begin(), i );
	}
}
bool			parseString( const std::string &source ) {
	bool value;
	std::istringstream iss(source);
	if ( source.length() == 1 )
		iss >> value;
	else
		iss >> std::boolalpha >> value;

	return value;
}
void			replace( std::string &source, std::string searchString, std::string replaceString, bool ignoreCase = true, bool replaceAll = true ) {
	// we create an uppercase version of the Strings
	
    std::string strUpper( source );
    std::string searchStringUpper( searchString ); 
    std::string replaceStringUpper( replaceString ); 

	std::transform(strUpper.begin(), strUpper.end(), strUpper.begin(), std::tolower); 
	std::transform(searchStringUpper.begin(), searchStringUpper.end(), searchStringUpper.begin(), std::tolower); 
	std::transform(replaceStringUpper.begin(), replaceStringUpper.end(), replaceStringUpper.begin(), std::tolower); 

    assert( searchString != replaceString );

	if (!ignoreCase)
	{
		std::string::size_type pos = 0;
		while ( (pos = source.find(searchString, pos)) != std::string::npos ) 
		{
			source.replace( pos, searchString.size(), replaceString );
			pos++;
			if (!replaceAll)
				break;
		}
	}
	else
	{
		std::string::size_type pos = 0;
		while ( (pos = strUpper.find(searchStringUpper, pos)) != std::string::npos ) 
		{
			strUpper.replace( pos, searchStringUpper.size(), replaceStringUpper );
			source.replace( pos, searchString.size(), replaceString );

			pos++;
			if (!replaceAll)
				break;
		}		
	}
}
void			rstrip( std::string &source, std::string separator = "" ) {
	std::string::iterator i;
	for (i = source.end() - 1; ; i-- ) {
		if ( separator.length() == 0 ) {
			if ( !isspace(*i) ) {
				source.erase( i+1, source.end() );
				break;
			}
		}
		else if ( separator.find(*i) == -1 ) {
			source.erase( i+1, source.end() );
			break;
		}
	}
}
Array*			split( std::string source, std::string separator = " " ) {
	one_typed_value_local( Array* out );
	vl.out = new Array(0);

	int iPos	= 0;
	int newPos	= -1;
	int sizeS2	= (int)separator.size();
	int isize	= (int)source.size();

	if ( isize != 0 && sizeS2 != 0 ) {
		std::vector<int> positions;
		newPos = source.find( separator, 0 );
		if ( newPos >= 0 ) {
			int numFound = 0;

			// Collect the positions of the separator string
			while ( newPos >= iPos ) {
				numFound++;
				positions.push_back(newPos);
				iPos	= newPos;
				newPos	= source.find( separator, iPos + sizeS2 );
			}

			if ( numFound != 0 ) {
				// Spit the string based on the positions and size of the deliminator
				for ( int i = 0; i <= (int)positions.size(); ++i ) {
					std::string s("");
					if ( i == 0 )
						s = source.substr( i, positions[i] );
					else {
						int offset = positions[i-1] + sizeS2;
						if ( offset < isize ) {
							if ( i == positions.size() )
								s = source.substr( offset );

							else if ( i > 0 )
								s = source.substr( positions[i-1] + sizeS2, positions[i] - positions[i-1] - sizeS2 );
						}
					}
					
					if ( s.size() > 0 )
						vl.out->append( new String( s.c_str() ) );
				}
			}
		}
		else
			vl.out->append( new String( source.c_str() ) );
	}

	return_value( vl.out );
}
bool			startswith( std::string source, std::string search ) {
	return source.find( search ) == 0;
}
// ------------------------------------------------------------------------------------------------------
//										BLURSTRING STRUCT METHODS
// ------------------------------------------------------------------------------------------------------
def_struct_primitive(	endswith,		blurString,		"endswith" );
def_struct_primitive(	format,			blurString,		"format" );
def_struct_primitive(	join,			blurString,		"join" );
def_struct_primitive(	lstrip,			blurString,		"lstrip" );
def_struct_primitive(	replace,		blurString,		"replace" );
def_struct_primitive(	rstrip,			blurString,		"rstrip" );
def_struct_primitive(	split,			blurString,		"split" );
def_struct_primitive(	startswith,		blurString,		"startswith" );
def_struct_primitive(	strip,			blurString,		"strip" );
def_struct_primitive(	toBool,			blurString,		"toBool" );
def_struct_primitive(	toId,			blurString,		"toId" );
def_struct_primitive(	toKey,			blurString,		"toKey" );
def_struct_primitive(	toLower,		blurString,		"toLower" );
def_struct_primitive(	toUpper,		blurString,		"toUpper" );

// ------------------------------------------------------------------------------------------------------

Value*		endswith_cf(	Value** arg_list, int count ) {
	/*!------------------------------------------------------------
		\remarks
			Checks to see if the inputed string ends with a second

		\param		rootStr		<string>
		\param		endStr		<string>

		\usage		bluString.endswith "Test" "est"	-> true

		\return
			<boolean>
	-------------------------------------------------------------*/

	check_arg_count( endswith, 2, count );
	return ( endswith( std::string( arg_list[0]->eval()->to_string() ), std::string( arg_list[1]->eval()->to_string() ) ) ) ? &true_value : &false_value;
}
Value*		format_cf(		Value** arg_list, int count ) {
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

	std::string outString			= "";
	std::string inString( arg_list[0]->to_string() );
	std::string formatChar			= "%";
	std::string typeChar			= "fis";
	std::string openKeyChar			= "(";
	std::string closeKeyChar		= ")";
	std::string optionChars			= " 01234567890.";
	std::string padChar				= "0";
	std::string quoteChar			= "'";

	Value* quoteCharVal				= key_arg(quoteChar);
	if ( quoteCharVal != &unsupplied )
		quoteChar					= std::string( quoteCharVal->to_string() );

	std::string key					= "";
	std::string options				= "";
	BOOL amFormating				= false;
	BOOL amKeyChar					= false;
	BOOL isDictionary				= is_dictionary( arg_list[1] );
	BOOL isArray					= is_array( arg_list[1] );
	int inputIndex					= 1;

	std::string::const_iterator it1 = inString.begin();
	while ( it1 != inString.end() ) {
		if ( formatChar.find(*it1) != -1 ) {
			if ( !amFormating )
				amFormating = true;
			else {
				amFormating = false;
				outString	+= formatChar;
				key			= "";
				options		= "";
			}
		}
		else if ( amFormating && openKeyChar.find(*it1) != -1 )
			amKeyChar	= true;
		else if ( amKeyChar && closeKeyChar.find(*it1) != -1 )
			amKeyChar = false;
		else if ( amFormating && amKeyChar )
			key			+= *it1;
		else if ( amFormating && optionChars.find(*it1) != -1 )
			options		+= *it1;
		else if ( amFormating && typeChar.find(*it1) != -1 ) {
			if ( !( key.empty() || isDictionary ) )
				throw RuntimeError( _T( "Invalid options input: format needs a Dictionary class when using keys." ) );
			else if ( key.empty() && isDictionary )
				throw RuntimeError( _T( "Invalid format string: Dictionary options need format keys." ) );
			else if ( !isDictionary && !isArray && inputIndex > 1 )
				throw RuntimeError( _T( "Invalid options input: multiple format values need an Array or Dictionary." ) );
			else if ( isArray && inputIndex > ( (Array*) arg_list[1] )->size )
				throw RuntimeError( _T( "Invalid format string: Too many format variables." ) );
			else {
				Value* val;
				if ( isDictionary ) {
					val = ( (Dictionary*) arg_list[1] )->get( (TSTR) key.c_str() );
					if ( val == NULL ) {
						TSTR errorMsg = _T( "Invalid format string: Dictionary key #" );
						errorMsg += (TSTR) key.c_str();
						errorMsg += " not found.";
						throw RuntimeError( errorMsg );
					}
				}
				else if ( isArray )
					val = ( (Array*) arg_list[1] )->get(inputIndex);
				else
					val = arg_list[1];

				StringStream* s = new StringStream();
				if ( std::string( "f" ).find( *it1 ) != -1 ) {
					float valFloat = val->to_float();
					options = "%" + options + "f";
					s->printf( (TSTR) options.c_str(), valFloat);
					outString += std::string( s->to_string() );
				}
				else if ( std::string( "i" ).find( *it1 ) != -1 ) {
					int valInt = val->to_int();
					options = "%" + options + "i";
					s->printf( (TSTR) options.c_str(), valInt );
					outString += std::string( s->to_string() );
				}
				else {
					if ( is_string(val) )
						outString += std::string( val->to_string() );
					else {
						val->sprin1( (CharStream*) s );
						outString += std::string( s->to_string() );
					}
				}
				
				options		= "";
				key			= "";
				inputIndex += 1;
				amFormating = false;
			}
		}
		else if ( amFormating ) {
			std::string errorMsg = "Cannot format character: ";
			errorMsg += *it1;
			throw RuntimeError( (TSTR) errorMsg.c_str() );
		}
		else if ( quoteChar.find( *it1 ) != -1 )
			outString += "\"";
		else
			outString += *it1;

		it1++;
	}

	one_value_local( result );
	vl.result = new String( (TSTR) outString.c_str() );
	return_value( vl.result );
}
Value*		join_cf(		Value** arg_list, int count ) {
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
		return new String( join( (Array*) arg_list[0], std::string( separatorVar->to_string() ) ).c_str() );
	else
		return new String( join( (Array*) arg_list[0] ).c_str() );
}
Value*		lstrip_cf(		Value** arg_list, int count ) {
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
	std::string source( arg_list[0]->to_string() );

	Value* separator = key_arg(separator);
	if ( separator != &unsupplied )
		lstrip( source, std::string( separator->to_string() ) );
	else
		lstrip( source );

	one_value_local( result );
	vl.result = new String( source.c_str() );
	return_value( vl.result );
}
Value*		replace_cf(		Value** arg_list, int count ) {
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

	TCHAR* textstring = arg_list[0]->to_string();
	TCHAR* searchStr = arg_list[1]->to_string();
	TCHAR* replaceStr = arg_list[2]->to_string();

	Value*		val;
	BOOL		ignoreCase = bool_key_arg(ignoreCase, val, TRUE);
	BOOL		replaceAll = bool_key_arg(all, val, FALSE);

	std::string str( textstring );
    std::string searchString( searchStr ); 
    std::string replaceString( replaceStr );

	replace( str, searchString, replaceString, (ignoreCase) ? true : false, (replaceAll) ? true : false );

	// convert to TCHAR
	const char* psz = str.c_str();
	int requiredSize = (int)str.length()+1;
	TCHAR* ptsz = new TCHAR[ requiredSize ];
	strcpy(ptsz, psz);

	return new String( ptsz );
}

Value*		rstrip_cf(		Value** arg_list, int count ) {
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
	std::string source( arg_list[0]->to_string() );

	Value* separator = key_arg(separator);
	if ( separator != &unsupplied )
		rstrip( source, std::string( separator->to_string() ) );
	else
		rstrip( source );

	one_value_local( result );
	vl.result = new String( source.c_str() );
	return_value( vl.result );
}

Value*		split_cf(		Value** arg_list, int count ) {
	check_arg_count_with_keys( blurString.split, 1, count );
	Value* separator = key_arg(separator);
	if ( separator != &unsupplied )
		return split( std::string( arg_list[0]->to_string() ), std::string( separator->to_string() ) );
	else
		return split( std::string( arg_list[0]->to_string() ) );
}
Value*		startswith_cf(	Value** arg_list, int count ) {
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
	return ( startswith( std::string( arg_list[0]->eval()->to_string() ), std::string( arg_list[1]->eval()->to_string() ) ) ) ? &true_value : &false_value;
}
Value*		strip_cf(		Value** arg_list, int count ) {
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
	std::string source( arg_list[0]->to_string() );

	Value* separator = key_arg(separator);
	if ( separator != &unsupplied ) {
		lstrip( source, std::string( separator->to_string() ) );
		rstrip( source, std::string( separator->to_string() ) );
	}
	else {
		lstrip( source );
		rstrip( source );
	}

	one_value_local( result );
	vl.result = new String( source.c_str() );
	return_value( vl.result );
}

Value*		toBool_cf(		Value** arg_list, int count ) {
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
	std::string source( arg_list[0]->to_string() );
	std::transform( source.begin(), source.end(), source.begin(), tolower );
	return ( parseString( source ) ) ? &true_value : &false_value;
}

Value*		toId_cf(		Value** arg_list, int count) {
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
	
	std::string source( arg_list[0]->to_string() );


	std::string outString			= "";
	std::string inString( arg_list[0]->to_string() );
	
	std::string validCharacters		= "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	std::string spaceChars			= "- _";
	std::string separator			= "_";
	if ( key_arg( separator ) != &unsupplied )
		separator					= std::string( key_arg( separator )->to_string() );

	std::string allowChars			= "";
	if ( key_arg( allowchars ) != &unsupplied )
		allowChars					= std::string( key_arg( allowchars )->to_string() );

	Value* separatorVal				= key_arg( separator );
	if ( separatorVal != &unsupplied )
		separator = separatorVal->to_string();

	std::string::const_iterator it1 = inString.begin();
	while ( it1 != inString.end() ) {
		if ( validCharacters.find(*it1) != -1 || allowChars.find(*it1) != -1 )
			outString += *it1;
		else if ( spaceChars.find(*it1) != -1 )
			outString += separator;
		it1++;
	}

	TSTR out			= outString.c_str();
	Value* lower		= key_arg_or_default( lowercase, &false_value );
	if ( lower == &true_value )
		out.toLower();

	one_value_local( result );
	vl.result = new String( out );
	return_value( vl.result );
}
Value*		toKey_cf(		Value** arg_list, int count ) {
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
Value*		toLower_cf(		Value** arg_list, int count ) {
	/*!------------------------------------------------------------
		\remarks
			Converts the inputed string to lowercase
		
		\param		rootStr		<string>

		\usage		blurString.toLower "TEST" -> "test"

		\return
			<string> Result
	-------------------------------------------------------------*/
	check_arg_count ( toLower, 1, count );
	std::string converter( arg_list[0]->to_string() );
	std::transform( converter.begin(), converter.end(), converter.begin(), tolower );

	one_value_local( result );
	vl.result = new String( converter.c_str() );
	return_value( vl.result );
}
Value*		toUpper_cf(		Value** arg_list, int count ) {
	/*!------------------------------------------------------------
		\remarks
			Converts the inputed string to uppercase
		
		\param		rootStr		<string>

		\usage		blurString.toUpper "test" -> "UPPER"

		\return
			<string> Result
	-------------------------------------------------------------*/
	check_arg_count ( toUpper, 1, count );
	std::string converter( arg_list[0]->to_string() );
	std::transform( converter.begin(), converter.end(), converter.begin(), toupper );
	
	one_value_local( result );
	vl.result = new String( converter.c_str() );
	return_value( vl.result );
}

//----------------------------------------------	blurString Init		---------------------------------------------------------
void BlurStringInit() { mprintf( "------ blurString Loaded - Version:%.2f ------\n", ((float) BLURSTRING_VERSION) / 1000.0f ); }