/*!
	\file		test.cpp

	\remarks	Maxscript extensions - testing bed
	
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


#include "MAXScrpt.h"
#include "Rollouts.h"
#include "Numbers.h"
#include "3DMath.h"
#include "MAXObj.h"
#include "MAXclses.h"
#include "Parser.h"
#include "Listener.h"
#include "richedit.h"
#include "richedit.h"
#include "hashtab.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "Resource.h"

#include "defextfn.h"
	// def_name ( nodeCreate )

#include "definsfn.h"
	def_name ( vp )
	def_name ( missingOnly )
	def_name ( 1stMissing )
	def_name ( dontRecurse )
	def_name ( checkAwork1 )

typedef struct{ int	id; Value* val; } paramType;

/******************************************************************************
*       This is a part of the Microsoft Source Code Samples. 
*       Copyright (C) 1993-1997 Microsoft Corporation.
*       All rights reserved. 
*       This source code is only intended as a supplement to 
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the 
*       Microsoft samples programs.
******************************************************************************/

HBITMAP CopyScreenToBitmap(HWND hWnd)
{
	// check for a valid window handle      
	if (!hWnd)         
		return NULL; 

	RECT rectClient;             
	POINT pt1, pt2;								// get client dimensions              
	GetClientRect(hWnd, &rectClient);              // convert client coords to screen coords              
	pt1.x = rectClient.left;             
	pt1.y = rectClient.top;             
	pt2.x = rectClient.right;             
	pt2.y = rectClient.bottom;             
	ClientToScreen(hWnd, &pt1);             
	ClientToScreen(hWnd, &pt2);             
	rectClient.left = pt1.x;             
	rectClient.top = pt1.y;             
	rectClient.right = pt2.x;             
	rectClient.bottom = pt2.y;  

	HDC         hScrDC, hMemDC;         // screen DC and memory DC     
	int         nX, nY, nX2, nY2;       // coordinates of rectangle to grab     
	int         nWidth, nHeight;        // DIB width and height     
	int         xScrn, yScrn;           // screen resolution      

	HGDIOBJ     hOldBitmap , hBitmap;

	// check for an empty rectangle 
	if (IsRectEmpty(&rectClient))       
		return NULL;      

	// create a DC for the screen and create     
	// a memory DC compatible to screen DC          

	hScrDC = CreateDC("DISPLAY", NULL, NULL, NULL);     
	hMemDC = CreateCompatibleDC(hScrDC);      // get points of rectangle to grab  

	nX = rectClient.left;     
	nY = rectClient.top;     
	nX2 = rectClient.right;     
	nY2 = rectClient.bottom;      // get screen resolution      

	xScrn = GetDeviceCaps(hScrDC, HORZRES);     
	yScrn = GetDeviceCaps(hScrDC, VERTRES);      

	//make sure bitmap rectangle is visible      

	if (nX < 0)         
	nX = 0;     

	if (nY < 0)         
	nY = 0;     

	if (nX2 > xScrn)         
	nX2 = xScrn;     

	if (nY2 > yScrn)         
	nY2 = yScrn;      

	nWidth = nX2 - nX;     
	nHeight = nY2 - nY;      

	// create a bitmap compatible with the screen DC     
	hBitmap = CreateCompatibleBitmap(hScrDC, nWidth, nHeight);      

	// select new bitmap into memory DC     
	hOldBitmap =   SelectObject (hMemDC, hBitmap);      

	// bitblt screen DC to memory DC     
	BitBlt(hMemDC, 0, 0, nWidth, nHeight, hScrDC, nX, nY, SRCCOPY);     

	// select old bitmap back into memory DC and get handle to     
	// bitmap of the screen          
	hBitmap = SelectObject(hMemDC, hOldBitmap);      

	// clean up      
	DeleteDC(hScrDC);     
	DeleteDC(hMemDC);      

	// return handle to the bitmap      
	return (HBITMAP)hBitmap; 
}

def_struct_primitive( getRolloutWindow, blurUtil, "getRolloutWindow");
Value* getRolloutWindow_cf(Value** arg_list, int count)
{
	Rollout* theRollout = (Rollout*) arg_list[0];
	return (Integer::intern( (int)theRollout->page ));
}

def_struct_primitive( getRelatedNodes, blurUtil, "getRelatedNodes");

