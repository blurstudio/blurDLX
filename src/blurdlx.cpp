//*****************************************************************************
/*
	\file  BlurDLX.cpp

	[TITLE]
	Blur Maxscript Extensions

	[DESCRIPTION]
	Contains the Dll Entry stuff

	[USAGE]

	[CREATION INFO]
	Author:Diego Garcia Huerta
	E-mail:diego@blur.com
	Company:Blur Studios
	Last Updated:July 27, 2007

	Copyright 2007, Blur Studio Inc.

	[KEYWORDS]


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
//*****************************************************************************


#include "MAXScrpt.h"

#pragma comment(lib, "comctl32.lib")

// Initialize Core
extern void blurDLX_init();
extern void ClassDefInit();

// Initialize Controls
extern void ButtonTagInit();
extern void ImgTagInit();
extern void ListBoxPlusCtrlInit();
//extern void MultiListBoxInit();
extern void MultiListBoxPlusCtrlInit();
extern void TreeViewInit();
extern void TreeViewPlusInit();

// Initialize Libraries
extern void BlurBitmapInit();
extern void BlurFileInit();
extern void blurNCInit();
extern void BlurStringInit();
extern void BlurSceneManagerInit();
extern void BlurUtilInit();
extern void BlurXMLInit();

extern void DictionaryInit();
//extern void sendEmailInit();

// Initialize Misc
extern void BBox3ValueInit();
extern void MaxFilePropertiesInit();
extern void PreviewParamsInit();
extern void testInit();
extern void toc_init();

void initialize() {
	// Initialize Core
	ClassDefInit();

	// Initialize Controls
	ButtonTagInit();
	ImgTagInit();
	ListBoxPlusCtrlInit();
	MultiListBoxPlusCtrlInit();
	TreeViewInit();
	TreeViewPlusInit();

	// Initialize Libraries
	BlurBitmapInit();
	blurNCInit();
	BlurStringInit();
	DictionaryInit();
//	sendEmailInit();
	BlurSceneManagerInit();
	BlurUtilInit();
	BlurXMLInit();

	// Initialize Misc
	BBox3ValueInit();
	MaxFilePropertiesInit();
	PreviewParamsInit();
	toc_init();
//	testInit();
}

//extern void BlurRegExInit();

//extern void MaxScriptThreadInit();


HMODULE hInstance = NULL;
HINSTANCE g_hInst;

BOOL APIENTRY
DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(icex);
	icex.dwICC  = ICC_TREEVIEW_CLASSES | ICC_LISTVIEW_CLASSES;

	static BOOL controlsInit = FALSE;
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			// Hang on to this DLL's instance handle.
			hInstance = hModule;
			g_hInst = hModule;
			DisableThreadLibraryCalls(hModule);
			/*if ( !controlsInit )
			{
				controlsInit = TRUE;				
				// Initialize Win95 controls
 				InitCommonControls();
				InitCommonControlsEx(&icex);
				InitCustomControls(hModule);
			}*/
			break;
	}
		
	return(TRUE);
}

__declspec( dllexport ) void LibInit() { 
	initialize();
}


__declspec( dllexport ) const TCHAR *
LibDescription() 
{ 
	return _T("Blur MAXScript Extension"); 
}

__declspec( dllexport ) ULONG
LibVersion() {  return VERSION_3DSMAX; }

