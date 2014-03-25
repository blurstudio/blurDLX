/*
	\file		imports.h

	\remarks	This file contains with it all the imports necessary
				for this plugin to compile, and in the precise order
				they need to be imported.

	\author		Blur Studio (c) 2010
	\email		beta@blur.com

	\license	This software is released under the GNU General Public License.  For more info, visit: http://www.gnu.org/
*/

#ifndef __IMPORTS_H__
#define __IMPORTS_H__


// As of 2013 max is compiled with UNICODE enabled - MCHAR == TCHAR == wchar, utf16
#ifdef __MAXSCRIPT_2013__
#define _UNICODE
#define UNICODE
#endif

// max 2012 switched the name of maxscript includes
#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__

#include <WindowsX.h>

#include "foundation/colors.h"
#include "foundation/numbers.h"
#define private public
#include "foundation/structs.h"
#undef private
#include "foundation/strings.h"
#include "foundation/streams.h"
#include "foundation/3dmath.h"
#include "foundation/mxstime.h"

#include "kernel/exceptions.h"
#include "kernel/value.h"

#include "maxwrapper/bitmaps.h"
#include "maxwrapper/maxclasses.h"
#include "maxwrapper/objectsets.h"
#include "maxwrapper/mxsobjects.h"
#include "maxwrapper/mxsmaterial.h"
#include "maxwrapper/meshselection.h"

#include "compiler/parser.h"
#include "compiler/thunks.h"

#include "macros/local_class.h"

#include "animtbl.h"
#include "maxicon.h"
#include "maxscript.h"
#include "ScripterExport.h"
#include "icolorman.h"
#include "CoreFunctions.h"
#include "units.h"

#include "UI/rollouts.h"

extern TCHAR* GetString(int id);

// these are the includes for previous versions of 3dsmax
#else

#include "MAXScrpt.h"
#include "Numbers.h"
#include "MAXclses.h"
#include "structs.h"
#include "strings.h"
#include "excepts.h"
#include "Parser.h"
#include "maxscrpt.h"
#include "value.h"
#include "rollouts.h"
#include "MAXObj.h"
#include "ColorVal.h"
#include "MAXMats.h"
#include "Streams.h"
#include "MSTime.h"
#include "LclClass.h"
#include "MeshSub.h"
#include "3dmath.h"
#include "maxmats.h"

#endif

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#endif __IMPORTS_H__
