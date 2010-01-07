/*!
	\file		ClassDef.h

	\remarks	Header file for the ClassDef MAXScript extension definition
	
	\author		Eric Hulser
	\author		Email: eric@blur.com
	\author		Company: Blur Studio
	\date		05/01/07

	\history
				- version 1.0 EKH 05/03/07: Created
*/

#ifndef _CLASSDEF_H_
#define _CLASSDEF_H_

#include "MAXScrpt.h"
#include "funcs.h"
#include "streams.h"
#include "resource.h"
#include "GenericMethod.h"

#include <list>

#define			CLASSDEF_VERSION									1000
#define			CLASSDEF_DATE										__DATE__

#define			DEFINE_CLASS( className )							( define_system_global( #className, get_##className, NULL ) )
#define			NEW_CLASS( className )								def_thread_local( className*, _##className, new className() ); \
																	Value* get_##className##() { \
																		if ( !_##className ) { _##className = new className(); } \
																		return ( _##className ); \
																	}

#define			METHOD_NOT_DEFINED( methodName )					TSTR errorMsg	= _T( #methodName ); \
																	errorMsg		+= _T( " not defined for " ); \
																	errorMsg		+= this->__str__(); \
																	throw RuntimeError( errorMsg );

#define			MEMBER_NOT_DEFINED( memberName, root )				TSTR errorMsg	= _T( "Unknown parameter #" ); \
																	errorMsg		+= memberName->to_string(); \
																	errorMsg		+= _T( " in " ); \
																	errorMsg		+= root->__str__(); \
																	throw RuntimeError( errorMsg );

class ClassValue;
applyable_class( BlurDebugger )
class BlurDebugger : public AppliedValue {
	public:
		BOOL			active;
		CharStream*		log;
		int				indentAmount;

		ScripterExport				BlurDebugger();
									~BlurDebugger() {};

		void						collect()					{ delete this; }
		void						gc_trace();
		ScripterExport void			sprin1( CharStream* s );
		
		classof_methods (BlurDebugger, Value);
		

		BOOL				startMethod( TSTR classID, TSTR methodID, TSTR variables );
		BOOL				logMessage( TSTR message );
		BOOL				endMethod();
		TSTR				indent();

		Value*				applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc );
		Value*				get_property( Value** arg_list, int count );
		Value*				set_property( Value** arg_list, int count );
};

applyable_class( ClassDef )
class ClassDef : public AppliedValue {
	public:
		StructDef*					structDefinition;			// Pointer to the StructDef that defines this ClassDef instance
		ClassDef*					baseClass;					// Pointer to the baseClass ClassDef for this instance
		Value*						className;

		// Constructor/Destructor
		ScripterExport				ClassDef( Value* className, StructDef* definition = NULL, ClassDef* baseClass = NULL );
									~ClassDef();

		void						collect()					{ delete this; }
		void						gc_trace();
		ScripterExport void			sprin1( CharStream* s )		{ s->puts( this->__str__() ); }

		//---------------------------------------------------------------------------------------------------------------------------------
		//							Virtual ClassDef Methods	( overload these functions to create new class types in C++ )
		//---------------------------------------------------------------------------------------------------------------------------------

		virtual Value*				__classof__()													{ if ( this->baseClass ) { return this->baseClass; } else { return this->tag; } }
		virtual Value*				__superclassof__()												{ return ( this->baseClass ) ? this->baseClass->__classof__() : &Value_class; }
		virtual Value*				__getattr__(	Value* key );
		virtual Value*				__init__( Value** arg_list, int count );
		virtual ClassValue*			__newinstance__();
		virtual Array*				__propnames__();
		virtual Value*				__run__(		Value* methodID, Value** arg_list, int count )	{ return &undefined; }
		virtual Value*				__setattr__(	Value* key, Value* val );
		virtual TSTR				__str__();

		
		//---------------------------------------------------------------------------------------------------------------------------------
		//								MAXScript Functions (default maxscript functions that call the above methods)
		//---------------------------------------------------------------------------------------------------------------------------------
# define							is_classdef(o) ((o)->tag == class_tag( ClassDef ))
#include "defimpfn.h"
		def_generic( get_props, "getPropNames" );