Value* getRelatedNodes_cf(Value** arg_list, int count)
{
	ReferenceTarget* refTar = arg_list[0]->to_reftarg();
	one_typed_value_local(Array* result);
	vl.result = new Array(0);

	RefList& theRefs = refTar->GetRefList();
	RefListItem* item;
	item = theRefs.FirstItem();
	
	while (item) 
	{
		vl.result->append(new String((item->maker)->NodeName()));
		(item->maker)->SuperClassID();
		item=item->next;
	}

	return_value(vl.result);
}


class MyEnumProc : public DependentEnumProc 
{
    public :
    virtual int proc(ReferenceMaker *rmaker); 
    INodeTab Nodes;              
	int count;
};

int MyEnumProc::proc(ReferenceMaker *rmaker) 
{ 
	if (rmaker->SuperClassID()==BASENODE_CLASS_ID)    
	{
        Nodes.Append(1, (INode **)&rmaker);                 
	}
    return 0;              
}

def_struct_primitive( getRelatedNodes2, blurUtil, "getRelatedNodes2");

Value* getRelatedNodes2_cf(Value** arg_list, int count)
{
	ReferenceTarget* refTar = arg_list[0]->to_reftarg();
	one_typed_value_local(Array* result);
	vl.result = new Array(0);

	MyEnumProc dep;              
	refTar->EnumDependents(&dep);

	for (int i = 0; i < dep.Nodes.Count(); i++)
	{
		Interval valid;
		INode *node = dep.Nodes[i];
		vl.result->append(new String(node->GetName()));
	}
	
	return_value(vl.result);
}

// blurutil.getViewportWindow()
/*def_struct_primitive( getViewportWindow, blurUtil, "getViewportWindow");
Value* getViewportWindow_cf(Value** arg_list, int count)
{
	ViewExp *vpt = MAXScript_interface->GetActiveViewport();
	GraphicsWindow* gw = vpt->getGW();
	HWND hVpt = (HWND) vpt->GetHWnd();
	
	RECT r;
	PAINTSTRUCT ps;
	
	HFONT hFont;
	HFONT hfontPrev;

	HDC hdc = BeginPaint(hVpt, &ps);
	
	SetBkMode(hdc, TRANSPARENT); 

	LOGFONT lf;
	GetObject(m_hFont, sizeof(LOGFONT), &lf);
	hFont = CreateFontIndirect(&lf);

	hfontPrev = SelectFont(hdc, hFont);

	GetClientRect(hVpt, &r);

	TCHAR text[ ] = "Defenestration can be hazardous"; 
	TextOut(hdc,r.left,r.top,text, 31);
	
	SelectObject(hdc, hfontPrev);

	EndPaint(hVpt, &ps);

	MAXScript_interface->ReleaseViewport(vpt);
	return Integer::intern((int)hVpt); 
}*/

def_struct_primitive( getMaxscriptWindow, blurUtil, "getMaxscriptWindow");
Value* getMaxscriptWindow_cf(Value** arg_list, int count)
{
	HWND windowHandle = (HWND)arg_list[0]->to_int();
	edit_window* ew = (edit_window*)GetWindowLong(windowHandle, GWL_USERDATA);
	if (!ew)
		return &undefined;
	if (!ew->window)
		return &undefined;
	
	Value* file_name = ew->file_name;
	
	HWND hRichEdit = ew->edit_box;

	// get the whole script text
	TSTR windowText;
	int len = SendMessage(hRichEdit, WM_GETTEXTLENGTH, 0, 0);
	windowText.Resize(len+1);
	SendMessage(hRichEdit, WM_GETTEXT, len+1, (LPARAM)windowText.data());

	// get the selected text in the script
	DWORD dwStart, dwEnd;
	SendMessage(hRichEdit, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);
	
	TSTR selectedText;
	selectedText.Resize(dwEnd - dwStart +1);
	SendMessage(hRichEdit, EM_GETSELTEXT, len+1, (LPARAM)selectedText.data());

	return new String(selectedText);

	//return file_name;
	//return Integer::intern((int)ew->window);
}


