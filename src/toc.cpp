/*!
	\file		toc.cpp

	\remarks	Maxscript extensions to export a table of content of the current max file opened
	
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


#include "atlbase.h"
#include "msxml2.h"
#include <time.h>

#ifdef __MAXSCRIPT_2012__
#include "maxscript\maxscript.h"
#include "maxscript\maxwrapper\mxsobjects.h"
#include "maxscript\maxwrapper\maxclasses.h"
#include "maxscript\foundation\strings.h"
#include "CoreFunctions.h"
#include "units.h"
#else
#include "MAXScrpt.h"
#include "MAXObj.h"
#include "MAXclses.h"
#include "strings.h"
#endif

// The three classes for managing layers in 3DSMax
#include "ilayermanager.h"		// Contains a pure virtual class. 
#include "iLayer.h"				// Contains a pure virtual class.
#include "ILayerProperties.h"	// Contains two function published class's that 
								// are the core for the maxscript exposure
#include "modstack.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "Resource.h"

#ifdef __MAXSCRIPT_2012__
#include "maxscript\macros\define_external_functions.h"
#include "maxscript\macros\define_instantiation_functions.h"
#else
#include "defextfn.h"
#include "definsfn.h"
#endif

#define EXPORT_PROPERTY_VALUES FALSE

void FPValueToUIString( FPValue* val, TCHAR* str ) {
	switch( val->type ) {
	case TYPE_WORLD:		_stprintf( str, "%s", FormatUniverseValue(val->f) ); break;
	case TYPE_FLOAT:		_stprintf( str, "%f", val->f );				break;
	case TYPE_INT:			_stprintf( str, "%i", val->i );				break;
	case TYPE_STRING:		_stprintf( str, "%s", val->s );				break;
	case TYPE_TSTR_BV:
	case TYPE_TSTR:			_stprintf( str, "%s", val->tstr->data() );	break;
	case TYPE_BOOL:
		_tcscpy( str, (val->i? _T("True") : _T("False")) );
		break;
	default:
		str[0] = 0;
		break;
	}
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	xml funcs
// -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

bool CreateXMLNode(IXMLDOMDocument * doc, IXMLDOMNode * node, TCHAR *nodeName, IXMLDOMNode ** newNode)
{
	IXMLDOMNode * sceneNode;
	doc->createNode(CComVariant(NODE_ELEMENT), CComBSTR(nodeName), NULL, &sceneNode);
	node->appendChild(sceneNode,newNode);
	return true;
}

bool AddXMLAttribute(IXMLDOMNode * node, TCHAR * name, TCHAR * value)
{
	CComQIPtr<IXMLDOMElement> element;
	element = node;
	element->setAttribute(CComBSTR(name), CComVariant(value));
	return true;
}

bool AddXMLText(IXMLDOMDocument * doc, IXMLDOMNode * node, TCHAR * text)
{
	CComPtr <IXMLDOMText> keydata = NULL;
	doc->createTextNode(CComBSTR(text), &keydata);
	node->appendChild(keydata,NULL);
	return true;

}
IXMLDOMText* CreateXMLText(IXMLDOMDocument* doc, IXMLDOMNode* parentNode, TCHAR* name)
{
	CComPtr<IXMLDOMNode> node;
	IXMLDOMText* textNode = NULL;

	if( CreateXMLNode(doc,parentNode, name,&node) == true) {
		doc->createTextNode(CComBSTR(""), &textNode);
		node->appendChild(textNode,NULL);
	}

	return textNode;
}

void PrettyPrint(const TCHAR* name, IXMLDOMDocument* pXMLDoc)
{
	// perform formatting XSLT transform to get indented XML output
	IXMLDOMDocument* pXSLDoc;
	BSTR outputXML;
	HRESULT hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,  IID_IXMLDOMDocument, (void**)&pXSLDoc);
	if (SUCCEEDED(hr)) {
		// load indenting XSL doc 
		VARIANT_BOOL result;
		CComBSTR indentXSL(
			"<xsl:stylesheet version=\"1.0\""
			"      xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\">"
			"   <xsl:output method=\"xml\" encoding=\"UTF-8\" />"
			"   <xsl:param name=\"indent-increment\" select=\"'\t'\" />"
			"    <xsl:template match=\"@*|node()\">"
			"      <xsl:copy>"
			"        <xsl:apply-templates select=\"@*|node()\"/>"
			"      </xsl:copy>"
			"    </xsl:template>"

		/*	"   <xsl:template match=\"node()\">"
			"      <xsl:param name=\"indent\" select=\"'&#xA;'\"/>"
			"      <xsl:value-of select=\"$indent\"/>"
			"      <xsl:copy>"
			"        <xsl:copy-of select=\"@*\" />"
			"        <xsl:apply-templates>"
			"          <xsl:with-param name=\"indent\""
			"               select=\"concat($indent, $indent-increment)\"/>" // it was making bigger and bigger with blank spaces the docs if we load/save them!
			"        </xsl:apply-templates>"
			"        <xsl:if test=\"node()\">"
			"          <xsl:value-of select=\"$indent\"/>"
			"        </xsl:if>"
			"      </xsl:copy>"
			"   </xsl:template>"*/