		Value*						applyMethod(		Value* methodID, Value** arg_list, int count, CallContext* cc );
		Value*						apply(				Value** arg_list, int count, CallContext* cc = NULL );
		Value*						get_property(		Value** arg_list, int count );
		Value*						set_property(		Value** arg_list, int count );
		Value*						classOf_vf(			Value** arg_list, int count )	{ return this->__classof__(); }
		Value*						superClassOf_vf(	Value** arg_list, int count )	{ return this->__superclassof__(); }
};

visible_class( ClassValue )
class ClassValue : public AppliedValue {
	public:
		ClassDef*						definition;				// Pointer to the ClassDef instance that defines what type of ClassDef this instance is
		Struct*							structInst;				// Pointer to the Struct that contains the active variables for this ClassDef instance
		ClassValue*						baseInst;				// Pointer to the ClassValue instance created by this ClassDef's baseClass ClassDef
		std::list<Value*>				overrides;

		// Constructor/Destructor
		ScripterExport					ClassValue( ClassDef* definition );
										~ClassValue();

		virtual void					gc_trace();
		virtual void					collect() { delete this; }
		ScripterExport virtual void		sprin1( CharStream* s );
		
		//---------------------------------------------------------------------------------------------------------------------------------
		//							Virtual ClassValue Methods	( overload these functions to create new class types in C++ )
		//---------------------------------------------------------------------------------------------------------------------------------

		virtual void		__init__( ClassValue* self, Value** arg_list, int count, BOOL runInit = true );										// this()
		virtual BOOL		__ismember__( Value* key );																			// this.__ismember__ #<member>
		virtual TSTR		__str__();																							// this as string, this.__str__(), print this, ..
		virtual Value*		__run__( Value* methodID, Value** arg_list, int count )		{ return &undefined; }					// this.<methodID>()
		virtual Value*		__getattr__( ClassValue* self, Value* key );														// getProperty this #<member>, this.<member>, this.__getattr__ #<member>
		virtual Value*		__setattr__( ClassValue* self, Value* key, Value* val );											// setProperty this #<member> <val>, this.#<member> = <val>, this.__setattr__ #<member> <val>

		// Class Description
		virtual Value*		__baseclass__();																					// this.__baseclass__
		virtual Value*		__baseinst__();																						// this.__baseinst__
		virtual Value*		__classname__();																					// this.__classname__
		virtual Value*		__classof__();																						// classOf this, this.__classof__
		virtual Value*		__iskindof__( Value* classID );																		// isKindOf this #<classID>, this.__iskindof__ #<classID>
		virtual Value*		__superclassof__();																					// superClassOf this, this.__superclassof__

		// Arithmatic
		virtual Value*		__add__( Value* other )					{ METHOD_NOT_DEFINED( __add__ ); }							// this + <other>
		virtual Value*		__div__( Value* other )					{ METHOD_NOT_DEFINED( __div__ ); }							// this / <other>
		virtual Value*		__mul__( Value* other )					{ METHOD_NOT_DEFINED( __mul__ ); }							// this * <other>
		virtual Value*		__sub__( Value* other )					{ METHOD_NOT_DEFINED( __sub__ ); }							// this - <other>

		// Get/Set Item
		virtual Value*		__getitem__( Value* key )				{ METHOD_NOT_DEFINED( __getitem__ ); }						// this[ #<key> ]
		virtual Value*		__setitem__( Value* key, Value* val )	{ METHOD_NOT_DEFINED( __setitem__ ); }						// this[ #<key> ] = <val>

		// Prop names
		virtual Array*		__propnames__();																					// getPropNames this, this.__propnames__