/*
def_struct_primitive( captureWindow, blurUtil, "captureWindow");

Value* captureWindow_cf(Value** arg_list, int count)
{
	HBITMAP hBitmap = CopyScreenToBitmap(GetCOREInterface()->GetMAXHWnd());

	BITMAPINFO	*bmi = NULL;
	Bitmap		*bmap;
	BitmapInfo	bi;
	
	int size;

	bmi  = (BITMAPINFO *)malloc(size);
	bmih = (BITMAPINFOHEADER *)bmi;

	// Initialize the BitmapInfo instance
	bi.SetType(BMM_TRUE_64);
	bi.SetWidth(320);
	bi.SetHeight(200);
	bi.SetFlags(MAP_HAS_ALPHA);
	bi.SetCustomFlag(0);

	// Create a new bitmap
	bmap = TheManager->Create(&bi);

	bmp->FromDib(bmi);
   	
	
	new MAXBitMap(bi, bmap);
}
*/


#define def_enumfile_types()							\
	paramType enumfileTypes[] = {						\
		{ FILE_ENUM_INACTIVE,		n_inactive },		\
		{ FILE_ENUM_VP,				n_vp },				\
		{ FILE_ENUM_RENDER,			n_render },			\
		{ FILE_ENUM_ALL,			n_all },			\
		{ FILE_ENUM_MISSING_ONLY,	n_missingOnly },	\
		{ FILE_ENUM_1STSUB_MISSING,	n_1stMissing },		\
		{ FILE_ENUM_DONT_RECURSE,	n_dontRecurse },	\
		{ FILE_ENUM_CHECK_AWORK1,	n_checkAwork1 },	\
	};

class TestNameEnumCallback:public NameEnumCallback
{
	void RecordName(TCHAR *name) {
			CharStream* out = thread_local(current_stdout); 
			out->printf(_T("name: %s\n"), name);
		}
};

TestNameEnumCallback nameEnum;

#define elements(array)			(sizeof(array)/sizeof((array)[0]))

int GetID(paramType params[], int count, Value* val, int def_val = -1)
{
	for(int p = 0; p < count; p++)
		if(val == params[p].val)
			return params[p].id;
	if (def_val != -1 ) return def_val;

	// Throw an error showing valid args incase an invalid arg is passed
	TCHAR buf[2056]; buf[0] = '\0';
	
	_tcscat(buf, GetString(IDS_FUNC_NEED_ARG_OF_TYPE));	
	for (int i=0; i < count; i++) 
	{
		_tcscat(buf, params[i].val->to_string());
		if (i < count-1) _tcscat(buf, _T(" | #"));
	}	
	_tcscat(buf, GetString(IDS_FUNC_GOT_ARG_OF_TYPE));
	throw RuntimeError(buf, val); 
	return -1;
}

def_struct_primitive( EnumAuxFiles, blurUtil, "EnumAuxFiles");

Value* EnumAuxFiles_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(EnumAuxFiles, 1, count);	
	def_enumfile_types();
	MAXScript_interface->EnumAuxFiles(nameEnum, GetID(enumfileTypes, elements(enumfileTypes), arg_list[0]));
	return &ok;
}

// filestorage.cpp : Defines the entry point for the console application.
//



/*#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[])
{
	return 0;
}*/


//
//+============================================================================

#define UNICODE
#define _UNICODE

#include <stdio.h>
#include <windows.h>
#include <wtypes.h>

#pragma comment( lib, "ole32.lib" )
#pragma comment( lib, "user32.lib" )


//+----------------------------------------------------------------------------
//
//  ConvertVarTypeToString
//  
//  Generate a string for a given variable type, vt. For the sake of simplicity it is assumed that 
//  the pwszType parameter points to a buffer that is at least MAX_PATH
//  in length.
//
//+----------------------------------------------------------------------------

