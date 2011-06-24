/*!
	\file		ClassDef.cpp

	\remarks	Header file for the ClassDef MAXScript extension definition
	
	\author		Eric Hulser
	\author		Email: eric@blur.com
	\author		Company: Blur Studio
	\date		05/01/07
*/
// max 2012 switched the name of maxscript includes
#ifdef __MAXSCRIPT_2012__
#include "maxscript\maxscript.h"
#include "maxscript\util\listener.h"
#include "maxscript\foundation\numbers.h"
#include "maxscript\maxwrapper\maxclasses.h"
#include "maxscript\foundation\arrays.h"
#include "maxscript\foundation\functions.h"
#else
#include "MAXScrpt.h"
#include "Listener.h"
#include "Numbers.h"
#include "MAXclses.h"
#include "arrays.h"
#include "funcs.h"
#endif

#include <list>

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "ClassDef.h"

// external name definitions
// max 2012 switched the name of maxscript includes
#ifdef __MAXSCRIPT_2012__
#include "maxscript\macros\define_external_functions.h"
#else
#include "defextfn.h"
#endif

// internal name definitions
// max 2012 switched the name of maxscript includes
#ifdef __MAXSCRIPT_2012__
#include "maxscript\macros\define_instantiation_functions.h"
#else
#include "definsfn.h"
#endif

#define			n___add__										( Name::intern( _T( "__add__" ) ) )
#define			n___baseclass__									( Name::intern( _T( "__baseclass__" ) ) )
#define			n___baseinst__									( Name::intern( _T( "__baseinst__" ) ) )
#define			n___cmp__										( Name::intern( _T( "__cmp__" ) ) )
#define			n___classname__									( Name::intern( _T( "__classname__" ) ) )
#define			n___classof__									( Name::intern( _T( "__classof__" ) ) )
#define			n___div__										( Name::intern( _T( "__div__" ) ) )
#define			n___eq__										( Name::intern( _T( "__eq__" ) ) )
#define			n___getattr__									( Name::intern( _T( "__getattr__" ) ) )
#define			n___getitem__									( Name::intern( _T( "__getitem__" ) ) )
#define			n___propnames__									( Name::intern( _T( "__propnames__" ) ) )
#define			n___ge__										( Name::intern( _T( "__ge__" ) ) )
#define			n___gt__										( Name::intern( _T( "__gt__" ) ) )
#define			n___init__										( Name::intern( _T( "__init__" ) ) )
#define			n___iskindof__									( Name::intern( _T( "__iskindof__" ) ) )
#define			n___len__										( Name::intern( _T( "__len__" ) ) )
#define			n___le__										( Name::intern( _T( "__le__" ) ) )
#define			n___lt__										( Name::intern( _T( "__lt__" ) ) )
#define			n___mul__										( Name::intern( _T( "__mul__" ) ) )
#define			n___ne__										( Name::intern( _T( "__ne__" ) ) )
#define			n___run__										( Name::intern( _T( "__run__" ) ) )
#define			n___setattr__									( Name::intern( _T( "__setattr__" ) ) )
#define			n___setitem__									( Name::intern( _T( "__setitem__" ) ) )
#define			n___str__										( Name::intern( _T( "__str__" ) ) )
#define			n___sub__										( Name::intern( _T( "__sub__" ) ) )
#define			n___superclassof__								( Name::intern( _T( "__superclassof__" ) ) )

#define			n_active										( Name::intern( _T( "active" ) ) )
#define			n_baseclass										( Name::intern( _T( "baseClass" ) ) )
#define			n_debugger										( Name::intern( _T( "debugger" ) ) )
#define			n_endmethod										( Name::intern( _T( "endMethod" ) ) )
#define			n_log											( Name::intern( _T( "log" ) ) )
#define			n_logmessage									( Name::intern( _T( "logMessage" ) ) )
#define			n_startmethod									( Name::intern( _T( "startMethod" ) ) )
#define			n_variables										( Name::intern( _T( "variables" ) ) )

#define			MEMBER_NOT_FOUND								( Name::intern( _T( "MEMBER_NOT_FOUND" ) ) )

