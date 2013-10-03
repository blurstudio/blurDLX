/*!
	\file		BoundingBox.cpp

	\remarks	Maxscript extension to create a new value for 3d bounding boxes
	
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
#include "resource.h"
#include "BoundingBox.h"

#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
#include "maxscript\macros\define_external_functions.h"
#else
#include "defextfn.h"
#endif
	//def_name ( BBox3 )
/*#	include "namedefs.h"*/

#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
#include "maxscript\macros\define_instantiation_functions.h"
#else
#include "definsfn.h"
#endif
	def_name ( BBox3 )
	def_name ( Outside )
	def_name ( isEqual )
	def_name ( isContained )
	def_name ( contains )
	def_name ( clips )

/* ------------------- BBox3ValueClass  instance -------------- */

visible_class_instance(BBox3Value, "BBox3")

Value* BBox3ValueClass::apply(Value** arg_list, int count, CallContext* cc)
{
	BBox3Value *result = new BBox3Value();

	check_arg_count_with_keys (apply, 0, count);

	Value* minValue = key_arg(min);
	Value* maxValue = key_arg(max);
	
	result->bb3.begin_grow();

	if ( minValue != &unsupplied )
		result->bb3.grow ( minValue->eval()->to_point3() );

	if ( maxValue != &unsupplied)
		result->bb3.grow ( maxValue->eval()->to_point3() );

	Value* nodeValue = key_arg(node);
	if(nodeValue != &unsupplied)
	{
		INode *node = nodeValue->eval()->to_node();

		TimeValue t = GetCOREInterface()->GetTime();
		Object *obj = node->EvalWorldState(t).obj;
		Box3 boundingBox;
		obj->GetDeformBBox(t, boundingBox, &node->GetObjectTM(t));

		result->bb3.grow(boundingBox.pmin);
		result->bb3.grow(boundingBox.pmax);
		
	}

	return result;	
}

/* -------------------- PreviewParamsValue methods ----------------------- */
BBox3Value::BBox3Value()
{
}


void BBox3Value::gc_trace()
{
	/* trace sub-objects & mark me */
	Value::gc_trace();
}

void
BBox3Value::sprin1(CharStream* s)
{
	s->puts(_T("BBox3 "));
	if ( !bb3.isEmpty() )
		s->printf(_T("min:[%f,%f,%f] max:[%f,%f,%f]"), bb3.vmin.x, bb3.vmin.y, bb3.vmin.z, bb3.vmax.x, bb3.vmax.y, bb3.vmax.z);
	else
		s->printf(_T("min:[-1.#INF, -1.#INF, -1.#INF] max:[1.#INF, 1.#INF, 1.#INF]"));
}
Value*	BBox3Value::get_property(Value** arg_list, int count)
{
	Value* prop = arg_list[0];

	if (prop == n_min)
		return new Point3Value(bb3.vmin);
	else if (prop == n_max)
		return new Point3Value(bb3.vmax);

	return &undefined;
}

Value*	BBox3Value::set_property(Value** arg_list, int count)
{
	Value* val = arg_list[0];
	Value* prop = arg_list[1];

	if (prop == n_min)
	{
		bb3.vmin = val->to_point3();
		return new Point3Value(bb3.vmin);
	}
	else if (prop == n_max)
	{
		bb3.vmax = val->to_point3();
		return new Point3Value(bb3.vmax);
	}
	else
		throw RuntimeError(_T("Unknown property: "), prop);

	return val;
}

Value* BBox3Value::append_vf(Value** arg_list, int count)
{
	check_arg_count(append, 1, count);
	Value* val = arg_list[0];

	if (val->is_kind_of(class_tag(Point3Value)))
	{
		bb3.grow(val->to_point3());	
	}
	else if (val->is_kind_of(class_tag(Array)))
	{
		Array* point3Array = (Array*)val;
		for (int i = 0; i < point3Array->size; i++)
		{
			Point3 pt = point3Array->data[i]->to_point3();
			bb3.grow(pt);	
		}
	}
	else if (val->is_kind_of(class_tag(BBox3Value)))
	{
		BBox3Value* box = (BBox3Value*)val;
		bb3.grow(box->bb3);
	}

	return this;
}

Value* BBox3Value::isEmpty_vf(Value** arg_list, int count)
{
	return bb3.isEmpty() ? &true_value : &false_value;
}
Value* BBox3Value::empty_vf(Value** arg_list, int count)
{
	bb3.makeEmpty();
	return this;
}

Value* BBox3Value::intersects_vf(Value** arg_list, int count)
{
//	check_arg_count(intersects, 2, count);
	Value* val = arg_list[0];
	
	BBox3Value* box = (BBox3Value*)val;
	int intValue = bb3.intersect(box->bb3);

	if (intValue == bb3.OUTSIDE)
		return n_Outside;

	if (intValue == bb3.ISEQUAL)
		return n_isEqual;

	if (intValue == bb3.ISCONTAINED)
		return n_isContained;

	if (intValue == bb3.CONTAINS)
		return n_contains;

	if (intValue == bb3.CLIPS)
		return n_clips;

	return &undefined;
}

void BBox3ValueInit()
{
#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
	#include "maxscript\macros\define_implementations.h"
#else
	#include "defimpfn.h"
#endif
		def_name ( BBox3 )
		def_name ( Outside )
		def_name ( isEqual )
		def_name ( isContained )
		def_name ( contains )
		def_name ( clips )
}