//			"   <xsl:template match=\"comment()|processing-instruction()\">"
//			"      <xsl:copy />"
//			"   </xsl:template>"
//			"   <!-- WARNING: this is dangerous. Handle with care -->"
//			"   <xsl:template match=\"text()[normalize-space(.)='']\"/>"
			"</xsl:stylesheet>"
			);
		hr = pXSLDoc->loadXML(indentXSL, &result);
		if (SUCCEEDED(hr)) {
			// perform transform
			hr = pXMLDoc->transformNode(pXSLDoc, &outputXML);
		}
	}

	// output transformed XML if previous sequence succeeded, else normal XMLDoc save
	if (SUCCEEDED(hr)) {
		FILE* out = _tfopen(name, _T("w"));
		if (out != NULL) {
			// hack the UTF-16 back to UTF-8 (there probably is a way to mod the stylesheet to do this)
			wchar_t* enc = wcsstr(outputXML, L"\"UTF-16\"");
			if (enc != NULL) memcpy(enc, L"\"utf-8\" ", 8 * sizeof(wchar_t));
			// convert BSTR to MBCS for output
			int len = WideCharToMultiByte(CP_ACP, 0, outputXML, -1, 0, 0, NULL, NULL);
			TCHAR* buf = (TCHAR*)malloc(len * sizeof(WCHAR));
			WideCharToMultiByte(CP_ACP, 0, outputXML, -1, buf, len, NULL, NULL);
			// write the XML
			_fputts(buf, out);   
			fclose(out);
			free(buf);
		}
		SysFreeString(outputXML);
	}
	else
		// save the XML graph out to the export file
		pXMLDoc->save(CComVariant(name));
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	toc funcs
// -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//user info
void ExportUserInfo ( IXMLDOMDocument* pXMLDoc, IXMLDOMNode* parentNode )
{
	CComPtr<IXMLDOMNode> tempNode;

	// time
	struct tm *newtime;
	time_t aclock;

	time( &aclock );
	newtime = localtime(&aclock);

	// date
	TSTR today = _tasctime(newtime);	// The date string has a \n appended.
	today.remove(today.length()-1);		// Remove the \n


	TCHAR username[MAX_PATH];
	DWORD namesize = MAX_PATH;
	GetUserName(username,&namesize);

	TCHAR computername[MAX_COMPUTERNAME_LENGTH+1];
	namesize = MAX_COMPUTERNAME_LENGTH+1;
	GetComputerName(computername,&namesize);

	TCHAR maxversion[MAX_PATH];
	sprintf(maxversion,"%d",Get3DSMAXVersion());

	tempNode = NULL;
	CreateXMLNode(pXMLDoc, parentNode, _T("user"), &tempNode);
	AddXMLText(pXMLDoc, tempNode, username);
	
	tempNode = NULL;
	CreateXMLNode(pXMLDoc, parentNode, _T("computer"), &tempNode);
	AddXMLText(pXMLDoc, tempNode, computername);

	tempNode = NULL;
	CreateXMLNode(pXMLDoc, parentNode, _T("localtime"), &tempNode);
	AddXMLText(pXMLDoc, tempNode, today.data());

	tempNode = NULL;
	CreateXMLNode(pXMLDoc, parentNode, _T("maxversion"), &tempNode);
	AddXMLText(pXMLDoc, tempNode, maxversion);

	tempNode = NULL;
}

