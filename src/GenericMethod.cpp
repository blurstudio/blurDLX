/*!
	\file		GenericMethod.cpp

	\remarks	Implementation for the GenericMethod MAXScript extension definition
	
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

#include "imports.h"

#include "GenericMethod.h"

// external name definitions
#if defined(__MAXSCRIPT_2012__) || defined(__MAXSCRIPT_2013__)
#include "macros/define_external_functions.h"
#include "macros/define_instantiation_functions.h"
#else
#include "defextfn.h"
#include "definsfn.h"
#endif


AppliedValue::AppliedValue() {
}

void AppliedValue::sprin1(CharStream* s)
{
	s->puts( _T( "AppliedValue" ) );
}

void AppliedValue::gc_trace()
{ Value::gc_trace(); }

//----------------------------------------------------------------------------------------------------

AppliedControl::AppliedControl( Value* name, Value* caption, Value** keyparms, int keyparm_count ) : RolloutControl( name, caption, keyparms, keyparm_count ) {
}

void AppliedControl::sprin1(CharStream* s) {
	s->puts( _T( "AppliedControl" ) );
}

void AppliedControl::gc_trace() { RolloutControl::gc_trace(); }

//----------------------------------------------------------------------------------------------------

GenericMethod::GenericMethod( AppliedValue* target, Value* methodID )
{
	target	= target;
	methodID	= methodID;
}

GenericMethod::~GenericMethod()
{}

void GenericMethod::gc_trace()
{
	Value::gc_trace();

	if (target	&& target->is_not_marked())
		target->gc_trace();
	if (methodID && methodID->is_not_marked())
		methodID->gc_trace();
}

void GenericMethod::sprin1(CharStream* s)
{
	if ( methodID != &undefined ) {
		s->puts( methodID->to_string() );
		s->puts(_T("()"));
	}
	else
		s->puts( _T("GenericMethod <undefined>") );
}

Value * GenericMethod::apply(Value** arg_list, int count, CallContext* cc)
{
	init_thread_locals();
	push_alloc_frame();
	Value** new_arg_list;
	value_local_array( new_arg_list, count );

	for ( int i = 0; i < count; i++ )
		new_arg_list[i] = arg_list[i]->eval();

	Value* result = target->applyMethod( methodID, new_arg_list, count, cc );

	pop_value_local_array( new_arg_list );
	pop_alloc_frame();

	return result;
}

//----------------------------------------------------------------------------------------------------

GenericControlMethod::GenericControlMethod( AppliedControl* target, Value* methodID )
{
	target	= target;
	methodID	= methodID;
}

GenericControlMethod::~GenericControlMethod()
{}

void GenericControlMethod::gc_trace()
{
	Value::gc_trace();

	if (target && target->is_not_marked())		target->gc_trace();
	if (methodID && methodID->is_not_marked())	methodID->gc_trace();
}

void GenericControlMethod::sprin1(CharStream* s)
{
	if ( methodID != &undefined ) {
		s->puts( methodID->to_string() );
		s->puts(_T("()"));
	}
	else
		s->puts( _T("GenericMethod <undefined>") );
}

Value* GenericControlMethod::apply(Value** arg_list, int count, CallContext* cc)
{
	init_thread_locals();
	push_alloc_frame();
	Value** new_arg_list;
	value_local_array( new_arg_list, count );

	for ( int i = 0; i < count; i++ )
		new_arg_list[i] = arg_list[i]->eval();

	Value* result = target->applyMethod( methodID, new_arg_list, count, cc );

	pop_value_local_array( new_arg_list );
	pop_alloc_frame();

	return result;
}
