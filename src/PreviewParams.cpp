/*!
	\file		PreviewParams.cpp

	\remarks	Maxscript extension to have access to the 'make preview' options
	
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
#include "maxscript\UI\rollouts.h"
#include "maxscript\foundation\numbers.h"
#include "maxscript\foundation\3dmath.h"
#include "maxscript\maxwrapper\mxsobjects.h"
#include "maxscript\maxwrapper\maxclasses.h"
#else
#include "MAXScrpt.h"
#include "Rollouts.h"
#include "Numbers.h"
#include "3DMath.h"
#include "MAXObj.h"
#include "MAXclses.h"
#endif

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "PreviewParams.h"

#ifdef __MAXSCRIPT_2012__
#include "maxscript\macros\define_external_functions.h"
#else
#include "defextfn.h"
#endif
	def_name ( previewparams )
/*#	include "namedefs.h"*/

#ifdef __MAXSCRIPT_2012__
#include "maxscript\macros\define_instantiation_functions.h"
#else
#include "definsfn.h"
#endif
	def_name ( previewparams )
	def_name ( outputType )
	def_name ( pctSize )
	//def_name ( start )
	def_name ( end )
	//def_name ( skip )
	def_name ( fps )
	def_name ( dspGeometry )
	def_name ( dspShapes )
	def_name ( dspLights )
	def_name ( dspCameras )
	def_name ( dspHelpers )
	def_name ( dspSpaceWarps )
	def_name ( dspGrid )
	def_name ( dspSafeFrame )
	def_name ( dspFrameNums )
	def_name ( rndLevel )
	def_name ( dspBkg )

/*#	include "bmatpro.h"
#	include "boxpro.h"
#	include "notespro.h"
#	include "xrefspro.h"*/



/* ------------------- PreviewParamsValue class instance -------------- */

visible_class_instance (PreviewParamsValue, _T("PreviewParams"))

Value* PreviewParamsValueClass::apply(Value** arg_list, int count, CallContext* cc)
{
//	check_arg_count_with_keys(PreviewParamsValue, 0, count);
	Interface *ip = GetCOREInterface();
	Interval animRange = ip->GetAnimRange();

//	Value* outputType = control_param_or_default(outputType , Integer::intern(0));	// 0=default AVI codec, 1=user picks file, 2=user picks device
	/*Value* pctSize = control_param_or_default(pctSize, Integer::intern(50));	// percent (0-100) of current rendering output res
	
	// frame limits
	Value* start = control_param_or_default(start, Integer::intern(animRange.Start()));
	Value* end = control_param_or_default(end, Integer::intern(animRange.End()));
	Value* skip = control_param_or_default(skip, Integer::intern(0));

	// frame rate
	Value* fps = control_param_or_default(fps, Integer::intern(GetFrameRate()));*/

	// display control
	/*Value* dspGeometry = control_param_or_default(dspGeometry);
	Value* dspShapes = control_param_or_default(dspShapes);
	Value* dspLights = control_param_or_default(dspLights);
	Value* dspCameras = control_param_or_default(dspCameras);
	Value* dspHelpers = control_param_or_default(dspHelpers);
	Value* dspSpaceWarps = control_param_or_default(dspSpaceWarps);
	Value* dspGrid = control_param_or_default(dspGrid);
	Value* dspSafeFrame = control_param_or_default(dspSafeFrame);
	Value* dspFrameNums = control_param_or_default(dspFrameNums);

	// rendering level
	Value* rndLevel = control_param_or_default(rndLevel);	// 0=smooth/hi, 1=smooth, 2=facet/hi, 3=facet
						// 4=lit wire, 6=wire, 7=box
	Value* dspBkg = control_param_or_default(dspBkg);*/

	PreviewParamsValue *result = new PreviewParamsValue();
	

	result->pvs.outputType = 0;

	return result;
}