void ExportClassInfo ( IXMLDOMDocument* pXMLDoc, IXMLDOMNode* parentNode , Animatable* pAnim )
{
	CComPtr<IXMLDOMNode> tempNode;
	TSTR buf;

	// object class and superclass
	Class_ID cid = pAnim->ClassID();
	SClass_ID sid = pAnim->SuperClassID();
	MAXClass* cls = lookup_MAXClass(&cid, sid);
	MAXSuperClass* scls = lookup_MAXSuperClass(sid);
	StringStream* clsStr = new StringStream(_T(""));
	StringStream* sclsStr = new StringStream(_T(""));
	cls->sprint(clsStr);
	scls->sprint(sclsStr);

	buf.printf( "%s", clsStr->to_string() );

	tempNode = NULL;
	CreateXMLNode(pXMLDoc, parentNode, _T("classof"), &tempNode);
	AddXMLText(pXMLDoc, tempNode, buf );
	
	buf.printf( "%s", sclsStr->to_string() );
	
	tempNode = NULL;
	CreateXMLNode(pXMLDoc, parentNode, _T("superclassof"), &tempNode);
	AddXMLText(pXMLDoc, tempNode, buf );

	tempNode = NULL;
}

void ExportPropertiesInfo ( IXMLDOMDocument* pXMLDoc, IXMLDOMNode* parentNode , Animatable *anim, BOOL showPropertyValues = FALSE )
{
	CComPtr<IXMLDOMNode> tempNode;
	CComPtr<IXMLDOMNode> propsNode;
	TSTR buf;

	Class_ID cid = anim->ClassID();
	SClass_ID sid = anim->SuperClassID();
	MAXClass* cls = lookup_MAXClass(&cid, sid);

	Value* item;

	if (cls == &inode_object)
	{
		item = MAXNode::intern((INode*)anim);
	}
	else if (cls->superclass->superclass == &node_class)
		item = MAXObject::intern((Object*)anim);
	else
		item = (*cls->superclass->maker)(cls, (ReferenceTarget*)anim, NULL, 0);

	propsNode = NULL;
	CreateXMLNode(pXMLDoc, parentNode, _T("properties"), &propsNode);

	if (item->derives_from_MAXWrapper())
	{
		MAXWrapper* itemWrapper = (MAXWrapper*)item;
		Value* props = itemWrapper->get_props_vf(NULL,0);
		int propsCount = 0;

		if (props != &undefined)
		{
			Array* propsArray = (Array*)props;

			StringStream* propNamesStr = new StringStream( _T("#()") );
			propsArray->sprint( propNamesStr );
			buf.printf ("%s", propNamesStr->to_string() );
			AddXMLAttribute( propsNode, _T("names"), buf.data() );
			propsCount = propsArray->size;

			if (showPropertyValues)
			{
				propsCount = 0;
				for ( int  i = 0; i < propsArray->size; ++i )
				{
					Value** args;
					value_local_array(args, 1);
					Value* propName = propsArray->data[i];

					if (propName != &undefined)
					{
						args[0] = propsArray->data[i];
						Value* propValue;

						StringStream* propNameStr = new StringStream( propName );
						buf.printf ("%s", propNameStr->to_string() );

						// properties that start with '_' we don't process them, skin modifier props like that give error when accesing them
						if ( buf[0] == _T('_') )
							propValue = &unsupplied;
						else
							propValue = itemWrapper->get_property(args, 1);

						tempNode = NULL;
						CreateXMLNode(pXMLDoc, propsNode, buf.data(), &tempNode);

						StringStream* propTagNameStr = new StringStream( _T("")  );
						propValue->tag->sprint(propTagNameStr);
						AddXMLAttribute( tempNode, _T("type"), propTagNameStr->to_string() );

						StringStream* propValueStr = new StringStream( _T("") );
						propValue->sprint(propValueStr);
						buf.printf( "%s", propValueStr->to_string() );
						
						AddXMLText(pXMLDoc, tempNode, buf.data() );
						propsCount++;
					}

					pop_value_local_array(args);
				}
			}
			
		}

		buf.printf( "%d", propsCount );
		AddXMLAttribute( propsNode, _T("count"), buf );
	}
	else
	{
		AddXMLAttribute( propsNode, _T("names"), _T("#()") );
	}


	tempNode = NULL;
	propsNode = NULL;
}