void
ConvertVarTypeToString( VARTYPE vt, WCHAR *pwszType )
{
    switch( vt & VT_TYPEMASK )
    {
    case VT_EMPTY:
        wcscpy( pwszType, L"VT_EMPTY" );
        break;
    case VT_NULL:
        wcscpy( pwszType, L"VT_NULL" );
        break;
    case VT_I2:
        wcscpy( pwszType, L"VT_I2" );
        break;
    case VT_I4:
        wcscpy( pwszType, L"VT_I4" );
        break;
    case VT_I8:
        wcscpy( pwszType, L"VT_I8" );
        break;
    case VT_UI2:
        wcscpy( pwszType, L"VT_UI2" );
        break;
    case VT_UI4:
        wcscpy( pwszType, L"VT_UI4" );
        break;
    case VT_UI8:
        wcscpy( pwszType, L"VT_UI8" );
        break;
    case VT_R4:
        wcscpy( pwszType, L"VT_R4" );
        break;
    case VT_R8:
        wcscpy( pwszType, L"VT_R8" );
        break;
    case VT_CY:
        wcscpy( pwszType, L"VT_CY" );
        break;
    case VT_DATE:
        wcscpy( pwszType, L"VT_DATE" );
        break;
    case VT_BSTR:
        wcscpy( pwszType, L"VT_BSTR" );
        break;
    case VT_ERROR:
        wcscpy( pwszType, L"VT_ERROR" );
        break;
    case VT_BOOL:
        wcscpy( pwszType, L"VT_BOOL" );
        break;
    case VT_VARIANT:
        wcscpy( pwszType, L"VT_VARIANT" );
        break;
    case VT_DECIMAL:
        wcscpy( pwszType, L"VT_DECIMAL" );
        break;
    case VT_I1:
        wcscpy( pwszType, L"VT_I1" );
        break;
    case VT_UI1:
        wcscpy( pwszType, L"VT_UI1" );
        break;
    case VT_INT:
        wcscpy( pwszType, L"VT_INT" );
        break;
    case VT_UINT:
        wcscpy( pwszType, L"VT_UINT" );
        break;
    case VT_VOID:
        wcscpy( pwszType, L"VT_VOID" );
        break;
    case VT_SAFEARRAY:
        wcscpy( pwszType, L"VT_SAFEARRAY" );
        break;
    case VT_USERDEFINED:
        wcscpy( pwszType, L"VT_USERDEFINED" );
        break;
    case VT_LPSTR:
        wcscpy( pwszType, L"VT_LPSTR" );
        break;
    case VT_LPWSTR:
        wcscpy( pwszType, L"VT_LPWSTR" );
        break;
    case VT_RECORD:
        wcscpy( pwszType, L"VT_RECORD" );
        break;
    case VT_FILETIME:
        wcscpy( pwszType, L"VT_FILETIME" );
        break;
    case VT_BLOB:
        wcscpy( pwszType, L"VT_BLOB" );
        break;
    case VT_STREAM:
        wcscpy( pwszType, L"VT_STREAM" );
        break;
    case VT_STORAGE:
        wcscpy( pwszType, L"VT_STORAGE" );
        break;
    case VT_STREAMED_OBJECT:
        wcscpy( pwszType, L"VT_STREAMED_OBJECT" );
        break;
    case VT_STORED_OBJECT:
        wcscpy( pwszType, L"VT_BLOB_OBJECT" );
        break;
    case VT_CF:
        wcscpy( pwszType, L"VT_CF" );
        break;
    case VT_CLSID:
        wcscpy( pwszType, L"VT_CLSID" );
        break;
    default:
		wcscpy( pwszType, L"Unknown" );
        //wsprintf( pwszType, L"Unknown (%d)", vt & VT_TYPEMASK );
        break;
    }

    if( vt & VT_VECTOR )
        wcscat( pwszType, L" | VT_VECTOR" );
    if( vt & VT_ARRAY )
        wcscat( pwszType, L" | VT_ARRAY" );
    if( vt & VT_RESERVED )
        wcscat( pwszType, L" | VT_RESERVED" );
}


//+----------------------------------------------------------------------------
//
//  ConvertValueToString
//  
//  Generate a string for the value in a given PROPVARIANT structure.
//  The most common types are supported (that is, those that can be displayed
//  with printf).  For other types, only an ellipse "..." is used.
//
//  It is assumed for simplicity that the pwszValue parameter points to
//  a buffer that is at least MAX_PATH in length.
//
//+----------------------------------------------------------------------------

