/*!
	\file		MaxFileProperties.cpp

	\remarks	Maxscript extension to have access to max file OLE container properties
	
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

#include "MaxFileProperties.h"
#include "Resource.h"

#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
#include "maxscript\macros\define_external_functions.h"
#else
#include "defextfn.h"
#endif
	def_name ( maxfileproperties )

#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
#include "maxscript\macros\define_instantiation_functions.h"
#else
#include "definsfn.h"
#endif
	def_name ( summaryInfo )
	def_name ( documentSummaryInfo )
	def_name ( userDefinedProperties )
	def_name ( general )
	def_name ( meshTotals )
	def_name ( sceneTotals )
	def_name ( externalDependences )
//	def_name ( objects )
	def_name ( materials )
	def_name ( usedPlugins )
	def_name ( renderData )

//	def_name ( title )
	def_name ( subject )
	def_name ( author )
	def_name ( keywords )
	def_name ( comments )

	def_name ( manager )
	def_name ( company )
//	def_name ( category )
	

#define PROPSET_SUMINFO			0x000000ff
#define PROPSET_DOCSUMINFO		0x0000ff00
#define PROPSET_USERDEF			0x00ff0000

#define ALL_PROPERTIES			0xffffffff	// All props
#define TITLE_PROP				0x00000001	// Summary Info
#define SUBJECT_PROP			0x00000002	// Summary Info
#define AUTHOR_PROP				0x00000004	// Summary Info
#define KEYWORDS_PROP			0x00000008	// Summary Info
#define COMMENTS_PROP			0x00000010	// Summary Info
#define MANAGER_PROP			0x00000100	// Document Summary Info
#define COMPANY_PROP			0x00000200	// Document Summary Info
#define CATEGORY_PROP			0x00000400	// Document Summary Info
#define EXT_DEPEND_PROP			0x00000800	// Document Summary Info
#define PLUGINS_PROP			0x00001000	// Document Summary Info
#define OBJECTS_PROP			0x00002000	// Document Summary Info
#define MATERIALS_PROP			0x00004000	// Document Summary Info
#define USER_PROP				0x00010000	// User Defined Properties

#define PID_TITLE				0x00000002
#define PID_SUBJECT				0x00000003
#define PID_AUTHOR				0x00000004
#define PID_KEYWORDS			0x00000005
#define PID_COMMENTS			0x00000006

#define PID_MANAGER				0x0000000E
#define PID_COMPANY				0x0000000F
#define PID_CATEGORY			0x00000002
#define PID_HEADINGPAIR			0x0000000C
#define PID_DOCPARTS			0x0000000D

TSTR TypeNameFromVariant(PROPVARIANT* pProp)
{
	switch (pProp->vt) {
		case VT_LPWSTR:
		case VT_LPSTR:
			return GetString(IDS_TYPE_TEXT);
		case VT_I4:
		case VT_R4:
		case VT_R8:
			return GetString(IDS_TYPE_NUMBER);
		case VT_BOOL:
			return GetString(IDS_TYPE_BOOL);
			break;
		case VT_FILETIME:
			return GetString(IDS_TYPE_DATE);
			break;
		default:
			break;
	}
	return TSTR();
}

TSTR VariantToString(PROPVARIANT* pProp)
{
	switch (pProp->vt) {
		case VT_LPWSTR:
			return TSTR::FromUTF16(pProp->pwszVal);
		case VT_LPSTR:
			return TSTR::FromACP(pProp->pszVal);
		case VT_I4:
		{
			TSTR ret;
			ret.printf( _T("%ld"), pProp->lVal );
			return ret;
		}
		case VT_R4:
		{
			TSTR ret;
			ret.printf( _T("%f"), pProp->fltVal );
			return ret;
		}
		case VT_R8:
		{
			TSTR ret;
			ret.printf( _T("%lf"), pProp->dblVal );
			return ret;
		}
		case VT_BOOL:
		{
			TSTR ret;
			ret.printf( _T("%s"), pProp->boolVal ? GetString(IDS_VAL_YES) : GetString(IDS_VAL_NO) );
			return ret;
		}
		case VT_FILETIME:
		{
			TSTR ret;
			SYSTEMTIME sysTime;
			FileTimeToSystemTime(&pProp->filetime, &sysTime);
			GetDateFormat(LOCALE_SYSTEM_DEFAULT,
							DATE_SHORTDATE,
							&sysTime,
							NULL,
							ret.dataForWrite(128),
							128);
			return ret;
		}
		default:
			break;
	}
	return TSTR();
}

// ============================================================================

visible_class_instance(MaxFileProperties, "MaxFileProperties");

// ============================================================================
MaxFileProperties::MaxFileProperties()
{
	m_summaryInfo			= new Array(0);
	m_documentSummaryInfo	= new Array(0);
	m_general				= new Array(0);
	m_meshTotals			= new Array(0);
	m_sceneTotals			= new Array(0);
	m_externalDependences	= new Array(0);
	m_objects				= new Array(0);
	m_materials				= new Array(0);
	m_usedPlugins			= new Array(0);
	m_renderData			= new Array(0);
	m_userDefinedProperties = new Array(0);
}

// ============================================================================
void MaxFileProperties::sprin1(CharStream* s)
{
	s->printf(_T("<MaxFileProperties:%s>"), (const TCHAR*)m_filename);
}


// ============================================================================
void MaxFileProperties::gc_trace()
{
	Value::gc_trace();

	if (m_summaryInfo && m_summaryInfo->is_not_marked())
		m_summaryInfo->gc_trace();
	if (m_documentSummaryInfo && m_documentSummaryInfo->is_not_marked())
		m_documentSummaryInfo->gc_trace();
	if (m_general && m_general->is_not_marked())
		m_general->gc_trace();
	if (m_meshTotals && m_meshTotals->is_not_marked())
		m_meshTotals->gc_trace();
	if (m_sceneTotals && m_sceneTotals->is_not_marked())
		m_sceneTotals->gc_trace();
	if (m_externalDependences && m_externalDependences->is_not_marked())
		m_externalDependences->gc_trace();
	if (m_objects && m_objects->is_not_marked())
		m_objects->gc_trace();
	if (m_materials && m_materials->is_not_marked())
		m_materials->gc_trace();
	if (m_usedPlugins && m_usedPlugins->is_not_marked())
		m_usedPlugins->gc_trace();
	if (m_renderData && m_renderData->is_not_marked())
		m_renderData->gc_trace();

}

Value*	MaxFileProperties::set_property(Value** arg_list, int count)
{
	Value* val = arg_list[0];
	Value* prop = arg_list[1];

	return &undefined;
}

Value* MaxFileProperties::get_property(Value** arg_list, int count)
{
	Value* prop = arg_list[0];

	if (prop == n_filename)
		return new String(m_filename);
	else if (prop == n_summaryInfo)
		return m_summaryInfo;
	else if (prop == n_documentSummaryInfo)
		return m_documentSummaryInfo;
	else if (prop == n_userDefinedProperties)
		return m_userDefinedProperties;
	else if (prop == n_general)
		return m_general;
	else if (prop == n_meshTotals)
		return m_meshTotals;
	else if (prop == n_sceneTotals)
		return m_sceneTotals;
	else if (prop == n_externalDependences)
		return m_externalDependences;
	else if (prop == n_objects)
		return m_objects;
	else if (prop == n_materials)
		return m_materials;
	else if (prop == n_usedPlugins)
		return m_usedPlugins;
	else if (prop == n_renderData)
		return m_renderData;

	else if (prop == n_title)
		return new String(m_title);
	else if (prop == n_subject)
		return new String(m_subject);
	else if (prop == n_author)
		return new String(m_author);
	else if (prop == n_keywords)
		return new String(m_keywords);
	else if (prop == n_comments)
		return new String(m_comments);

	else if (prop == n_manager)
		return new String(m_manager);
	else if (prop == n_company)
		return new String(m_company);
	else if (prop == n_category)
		return new String(m_category);

	return &undefined;
}

Value* MaxFileProperties::show_props_vf(Value** arg_list, int count)
{
	return &ok;
}


Value* MaxFileProperties::get_props_vf(Value** arg_list, int count)
{
	Array* propNames = new Array(0);
	propNames->append(n_summaryInfo);
	propNames->append(n_title);
	propNames->append(n_subject);
	propNames->append(n_author);
	propNames->append(n_keywords);
	propNames->append(n_comments);
	propNames->append(n_documentSummaryInfo);
	propNames->append(n_manager);
	propNames->append(n_company);
	propNames->append(n_category);
	propNames->append(n_general);
	propNames->append(n_meshTotals);
	propNames->append(n_sceneTotals);
	propNames->append(n_externalDependences);
	propNames->append(n_objects);
	propNames->append(n_materials);
	propNames->append(n_usedPlugins);
	propNames->append(n_renderData);
	propNames->append(n_userDefinedProperties);

	return propNames;
}

static TSTR tstrFromPropVar( const PROPVARIANT & propVar )
{
	if( propVar.vt == VT_LPSTR )
		return TSTR::FromACP(propVar.pszVal);
	return TSTR();
}

//getmaxfileproperties "C:/temp/temp.max"
def_struct_primitive( getmaxfileproperties, blurUtil, "getmaxfileproperties");

Value* getmaxfileproperties_cf(Value** arg_list, int count)
{
	MaxFileProperties *mProps = new MaxFileProperties();

	LPSTORAGE				pStorage = NULL;
	IPropertySetStorage*	pPropertySetStorage = NULL;
	IPropertyStorage*		pSummaryInfoStorage = NULL;
	IPropertyStorage*		pDocumentSummaryInfoStorage = NULL;
	IPropertyStorage*		pUserDefinedPropertyStorage = NULL;
	const TCHAR*			filename = arg_list[0]->to_string();
	mProps->m_filename = filename;

#ifdef UNICODE
#define wfilename filename
#else
	wchar_t wfilename[_MAX_PATH];
	MultiByteToWideChar(CP_ACP, 0, filename, -1, wfilename, _MAX_PATH);
#endif

	HRESULT	res = StgOpenStorage(wfilename, (LPSTORAGE)0, STGM_DIRECT|STGM_READ|STGM_SHARE_EXCLUSIVE,	NULL,0,&pStorage);
#undef wfilename
	if (res!=S_OK)
	{
		return mProps;
	}

	// Get the Storage interface
	if (S_OK != pStorage->QueryInterface(IID_IPropertySetStorage, (void**)&pPropertySetStorage)) 
	{
		pStorage->Release();
		return mProps;
	}

	// Get the SummaryInfo property set interface
	if (S_OK == pPropertySetStorage->Open(FMTID_SummaryInformation, STGM_READ|STGM_SHARE_EXCLUSIVE, &pSummaryInfoStorage)) 
	{
		BOOL bFound = FALSE;

		PROPSPEC	PropSpec[5];
		PROPVARIANT	PropVar[5];

		PropSpec[0].ulKind = PRSPEC_PROPID;
		PropSpec[0].propid = PID_TITLE;

		PropSpec[1].ulKind = PRSPEC_PROPID;
		PropSpec[1].propid = PID_SUBJECT;

		PropSpec[2].ulKind = PRSPEC_PROPID;
		PropSpec[2].propid = PID_AUTHOR;

		PropSpec[3].ulKind = PRSPEC_PROPID;
		PropSpec[3].propid = PID_KEYWORDS;

		PropSpec[4].ulKind = PRSPEC_PROPID;
		PropSpec[4].propid = PID_COMMENTS;

		HRESULT hr = pSummaryInfoStorage->ReadMultiple(5, PropSpec, PropVar);
		
		if (S_OK == hr)
		{
			TSTR * dests[5] = { &mProps->m_title, &mProps->m_subject, &mProps->m_author, &mProps->m_keywords, &mProps->m_comments };
			for( int i=0; i < 5; ++i ) {
				TSTR str = tstrFromPropVar(PropVar[i]);
				mProps->m_summaryInfo->append(new String(str));
				*dests[i] = str;
			}
		}
		
		FreePropVariantArray(5, PropVar);
		pSummaryInfoStorage->Release();
	}

	// Get the DocumentSummaryInfo property set interface
	if (S_OK == pPropertySetStorage->Open(FMTID_DocSummaryInformation, STGM_READ|STGM_SHARE_EXCLUSIVE, &pDocumentSummaryInfoStorage)) 
	{
		BOOL bFound = FALSE;

		PROPSPEC	PropSpec[5];
		PROPVARIANT	PropVar[5];

		PropSpec[0].ulKind = PRSPEC_PROPID;
		PropSpec[0].propid = PID_MANAGER;

		PropSpec[1].ulKind = PRSPEC_PROPID;
		PropSpec[1].propid = PID_COMPANY;

		PropSpec[2].ulKind = PRSPEC_PROPID;
		PropSpec[2].propid = PID_CATEGORY;

		PropSpec[3].ulKind = PRSPEC_PROPID;
		PropSpec[3].propid = PID_HEADINGPAIR;

		PropSpec[4].ulKind = PRSPEC_PROPID;
		PropSpec[4].propid = PID_DOCPARTS;
		

		HRESULT hr = pDocumentSummaryInfoStorage->ReadMultiple(5, PropSpec, PropVar);
		if (S_OK == hr) 
		{
			for( int i=0; i<3; ++i ) {
				TSTR str = tstrFromPropVar(PropVar[i]);
				mProps->m_documentSummaryInfo->append(new String(str));
				switch(i) {
					case 0:
						mProps->m_manager = str;
						break;
					case 1:
						mProps->m_company = str;
						break;
					case 2:
						mProps->m_category = str;
						break;
				}
			}

			if ((PropVar[3].vt == (VT_VARIANT | VT_VECTOR)) && (PropVar[4].vt == (VT_LPSTR | VT_VECTOR)))
			{
				CAPROPVARIANT*	pHeading = &PropVar[3].capropvar;
				CALPSTR*		pDocPart = &PropVar[4].calpstr;

				// Headings:
				// =========
				// 0  - General
				// 2  - Mesh Totals
				// 4  - Scene Totals
				// 6  - External Dependencies
				// 8  - Objects
				// 10 - Materials
				// 12 - Plug-Ins
				// 14 - Render Data

				int nDocPart = 0;
				for (UINT i=0; i<pHeading->cElems; i+=2)
				{
					for (int j=0; j<pHeading->pElems[i+1].lVal; j++)
					{
						TSTR str = TSTR::FromACP(pDocPart->pElems[nDocPart]);
						Value * propVar = new String(str);
						switch ( i )
						{
							case 0: // m_general
								mProps->m_general->append(propVar);
								break;
							case 2: // m_meshTotals
								mProps->m_meshTotals->append(propVar);
								break;
							case 4: // m_sceneTotals
								mProps->m_sceneTotals->append(propVar);
								break;
							case 6: // m_externalDependences
								mProps->m_externalDependences->append(propVar);
								break;
							case 8: // m_objects
								mProps->m_objects->append(propVar);
								break;
							case 10: // m_materials
								mProps->m_materials->append(propVar);
								break;
							case 12: // m_usedPlugins
								mProps->m_usedPlugins->append(propVar);
								break;
							case 14: // m_renderData
								mProps->m_renderData->append(propVar);
								break;
						}
						nDocPart++;
					}
				}
			}
		}
		
		mProps->m_documentSummaryInfo->append(mProps->m_general);
		mProps->m_documentSummaryInfo->append(mProps->m_meshTotals);
		mProps->m_documentSummaryInfo->append(mProps->m_sceneTotals);
		mProps->m_documentSummaryInfo->append(mProps->m_externalDependences);
		mProps->m_documentSummaryInfo->append(mProps->m_objects);
		mProps->m_documentSummaryInfo->append(mProps->m_materials);
		mProps->m_documentSummaryInfo->append(mProps->m_usedPlugins);
		mProps->m_documentSummaryInfo->append(mProps->m_renderData);

		FreePropVariantArray(5, PropVar);
		pDocumentSummaryInfoStorage->Release();
	}

	if (S_OK == pPropertySetStorage->Open(FMTID_UserDefinedProperties, STGM_READ|STGM_SHARE_EXCLUSIVE, &pUserDefinedPropertyStorage)) 
	{
		int numUserProps = 0;

		// First we need to count the properties
		IEnumSTATPROPSTG*	pIPropertyEnum;
		if (S_OK == pUserDefinedPropertyStorage->Enum(&pIPropertyEnum))
		{
			STATPROPSTG property;
			while (pIPropertyEnum->Next(1, &property, NULL) == S_OK)
			{
				if (property.lpwstrName)
				{
					CoTaskMemFree(property.lpwstrName);
					property.lpwstrName = NULL;
					numUserProps++;
				}
			}

			PROPSPEC* pPropSpec = new PROPSPEC[numUserProps];
			PROPVARIANT* pPropVar = new PROPVARIANT[numUserProps];

			ZeroMemory(pPropVar, numUserProps*sizeof(PROPVARIANT));
			ZeroMemory(pPropSpec, numUserProps*sizeof(PROPSPEC));

			pIPropertyEnum->Reset();
			int idx = 0;
			while (pIPropertyEnum->Next(1, &property, NULL) == S_OK)
			{
				if (property.lpwstrName)
				{
					pPropSpec[idx].ulKind = PRSPEC_LPWSTR;
					pPropSpec[idx].lpwstr = (LPWSTR)CoTaskMemAlloc(sizeof(wchar_t)*(wcslen(property.lpwstrName)+1));
					wcscpy(pPropSpec[idx].lpwstr, property.lpwstrName);
					idx++;
					CoTaskMemFree(property.lpwstrName);
					property.lpwstrName = NULL;
				}
			}
			pIPropertyEnum->Release();

			HRESULT hr = pUserDefinedPropertyStorage->ReadMultiple(idx, pPropSpec, pPropVar);
			if (S_OK == hr)
			{
				for (int i=0; i<idx; i++) 
				{
					Array* newProp = new Array(0);
					newProp->append(new String(TSTR::FromUTF16(pPropSpec[i].lpwstr)));
					newProp->append(new String(VariantToString(&pPropVar[i])));
					newProp->append(new String(TypeNameFromVariant(&pPropVar[i])));
					mProps->m_userDefinedProperties->append(newProp);
				}
			}

			for (int i=0; i<idx; i++)
			{
				CoTaskMemFree(pPropSpec[i].lpwstr);
			}

			FreePropVariantArray(numUserProps, pPropVar);

			delete [] pPropSpec;
			delete [] pPropVar;
		}

		pUserDefinedPropertyStorage->Release();
	}

	pPropertySetStorage->Release();
	pStorage->Release();

	return mProps;
}

void MaxFilePropertiesInit()
{
#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
	#include "maxscript\macros\define_implementations.h"
#else
	#include "defimpfn.h"
#endif
		def_name ( summaryInfo )
		def_name ( documentSummaryInfo )
		def_name ( userDefinedProperties )
		def_name ( general )
		def_name ( meshTotals )
		def_name ( sceneTotals )
		def_name ( externalDependences )
	//	def_name ( objects )
		def_name ( materials )
		def_name ( usedPlugins )
		def_name ( renderData )

	//	def_name ( title )
		def_name ( subject )
		def_name ( author )
		def_name ( keywords )
		def_name ( comments )

		def_name ( manager )
		def_name ( company )
	//	def_name ( category )


}