/* -------------------- PreviewParamsValue methods ----------------------- */
PreviewParamsValue::PreviewParamsValue()
{
	tag = class_tag(PreviewParamsValue);

	Interface *ip = GetCOREInterface();
	Interval animRange = ip->GetAnimRange();

	pvs.outputType = 0;	// 0=default AVI codec, 1=user picks file, 2=user picks device
	pvs.pctSize = 50;	// percent (0-100) of current rendering output res
	
	// frame limits
	pvs.start = animRange.Start() / GetTicksPerFrame();
	pvs.end = animRange.End() / GetTicksPerFrame();
	pvs.skip = 1;

	// frame rate
	pvs.fps = GetFrameRate();

	// display control
	pvs.dspGeometry = TRUE;
	pvs.dspShapes = FALSE;
	pvs.dspLights = FALSE;;
	pvs.dspCameras = FALSE;
	pvs.dspHelpers = FALSE;
	pvs.dspSpaceWarps = FALSE;
	pvs.dspGrid = FALSE;
	pvs.dspSafeFrame = FALSE;
	pvs.dspFrameNums = TRUE;

	// rendering level
	pvs.rndLevel = 0;	// 0=smooth/hi, 1=smooth, 2=facet/hi, 3=facet
						// 4=lit wire, 6=wire, 7=box
	pvs.dspBkg = TRUE;
}

void PreviewParamsValue::gc_trace()
{
	/* trace sub-objects & mark me */
	Value::gc_trace();
}

void
PreviewParamsValue::sprin1(CharStream* s)
{
	s->puts(_T("PreviewParams "));
	s->printf(_T("outputType:%d "), pvs.outputType);
	s->printf(_T("pctSize:%d "), pvs.pctSize);
	s->printf(_T("start:%d "), pvs.start);
	s->printf(_T("end:%d "), pvs.end);
	s->printf(_T("dspGeometry:%d "), pvs.dspGeometry);
	s->printf(_T("dspShapes:%d "), pvs.dspShapes);
	s->printf(_T("dspLights:%d "), pvs.dspLights);
	s->printf(_T("dspCameras:%d "), pvs.dspCameras);
	s->printf(_T("dspHelpers:%d "), pvs.dspHelpers);
	s->printf(_T("dspSpaceWarps:%d "), pvs.dspSpaceWarps);
	s->printf(_T("dspSafeFrame:%d "), pvs.dspSafeFrame);
	s->printf(_T("dspFrameNums:%d "), pvs.dspFrameNums);
	s->printf(_T("rndLevel:%d "), pvs.rndLevel);
	s->printf(_T("dspBkg:%d \n"), pvs.dspBkg);
}
Value*	PreviewParamsValue::get_property(Value** arg_list, int count)
{
	Value* prop = arg_list[0];

	if (prop == n_outputType)
		return Integer::intern(pvs.outputType);
	else if (prop == n_pctSize)
		return Integer::intern(pvs.pctSize);
	else if (prop == n_start)
		return Integer::intern(pvs.start);
	else if (prop == n_end)
		return Integer::intern(pvs.end);
	else if (prop == n_skip)
		return Integer::intern(pvs.skip);
	else if (prop == n_fps)
		return Integer::intern(pvs.fps);
	else if (prop == n_dspGeometry)
		return pvs.dspGeometry ? &true_value : &false_value;
	else if (prop == n_dspShapes)
		return pvs.dspShapes ? &true_value : &false_value;
	else if (prop == n_dspLights)
		return pvs.dspLights ? &true_value : &false_value;
	else if (prop == n_dspCameras)
		return pvs.dspCameras ? &true_value : &false_value;
	else if (prop == n_dspHelpers)
		return pvs.dspHelpers ? &true_value : &false_value;
	else if (prop == n_dspSpaceWarps)
		return pvs.dspSpaceWarps ? &true_value : &false_value;
	else if (prop == n_dspGrid)
		return pvs.dspGrid ? &true_value : &false_value;
	else if (prop == n_dspSafeFrame)
		return pvs.dspSafeFrame ? &true_value : &false_value;
	else if (prop == n_dspFrameNums)
		return pvs.dspFrameNums ? &true_value : &false_value;
	else if (prop == n_rndLevel)
		return Integer::intern(pvs.rndLevel);
	else if (prop == n_dspBkg)
		return pvs.dspBkg ? &true_value : &false_value;
/*	else
		return RolloutControl::get_property(arg_list, count);*/

	return &undefined;
}

