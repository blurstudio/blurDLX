/*!
	\file		dictionary.cpp

	\remarks	Implementation file for the Dictionary MAXScript extension definition
	
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

#ifdef __MAXSCRIPT_2012__
#include "maxscript\maxscript.h"
#include "maxscript\foundation\numbers.h"
#include "maxscript\maxwrapper\maxclasses.h"
#include "maxscript\foundation\strings.h"
#else
#include "MAXScrpt.h"
#include "Numbers.h"
#include "MAXclses.h"
#include "strings.h"
#endif

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "dictionary.h"

#ifdef __MAXSCRIPT_2012__
#include "maxscript\macros\define_external_functions.h"
#include "maxscript\macros\define_instantiation_functions.h"
#else
// external name definitions
#include "defextfn.h"

// internal name definitions
#include "definsfn.h"
#endif

#define			n_clear					(Name::intern(_T("clear")))
#define			n_count					(Name::intern(_T("count")))
#define			n_dictionary			(Name::intern(_T("dictionary")))
#define			n_fail					(Name::intern(_T("fail")))
#define			n_get					(Name::intern(_T("get")))
#define			n_haskey				(Name::intern(_T("haskey")))
#define			n_index					(Name::intern(_T("index")))
#define			n_items					(Name::intern(_T("items")))
#define			n_keys					(Name::intern(_T("keys")))
#define			n_keyAtIndex			(Name::intern(_T("keyAtIndex")))
#define			n_pop					(Name::intern(_T("pop")))
#define			n_popitem				(Name::intern(_T("popitem")))
#define			n_set					(Name::intern(_T("set")))
#define			n_setKeyAtIndex			(Name::intern(_T("setKeyAtIndex")))
#define			n_sorted				(Name::intern(_T("sorted")))
#define			n_values				(Name::intern(_T("values")))

visible_class_instance( Dictionary, _T("Dictionary") );
Value*			DictionaryClass::apply( Value** arg_list, int count, CallContext* cc ) {
	check_arg_count_with_keys( Dictionary, 0, count );
	one_typed_value_local( Dictionary* out );
	vl.out = new Dictionary();
	return_value( vl.out );
}
Dictionary::Dictionary() { this->tag		= class_tag( Dictionary ); }
Dictionary::Dictionary( Dictionary* other ) {
	this->tag		= class_tag( Dictionary );

	HashMapIterator iter;
	for ( iter = other->_dict.begin(); iter != other->_dict.end(); iter++ )
		this->_dict[ (*iter).first ] = (*iter).second;
}
Dictionary::~Dictionary() {}
void			Dictionary::clear()			{ this->_dict.clear(); }
Dictionary*		Dictionary::copy() {
	one_typed_value_local( Dictionary* out );
	vl.out = new Dictionary(this);
	return_value( vl.out );
}
Value*			Dictionary::get( TSTR key, Value* fail ) {
	HashMapIterator iter = this->_dict.find( Name::intern( key ) );
	if ( iter != this->_dict.end() ) 
		return_protected( (*iter).second );
	return fail;
}
bool			Dictionary::haskey( TSTR key )						{ return ( this->_dict.find( Name::intern(key) ) != this->_dict.end() ); }
bool			Dictionary::isEmpty()								{ return ( this->length() == 0 ); }
Array*			Dictionary::items() {
	two_typed_value_locals( Array* item, Array* out );
	vl.out = new Array(0);
	HashMapIterator iter;
	for ( iter = this->_dict.begin(); iter != this->_dict.end(); iter++ ) {
		vl.item = new Array(2);
		vl.item->append( (*iter).first );
		vl.item->append( (*iter).second );
		vl.out->append( vl.item );
	}
	return_value( vl.out );
}
Array*			Dictionary::keys( bool sorted ) {
	one_typed_value_local( Array* out );

	vl.out = new Array(this->length());
	HashMapIterator iter;
	for ( iter = this->_dict.begin(); iter != this->_dict.end(); iter++ )
		vl.out->append( (*iter).first );
	
	if ( sorted )
		vl.out->sort_vf( NULL, 0 );

	return_value( vl.out );
}
int				Dictionary::length()								{ return (int)this->_dict.size(); }
Value*			Dictionary::pop( TSTR key ) {
	HashMapIterator iter = this->_dict.find( Name::intern(key) );
	if ( iter != this->_dict.end() ) {
		one_value_local( out );
		vl.out = (*iter).second;
		this->_dict.erase(iter);
		return_value( vl.out );
	}
	return &undefined;
}
Array*			Dictionary::popitem( TSTR key ) {
	HashMapIterator iter = this->_dict.find( Name::intern(key) );
	if ( iter != this->_dict.end() ) {
		one_typed_value_local( Array* out );
		vl.out			= new Array(2);
		vl.out->append( (*iter).first );
		vl.out->append( (*iter).second );
		this->_dict.erase(iter);
		return_value( vl.out );
	}
	return (Array*) &undefined;
}
bool			Dictionary::set( TSTR key, Value* val )				{ 
	if ( val == this )
		throw RuntimeError( "Cyclic storing of dictionary instance inside itself is not allowed" );
	this->_dict[ Name::intern(key) ] = val->get_heap_ptr()->eval();
	return true;
}
Array*			Dictionary::values( bool sorted ) {
	if ( sorted ) {
		one_typed_value_local( Array* out );
		vl.out		= new Array(0);
		Array* keys = this->keys( true );
		for ( int i = 0; i < keys->size; i++ )
			vl.out->append( this->_dict[ keys->data[i] ] );
		return_value( vl.out );
	}
	else {
		one_typed_value_local( Array* out );
		vl.out		= new Array(0);
		HashMapIterator iter;
		for ( iter = this->_dict.begin(); iter != this->_dict.end(); iter++ )
			vl.out->append( (*iter).second );
		return_value( vl.out );
	}
}
//--------------------------------------	MAXScript Methods		---------------------------------------

Value*			Dictionary::applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc ) {
	if ( methodID == n_clear ) {
		check_arg_count( clear, 0, count );
		this->clear();
		return &ok;
	}
	else if ( methodID == n_copy ) {
		check_arg_count (copy, 0, count);
		return this->copy();
	}
	else if ( methodID == n_get ) {
		check_arg_count_with_keys( get, 1, count );
		Value* fail = key_arg_or_default( fail, &undefined );
		return this->get( arg_list[0]->to_string(), fail );
	}
	else if ( methodID == n_haskey ) {
		check_arg_count( haskey, 1, count );
		Value* key = arg_list[0]->eval();	
		return ( this->haskey( key->to_string() ) ) ? &true_value : &false_value;
	}
	else if ( methodID == n_items ) {
		check_arg_count ( items, 0, count);
		return this->items();
	}
	else if ( methodID == n_keys ) {
		check_arg_count_with_keys( keys, 0, count );
		bool sorted = key_arg( sorted ) == &true_value;
		return this->keys(sorted);
	}
	else if ( methodID == n_pop ) {
		check_arg_count( pop, 1, count );
		return this->pop( arg_list[0]->to_string() );
	}
	else if ( methodID == n_popitem ) {
		check_arg_count( popitem, 1, count );
		return this->popitem( arg_list[0]->to_string() );
	}
	else if ( methodID == n_set ) {
		check_arg_count( set, 2, count );
		return ( this->set( arg_list[0]->eval()->to_string(), arg_list[1]->eval() ) ) ? &true_value : &false_value;
	}
	else if ( methodID == n_values ) {
		check_arg_count_with_keys( values, 0, count );
		bool sorted = key_arg( sorted ) == &true_value;
		return this->values(sorted);
	}
	return &undefined;
}
void			Dictionary::gc_trace() {
	Value::gc_trace();

	// trace sub-objects & mark me
	HashMapIterator iter;
	for(iter = this->_dict.begin(); iter != this->_dict.end(); iter++) {
		if ( (*iter).first && (*iter).first->is_not_marked() ) (*iter).first->gc_trace();
		if ( (*iter).second && (*iter).second->is_not_marked() ) (*iter).second->gc_trace();
	}
}
Value*			Dictionary::get_vf(Value** arg_list, int count)		{ return this->get( arg_list[0]->eval()->to_string(), &undefined ); }
Value*			Dictionary::get_props_vf(Value** arg_list, int count) {
	return &undefined;
}
Value*			Dictionary::put_vf( Value** arg_list, int count )	{ return this->set( arg_list[0]->eval()->to_string(), arg_list[1]->eval() ) ? &true_value : &false_value; }
void			Dictionary::sprin1( CharStream* s ) {
	s->puts(_T("(Dictionary items:"));
	this->items()->sprin1(s);
	s->puts(_T(")"));
}
Value*			Dictionary::show_props_vf(Value** arg_list, int count) {
	return &ok;
}

Value*			Dictionary::get_property(Value** arg_list, int count) {
	Value* prop = arg_list[0];

	if		(prop == n_clear)			return ( NEW_GENERIC_METHOD( clear ) );
	else if (prop == n_count)			return ( Integer::intern( this->length() ) );
	else if (prop == n_copy)			return ( this->copy() );
	else if (prop == n_get)				return ( NEW_GENERIC_METHOD( get ) );
	else if (prop == n_haskey)			return ( NEW_GENERIC_METHOD( haskey ) );
	else if (prop == n_items)			return ( NEW_GENERIC_METHOD( items ) );
	else if (prop == n_keys)			return ( NEW_GENERIC_METHOD( keys ) );
	else if (prop == n_pop)				return ( NEW_GENERIC_METHOD( pop ) );
	else if (prop == n_popitem)			return ( NEW_GENERIC_METHOD( popitem ) );
	else if (prop == n_set)				return ( NEW_GENERIC_METHOD( set ) );
	else if (prop == n_values)			return ( NEW_GENERIC_METHOD( values ) );

	return Value::get_property( arg_list, count );
}
Value*			Dictionary::map( node_map& m ) {
	two_typed_value_locals( Array* item, Value* out );
	HashMapIterator iter;
	for ( iter = this->_dict.begin(); iter != this->_dict.end(); iter++ ) {
		if ( m.vfn_ptr != NULL )
			vl.out = ((*iter).second->*(m.vfn_ptr))(m.arg_list, m.count );
		else {
			Value* arg_save		= m.arg_list[0];
			vl.item				= new Array(2);
			vl.item->append( (*iter).first );
			vl.item->append( (*iter).second );

			m.arg_list[0]		= vl.item;

			if ( m.flags & NM_MXS_FN )
				vl.out			= ((MAXScriptFunction*) m.cfn_ptr)->apply( m.arg_list, m.count );
			else
				vl.out			= arg_save;

			m.arg_list[0]		= arg_save;
		}
		if ( m.collection != NULL )
			m.collection->append( vl.out );
	}
	pop_value_locals();
	return &ok;
}
Value*			Dictionary::set_property( Value** arg_list, int count ) {
	return Value::set_property( arg_list, count );
}

//------------------------------------------------------------------------------

void DictionaryInit() { }