void
ConvertValueToString( const PROPVARIANT &propvar, WCHAR *pwszValue )
{
    switch( propvar.vt )
    {
    case VT_EMPTY:
        wcscpy( pwszValue, L"" );
        break;
    case VT_NULL:
        wcscpy( pwszValue, L"" );
        break;
    case VT_I2:
        swprintf( pwszValue, L"%i", propvar.iVal );
        break;
    case VT_I4:
    case VT_INT:
        swprintf( pwszValue, L"%li", propvar.lVal );
        break;
    case VT_I8:
        swprintf( pwszValue, L"%I64i", propvar.hVal );
        break;
    case VT_UI2:
        swprintf( pwszValue, L"%u", propvar.uiVal );
        break;
    case VT_UI4:
    case VT_UINT:
        swprintf( pwszValue, L"%lu", propvar.ulVal );
        break;
    case VT_UI8:
        swprintf( pwszValue, L"%I64u", propvar.uhVal );
        break;
    case VT_R4:
        swprintf( pwszValue, L"%f", propvar.fltVal );
        break;
    case VT_R8:
        swprintf( pwszValue, L"%lf", propvar.dblVal );
        break;
    case VT_BSTR:
        swprintf( pwszValue, L"\"%s\"", propvar.bstrVal );
        break;
    case VT_ERROR:
        swprintf( pwszValue, L"0x%08X", propvar.scode );
        break;
    case VT_BOOL:
        swprintf( pwszValue, L"%s", VARIANT_TRUE == propvar.boolVal ? L"True" : L"False" );
        break;
    case VT_I1:
        swprintf( pwszValue, L"%i", propvar.cVal );
        break;
    case VT_UI1:
        swprintf( pwszValue, L"%u", propvar.bVal );
        break;
    case VT_VOID:
        wcscpy( pwszValue, L"" );
        break;
    case VT_LPSTR:
        swprintf( pwszValue, L"\"%hs\"", propvar.pszVal );
        break;
    case VT_LPWSTR:
        swprintf( pwszValue, L"\"%s\"", propvar.pwszVal );
        break;
    case VT_FILETIME:
        swprintf( pwszValue, L"%08x:%08x", propvar.filetime.dwHighDateTime, propvar.filetime.dwLowDateTime );
        break;
    case VT_CLSID:
        pwszValue[0] = L'\0';
        StringFromGUID2( *propvar.puuid, pwszValue, MAX_PATH );
        break;
    default:
        wcscpy( pwszValue, L"..." );
        break;
    }

}


//+----------------------------------------------------------------------------
//
//  DumpProperty
//
//  Dump the ID, name, type, and value of a property.
//
//+----------------------------------------------------------------------------

void
DumpProperty( const PROPVARIANT &propvar, const STATPROPSTG &statpropstg )
{
    WCHAR wsz[ MAX_PATH + 1 ];

    ConvertVarTypeToString( statpropstg.vt, wsz );
    wprintf( L"   ----------------------------------------------------\n"
             L"   PropID = %-5d VarType = %-23s", statpropstg.propid, wsz );

    if( NULL != statpropstg.lpwstrName )
        wprintf( L" Name = %s", statpropstg.lpwstrName );

    ConvertValueToString( propvar, wsz );
    wprintf( L"\n   Value = %s\n", wsz ); 

}


//+----------------------------------------------------------------------------
//
//  DumpPropertySet
//
//  Dump all the properties into a given property set.
//
//+----------------------------------------------------------------------------

