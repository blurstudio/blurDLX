/*!
	\file		dictionary.h

	\remarks	Header file for the Dictionary MAXScript extension definition
	
	\author		Diego Garcia Huerta
	\author		Email: diego@blur.com
	\author		Company: Blur Studio
	\date		05/01/07

	\history
				- version 1.0 DGH 05/03/07: Created

	\note
				Copyright (c) 2006, Blur Studio Inc.
				All rights reserved.

				Redistribution and use in source and binary forms, with or without 
				modification, are permitted provided that the following conditions 
				are met:

					* Redistributions of source code must retain the above copyright 
					notice, this list of conditions and the following disclaimer.
					* Redistributions in binary form must reproduce the above 
					copyright notice, this list of conditions and the following 
					disclaimer in the documentation and/or other materials provided 
					with the distribution.
					* Neither the name of the Blur Studio Inc. nor the names of its 
					contributors may be used to endorse or promote products derived 
					from this software without specific prior written permission.

				THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
				"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
				LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
				FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
				COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
				INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
				BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
				LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
				CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
				LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
				ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
				POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#include "imports.h"
#include "GenericMethod.h"
#include <hash_map>

#define _DEFINE_DEPRECATED_HASH_CLASSES 0

typedef stdext::hash_map< Value*, Value* > HashMap;
typedef HashMap::iterator HashMapIterator;

applyable_class( Dictionary );
class Dictionary : public AppliedValue {
private:
	int					_length;
	HashMap				_dict;

public:
	Dictionary();
	Dictionary( Dictionary* other );
	~Dictionary();

	//---------------------------------			C++ Methods			-----------------------------------------
	void				clear();
	Dictionary*			copy();
	Value*				get( TSTR key, Value* fail = NULL );
	bool				haskey( TSTR key );
	bool				isEmpty();
	Array*				items();
	Array*				keys( bool sorted = false );
	int					length();
	Value*				pop( TSTR key );
	Array*				popitem( TSTR key );
	bool				set( TSTR key, Value* val );
	Array*				values( bool sorted = false );

	//---------------------------------		MAXScript Methods		-----------------------------------------
						classof_methods( Dictionary, Value );
	void				collect() { delete this; }
	BOOL				_is_collection() { return TRUE; }
	void				sprin1(CharStream* s);
	void				gc_trace();

#define						is_dictionary(p) ((p)->tag == class_tag(Dictionary))

	Value*				applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc = NULL );
	Value*				get_props_vf(Value** arg_list, int count);
	Value*				get_property(Value** arg_list, int count);
	Value*				map(node_map& m);
	Value*				set_property( Value** arg_list, int count );

// max 2012 switched the name of maxscript includes
#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
	#include "maxscript\macros\define_implementations.h"
#else
	#include "defimpfn.h"
#endif
	def_generic( get, "get");
	def_generic( put, "put");
	def_generic( show_props, "showProperties");
};

#endif __DICTIONARY_H__