void ExportLayersInfo ( IXMLDOMDocument* pXMLDoc, IXMLDOMNode* parentNode )
{
	CComPtr<IXMLDOMNode> layersRootNode;
	CComPtr<IXMLDOMNode> tempNode;
	
	TSTR buf;

	//ILayerManager* lm = (ILayerManager*) GetCOREInterface()->GetScenePointer()->GetReference(10);
	IFPLayerManager* lm = static_cast<IFPLayerManager *>(GetCOREInterface(LAYERMANAGER_INTERFACE));
	
	int layerCount = lm->getCount();
	buf.printf("%d",layerCount);

	layersRootNode = NULL;
	CreateXMLNode(pXMLDoc, parentNode, _T("Layers"), &layersRootNode);
	AddXMLAttribute(layersRootNode, _T("count"), buf );
	
	for (int i = 0; i < layerCount; i++)
	{
		//Get a pointer to the layer
		ILayerProperties* hLayer = lm->getLayer(i);

		CComPtr<IXMLDOMNode> layer;
		layer = NULL;
		CreateXMLNode( pXMLDoc, layersRootNode, _T("Layer"), &layer );
		
		// layer name
		buf.printf( "%s", hLayer->getName() );
		AddXMLAttribute( layer, _T("name"), buf );
		
		// layer index
		buf.printf( "%d", i );
		AddXMLAttribute( layer, _T("index"), buf );

		//names of the objects on each layer
		Tab<INode*> objs; 
		hLayer->Nodes(objs);
					
		TSTR objNames = _T("#(");
		for (int j = 0; j < objs.Count(); j++)
		{
			INode* hObj = objs[j];
			TSTR hObjName = hObj->GetName();
			objNames += _T("$'");
			objNames += hObjName;
			objNames += _T("'");

			if (j < objs.Count()-1)
				objNames += _T(",");
		}
		objNames += _T(")");
		
		tempNode = NULL;
		CreateXMLNode(pXMLDoc, layer, _T("objects"), &tempNode);
		AddXMLText(pXMLDoc, tempNode, objNames.data() );

		buf.printf("%s",hLayer->getCurrent()?"true":"false" );
		tempNode = NULL;
		CreateXMLNode(pXMLDoc, layer, _T("current"), &tempNode);
		AddXMLText(pXMLDoc, tempNode, buf );

		buf.printf("%s",hLayer->getOn()?"true":"false" );
		tempNode = NULL;
		CreateXMLNode(pXMLDoc, layer, _T("on"), &tempNode);
		AddXMLText(pXMLDoc, tempNode, buf );

		buf.printf("%s",hLayer->getLock()?"true":"false" );
		tempNode = NULL;
		CreateXMLNode(pXMLDoc, layer, _T("lock"), &tempNode);
		AddXMLText(pXMLDoc, tempNode, buf );

		Color hcolor = hLayer->getWireColor();
		buf.printf("( color %f %f %f )",hcolor.r, hcolor.g, hcolor.b );
		tempNode = NULL;
		CreateXMLNode(pXMLDoc, layer, _T("wirecolor"), &tempNode);
		AddXMLText(pXMLDoc, tempNode, buf );

		buf.printf("%s",hLayer->getGIIsExcluded()?"true":"false" );
		tempNode = NULL;
		CreateXMLNode(pXMLDoc, layer, _T("isGIExcluded"), &tempNode);
		AddXMLText(pXMLDoc, tempNode, buf );
		// layer properties
	/*	FPInterfaceDesc* layerIntDesc = hLayer->GetDesc();
		for (int j = 0; j < layerIntDesc->functions.Count(); j++)
		{
			FPFunctionDef* fn = layerIntDesc->functions[j];
			buf.printf( "%s", fn->internal_name );
			buf.printf( "%s", fn->description );
			
		}
		for (int j = 0; j < layerIntDesc->props.Count(); j++)
		{
			FPPropDef* layerProp = layerIntDesc->props[j];
			FunctionID fid = layerProp->getter_ID;
			FPFunctionDef* fn = layerIntDesc->GetFnDef(fid);
            
			buf.printf( "%s", layerProp->internal_name );
			
			tempNode = NULL;
			CreateXMLNode(pXMLDoc, layer, buf, &tempNode);
			
			FPValue result;
			layerIntDesc->Invoke(fid, result);
			
			Value* layerPropValue = InterfaceFunction::FPValue_to_val( result );

			StringStream* layerPropValueStr = new StringStream( _T("") );
			layerPropValue->sprint( layerPropValueStr );
			buf.printf( "%s", layerPropValueStr );

			AddXMLText(pXMLDoc, tempNode, buf );
		}*/
	}

	layersRootNode = NULL;
	tempNode = NULL;
}