void
DumpPropertySet( IPropertyStorage *pPropStg )
{
    IEnumSTATPROPSTG *penum = NULL;
    HRESULT hr = S_OK;
    STATPROPSTG statpropstg;
    PROPVARIANT propvar;
    PROPSPEC propspec;

    PropVariantInit( &propvar );

    try
    {
        // If this property set has a friendly name, display it now.

        PROPID propid = PID_DICTIONARY;
        WCHAR *pwszFriendlyName = NULL;
        hr = pPropStg->ReadPropertyNames( 1, &propid, &pwszFriendlyName );
        if( S_OK == hr )
        {
            wprintf( L" (\"%s\")\n\n", pwszFriendlyName );
            CoTaskMemFree( pwszFriendlyName );
            pwszFriendlyName = NULL;
        }
        else
            wprintf( L"\n" );

        // Get a property enumerator

        hr = pPropStg->Enum( &penum );
        if( FAILED(hr) ) throw L"Failed IPropertyStorage::Enum";

        // Get the first property in the enumeration

        memset( &statpropstg, 0, sizeof(statpropstg) );
        hr = penum->Next( 1, &statpropstg, NULL );
        while( S_OK == hr )
        {

            // Read the property out of the property set

            PropVariantInit( &propvar );
            propspec.ulKind = PRSPEC_PROPID;
            propspec.propid = statpropstg.propid;

            hr = pPropStg->ReadMultiple( 1, &propspec, &propvar );
            if( FAILED(hr) ) throw L"Failed IPropertyStorage::ReadMultiple";

            // Display the property value, type, etc.

            DumpProperty( propvar, statpropstg );

            // Free buffers that were allocated during the read and
            // by the enumerator.

            PropVariantClear( &propvar );
            CoTaskMemFree( statpropstg.lpwstrName );
            statpropstg.lpwstrName = NULL;

            // Move to the next property in the enumeration

            hr = penum->Next( 1, &statpropstg, NULL );
        }
        if( FAILED(hr) ) throw L"Failed IEnumSTATPROPSTG::Next";
    }
    catch( LPCWSTR pwszErrorMessage )
    {
        wprintf( L"Error in DumpPropertySet: %s (hr = %08x)\n", pwszErrorMessage, hr );
    }

    if( NULL != penum )
        penum->Release();
    if( NULL != statpropstg.lpwstrName )
        CoTaskMemFree( statpropstg.lpwstrName );
    PropVariantClear( &propvar );
}


//+----------------------------------------------------------------------------
//
//  DumpPropertySetsInStorage
//
//  Dump the property sets in the top level of a given storage.
//
//+----------------------------------------------------------------------------

void
DumpPropertySetsInStorage( const WCHAR *pwszStorageName, 
                           IPropertySetStorage *pPropSetStg )
{
    IEnumSTATPROPSETSTG *penum = NULL;
    HRESULT hr = S_OK;
    IPropertyStorage *pPropStg = NULL;
    STATPROPSETSTG statpropsetstg;

    try
    {
        // Get a property-set enumerator (which only enumerates the property
        // sets at this level of the storage, not its children).

        hr = pPropSetStg->Enum( &penum );
        if( FAILED(hr) ) throw L"failed IPropertySetStorage::Enum";

        // Get the first property set in the enumeration.

        memset( &statpropsetstg, 0, sizeof(statpropsetstg) );
        hr = penum->Next( 1, &statpropsetstg, NULL );
        while( S_OK == hr )
        {
            WCHAR wszFMTID[ CCH_MAX_PROPSTG_NAME + 1 ] = { L"" };

            // Open the property set

            hr = pPropSetStg->Open( statpropsetstg.fmtid,
                                    STGM_READ | STGM_SHARE_EXCLUSIVE,
                                    &pPropStg );
            if( FAILED(hr) ) throw L"failed IPropertySetStorage::Open";

            
            // Display the name of the property set

            StringFromGUID2( statpropsetstg.fmtid,
                             wszFMTID,
                             sizeof(wszFMTID)/sizeof(wszFMTID[0]) );
            wprintf( L"\n Property Set %s in %s:\n",
                     wszFMTID, pwszStorageName );

            // Display the properties in the property set

            DumpPropertySet( pPropStg );

            pPropStg->Release();
            pPropStg = NULL;

            // Get the FMTID of the next property set in the enumeration.

            hr = penum->Next( 1, &statpropsetstg, NULL );
        }
        if( FAILED(hr) ) throw L"Failed IEnumSTATPROPSETSTG::Next";
    }
    catch( LPCWSTR pwszErrorMessage )
    {
        wprintf( L"Error in DumpPropertySetsInStorage: %s (hr = %08x)\n",
                 pwszErrorMessage, hr );
    }

    if( NULL != pPropStg )
        pPropStg->Release();
    if( NULL != penum )
        penum->Release();
}


//+----------------------------------------------------------------------------
//
//  DumpStorageTree
//
//  Dump all the property sets in the given storage and recursively in
//  all its children.
//
//+----------------------------------------------------------------------------