		// Count
		virtual Value*		__len__()								{ METHOD_NOT_DEFINED( __len__ ); }							// this.count

		// Comparisons
		virtual Value*		__eq__( Value* other )					{ return ( this == other ) ? &true_value : &false_value; }	// this == <other>
		virtual Value*		__ne__( Value* other )					{ return ( this != other ) ? &true_value : &false_value; }	// this != <other>
		virtual Value*		__cmp__( Value* other )					{ return NULL; }											// cmp( this, <other> ) ? ( 1: this > <other> ) | ( 0: this == <other> ) | ( -1: this < <other> )
		virtual Value*		__ge__( Value* other )					{ return NULL; }											// this >= <other>
		virtual Value*		__gt__( Value* other )					{ return NULL; }											// this > <other>
		virtual Value*		__le__( Value* other )					{ return NULL; }											// this <= <other>
		virtual Value*		__lt__( Value* other )					{ return NULL; }											// this < <other>

		BOOL				isOverrideEnabled( Value* methodName );
		void				setOverrideEnabled( Value* methodName, BOOL state );

		//---------------------------------------------------------------------------------------------------------------------------------
		//								MAXScript Functions (default maxscript functions that call the above methods)
		//---------------------------------------------------------------------------------------------------------------------------------
# define							is_classvalue(o) ((o)->tag == class_tag( ClassValue ))
#include "defimpfn.h"
		def_generic		( get_props,	"getPropNames" );
		def_generic		( get,			"get");
		def_generic		( put,			"put");

		def_generic		( plus,			"+" );
		def_generic		( minus,		"-" );
		def_generic		( times,		"*" );
		def_generic		( div,			"/" );

		def_generic		( eq,			"==" );
		def_generic		( ne,			"!=" );
		def_generic		( gt,			">" );
		def_generic		( ge,			">=" );
		def_generic		( lt,			"<" );
		def_generic		( le,			"<=" );

		Value*						isKindOf_vf(Value** arg_list, int count);
		Value*						classOf_vf( Value** arg_list, int count );
		Value*						superClassOf_vf( Value** arg_list, int count );

		Value*						get_prop( ClassValue* self, Value* key );
		Value*						get_property( Value** arg_list, int count );

		Value*						runStructMethod( ClassValue* self, Value* methodName, Value** arg_list = NULL, int count = 0, BOOL insertSelf = true );

		Value*						set_prop( ClassValue* self, Value* key, Value* val );
		Value*						set_property( Value** arg_list, int count );

		Value*						applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc );
		
		Value*						run( Value* methodName, TCHAR* errorMsg, Value** arg_list, int count, BOOL insertThis = true );		// Runs a function by name, inserting this instance value as the primary input
};

class ClassMethod : public Value {
	public:
		Value*						functionInst;										// Pointer to the MAXScript function instance
		ClassValue*					self;												// Pointer to the ClassValue instance that owns this ClassMethod

		// Constructor/Destructor
		ScripterExport				ClassMethod( ClassValue* self, Value* functionInst = NULL );
									~ClassMethod();

		// Required MAXScript methods
#define								is_classfunction(o) ((o)->tag == class_tag( ClassMethod ) )

		BOOL						_is_function()				{ return TRUE; }
		void						collect()					{ delete this; }
		void						gc_trace();
		ScripterExport void			sprin1( CharStream* s )		{ if ( this->functionInst ) this->functionInst->sprin1( s ); }

		// MAXScript usage methods
#include "defimpfn.h"

		Value*						classOf_vf( Value** arg_list, int count )						{ return this->functionInst->classOf_vf( arg_list, count ); }
		Value*						superClassOf_vf( Value** arg_list, int count )					{ return this->functionInst->superClassOf_vf( arg_list, count ); }
		Value*						apply( Value** arg_list, int count, CallContext* cc = NULL );
};
#endif _CLASSDEF_H_