void ExportModifiersInfo ( IXMLDOMDocument* pXMLDoc, IXMLDOMNode* parentNode, INode* pNode )
{
	CComPtr<IXMLDOMNode> modsRootNode ;
	CComPtr<IXMLDOMNode> tempNode;

	TSTR buf;

	modsRootNode = NULL;
	CreateXMLNode(pXMLDoc, parentNode, _T("modifiers"), &modsRootNode);
	
	Object* obj = pNode->GetObjectRef();
	while (obj && (obj->SuperClassID() == GEN_DERIVOB_CLASS_ID)) 
	{
	    IDerivedObject* dobj = (IDerivedObject*)obj;
	    int numMods = dobj->NumModifiers();

		buf.printf( "%d", numMods );
		AddXMLAttribute(modsRootNode, _T("count"), buf.data() );

		for (int i = 0; i < numMods; i++) 
		{
			TSTR modName;
			Modifier *m = dobj->GetModifier(i);
			buf.printf( "%s", m->GetName() );

			tempNode = NULL;
			CreateXMLNode(pXMLDoc, modsRootNode, _T("modifier"), &tempNode);
			AddXMLAttribute(tempNode, _T("name"), buf.data() );

			ExportClassInfo ( pXMLDoc, tempNode, (Animatable*)m );
			ExportPropertiesInfo( pXMLDoc, tempNode, (Animatable*)m , EXPORT_PROPERTY_VALUES );
		}
		obj = dobj->GetObjRef();
	}	

	modsRootNode = NULL;
	tempNode = NULL;
}



void ExportNodeInfo ( IXMLDOMDocument* pXMLDoc, IXMLDOMNode* parentNode , INode* pNode )
{
	CComPtr<IXMLDOMNode> objectNode;
	CComPtr<IXMLDOMNode> tempNode;

	TSTR buf;
	Interface * ip = GetCOREInterface();
	TimeValue t = ip->GetTime();
	const ObjectState& os = pNode->EvalWorldState(t);
	
	CreateXMLNode(pXMLDoc, parentNode, _T("Object"), &objectNode);

	// object name
	buf.printf( "%s", pNode->GetName() );
	AddXMLAttribute( objectNode, _T("name"), buf );

	// object class and superclass
	ExportClassInfo ( pXMLDoc, objectNode , (Animatable*)pNode );

	// transform
	TSTR transformStr = _T("( matrix3 ");

	Matrix3 nodeTM = pNode->GetNodeTM(t);
	for (int i = 0; i < 4; i++)
	{
		TSTR rowStr = _T("");
		Point3 row = nodeTM.GetRow(i);

		rowStr.printf("[%f, %f, %f] ", row.x, row.y, row.z);
		transformStr += rowStr;
	}

	buf.printf( "%s", pNode->GetName() );
	transformStr += _T(" )");

	tempNode = NULL;
	CreateXMLNode(pXMLDoc, objectNode, _T("transform"), &tempNode);
	AddXMLText(pXMLDoc, tempNode, transformStr );

	// material
	Mtl* nodeMat = pNode->GetMtl();
	nodeMat ? buf.printf( "%s", nodeMat->GetName() ) : buf.printf( "undefined" );

	tempNode = NULL;
	CreateXMLNode(pXMLDoc, objectNode, _T("material"), &tempNode);
	AddXMLText(pXMLDoc, tempNode, buf );

	// handle
	buf.printf( "%u", pNode->GetHandle() );

	tempNode = NULL;
	CreateXMLNode(pXMLDoc, objectNode, _T("handle"), &tempNode);
	AddXMLText(pXMLDoc, tempNode, buf );

	// parent name
	INode* parent = pNode->GetParentNode();
	parent ? buf.printf( "%s", parent->GetName() ) : buf.printf( "undefined" );

	tempNode = NULL;
	CreateXMLNode(pXMLDoc, objectNode, _T("parent"), &tempNode);
	AddXMLText(pXMLDoc, tempNode, buf );

	// parent handle
	parent ? buf.printf( "%u", parent->GetHandle() ) : buf.printf( "undefined" );

	tempNode = NULL;
	CreateXMLNode(pXMLDoc, objectNode, _T("parentHandle"), &tempNode);
	AddXMLText(pXMLDoc, tempNode, buf );

	// layer
	ILayer* nodeLayer = (ILayer*) pNode->GetReference(NODE_LAYER_REF);
	nodeLayer ? buf.printf( "%s", nodeLayer->GetName() ) : buf.printf( "undefined" );

	tempNode = NULL;
	CreateXMLNode(pXMLDoc, objectNode, _T("layer"), &tempNode);
	AddXMLText(pXMLDoc, tempNode, buf );

	// ishiddeninviewport
	buf.printf( "%s", pNode->IsHidden()?_T("true"):_T("false") );

	tempNode = NULL;
	CreateXMLNode(pXMLDoc, objectNode, _T("isHidden"), &tempNode);
	AddXMLText(pXMLDoc, tempNode, buf );

	// isanimated
	buf.printf( "%s", pNode->IsAnimated()?_T("true"):_T("false") );

	tempNode = NULL;
	CreateXMLNode(pXMLDoc, objectNode, _T("isAnimated"), &tempNode);
	AddXMLText(pXMLDoc, tempNode, buf );

	// isfrozen
	buf.printf( "%s", pNode->IsFrozen()?_T("true"):_T("false") );

	tempNode = NULL;
	CreateXMLNode(pXMLDoc, objectNode, _T("isFrozen"), &tempNode);
	AddXMLText(pXMLDoc, tempNode, buf );
	
	// properties
	ExportPropertiesInfo(pXMLDoc, objectNode, (Animatable*)pNode, EXPORT_PROPERTY_VALUES );

	// modifiers
	ExportModifiersInfo(pXMLDoc, objectNode, pNode);

	// recurse children nodes
	int numChildren = pNode->NumberOfChildren();
    for(int i = 0; i < numChildren; i++)
        ExportNodeInfo(pXMLDoc, objectNode, pNode->GetChildNode(i));

	objectNode = NULL;
	tempNode = NULL;
}