#define			RUN_METHOD( methodName, argCount, cppMethod )		one_value_local( result ); \
																	vl.result = this->runStructMethod( this, n_##methodName, arg_list, count );\
																	if ( !vl.result ) {\
																		check_arg_count_with_keys( ##methodName, argCount, count );\
																		vl.result = this->cppMethod; \
																	}\
																	return_value( vl.result )

#define			GET_METHOD_VALUE( methodName )						this->runStructMethod( this, n_##methodName, NULL, 0 ); \
																	if ( !vl.result ) \
																		vl.result = this->##methodName()

#define			RUN_CMP_METHOD( methodName, cmpSuccess )			check_arg_count_with_keys( __cmp__, 1, count );\
																	one_value_local( result ); \
																	vl.result = ( this->runStructMethod( this, n_##methodName, arg_list, count ) ); \
																	if ( !vl.result ) \
																		vl.result = this->##methodName( arg_list[0] ); \
																	if ( !vl.result ) { \
																		if ( !vl.result ) {\
																			vl.result = this->runStructMethod( this, n___cmp__, arg_list, count ); \
																			if ( !vl.result ) {\
																				vl.result = this->__cmp__( arg_list[0] ); \
																				if ( !vl.result ) { \
																					if ( n_##methodName != n___cmp__ ) { \
																						TSTR errorMsg	= _T( "neither __cmp__ nor " ); \
																						errorMsg		+= #methodName; \
																						errorMsg		+= _T( " is defined for " ); \
																						errorMsg		+= this->__str__(); \
																						throw RuntimeError( errorMsg ); \
																					}\
																					else { METHOD_NOT_DEFINED( __cmp__ ); } \
																				} \
																			} \
																			vl.result = ( cmpSuccess ) ? &true_value : &false_value; \
																		} \
																	} \
																	return_value( vl.result )

visible_class_instance( BlurDebugger, _T( "BlurDebugger" ) )
Value* BlurDebuggerClass::apply( Value** arg_list, int count, CallContext* cc ) {
	check_arg_count( BlurDebugger, 0, count );
	one_typed_value_local( BlurDebugger* result );

	vl.result = new BlurDebugger();
	return_value( vl.result );
}

BlurDebugger::BlurDebugger() {
	this->tag			= class_tag(BlurDebugger);
	this->active		= false;
	this->log			= NULL;
	this->indentAmount	= 0;
}
void BlurDebugger::gc_trace() {
	if ( this->log && this->log->is_not_marked() )
		this->log->gc_trace();
	Value::gc_trace();
}
void BlurDebugger::sprin1( CharStream* s ) {
	s->puts( "(BlurDebugger())" );
}
BOOL BlurDebugger::endMethod() {
	this->indentAmount--;
	if ( this->active ) {
		if ( this->log )
			this->log->puts( this->indent() + _T( "}\n" ) );
		else
			the_listener->edit_stream->puts( this->indent() + _T( "}\n" ) );
	}
	return this->active;
}
BOOL BlurDebugger::logMessage( TSTR message ) {
	if ( this->active ) {
		if ( this->log )
			this->log->puts( this->indent() + message + _T( "\n" ) );
		else
			the_listener->edit_stream->puts( this->indent() + message + _T( "\n" ) );
	}
	return this->active;
}
BOOL BlurDebugger::startMethod( TSTR classID, TSTR methodID, TSTR variables ) {
	if ( this->active ) {
		if ( this->log )
			this->log->puts( this->indent() + classID + _T( "::" ) + methodID + _T( "(" ) + variables + _T( ") {\n" ) );
		else
			the_listener->edit_stream->puts( this->indent() + classID + _T( "::" ) + methodID + _T( "(" ) + variables + _T( ") {\n" ) );
	}
	this->indentAmount++;
	return this->active;
}
TSTR BlurDebugger::indent() {
	TSTR outIndent = _T("");
	for ( int i = 1; i <= this->indentAmount; i++ )
		outIndent += _T("\t");
	return outIndent;
}
Value* BlurDebugger::applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc ) {
	if ( methodID == n_logmessage ) {
		check_arg_count( logMessage, 1, count );
		this->logMessage( arg_list[0]->to_string() );
	}
	else if ( methodID == n_endmethod ) {
		check_arg_count( endMethod, 0, count );
		this->endMethod();
	}
	else if ( methodID == n_startmethod ) {
		check_arg_count_with_keys( startMethod, 2, count );
		Value* variableValue = (key_arg_or_default(variables, NULL));
		TSTR variables = _T( "" );
		if ( variableValue )
			variables = variableValue->to_string();

		this->startMethod( arg_list[0]->to_string(), arg_list[1]->to_string(), variables );
	}
	return &ok;
}
Value* BlurDebugger::get_property( Value** arg_list, int count ) {
	Value* key = arg_list[0];
	one_value_local( result );
	if ( key == n_active )				{ vl.result = ( this->active ) ? &true_value : &false_value; }
	else if ( key == n_log )			{ vl.result = ( this->log != NULL ) ? this->log : the_listener->edit_stream; }
	else if ( key == n_logmessage )		{ vl.result = NEW_GENERIC_METHOD( logMessage ); }
	else if ( key == n_endmethod )		{ vl.result = NEW_GENERIC_METHOD( endMethod ); }
	else if ( key == n_startmethod )	{ vl.result = NEW_GENERIC_METHOD( startMethod ); }
	else { throw RuntimeError( (TSTR) key->to_string() + _T( " is an unknown parameter of BlurDebugger" ) ); }
	return vl.result;
}
Value* BlurDebugger::set_property( Value** arg_list, int count ) {
	Value* key = arg_list[1];
	Value* val = arg_list[0];
	if ( key == n_active )				{ this->active = ( val == &true_value ); }
	else if ( key == n_log )			{ this->log = (CharStream*) val; }
	else { throw RuntimeError( (TSTR) key->to_string() + _T( " cannot be set for a BlurDebugger value." ) ); }
	return &ok;
}
//---------------------------------------------------------------------------------------------------
//										ClassDef
//---------------------------------------------------------------------------------------------------

visible_class_instance( ClassDef, _T( "ClassDef" ) )

Value* ClassDefClass::apply( Value** arg_list, int count, CallContext* cc ) {
	check_arg_count_with_keys( arg_list, 2, count );
	one_typed_value_local( ClassDef* result );

	Value* tBaseClass = (key_arg_or_default(baseclass, NULL));
	if ( tBaseClass != NULL && is_classdef(tBaseClass->eval()) )
		vl.result = new ClassDef( arg_list[0]->eval(), (StructDef*) arg_list[1]->eval(), (ClassDef*) tBaseClass->eval() );
	else
		vl.result = new ClassDef( arg_list[0]->eval(), (StructDef*) arg_list[1]->eval() );
	return_value( vl.result );
}

// Constructor function
ClassDef::ClassDef( Value* className, StructDef* structDefinition, ClassDef* baseClass ) {
	this->tag				= class_tag( ClassDef );
	this->className			= className;
	this->structDefinition	= structDefinition;
	this->baseClass			= baseClass;
}
ClassDef::~ClassDef() {}

void			ClassDef::gc_trace() {
	if ( this->structDefinition && this->structDefinition->is_not_marked() )
		this->structDefinition->gc_trace();
	if ( this->className && this->className->is_not_marked() )
		this->className->gc_trace();
	if ( this->baseClass && this->baseClass->is_not_marked() )
		this->baseClass->gc_trace();
	Value::gc_trace();
}
//---------------------------------------------------------------------------------------------------------------------------------
//							Virtual ClassDef Methods	( overload these functions to create new class types in C++ )
//---------------------------------------------------------------------------------------------------------------------------------

Value*			ClassDef::__init__( Value** arg_list, int count ) {
	BOOL success = false;
	if ( this->structDefinition ) {
		// HACK: to get to compile, make members, member_inits public in structs.h
		Value* member = this->structDefinition->members->get( n___init__ );
		if ( member && is_function( member ) )
			return ( member->apply( arg_list, count ) );
	}
	if ( this->baseClass ) return this->baseClass->__init__( arg_list, count );
	return &ok;
}
ClassValue*		ClassDef::__newinstance__()				{ return ( new ClassValue( this ) ); }
Value*			ClassDef::__getattr__( Value* key ) {
	Value* result	= NULL;
	if ( this->structDefinition ) {
		Value* member = this->structDefinition->members->get( key );
		if ( member ) {
			if ( !is_function( member ) )
				// HACK: to get to compile, make members, member_inits public in structs.h
				result = this->structDefinition->member_inits[ member->to_int() ];
			else
				result = member;
		}
	}
	return result;
}
Value*			ClassDef::__setattr__( Value* key, Value* val ) {
	Value* result	= NULL;
	if ( this->structDefinition ) {
		Value* member = this->structDefinition->members->get( key );
		if ( member && !is_function( member ) ) {
			this->structDefinition->member_inits[ member->to_int() ] = val;
			result = val;
		}
	}
	return result;
}
Array*			ClassDef::__propnames__() {
	one_typed_value_local( Array* result );
	vl.result = new Array(0);
	if ( this->structDefinition )
		vl.result->join( this->structDefinition->get_props_vf( NULL, 0 ) );
	if ( this->baseClass ) {
		Array* baseProps	= this->baseClass->__propnames__();
		BOOL found			= false;
		for ( int i = 0; i < baseProps->size; i++ ) {
			found = false;
			for ( int y = 0; y < vl.result->size; y++ ) {
				if ( vl.result->data[y] == baseProps->data[i] ) {
					found = true;
					break;
				}
			}
			if ( !found )
				vl.result->append( baseProps->data[i] );
		}
	}
	return_value( vl.result );
}
TSTR			ClassDef::__str__() {
	TSTR out	= _T( "<ClassDef " );
	out			+= this->className->to_string();
	out			+= ">";
	return out;
}
//---------------------------------------------------------------------------------------------------------------------------------
//							MAXScript Parser Functions (handles default get/set/run functionality for classValues)
//---------------------------------------------------------------------------------------------------------------------------------

Value*			ClassDef::get_props_vf( Value** arg_list, int count )	{ return ( this->__propnames__() ); }
Value*			ClassDef::apply( Value** arg_list, int count, CallContext* cc ) {
	one_typed_value_local( ClassValue* result );
	vl.result = this->__newinstance__();
	vl.result->__init__( vl.result, arg_list, count );
	return_value( vl.result );
}
Value*			ClassDef::applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc ) { 
	if ( methodID == n___init__ )			{ return this->__init__( arg_list, count ); }
	else if ( methodID == n___getattr__ )	{ return this->get_property( arg_list, count ); }
	else if ( methodID == n___setattr__ )	{ return this->set_property( arg_list, count ); }
	else { return this->__run__( methodID, arg_list, count ); }
}
Value*			ClassDef::get_property( Value** arg_list, int count ) {
	check_arg_count( __getattr__, 1, count );
	one_value_local( result );
	Value* key	= arg_list[0];
	vl.result	= this->__getattr__( key );

	if ( !vl.result ) {
		if ( key == n___classname__ )		{ vl.result = this->className; }
		else if ( key == n___init__ )		{ vl.result = NEW_GENERIC_METHOD( __init__ ); }
		else if ( key == n___getattr__ )	{ vl.result = NEW_GENERIC_METHOD( __getattr__ ); }
		else if ( this->baseClass )			{ vl.result = this->baseClass->get_property( arg_list, count ); }
	}

	if ( !vl.result ) {
		MEMBER_NOT_DEFINED( key, this );
	}

	return_value( vl.result );
}
Value*			ClassDef::set_property( Value** arg_list, int count ) {
	check_arg_count( __getattr__, 2, count );
	one_value_local( result );
	Value* key	= arg_list[1];
	Value* val	= arg_list[0];

	vl.result	= this->__setattr__( key, val );
	if ( !vl.result ) {
		if ( this->baseClass ) { this->baseClass->set_property( arg_list, count ); }
	}

	if ( !vl.result ) {
		MEMBER_NOT_DEFINED( key, this );
	}
	
	return_value( vl.result );
}

//---------------------------------------------------------------------------------------------------
//										ClassValue
//---------------------------------------------------------------------------------------------------
visible_class_instance( ClassValue, "ClassValue" )
ClassValue::ClassValue( ClassDef* definition ) {
	this->tag					= class_tag( ClassValue );
	this->definition			= definition;
	this->structInst			= NULL;
	this->baseInst				= NULL;
}
ClassValue::~ClassValue() {
}
void		ClassValue::gc_trace() {
	if ( this->definition && this->definition->is_not_marked() )
		this->definition->gc_trace();
	if ( this->baseInst && this->baseInst->is_not_marked() )
		this->baseInst->gc_trace();
	if ( this->structInst && this->structInst->is_not_marked() )
		this->structInst->gc_trace();
	
	std::list<Value*>::const_iterator iter;
	for ( iter = this->overrides.begin(); iter != this->overrides.end(); iter++ ) {
		(*iter)->gc_trace();
	}
	
	Value::gc_trace();
}
//---------------------------------------------------------------------------------------------------------------------------------
//							Virtual ClassValue Methods	( overload these functions to create new class types in C++ )
//---------------------------------------------------------------------------------------------------------------------------------

Value*		ClassValue::__getattr__(	ClassValue* self, Value* key ) {
	Value* result = NULL;

	if ( this->__ismember__( key ) ) {
		Value** new_arg_list;
		value_local_array(new_arg_list, 1);
		new_arg_list[0] = key;
		push_alloc_frame();

		result = this->definition->structDefinition->members->get( key );
		if ( result ) {
			if ( is_function( result ) )
				result = new ClassMethod( self, result );
			else
				result = this->structInst->member_data[ result->to_int() ];
		}

		pop_alloc_frame();
		pop_value_local_array(new_arg_list);
	}

	return result;
}
Value*		ClassValue::__setattr__(	ClassValue* self, Value* key, Value* val ) {
	Value* result = NULL;
	if ( this->__ismember__( key ) ) {
		Value** new_arg_list;
		value_local_array(new_arg_list, 2);
		new_arg_list[0] = val;
		new_arg_list[1] = key;
		push_alloc_frame();

		result = this->structInst->set_property( new_arg_list, 2 );

		pop_alloc_frame();
		pop_value_local_array(new_arg_list);
	}
	return result;
}
void		ClassValue::__init__(		ClassValue* self, Value** arg_list, int count, BOOL runInit ) {
	if ( this->definition->structDefinition )
		this->structInst = (Struct*) this->definition->structDefinition->apply( arg_list, 0, NULL );

	if ( this->definition->baseClass ) {
		this->baseInst = this->definition->baseClass->__newinstance__();
		this->baseInst->__init__( self, arg_list, count, runInit && !this->__ismember__( n___init__ ) );
	}
	if ( runInit && this->__ismember__( n___init__ ) )
		this->runStructMethod( self, n___init__, arg_list, count );
}
BOOL		ClassValue::__ismember__(	Value* key ) {
	if ( this->definition && this->definition->structDefinition )
		return ( this->definition->structDefinition->members->get( key ) != NULL );
	return false;
}
Array*		ClassValue::__propnames__() {
	if ( this->definition ) {
		return ( this->definition->__propnames__() );
	}
	else {
		METHOD_NOT_DEFINED( __propnames__ );
	}
}
TSTR		ClassValue::__str__() {
	TSTR out	= _T( "" );
	Value* result = this->runStructMethod( this, n___str__ );
	if ( result ) {
		out += (TCHAR*) result->to_string();
	}
	else {
		out		+= _T( "<" );
		out		+= this->definition->className->to_string();
		out		+= _T( " Instance>" );
	}
	return out;
}
Value*		ClassValue::__baseclass__()		{ if ( this->baseInst ) return this->baseInst->definition;	else return &undefined; }
Value*		ClassValue::__baseinst__()		{ if ( this->baseInst ) return this->baseInst;				else return &undefined; }
Value*		ClassValue::__classname__()		{ return this->definition->className; }
Value*		ClassValue::__classof__()		{ return this->definition; }
Value*		ClassValue::__superclassof__()	{ return ( this->baseInst ) ? this->baseInst->classOf_vf(NULL,0) : this->definition->tag; }
Value*		ClassValue::__iskindof__( Value* classID ) {
	if ( this->definition && this->definition->className == classID || this->definition == classID )
		return &true_value;
	else if ( this->baseInst )
		return this->baseInst->__iskindof__( classID );
	return &false_value;
}

//---------------------------------------------------------------------------------------------------------------------------------
//								MAXScript Parser Functions (handles default get/set/run functionality for classValues)
//---------------------------------------------------------------------------------------------------------------------------------

BOOL		ClassValue::isOverrideEnabled( Value* methodName ) {
	BOOL found = false;
	std::list<Value*>::const_iterator iter;
	for ( iter = this->overrides.begin(); iter != this->overrides.end(); iter++ ) {
		if ((*iter) == methodName) {
			found = true;
			break;
		}
	}
	return (!found);
}
void		ClassValue::setOverrideEnabled( Value* methodName, BOOL state ) {
	if ( !state )
		this->overrides.push_back( methodName );
	else
		this->overrides.remove( methodName );
}

Value*		ClassValue::applyMethod(		Value* methodID, Value** arg_list, int count, CallContext* cc ) {
	one_value_local( result );
	vl.result = &undefined;

	if ( methodID == n___getattr__ )		{ 
		check_arg_count( __getattr__, 1, count);
		vl.result = this->get_prop( this, arg_list[0] ); 
	}
	else if ( methodID == n___setattr__ )	{ 
		check_arg_count( __setattr__, 2, count );
		vl.result = this->set_prop( this, arg_list[0], arg_list[1] ); 
	}
	else if ( methodID == n___iskindof__ )	{ 
		check_arg_count( __iskindof__, 1, count );
		vl.result = this->__iskindof__( arg_list[0] ); 
	}
	else									{ vl.result = this->__run__( methodID, arg_list, count ); }
	
	return vl.result;
}
Value*		ClassValue::get_prop(			ClassValue* self, Value* key ) {
	one_value_local( result );

	vl.result	= this->__getattr__( self, key );

	if ( vl.result == NULL ) {
		if (	  key == n___baseclass__ )				{ vl.result = GET_METHOD_VALUE( __baseclass__ ); }
		else if ( key == n___baseinst__ )				{ vl.result = GET_METHOD_VALUE( __baseinst__ ); }
		else if ( key == n___classname__ )				{ vl.result = GET_METHOD_VALUE( __classname__ ); }
		else if ( key == n___classof__ )				{ vl.result = GET_METHOD_VALUE( __classof__ ); }
		else if ( key == n_count )						{ vl.result = GET_METHOD_VALUE( __len__ ); }
		else if ( key == n___iskindof__ )				{ vl.result = NEW_GENERIC_METHOD( __iskindof__ ); }
		else if ( key == n___getattr__ )				{ vl.result = NEW_GENERIC_METHOD( __getattr__ ); }
		else if ( key == n___len__ )					{ vl.result = NEW_GENERIC_METHOD( __len__ ); }
		else if ( key == n___setattr__ )				{ vl.result = NEW_GENERIC_METHOD( __setattr__ ); }
		else if ( key == n___init__ )					{ vl.result = NEW_GENERIC_METHOD( __init__ ); }
		else if ( this->__ismember__( n___getattr__ ) ) { 
			Value** new_arg_list;
			value_local_array( new_arg_list, 2 );
			new_arg_list[0] = self;
			new_arg_list[1] = key;
			
			push_alloc_frame();
			vl.result = this->runStructMethod( this, n___getattr__, new_arg_list, 2, false ); 

			pop_alloc_frame();
			pop_value_local_array( new_arg_list );
		}
		else if ( this->baseInst )						{ vl.result = this->baseInst->get_prop( self, key ); }
	}
	
	if ( vl.result == NULL ) {
		MEMBER_NOT_DEFINED( key, self );
	}

	return_value( vl.result );
}
Value*		ClassValue::set_prop(			ClassValue* self, Value* key, Value* val ) {
	one_value_local( result );

	vl.result = this->__setattr__( self, key, val );

	if ( vl.result == NULL ) {
		if ( this->__ismember__( n___setattr__ ) )	{ 
			Value** new_arg_list;
			value_local_array( new_arg_list, 3 );
			new_arg_list[0] = self;
			new_arg_list[1] = key;
			new_arg_list[2] = val;
			
			push_alloc_frame();
			vl.result = this->runStructMethod( this, n___setattr__, new_arg_list, 3, false ); 

			pop_alloc_frame();
			pop_value_local_array( new_arg_list );
		}
		else if ( this->baseInst )					{ vl.result = this->baseInst->set_prop( self, key, val ); }
	}
	if ( vl.result == NULL ) {
		MEMBER_NOT_DEFINED( key, self );
	}
	return_value( vl.result );
}
Value*		ClassValue::runStructMethod(	ClassValue* self, Value* methodName, Value** arg_list, int count, BOOL insertSelf ) {
	Value* result = NULL;
	if ( this->definition && this->definition->structDefinition ) {
		/*if ( methodName == this->activeStructMethodName ) {
			TSTR errorMSG	= _T( "Warning: Probable infinite loop found in " );
			errorMSG		+= this->definition->className->to_string();
			errorMSG		+= _T( "::" );
			errorMSG		+= methodName->to_string();
			errorMSG		+= _T( "()" );
			throw RuntimeError(errorMSG);
		}*/
		if ( this->__ismember__( methodName ) && this->isOverrideEnabled( methodName ) ) {
			this->setOverrideEnabled( methodName, false );
			Value* structMethod				= this->definition->structDefinition->members->get( methodName );
			if ( is_function( structMethod ) ) {
				if ( insertSelf ) {
					Value** new_arg_list;
					value_local_array( new_arg_list, count + 1 );
					new_arg_list[0] = self;
					for ( int i = 0; i < count; i++ )
						new_arg_list[ i+1 ] = arg_list[i]->eval();
					
					push_alloc_frame();
					result = structMethod->apply( new_arg_list, count + 1 );

					pop_alloc_frame();
					pop_value_local_array( new_arg_list );
				}
				else
					result = structMethod->apply( arg_list, count );
			}
			this->setOverrideEnabled( methodName, true );
		}
	}
	if ( !result && this->baseInst )
		result = this->baseInst->runStructMethod( self, methodName, arg_list, count, insertSelf );

	return result;
}
//---------------------------------------------------------------------------------------------------------------------------------
//								MAXScript Functions (normal maxscript calls mapped to classvalue functions)
//---------------------------------------------------------------------------------------------------------------------------------

void		ClassValue::sprin1( CharStream* s ) { s->puts( this->__str__() ); }

Value*		ClassValue::classOf_vf(			Value** arg_list, int count )	{ RUN_METHOD( __classof__, 0, __classof__() ); }
Value*		ClassValue::isKindOf_vf(		Value** arg_list, int count )	{ RUN_METHOD( __iskindof__, 1, __iskindof__( arg_list[0] ) ); }
Value*		ClassValue::superClassOf_vf(	Value** arg_list, int count )	{ RUN_METHOD( __superclassof__, 0, __superclassof__() ); }

Value*		ClassValue::get_props_vf(		Value** arg_list, int count)	{ RUN_METHOD( __propnames__, 0, __propnames__() ); }

Value*		ClassValue::get_vf(				Value** arg_list, int count )	{ RUN_METHOD( __getitem__, 1, __getitem__( arg_list[0] ) ); }
Value*		ClassValue::put_vf(				Value** arg_list, int count )	{ RUN_METHOD( __setitem__, 2, __setitem__( arg_list[0], arg_list[1] ) ); }

Value*		ClassValue::plus_vf(			Value** arg_list, int count )	{ RUN_METHOD( __add__, 1, __add__( arg_list[0] ) ); }
Value*		ClassValue::minus_vf(			Value** arg_list, int count )	{ RUN_METHOD( __sub__, 1, __sub__( arg_list[0] ) ); }
Value*		ClassValue::times_vf(			Value** arg_list, int count )	{ RUN_METHOD( __mul__, 1, __mul__( arg_list[0] ) ); }
Value*		ClassValue::div_vf(				Value** arg_list, int count )	{ RUN_METHOD( __div__, 1, __div__( arg_list[0] ) ); }

Value*		ClassValue::eq_vf(				Value** arg_list, int count )	{ RUN_METHOD( __eq__, 1, __eq__( arg_list[0] ) ); }
Value*		ClassValue::ne_vf(				Value** arg_list, int count )	{ RUN_METHOD( __ne__, 1, __ne__( arg_list[0] ) ); }
Value*		ClassValue::gt_vf(				Value** arg_list, int count )	{ RUN_CMP_METHOD( __gt__, vl.result->to_int() == 1 ); }
Value*		ClassValue::ge_vf(				Value** arg_list, int count )	{ RUN_CMP_METHOD( __ge__, vl.result->to_int() == 1 || vl.result->to_int() == 0 ); }
Value*		ClassValue::lt_vf(				Value** arg_list, int count )	{ RUN_CMP_METHOD( __lt__, vl.result->to_int() == -1 ); }
Value*		ClassValue::le_vf(				Value** arg_list, int count )	{ RUN_CMP_METHOD( __le__, vl.result->to_int() == -1 || vl.result->to_int() == 0 ); }

Value*		ClassValue::get_property(		Value** arg_list, int count )	{ return this->get_prop( this, arg_list[0] ); }
Value*		ClassValue::set_property(		Value** arg_list, int count )	{ return this->set_prop( this, arg_list[1], arg_list[0] ); }

//---------------------------------------------------------------------------------------------------------------------------------
//													ClassMethod Methods
//---------------------------------------------------------------------------------------------------------------------------------
ClassMethod::ClassMethod( ClassValue* self, Value* functionInst ) {
	this->tag				= functionInst->tag;
	this->self				= self;
	this->functionInst		= functionInst;
}
ClassMethod::~ClassMethod() {
}
void		ClassMethod::gc_trace() {
	if ( this->self && this->self->is_not_marked() )
		this->self->gc_trace();
	if ( this->functionInst && this->functionInst->is_not_marked() )
		this->functionInst->gc_trace();
	Value::gc_trace();
}

Value*		ClassMethod::apply( Value** arg_list, int count, CallContext* cc ) {
	one_value_local( result );
	vl.result = &undefined;
	if ( this->functionInst ) {
		Value** new_arg_list;
		value_local_array( new_arg_list, count+1 );
		new_arg_list[0] = this->self;
		for ( int i = 0; i < count; i++ )
			new_arg_list[ i+1 ] = arg_list[ i ];

		vl.result = this->functionInst->apply( new_arg_list, (count+1), cc );

		pop_value_local_array( new_arg_list );
	}
	return_value( vl.result );
}

//---------------------------------------------------------------------------------------------------------------------------------
//												New Maxscript Functions
//---------------------------------------------------------------------------------------------------------------------------------
def_visible_primitive( isClassDef, "isClassDef");
Value* isClassDef_cf( Value** arg_list, int count ) {
	check_arg_count( isClassDef, 1, count );
	if ( is_classdef( arg_list[0]->eval() ) )
		return &true_value;
	return &false_value;
}

def_visible_primitive( isClass, "isClass" );
Value* isClass_cf( Value** arg_list, int count ) {
	check_arg_count( isClass, 1, count );
	if ( is_classvalue( arg_list[0]->eval() ) )
		return &true_value;
	return &false_value;
}

//---------------------------------------------------------------------------------------------------------------------------------
//												ClassDef Init
//---------------------------------------------------------------------------------------------------------------------------------
void ClassDefInit() {
	CharStream* out = thread_local(current_stdout);
	out->puts(_T("--* ClassDef loaded"));
	out->printf(" ( Version %.2f | %s | 3DStudio Max %.1f build ) *--\n", ((float) CLASSDEF_VERSION )/1000.0f, _T(CLASSDEF_DATE),(float)GET_MAX_RELEASE(VERSION_3DSMAX)/1000);
}