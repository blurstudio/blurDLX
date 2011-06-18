/*!
	\file		GenericMethod.h

	\remarks	Header file for the GenericMethod MAXScript extension definition
	
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

#ifndef _GENERICMETHOD_H_
#define _GENERICMETHOD_H_

#ifdef __MAXSCRIPT_2012__
#include "maxscript\maxscript.h"
#include "maxscript\foundation\structs.h"
#else
#include "MAXScrpt.h"
#include "structs.h"
#endif
#include "resource.h"

#define			NEW_GENERIC_METHOD( methodName )			( new GenericMethod( this, Name::intern( _T( #methodName ) ) ) )
#define			NEW_CONTROL_METHOD( methodName )			( new GenericControlMethod( this, Name::intern( _T( #methodName ) ) ) )

class AppliedValue : public Value {
	public:
								AppliedValue();

		void					collect() { delete this; }
		void					sprin1(CharStream* s);
		void					gc_trace();
#	define						is_appliedvalue(p) ((p)->tag == class_tag(AppliedValue));
		virtual Value*			applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc ) = 0;
};

class AppliedControl : public RolloutControl {
	public:
								AppliedControl( Value* name, Value* caption, Value** keyparms, int keyparm_count );

		void					collect() { delete this; }
		void					sprin1( CharStream* s );
		void					gc_trace();
#		define					is_appliedrolloutcontrol(p) ((p)->tag == class_tag(AppliedRolloutControl))
		virtual Value*			applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc ) = 0;
};

class GenericMethod : public Value {
	public:
		AppliedValue*			target;
		Value*					methodID;

								GenericMethod(AppliedValue* target, Value* methodID);
								~GenericMethod();

		void					collect() { delete this; }
		void					sprin1(CharStream* s);
		void					gc_trace();
#	define						is_genericmethod(p) ((p)->tag == class_tag(GenericMethod));
		Value*					apply(Value** arg_list, int count, CallContext* cc);

};

class GenericControlMethod : public Value {
	public:
		AppliedControl*	target;
		Value*					methodID;
		
								GenericControlMethod( AppliedControl* target, Value* methodID );
								~GenericControlMethod();

		void					collect() { delete this; }
		void					sprin1( CharStream* s );
		void					gc_trace();
		Value*					apply( Value** arg_list, int count, CallContext* cc );
};

#endif _GENERICMETHOD_H_