// -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//	maxscript funcs
// -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// blurutil.exportTOCFile()
def_struct_primitive( exportTOCFile, blurUtil, "exportTOCFile");

Value* exportTOCFile_cf(Value** arg_list, int arg_count)
{
	time_t   start, finish;
	double   elapsed_time;
	time( &start );

	IXMLDOMDocument * pXMLDoc;
	IXMLDOMNode * pRoot;	
	pRoot = NULL;
	
	CComPtr<IXMLDOMNode> sceneNode;
	CComPtr<IXMLDOMNode> tocNode;	//the IGame child - which is the main node
	CComPtr<IXMLDOMNode> rootNode;
	CComPtr<IXMLDOMNode> tempNode;

	Interface * ip = GetCOREInterface();
	BOOL print20elementsState = SetPrintAllElements(TRUE);

	// InitialiseXML
	HRESULT hr;
	hr = CoInitialize(NULL); 
	hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,  IID_IXMLDOMDocument, (void**)&pXMLDoc);
	if(FAILED(hr))
		return &undefined;
	
	hr = pXMLDoc->QueryInterface(IID_IXMLDOMNode, (void **)&pRoot);
	if(FAILED(hr))
		return &undefined;
	
	// set xml doc props
	pXMLDoc->put_async( FALSE );
	pXMLDoc->put_preserveWhiteSpace( TRUE );

	// rootnode
	tocNode = NULL;
	CreateXMLNode(pXMLDoc, pRoot, _T("toc"), &tocNode);
	AddXMLAttribute(tocNode, _T("filename"), ip->GetCurFileName());

	//user info
	ExportUserInfo( pXMLDoc, tocNode );

	// layers
	ExportLayersInfo( pXMLDoc, tocNode );

	// objects
	INode* pRootNode = ip->GetRootNode();
	ExportNodeInfo ( pXMLDoc, tocNode , pRootNode );

	// compute the time elapsed
	time( &finish );
	elapsed_time = difftime( finish, start );

	TSTR buf;
	buf.printf("%f", elapsed_time);

	tempNode = NULL;
	CreateXMLNode(pXMLDoc, tocNode, _T("timeToCalculateTOC"), &tempNode);
	AddXMLText( pXMLDoc, tempNode, buf );

	PrettyPrint(_T("c:/temp/temp2.xml"), pXMLDoc);

	// clean xml stuff
	CoUninitialize();

	// delete pointers
	sceneNode = NULL;
	rootNode = NULL;
	tocNode = NULL;
	tempNode = NULL;

	if(pRoot)
		pRoot->Release(); 
	pRoot = NULL;
	if(pXMLDoc)
		pXMLDoc->Release();
	
	pXMLDoc = NULL;

	SetPrintAllElements(print20elementsState);

	return &undefined;
}


void toc_init()
{
	CharStream* out = thread_local(current_stdout);
	out->puts(_T("--* toc loaded *--\n"));
}