void
DumpStorageTree( const WCHAR *pwszStorageName, IStorage *pStg )
{
    IPropertySetStorage *pPropSetStg = NULL;
    IStorage *pStgChild = NULL;
    WCHAR *pwszChildStorageName = NULL;
    IEnumSTATSTG *penum = NULL;
    HRESULT hr = S_OK;
    STATSTG statstg;

    try
    {
        // Dump the property sets at this storage level

        hr = pStg->QueryInterface( IID_IPropertySetStorage, reinterpret_cast<void**>(&pPropSetStg) );
        if( FAILED(hr) ) throw L"Failed IStorage::QueryInterface(IID_IPropertySetStorage)";

        DumpPropertySetsInStorage( pwszStorageName, pPropSetStg );

        // Get an enumerator for this storage.

        hr = pStg->EnumElements( NULL, NULL, NULL, &penum );
        if( FAILED(hr) ) throw L"failed IStorage::Enum";

        // Get the name of the first element (stream/storage) in the enumeration.

        memset( &statstg, 0, sizeof(statstg) );
        hr = penum->Next( 1, &statstg, 0 );
        while( S_OK == hr )
        {
            // Check if this is a storage that isn't a property set
            // (since we already displayed the property sets above)?

            if( STGTY_STORAGE == statstg.type
                &&
                L'\005' != statstg.pwcsName[0] )
            {
                // Yes, this is a normal storage.
                // Open the storage.

                hr = pStg->OpenStorage( statstg.pwcsName,
                                        NULL,
                                        STGM_READ | STGM_SHARE_EXCLUSIVE,
                                        NULL, 0,
                                        &pStgChild );
                if( FAILED(hr) ) throw L"failed IStorage::OpenStorage";

                // Compose a name of the form "Storage\ChildStorage\..."

                pwszChildStorageName = new WCHAR[ wcslen(pwszStorageName)
                                                  +
                                                  wcslen(statstg.pwcsName) + 2 ];
                if( NULL == pwszChildStorageName )
                {
                    hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
                    throw L"couldn't allocate memory";
                }

                wcscpy( pwszChildStorageName, pwszStorageName );
                wcscat( pwszChildStorageName, L"\\" );
                wcscat( pwszChildStorageName, statstg.pwcsName );

                // Dump all the property sets under this child storage.

                DumpStorageTree( pwszChildStorageName, pStgChild );

                pStgChild->Release();
                pStgChild = NULL;

                delete pwszChildStorageName;
                pwszChildStorageName = NULL;
            }

            // Move on to the next element in the enumeration of this storage.

            CoTaskMemFree( statstg.pwcsName );
            statstg.pwcsName = NULL;

            hr = penum->Next( 1, &statstg, 0 );
        }
        if( FAILED(hr) ) throw L"failed IEnumSTATSTG::Next";
    }
    catch( LPCWSTR pwszErrorMessage )
    {
        wprintf( L"Error in DumpStorageTree: %s (hr = %08x)\n",
                 pwszErrorMessage, hr );
    }

    if( NULL != pStgChild )
        pStgChild->Release();
    if( NULL != penum )
        penum->Release();
    if( NULL != pwszChildStorageName )
        delete pwszChildStorageName;
    
}


//+----------------------------------------------------------------------------
//
//  wmain
//
//  Dump all the property sets in a file which is a storage.
//
//+----------------------------------------------------------------------------
def_struct_primitive( showFileproperties, blurUtil, "showFileproperties");

Value* showFileproperties_cf(Value** arg_list, int count)
{
    HRESULT hr = S_OK;
    IStorage *pStg = NULL;
	WStr filename = arg_list[0]->to_string();
    	

    // Display usage information if necessary.

	hr = StgOpenStorageEx( filename,
                           STGM_READ | STGM_SHARE_DENY_WRITE,
                           STGFMT_ANY,
                           0,
                           NULL,
                           NULL,
                           IID_IStorage,
                           reinterpret_cast<void**>(&pStg) );

    // Dump all the properties in all the property sets within this
    // storage.

    if( FAILED(hr) )
    {
        wprintf( L"Error:  couldn't open file %s (hr = %08x)\n",
                 filename, hr );
    }
    else
    {
        printf( "\n" );
        DumpStorageTree( filename, pStg );
    }

    if( NULL != pStg )
        pStg->Release();
	
	return &ok;
}


void testInit()
{
	#include "defimpfn.h"
		def_name ( vp )
		def_name ( missingOnly )
		def_name ( 1stMissing )
		def_name ( dontRecurse )
		def_name ( checkAwork1 )
}