Value*	PreviewParamsValue::set_property(Value** arg_list, int count)
{
	Value* val = arg_list[0];
	Value* prop = arg_list[1];

	if (prop == n_outputType)
		pvs.outputType = val->to_int();
	else if (prop == n_pctSize)
		pvs.pctSize = val->to_int();
	else if (prop == n_start)
		pvs.start = val->to_int();
	else if (prop == n_end)
		pvs.end = val->to_int();
	else if (prop == n_skip)
		pvs.skip = val->to_int();
	else if (prop == n_fps)
		pvs.fps = val->to_int();
	else if (prop == n_dspGeometry)
		pvs.dspGeometry = val->to_bool();
	else if (prop == n_dspShapes)
		pvs.dspShapes = val->to_bool();
	else if (prop == n_dspLights)
		pvs.dspLights = val->to_bool();
	else if (prop == n_dspCameras)
		pvs.dspCameras = val->to_bool();
	else if (prop == n_dspHelpers)
		pvs.dspHelpers = val->to_bool();
	else if (prop == n_dspSpaceWarps)
		pvs.dspSpaceWarps = val->to_bool();
	else if (prop == n_dspGrid)
		pvs.dspGrid = val->to_bool();
	else if (prop == n_dspSafeFrame)
		pvs.dspSafeFrame = val->to_bool();
	else if (prop == n_dspFrameNums)
		pvs.dspFrameNums = val->to_bool();
	else if (prop == n_rndLevel)
		pvs.rndLevel = val->to_int();
	else if (prop == n_dspBkg)
		pvs.dspBkg = val->to_bool();
	else
		throw RuntimeError(_T("Unknown property: "), prop);

	return val;
}

def_visible_primitive( MakePreview, "MakePreview");

Value* MakePreview_cf(Value** arg_list, int count)
{
	//check_arg_count(MakePreview, 1, count);
	
	//if (is_previewparams(arg_list[0]))
	{
		PreviewParams pvs = PreviewParams();//(PreviewParamsValue*) arg_list[0];
		
		Interface *ip = GetCOREInterface();
		Interval animRange = ip->GetAnimRange();

		pvs.outputType = 0;	// 0=default AVI codec, 1=user picks file, 2=user picks device
		pvs.pctSize = 100;	// percent (0-100) of current rendering output res
		
		// frame limits
		pvs.start = animRange.Start() / GetTicksPerFrame();
		pvs.end = animRange.End() / GetTicksPerFrame();
		pvs.skip = 1;

		// frame rate
		pvs.fps = GetFrameRate();

		// display control
		pvs.dspGeometry = 1;
		pvs.dspShapes = 1;
		pvs.dspLights = 1;;
		pvs.dspCameras = 1;
		pvs.dspHelpers = 1;
		pvs.dspSpaceWarps = 1;
		pvs.dspGrid = 1;
		pvs.dspSafeFrame = 1;
		pvs.dspFrameNums = 0;

		// rendering level
		pvs.rndLevel = 0;	// 0=smooth/hi, 1=smooth, 2=facet/hi, 3=facet
							// 4=lit wire, 6=wire, 7=box
		pvs.dspBkg = TRUE;		
		
		MAXScript_interface->CreatePreview(&pvs);
		//PreviewParamsValue* pvsValue = (PreviewParamsValue*) arg_list[0];
		//MAXScript_interface->CreatePreview((PreviewParams*)&pvsValue->pvs);
	}
	return &ok;
}

void PreviewParamsInit()
{
	#ifdef __MAXSCRIPT_2012__
	#include "maxscript\macros\define_implementations.h"
	#else
	#include "defimpfn.h"
	#endif
		def_name ( previewparams )
		def_name ( outputType )
		def_name ( pctSize )
		//def_name ( start )
		def_name ( end )
		//def_name ( skip )
		def_name ( fps )
		def_name ( dspGeometry )
		def_name ( dspShapes )
		def_name ( dspLights )
		def_name ( dspCameras )
		def_name ( dspHelpers )
		def_name ( dspSpaceWarps )
		def_name ( dspGrid )
		def_name ( dspSafeFrame )
		def_name ( dspFrameNums )
		def_name ( rndLevel )
		def_name ( dspBkg )
}