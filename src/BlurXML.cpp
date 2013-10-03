
#include "imports.h"
#include "resource.h"


#include "GenericMethod.h"
#include "BlurXML.h"

#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
#include "maxscript\macros\define_external_functions.h"
#include "maxscript\macros\define_instantiation_functions.h"
#else
#include "defextfn.h"
#include "definsfn.h"
#endif

//---------------------------------------------------------------------------------------------------
//										GLOBAL NAMES
//---------------------------------------------------------------------------------------------------

#define n_addNode			( Name::intern(_T("addNode")) )
#define n_attrCount			( Name::intern(_T("attrCount")) )
#define n_attributes		( Name::intern(_T("attributes")) )
#define n_baseName			( Name::intern(_T("baseName")) )
#define n_firstChild		( Name::intern(_T("firstChild")) )
#define n_isProperty		( Name::intern(_T("isProperty")) )
#define n_lastChild			( Name::intern(_T("lastChild")) )
#define n_loadFromFile		( Name::intern(_T("loadFromFile")) )
#define n_namespaceURI		( Name::intern(_T("namespaceURI")) )
#define n_nextSibling		( Name::intern(_T("nextSibling")) )
#define n_nodeCount			( Name::intern(_T("nodeCount")) )
#define n_nodeName			( Name::intern(_T("nodeName")) )
#define n_nodeType			( Name::intern(_T("nodeType")) )
#define n_nodeTypeString	( Name::intern(_T("nodeTypeString")) )
#define n_nodeValue			( Name::intern(_T("nodeValue")) )
#define n_parentNode		( Name::intern(_T("parentNode")) )
#define n_parsed			( Name::intern(_T("parsed")) )
#define n_parseXML			( Name::intern(_T("parseXML")) )
#define n_prefix			( Name::intern(_T("prefix")) )
#define n_previousSibling	( Name::intern(_T("previousSibling")) )
#define n_propCount			( Name::intern(_T("propCount")) )
#define n_properties		( Name::intern(_T("properties")) )
#define n_rootNode			( Name::intern(_T("rootNode")) )
#define n_saveToFile		( Name::intern(_T("saveToFile")) )
#define n_setAttribute		( Name::intern(_T("setAttribute")) )
#define n_setProperty		( Name::intern(_T("setProperty")) )
#define n_setText			( Name::intern(_T("setText")) )
#define n_typeString		( Name::intern(_T("typeString")) )
#define n_xml				( Name::intern(_T("xml")) )

//---------------------------------------------------------------------------------------------------
//										UTILITY METHODS
//---------------------------------------------------------------------------------------------------
TSTR BstrToTstr( BSTR str ) {
	return TSTR::FromBSTR(str);
}

// FIXME: Returns pointer to freed data
CComVariant VariantFromValue( Value * val )
{
	if ( is_bool(val) )
		return ( val->to_bool() ) ? CComVariant( "true" ) : CComVariant( "false" );
	else if ( is_integer(val) )
		return CComVariant( val->to_int() );
	else if ( is_float(val) )
		return CComVariant( val->to_float() );
	else if ( is_string(val) || is_name(val) )
		return CComVariant( val->to_string() );

	StringStream* s = new StringStream();
	val->sprin1(s);
	CComVariant out = CComVariant(s->to_string());
	DeleteObject(s);
	return out;
}

// FIXME: Audit
Value * ValueFromVariant( VARIANT* var )
{
	one_typed_value_local(Value* result);
	vl.result = &undefined;

	HRESULT hr = S_OK;

	switch (V_VT(var)) {
		// coerce those types we handle at this point, only by-value for now
		case VT_EMPTY:
		case VT_NULL:
			vl.result = &undefined; break;
		case VT_EMPTY | VT_BYREF:
			vl.result = &undefined; break;

		case VT_VOID:
			vl.result = &ok; break;

		case VT_UI1:
			vl.result = Integer::intern((int)V_UI1(var)); break;
		case VT_UI1 | VT_BYREF:
			vl.result = Integer::intern((int)*V_UI1REF(var)); break;

		case VT_UI2:
			vl.result = Integer::intern((int)V_UI2(var)); break;
		case VT_UI2 | VT_BYREF:
			vl.result = Integer::intern((int)*V_UI2REF(var)); break;

		case VT_UI4:
			vl.result = Integer::intern((int)V_UI4(var)); break;
		case VT_UI4 | VT_BYREF:
			vl.result = Integer::intern((int)*V_UI4REF(var)); break;

		case VT_I2:
			vl.result = Integer::intern((int)V_I2(var)); break;
		case VT_I2 | VT_BYREF:
			vl.result = Integer::intern((int)*V_I2REF(var)); break;

		case VT_I4:
			vl.result = Integer::intern((int)V_I4(var)); break;
		case VT_I4 | VT_BYREF:
			vl.result = Integer::intern((int)*V_I4REF(var)); break;

		case VT_INT:
			vl.result = Integer::intern((int)V_INT(var)); break;
		case VT_INT | VT_BYREF:
			vl.result = Integer::intern((int)*V_INTREF(var)); break;

		case VT_R4: {
#ifdef MAX_TRACE
				_variant_t vDisp;
				_variant_t vSrc(var);
				vDisp.ChangeType(VT_BSTR, &vSrc);
				DebugPrint("%s\n", OLE2T(vDisp.bstrVal));
#endif
				vl.result = Float::intern((float)V_R4(var)); break;
			}
		case VT_R4 | VT_BYREF:
			vl.result = Float::intern((float)*V_R4REF(var)); break;

		case VT_R8:
			vl.result = Float::intern((float)V_R8(var)); break;
		case VT_R8 | VT_BYREF:
			vl.result = Float::intern((float)*V_R8REF(var)); break;

		case VT_CY: {
				float v;
				VarR4FromCy(V_CY(var), &v);
				vl.result = Float::intern(v);
			}
			break;
		case VT_CY | VT_BYREF: {
				float v;
				VarR4FromCy(*V_CYREF(var), &v);
				vl.result = Float::intern(v);
			}
			break;

		case VT_BSTR: {
				vl.result = new String( TSTR::FromBSTR(V_BSTR(var)) );
				SysFreeString(V_BSTR(var));   // free string
			}
			break;
		case VT_BSTR | VT_BYREF: {
				vl.result = new String( TSTR::FromBSTR(*V_BSTRREF(var)) );
			}
			break;

		case VT_BOOL:
			vl.result = V_BOOL(var) != 0 ? &true_value : &false_value; break;
		case VT_BOOL | VT_BYREF:
			vl.result = *V_BOOLREF(var) != 0 ? &true_value : &false_value; break;
#ifdef NOWEIRDTYPES	
		case VT_COLOR:
			// convert BGR(OLE_COLOR) to RGB
			OLE_COLOR color =  V_I4(var);
			*pval = new ColorValue(GetBValue(color), GetGValue(color), GetRValue(color)); break;
		case VT_COLOR | VT_BYREF:
			OLE_COLOR color =  *V_I4REF(var);
			*pval = new ColorValue(GetBValue(color), GetGValue(color), GetRValue(color)); break;
		

		case VT_DISPATCH:
			if (V_DISPATCH(var))
				*pval = new MSDispatch (V_DISPATCH(var), _T("MSDispatch") );
			else
				*pval = &undefined; 
			break;
		case VT_DISPATCH | VT_BYREF: 
			if((*V_DISPATCHREF(var)))
				*pval = new MSDispatch (*V_DISPATCHREF(var), _T("MSDispatch") );
			else
				*pval = &undefined; 
			break;
		case VT_VARIANT | VT_BYREF:
			return ValueFromVariant(V_VARIANTREF(var), pval, pTypeInfo, progID);
#endif
		case VT_ERROR:
			if (var->scode != DISP_E_PARAMNOTFOUND) {
				ReferenceTarget* ref = (ReferenceTarget*)var->byref;// SR FIXME64: !!!
				if (ref->SuperClassID() == INODE_SUPERCLASS_ID)
					vl.result = MAXNode::intern((INode*)ref);
				else
					vl.result = MAXClass::make_wrapper_for(ref);
			}
		default:
			hr = ResultFromScode(DISP_E_BADVARTYPE);
			break;
	}

	return_value( vl.result );
}

// FIXME: Returns pointer to freed
TSTR ValueToTstr( Value* val )
{
	if ( is_string(val) || is_name(val) )
		return TSTR(val->to_string());
	else {
		StringStream* s = new StringStream();
		val->sprin1(s);
		TSTR ret(s->to_string());
		DeleteObject(s);
		return ret;
	}
	return TSTR();
}

Value * NodeTypeToValue( int type )
{
	switch ( type ) {
		case NODE_INVALID:					return Name::intern(_T("invalid"));
		case NODE_ELEMENT:					return Name::intern(_T("element"));
		case NODE_ATTRIBUTE:				return Name::intern(_T("attribute"));
		case NODE_TEXT:						return Name::intern(_T("text"));
		case NODE_CDATA_SECTION:			return Name::intern(_T("cdata"));
		case NODE_ENTITY_REFERENCE:			return Name::intern(_T("entityReference"));
		case NODE_ENTITY:					return Name::intern(_T("entity"));
		case NODE_PROCESSING_INSTRUCTION:	return Name::intern(_T("processingInstruction"));
		case NODE_COMMENT:					return Name::intern(_T("comment"));
		case NODE_DOCUMENT:					return Name::intern(_T("document"));
		case NODE_DOCUMENT_TYPE:			return Name::intern(_T("documentType"));
		case NODE_DOCUMENT_FRAGMENT:		return Name::intern(_T("documentFragment"));
		case NODE_NOTATION:					return Name::intern(_T("notation"));
	}
	return &undefined;
}

bool IsProperty( IXMLDOMNode* xmlNode)
{
	// properties are nodes that contain one children which is text type
	long nodeCount = 0;

	IXMLDOMNodeList*  nodeList = NULL;
	xmlNode->get_childNodes(&nodeList);
	nodeList->get_length( &nodeCount );
	
	if ( nodeCount == 1 ) {
		IXMLDOMNode* subXmlNode = NULL;
        nodeList->get_item( 0, &subXmlNode );
		
		DOMNodeType type;
		subXmlNode->get_nodeType( &type );

		if (type == NODE_TEXT)
			return true;
	}

	return false;
}

//---------------------------------------------------------------------------------------------------
//										XML DOCUMENT
//---------------------------------------------------------------------------------------------------
visible_class_instance( XMLDocument, "XMLDocument" );

Value * XMLDocumentClass::apply( Value** arg_list, int count, CallContext* cc )
{
	check_arg_count( XMLDocument, 0, count );
	one_typed_value_local( XMLDocument* result );
	vl.result = new XMLDocument();
	if ( !vl.result->isValid() )
		return &undefined;
	return_value( vl.result );
}

XMLDocument::XMLDocument()
{
	tag = class_tag( XMLDocument );

	_document		= NULL;
	_root			= NULL;

	// Initialize XML
	HRESULT hr;
	hr = CoInitialize(NULL);
	hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,  IID_IXMLDOMDocument, (void**)&_document);
	if (FAILED(hr))
		return;

	IXMLDOMNode* root = NULL;
	hr = _document->QueryInterface( IID_IXMLDOMNode, (void **)&root);
	if (FAILED(hr))
		return;

	// Set XML Doc Props
	_document->put_async(false);
	_root = new XMLNode( root, _document );
}

XMLDocument::~XMLDocument() {
	if ( this->_document )
		this->_document->Release();

	this->_document	= NULL;
}

//--------------------------------		C++ Methods		---------------------------------------------
XMLNode * XMLDocument::addNode( TSTR nodeName )
{ return this->_root->addNode( nodeName ); }

IXMLDOMDocument * XMLDocument::document()
{ return this->_document; }

bool XMLDocument::loadFromFile( TSTR fileName )
{
	if ( this->isValid() ) {
		VARIANT_BOOL success;
		this->_document->load( CComVariant( fileName ), &success );
		return ( success ) ? true : false;
	}
	return false;
}

bool XMLDocument::parseXML( TSTR xml )
{
	if ( this->isValid() ) {
		VARIANT_BOOL success;
		_document->loadXML( CComBSTR( xml ), &success );
		return ( success ) ? true : false;
	}
	return false;
}

XMLNode * XMLDocument::root()
{ return _root; }

bool XMLDocument::saveToFile( TSTR fileName )
{
	if ( isValid() ) {
		// perform formatting XSLT transform to get indented XML output
		IXMLDOMDocument* xslDoc;
		BSTR outputXML;
		HRESULT hr = CoCreateInstance(CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER,  IID_IXMLDOMDocument, (void**)&xslDoc);
		if (SUCCEEDED(hr)) {
			// load indenting XSL doc 
			VARIANT_BOOL result;
			CComBSTR indentXSL(
				"<xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\">"
				"	<xsl:output method=\"xml\" encoding=\"ISO-8859-1\"/>"
				"	<xsl:param name=\"indent-increment\" select=\"'  '\"/>"

				"	<xsl:template name=\"newline\">"
				"		<xsl:text disable-output-escaping=\"yes\">&#10;</xsl:text>"
				"	</xsl:template>"

				"	<xsl:template match=\"comment() | processing-instruction()\">"
				"		<xsl:param name=\"indent\" select=\"''\"/>"
				"		<xsl:call-template name=\"newline\"/>"
				"		<xsl:value-of select=\"$indent\"/>"
				"		<xsl:copy />"
				"	</xsl:template>"

				"	<xsl:template match=\"text()\">"
				"		<xsl:param name=\"indent\" select=\"''\"/>"
				"		<xsl:call-template name=\"newline\"/>"
				"		<xsl:value-of select=\"$indent\"/>"
				"		<xsl:value-of select=\"normalize-space(.)\"/>"
				"	</xsl:template>"
				
				"	<xsl:template match=\"text()[normalize-space(.)='']\"/>"
				
				"	<xsl:template match=\"*\">"
				"		<xsl:param name=\"indent\" select=\"''\"/>"
				"		<xsl:call-template name=\"newline\"/>"
				"		<xsl:value-of select=\"$indent\"/>"
				"		<xsl:choose>"
				"			<xsl:when test=\"count(child::*) > 0\">"
				"				<xsl:copy>"
				"					<xsl:copy-of select=\"@*\"/>"
				"					<xsl:apply-templates select=\"*|text()\">"
				"						<xsl:with-param name=\"indent\" select=\"concat ($indent, $indent-increment)\"/>"
				"					</xsl:apply-templates>"
				"					<xsl:call-template name=\"newline\"/>"
				"					<xsl:value-of select=\"$indent\"/>"
				"				</xsl:copy>"
				"			</xsl:when>"
				"			<xsl:otherwise>"
				"				<xsl:copy-of select=\".\"/>"
				"			</xsl:otherwise>"
				"		</xsl:choose>"
				"	</xsl:template>"
				"</xsl:stylesheet>"
			);
			hr = xslDoc->loadXML(indentXSL, &result);
			if (SUCCEEDED(hr)) {
				// perform transform
				hr = document()->transformNode(xslDoc, &outputXML);
			}
		}

		// output transformed XML if previous sequence succeeded, else normal XMLDoc save
		if (SUCCEEDED(hr)) {
			FILE* out = _tfopen(fileName, _T("w"));
			if (out != NULL) {
				// hack the UTF-16 back to UTF-8 (there probably is a way to mod the stylesheet to do this)
				wchar_t* enc = wcsstr(outputXML, L"\"UTF-16\"");
				if (enc != NULL)
					memcpy(enc, L"\"utf-8\" ", 8 * sizeof(wchar_t));
				// convert BSTR to MBCS for output
				int len = WideCharToMultiByte(CP_UTF8, 0, outputXML, -1, 0, 0, NULL, NULL);
				char * buf = (char*)malloc(len);
				WideCharToMultiByte(CP_UTF8, 0, outputXML, -1, buf, len, NULL, NULL);
				// write the XML
				fputs(buf, out);
				fclose(out);
				free(buf);
			}
			SysFreeString(outputXML);
		}
		else
			// save the XML graph out to the export file
			document()->save(CComVariant(fileName));

		return true;
	}
	return false;
}

TSTR XMLDocument::writeXML()
{
	return _root->writeXML();
}

//--------------------------------	 Maxscript Methods	---------------------------------------------
Value * XMLDocument::applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc )
{
	if ( methodID == n_addNode ) {
		check_arg_count( addNode, 1, count );
		return addNode( arg_list[0]->eval()->to_string() );
	}
	else if ( methodID == n_loadFromFile ) {
		check_arg_count( loadFromFile, 1, count );
		return loadFromFile( arg_list[0]->eval()->to_string() ) ? &true_value : &false_value;
	}
	else if ( methodID == n_parseXML ) {
		check_arg_count( parseXML, 1, count );
		return parseXML( arg_list[0]->eval()->to_string() ) ? &true_value : &false_value;
	}
	else if ( methodID == n_saveToFile ) {
		check_arg_count( saveToFile, 1, count );
		return saveToFile( arg_list[0]->eval()->to_string() ) ? &true_value : &false_value;
	}
	return &ok;
}

void XMLDocument::sprin1( CharStream* s )
{
	s->puts( _T("(XMLDocument())") );
}

void XMLDocument::gc_trace()
{
	Value::gc_trace();

	if ( _root && _root->is_not_marked() )
		_root->gc_trace();
}

Value * XMLDocument::get_property( Value** arg_list, int count )
{
	Value* prop = arg_list[0];

	if ( prop == n_addNode )
		return NEW_GENERIC_METHOD( addNode );
	else if ( prop == n_loadFromFile )
		return NEW_GENERIC_METHOD( loadFromFile );
	else if ( prop == n_parseXML )
		return NEW_GENERIC_METHOD( parseXML );
	else if ( prop == n_rootNode )
		return this->root();
	else if ( prop == n_saveToFile )
		return NEW_GENERIC_METHOD( saveToFile );
	
	return root()->get_property( arg_list, count );
}

Value * XMLDocument::set_property( Value** arg_list, int count )
{
	throw RuntimeError( _T("Cannot set the property #"), arg_list[1]->to_string() );
}

//---------------------------------------------------------------------------------------------------
//										XML NODE
//---------------------------------------------------------------------------------------------------

visible_class_instance( XMLNode, "XMLNode" );

XMLNode::XMLNode( IXMLDOMNode* node, IXMLDOMDocument* document )
{
	tag = class_tag( XMLNode );
	_node = node;
	_document = document;
}

XMLNode::~XMLNode()
{
	_document = NULL;
	_node = NULL;
}

//--------------------------------		C++ Methods		---------------------------------------------
XMLNode * XMLNode::addNode( TSTR nodeName )
{
	IXMLDOMNode* newChild		= NULL;
	IXMLDOMNode* outNewChild	= NULL;
	document()->createNode( CComVariant(NODE_ELEMENT), CComBSTR(nodeName), NULL, &newChild );
	node()->appendChild( newChild, &outNewChild );
	if ( outNewChild ) {
		one_typed_value_local( XMLNode* out );
		vl.out = new XMLNode( outNewChild, document() );
		return_value( vl.out );
	}
	return (XMLNode*) &undefined;
}

XMLNodeList * XMLNode::attributes()
{
	IXMLDOMNamedNodeMap* attrList = NULL;
	one_typed_value_local( XMLNodeList* result );
	_node->get_attributes(&attrList);
	vl.result = new XMLNodeList( document(), attrList );
	return_value( vl.result );
}

XMLNode * XMLNode::attributeByName( TSTR name )
{
	CComBSTR attrName( name );

	IXMLDOMNode* outNode			= NULL;
	IXMLDOMNamedNodeMap* attrList	= NULL;
	_node->get_attributes( &attrList );

	long attrCount = 0;
	attrList->get_length( &attrCount );

	for (int i = 0; i < attrCount; i++ ) {
		IXMLDOMNode* subNode = NULL;
		attrList->get_item( i, &subNode );

		CComBSTR subNodeName;
		subNode->get_nodeName( &subNodeName );

		attrName.ToLower();
		subNodeName.ToLower();

		if ( attrName == subNodeName ) {
			outNode = subNode;
			break;
		}
	}

	if ( outNode ) {
		one_typed_value_local( XMLNode* out );
		vl.out = new XMLNode( outNode, document() );
		return_value( vl.out );
	}
	return (XMLNode*) &undefined;
}

TSTR XMLNode::baseName()
{
	BSTR outString;
	_node->get_baseName( &outString );
	return BstrToTstr( outString );
}

XMLNodeList * XMLNode::children()
{
	IXMLDOMNodeList* nodeList = NULL;
	one_typed_value_local( XMLNodeList* result );
	_node->get_childNodes(&nodeList);
	vl.result = new XMLNodeList(document(), nodeList);
	return_value( vl.result );
}

XMLNode * XMLNode::childByIndex( int index )
{ return children()->getItem(index); }

XMLNode * XMLNode::childByName( TSTR name )
{ return children()->getItem(name); }

Array * XMLNode::childrenByName( TSTR name )
{
	IXMLDOMNodeList* nodeList	= NULL;
	one_typed_value_local( Array* out );
	vl.out = new Array(0);

	CComBSTR childName(name);
	childName.ToLower();

	_node->get_childNodes(&nodeList);

	if ( nodeList ) {
		long nodeCount;
		nodeList->get_length(&nodeCount);
		for (int i = 0; i < nodeCount; i++ ) {
			IXMLDOMNode* subNode = NULL;
			nodeList->get_item( i, &subNode );
			
			CComBSTR subNodeName;
			subNode->get_nodeName( &subNodeName );
			subNodeName.ToLower();

			if ( subNodeName == childName ) {
				vl.out->append( new XMLNode( subNode, document() ) );
			}
		}
	}
	return_value( vl.out );
}

IXMLDOMDocument * XMLNode::document()
{ return _document; }

XMLNode * XMLNode::firstChild()
{
	IXMLDOMNode* childNode = NULL;
	_node->get_firstChild( &childNode );
	if ( childNode ) {
		one_typed_value_local( XMLNode* out );
		vl.out = new XMLNode( childNode, document() );
		return_value( vl.out );
	}
	return (XMLNode*) &undefined;
}

bool XMLNode::isParsed()
{
	VARIANT_BOOL* parsed = NULL;
	_node->get_parsed( parsed );
	return parsed ? true : false;
}

bool XMLNode::isProperty()
{ return IsProperty( _node ); }

XMLNode * XMLNode::lastChild()
{
	IXMLDOMNode* childNode = NULL;
	_node->get_lastChild( &childNode );
	if ( childNode ) {
		one_typed_value_local( XMLNode* out );
		vl.out = new XMLNode( childNode, document() );
		return_value( vl.out );
	}
	return (XMLNode*) &undefined;
}

TSTR XMLNode::namespaceURI()
{
	BSTR outString;
	_node->get_namespaceURI(&outString);
	return BstrToTstr( outString );
}

XMLNode * XMLNode::nextSibling()
{
	IXMLDOMNode* outSibling = NULL;
	_node->get_nextSibling( &outSibling );
	if ( outSibling ) {
		one_typed_value_local( XMLNode* out );
		vl.out = new XMLNode( outSibling, document() );
		return_value( vl.out );
	}
	return (XMLNode*) &undefined;
}

IXMLDOMNode * XMLNode::node()
{ return _node; }

TSTR XMLNode::nodeName()
{
	BSTR outString;
	_node->get_nodeName( &outString );
	return BstrToTstr( outString );
}

int XMLNode::nodeType()
{
	DOMNodeType type;
	_node->get_nodeType( &type );
	return type;	
}

long XMLNode::numAttributes()
{
	long attrCount = 0;
	IXMLDOMNamedNodeMap* attrList = NULL;
	_node->get_attributes( &attrList );
	if ( attrList )
		attrList->get_length(&attrCount);
	return attrCount;
}

long XMLNode::numChildren()
{
	long childCount = 0;
	IXMLDOMNodeList* nodeList = NULL;
	_node->get_childNodes( &nodeList );
	if ( nodeList )
		nodeList->get_length( &childCount );
	return childCount;
}

long XMLNode::numNodes()
{
	long childCount = 0;
	IXMLDOMNodeList* nodeList = NULL;
	_node->get_childNodes( &nodeList );
	if ( nodeList )
		nodeList->get_length( &childCount );

	long nodeCount = 0;
	for ( int i = 0; i < childCount; i++ ) {
		IXMLDOMNode* xmlNode = NULL;
		nodeList->get_item( i, &xmlNode );
		if ( !IsProperty( xmlNode ) )
			nodeCount++;
	}

	return nodeCount;
}

long XMLNode::numProperties()
{
	long childCount = 0;

	IXMLDOMNodeList* nodeList = NULL;
	_node->get_childNodes( &nodeList );

	if ( nodeList )
		nodeList->get_length( &childCount );

	long propCount = 0;
	for (int i = 0; i < childCount; i++ ) {
		IXMLDOMNode* xmlNode = NULL;
		nodeList->get_item( i, &xmlNode );
		if ( IsProperty( xmlNode ) )
			propCount++;
	}
	return propCount;
}

XMLNode * XMLNode::parent()
{
	IXMLDOMNode* outParent = NULL;
	_node->get_parentNode( &outParent );
	if ( outParent ) {
		one_typed_value_local( XMLNode* out );
		vl.out = new XMLNode( outParent, document() );
		return_value( vl.out );
	}
	return (XMLNode*) &undefined;
}

TSTR XMLNode::prefix()
{
	BSTR outString;
	_node->get_prefix( &outString );
	return BstrToTstr( outString );
}

XMLNode * XMLNode::previousSibling()
{
	IXMLDOMNode* outSibling = NULL;
	_node->get_previousSibling( &outSibling );

	if ( outSibling ) {
		one_typed_value_local( XMLNode* out );
		vl.out = new XMLNode( outSibling, document() );
		return_value( vl.out );
	}
	return (XMLNode*) &undefined;
}

XMLNodeList * XMLNode::properties()
{
	IXMLDOMNodeList* nodeList = NULL;
	_node->get_childNodes( &nodeList );

	long nodeCount = 0;
	nodeList->get_length( &nodeCount );

	one_typed_value_local( XMLNodeList* out );
	vl.out = new XMLNodeList( document() );

	for ( int i = 0; i < nodeCount; i++ ) {
		IXMLDOMNode* subNode = NULL;
		nodeList->get_item( i, &subNode );
		if ( IsProperty( subNode ) ) {
			vl.out->addItem( new XMLNode(subNode, document()) );
		}
	}
	return_value( vl.out );
}

XMLNode * XMLNode::propertyByName( TSTR name )
{
	CComBSTR propName(name);

	IXMLDOMNode* outNode		= NULL;
	IXMLDOMNodeList* nodeList	= NULL;
	_node->get_childNodes( &nodeList );

	long nodeCount = 0;
	nodeList->get_length( &nodeCount );

	for ( int i = 0; i < nodeCount; i++ ) {
		IXMLDOMNode* subNode = NULL;
		nodeList->get_item( i, &subNode );

		if ( IsProperty( subNode ) ) {
			CComBSTR subNodeName;
			subNode->get_nodeName( &subNodeName );

			propName.ToLower();
			subNodeName.ToLower();

			if ( propName == subNodeName ) {
				outNode = subNode;
				break;
			}
		}
	}

	if ( outNode ) {
		one_typed_value_local( XMLNode* out );
		vl.out = new XMLNode( outNode, document() );
		return_value( vl.out );
	}
	return (XMLNode*) &undefined;
}

Array * XMLNode::propertiesByName( TSTR name )
{
	IXMLDOMNodeList* nodeList	= NULL;
	one_typed_value_local( Array* out );
	vl.out = new Array(0);

	CComBSTR childName(name);
	childName.ToLower();

	this->_node->get_childNodes(&nodeList);

	if ( nodeList ) {
		long nodeCount;
		nodeList->get_length(&nodeCount);
		for (int i = 0; i < nodeCount; i++ ) {
			IXMLDOMNode* subNode = NULL;
			nodeList->get_item( i, &subNode );
			
			if ( IsProperty( subNode ) ) {
				CComBSTR subNodeName;
				subNode->get_nodeName( &subNodeName );
				subNodeName.ToLower();

				if ( subNodeName == childName ) {
					vl.out->append( new XMLNode( subNode, document() ) );
				}
			}
		}
	}
	return_value( vl.out );
}

bool XMLNode::setAttribute( TSTR name, bool val )
{ return setAttribute( name, val ? &true_value : &false_value ); }

bool XMLNode::setAttribute( TSTR name, Color val )
{ return this->setAttribute( name, ColorValue::intern(val) ); }

bool XMLNode::setAttribute( TSTR name, float val )
{ return this->setAttribute( name, Float::intern(val) ); }

bool XMLNode::setAttribute( TSTR name, int val )
{ return this->setAttribute( name, Integer::intern(val) ); }

bool XMLNode::setAttribute( TSTR name, TSTR val )
{ return this->setAttribute( name, new String(val) ); }

bool XMLNode::setAttribute( TSTR name, Value* val )
{ 
	CComQIPtr<IXMLDOMElement> element;
	element = _node;
	element->setAttribute( CComBSTR(name), VariantFromValue(val) );
	return true;
}

bool XMLNode::setProperty( TSTR name, bool val )
{ return setProperty( name, (val) ? &true_value : &false_value ); }

bool XMLNode::setProperty( TSTR name, Color val )
{ return setProperty( name, ColorValue::intern(val) ); }

bool XMLNode::setProperty( TSTR name, float val )
{ return setProperty( name, Float::intern(val) ); }

bool XMLNode::setProperty( TSTR name, int val )
{ return setProperty( name, Integer::intern(val) ); }

bool XMLNode::setProperty( TSTR name, TSTR val )
{ return setProperty( name, new String(val) ); }

bool XMLNode::setProperty( TSTR name, Value* val )
{
	XMLNode* propChild = childByName( name );
	if ( propChild == (XMLNode*) &undefined )
		propChild = addNode( name );

	if ( propChild != (XMLNode*) &undefined )
		return propChild->setText( ValueToTstr( val ) );
	return false;
}

bool XMLNode::setText( TSTR text )
{
	long nodeCount					= 0;
	bool found						= false;
	IXMLDOMNodeList*	nodeList	= NULL;

	node()->get_childNodes(&nodeList);
	nodeList->get_length( &nodeCount );
	for ( int i = 0; i < nodeCount; i++ ) {
		IXMLDOMNode* subNode = NULL;
		nodeList->get_item( i, &subNode );
		DOMNodeType type;
		subNode->get_nodeType( &type );

		if ( type == NODE_TEXT ) {
			subNode->put_text( CComBSTR(text) );
			found = true;
			break;
		}
	}
	if ( !found ) {
		CComPtr<IXMLDOMText> tempNode = NULL;
		document()->createTextNode( CComBSTR(text), &tempNode );
		node()->appendChild( tempNode, NULL );
	}
	return true;
}

bool XMLNode::setValue( bool val )
{ return setValue( (val) ? &true_value : &false_value ); }

bool XMLNode::setValue( Color val )
{ return setValue( ColorValue::intern(val) ); }

bool XMLNode::setValue( float val )
{ return setValue( Float::intern(val) ); }

bool XMLNode::setValue( int val )
{ return setValue( Integer::intern(val) ); }

bool XMLNode::setValue( TSTR val )
{ return setValue( new String(val) ); }

bool XMLNode::setValue( Value* val )
{
	if ( isProperty() )
		setText( ValueToTstr( val ) );
	else
		_node->put_nodeTypedValue( VariantFromValue(val) );
	return true;
}

TSTR XMLNode::text()
{
	BSTR outString;
	_node->get_text(&outString);
	return BstrToTstr(outString);
}

Value * XMLNode::value()
{
	VARIANT vResult;
	vResult.vt		= VT_BYREF;
	vResult.byref	= NULL;
	_node->get_nodeTypedValue( &vResult );
	return ( ValueFromVariant( &vResult ) );
}

TSTR XMLNode::writeXML()
{
	BSTR outString;
	_node->get_xml( &outString );
	return BstrToTstr( outString );
}

//--------------------------------	 Maxscript Methods	---------------------------------------------
Value * XMLNode::applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc )
{
	if ( methodID == n_addNode ) {
		check_arg_count( addNode, 1, count );
		return addNode( arg_list[0]->eval()->to_string() );
	}
	else if ( methodID == n_setAttribute ) {
		check_arg_count( setAttribute, 2, count );
		return setAttribute( arg_list[0]->eval()->to_string(), arg_list[1]->eval() ) ? &true_value : &false_value;
	}
	else if ( methodID == n_setProperty ) {
		check_arg_count( setProperty, 2, count );
		return setProperty( arg_list[0]->eval()->to_string(), arg_list[1]->eval() ) ? &true_value : &false_value;
	}
	return &ok;
}

Value * XMLNode::get_vf( Value** arg_list, int count )
{
	Value* prop		= arg_list[0];
	Value* result	= &undefined;

	if ( is_number(prop) )
		result = childByIndex( prop->to_int() );
	else {
		result = attributeByName( prop->to_string() );
		if ( result == &undefined )
			result = childByName( prop->to_string() );
	}
	return result;
}

void XMLNode::gc_trace()
{
	Value::gc_trace();
}

void XMLNode::sprin1(CharStream *s)
{
	s->puts( _T("(XMLNode nodeName:\"") );
	s->puts( nodeName() );
	s->puts( _T("\" nodeType:#") );
	s->puts( NodeTypeToValue( nodeType() )->to_string() );
	s->puts( _T(")") );
}

Value * XMLNode::get_property( Value** arg_list, int count )
{
	Value* prop		= arg_list[0];

	if ( prop == n_addNode )
		return NEW_GENERIC_METHOD( addNode );
	else if	( prop == n_attrCount )
		return Integer::intern( numAttributes() );
	else if ( prop == n_attributes )
		return attributes();
	else if ( prop == n_baseName )
		return new String( baseName() );
	else if ( prop == n_children )
		return children();
	else if ( prop == n_count )
		return Integer::intern(numChildren());
	else if ( prop == n_firstChild )
		return firstChild();
	else if ( prop == n_isProperty )
		return isProperty() ? &true_value : &false_value;
	else if ( prop == n_lastChild )
		return lastChild();
	else if ( prop == n_namespaceURI )
		return new String( namespaceURI() );
	else if ( prop == n_nextSibling )
		return nextSibling();
	else if ( prop == n_nodeCount )
		return Integer::intern( numNodes() );
	else if ( prop == n_nodeName )
		return new String( nodeName() );
	else if ( prop == n_nodeType )
		return NodeTypeToValue( nodeType() );
	else if ( prop == n_nodeTypeString ) {
		BSTR nodeTypeString;
		_node->get_nodeTypeString(&nodeTypeString);
		return new String( BstrToTstr( nodeTypeString ) );
	}
	else if ( prop == n_parent )
		return parent();
	else if ( prop == n_parsed )
		return isParsed() ? &true_value : &false_value;
	else if ( prop == n_prefix )
		return new String(prefix());
	else if ( prop == n_properties )
		return properties();
	else if ( prop == n_previousSibling )
		return previousSibling();
	else if ( prop == n_propCount )
		return Integer::intern(numProperties());
	else if ( prop == n_setAttribute )
		return NEW_GENERIC_METHOD( setAttribute );
	else if ( prop == n_setProperty )
		return NEW_GENERIC_METHOD( setProperty );
	else if ( prop == n_setText )
		return NEW_GENERIC_METHOD( setText );
	else if ( prop == n_text )
		return new String(text());
	else if ( prop == n_value )
		return value();
	else if ( prop == n_xml )
		return new String(writeXML());

	return Value::get_property( arg_list, count );
}

Value * XMLNode::set_property( Value** arg_list, int count )
{
	Value* prop = arg_list[1];
	Value* val	= arg_list[0];

	if ( prop == n_value )
		return setValue( val ) ? &true_value : &false_value;

	return &undefined;
}

//---------------------------------------------------------------------------------------------------
//										XML NODE LIST
//---------------------------------------------------------------------------------------------------
visible_class_instance( XMLNodeList, "XMLNodeList" );

XMLNodeList::XMLNodeList( IXMLDOMDocument* document, IXMLDOMNodeList* nodeList )
{
	tag		= class_tag( XMLNodeList );
	_document	= document;
	_nodeList = nodeList;
	_attrList	= NULL;
	_propList	= NULL;
}

XMLNodeList::XMLNodeList(IXMLDOMDocument* document, IXMLDOMNamedNodeMap* attrList )
{
	tag		= class_tag( XMLNodeList );
	_document	= document;
	_nodeList	= NULL;
	_attrList = attrList;
	_propList	= NULL;
}

XMLNodeList::XMLNodeList(IXMLDOMDocument* document)
{
	tag		= class_tag( XMLNodeList );
	_document = document;
	_nodeList	= NULL;
	_attrList	= NULL;
	_propList = new Array(0);
}

XMLNodeList::~XMLNodeList()
{
}

//--------------------------------		C++ Methods		---------------------------------------------
bool XMLNodeList::addItem( XMLNode* item )
{
	if( isPropList() ) {
		_propList->append( item );
		return true;
	}
	return false;
}

long XMLNodeList::length()
{
	long outLength = 0;
	if( isNodeList() ) {
		_nodeList->get_length( &outLength );
	}
	else if( isAttrList() ) {
		_attrList->get_length( &outLength );
	}
	else if( isPropList() ) {
		return _propList->size;
	}
	return outLength;
}

XMLNode * XMLNodeList::getItem( int index )
{
	long nodeCount = length();
	if ( index >= nodeCount )
		return (XMLNode*) &undefined;
	else if ( index < 0 )
		throw RuntimeError( GetString( IDS_ARRAY_INDEX_MUST_BE_VE_NUMBER_GOT), Integer::intern(index) );

	IXMLDOMNode* subNode = NULL;

	if ( isNodeList() )
		_nodeList->get_item( index, &subNode );
	else if( isAttrList() )
		_attrList->get_item( index, &subNode );
	else if ( isPropList() )
		return ((XMLNode*) _propList->data[index]);

	if ( subNode ) {
		one_typed_value_local( XMLNode* out );
		vl.out = new XMLNode( subNode, document() );
		return_value( vl.out );
	}
	return (XMLNode*) &undefined;
}

XMLNode * XMLNodeList::getItem( const TCHAR * name )
{
	CComBSTR propName( name );

	long nodeCount = length();
	
	IXMLDOMNode* subNode = NULL;

	if( isNodeList() ) {
		propName.ToLower();
		IXMLDOMNode* tempNode = NULL;
		for(int i = 0; i < nodeCount; i++ ) {
			_nodeList->get_item( i, &tempNode );

			CComBSTR tempNodeName;
			tempNode->get_nodeName( &tempNodeName );
			tempNodeName.ToLower();

			if ( propName == tempNodeName ) {
				subNode = tempNode;
				break;
			}
		}
	}
	else if( isPropList() ) {
		propName.ToLower();
		for ( int i = 0; i < nodeCount; i++ ) {
			CComBSTR tempNodeName( ((XMLNode*) _propList->data[i])->nodeName() );
			tempNodeName.ToLower();
			if ( tempNodeName == propName ) {
				return_protected( (XMLNode*) _propList->data[i] );
			}
		}
	}
	else if ( isAttrList() ) {
		_attrList->getNamedItem( propName, &subNode );
	}

	if ( subNode ) {
		one_typed_value_local( XMLNode* out );
		vl.out = new XMLNode( subNode, document() );
		return_value( vl.out );
	}
	return (XMLNode*) &undefined;
}

Array * XMLNodeList::toArray()
{
	if ( isPropList() )
		return _propList;

	one_typed_value_local( Array* result );
	vl.result = new Array(0);

	long nodeCount = length();
	for ( int i = 0; i < nodeCount; i++ ) {
		IXMLDOMNode* xmlNode = NULL;

		if ( isNodeList() )
			_nodeList->get_item( i, &xmlNode );
		else if ( isAttrList() )
			_attrList->get_item( i, &xmlNode );

		if ( xmlNode ) 
			vl.result->append( new XMLNode( xmlNode, document() ) );
	}
	return vl.result;
}
//--------------------------------	 Maxscript Methods	---------------------------------------------
Value * XMLNodeList::coerce_vf( Value** arg_list, int count )
{
	return toArray();
}

void XMLNodeList::gc_trace()
{
	Value::gc_trace();
	if ( _propList ) _propList->gc_trace();
}

Value * XMLNodeList::get_vf( Value** arg_list, int count )
{
	Value* prop = arg_list[0];

	// Look Up By Index
	if ( is_number(prop) )
		return getItem( prop->to_int() - 1 );

	// Look Up By Name
	else if ( is_name(prop) || is_string(prop) )
		return getItem( prop->to_string() );

	return &undefined;
}

Value * XMLNodeList::get_names( BOOL onlyProps, BOOL onlyNodes )
{
	one_typed_value_local( Array* result );
	vl.result = new Array(0);

	long nodeCount = length();
	for ( int i = 0; i < nodeCount; i++ ) {
		IXMLDOMNode* xmlNode = NULL;
		
		if ( isNodeList() )
			_nodeList->get_item( i, &xmlNode );
		else if ( isAttrList() )
			_attrList->get_item( i, &xmlNode );
		else if ( isPropList() )
			xmlNode = ((XMLNode*) _propList->data[i])->node();

		if ( xmlNode ) {
			if ( (onlyProps && IsProperty( xmlNode )) || !onlyNodes ) {
				BSTR outString;
				xmlNode->get_nodeName(&outString);
				vl.result->append( new String( BstrToTstr( outString ) ) );
			}
			else if ( (onlyNodes && !IsProperty( xmlNode )) || !onlyProps ) {
				BSTR outString;
				xmlNode->get_nodeName(&outString);
				vl.result->append( new String( BstrToTstr( outString ) ) );
			}
		}
	}
	return_value( vl.result );
}

Value * XMLNodeList::get_nodes( BOOL onlyProps, BOOL onlyNodes )
{
	one_typed_value_local( Array* result );
	vl.result = new Array(0);

	long nodeCount = length();
	for ( int i = 0; i < nodeCount; i++ ) {
		IXMLDOMNode* xmlNode = NULL;
		
		if ( isNodeList() )
			_nodeList->get_item( i, &xmlNode );
		else if ( isAttrList() )
			_attrList->get_item( i, &xmlNode );
		else if ( isPropList() )
			xmlNode = ((XMLNode*) _propList->data[i])->node();

		if ( xmlNode ) {
			if ( (onlyProps && IsProperty( xmlNode )) || !onlyNodes ) {
				vl.result->append( new XMLNode( xmlNode, document() ) );
			}
			else if ( (onlyNodes && !IsProperty( xmlNode )) || !onlyProps ) {
				BSTR outString;
				xmlNode->get_nodeName(&outString);
				vl.result->append( new XMLNode( xmlNode, document() ) );
			}
		}
	}
	return_value( vl.result );
}

Value * XMLNodeList::get_property( Value** arg_list, int count )
{
	Value* prop = arg_list[0];

	if ( prop == n_count )		{ return Integer::intern( length() ); }

	return Value::get_property( arg_list, count );
}

Value * XMLNodeList::map(node_map &m)
{
	two_typed_value_locals( Value* node, Value* result );

	long nodeCount = length();
	for ( int i = 0; i < nodeCount; i++ ) {
		IXMLDOMNode* xmlNode = NULL;
		
		if ( isNodeList() )
			_nodeList->get_item( i, &xmlNode );
		else if ( isAttrList() )
			_attrList->get_item( i, &xmlNode );
		else if ( isPropList() )
			xmlNode = ((XMLNode*) _propList->data[i])->node();

		//--------------------------- Map Node ---------------------------
		if ( xmlNode ) {
			vl.node = new XMLNode( xmlNode, document() );
			if (m.vfn_ptr != NULL)
				vl.result = (vl.node->*(m.vfn_ptr))(m.arg_list, m.count);		
			else
			{
				// temporarily replace 1st arg with this node
				Value* arg_save = m.arg_list[0];

				m.arg_list[0] = vl.node;

				if (m.flags & NM_MXS_FN)
					vl.result = ((MAXScriptFunction*)m.cfn_ptr)->apply(m.arg_list, m.count);
				else
					vl.result = (*m.cfn_ptr)(m.arg_list, m.count);
				m.arg_list[0] = arg_save;
			}
			if (m.collection != NULL)
				m.collection->append(vl.result);
		}
	}
	pop_value_locals();
	return &ok;
}

Value * XMLNodeList::set_property( Value** arg_list, int count )
{
	return &undefined;
}

void XMLNodeList::sprin1( CharStream* s )
{
	s->puts( _T("(XMLNodeList(") );
	s->printf( _T("%d"), length() );
	s->puts( _T("))") );
}

//---------------------------------------------------------------------------------------------------
//										BLURXML LIBRARY
//---------------------------------------------------------------------------------------------------

def_struct_primitive	( addAttribute,			blurXML,		"addAttribute");
def_struct_primitive	( addNode,				blurXML,		"addNode");
def_struct_primitive	( addProperty,			blurXML,		"addProperty");
def_struct_primitive	( addXMLText,			blurXML,		"addXMLText");
def_struct_primitive	( getAttribute,			blurXML,		"getAttribute");
def_struct_primitive	( getAttributeNames,	blurXML,		"getAttributeNames");
def_struct_primitive	( getAttributeNodes,	blurXML,		"getAttributeNodes");
def_struct_primitive	( getProperty,			blurXML,		"getProperty");
def_struct_primitive	( getPropertyNames,		blurXML,		"getPropertyNames");
def_struct_primitive	( getPropertyNodes,		blurXML,		"getPropertyNodes");
def_struct_primitive	( getPropertiesByName,	blurXML,		"getPropertiesByName");
def_struct_primitive	( getNode,				blurXML,		"getNode");
def_struct_primitive	( getNodes,				blurXML,		"getNodes");
def_struct_primitive	( getNodeNames,			blurXML,		"getNodeNames");
def_struct_primitive	( getNodesByName,		blurXML,		"getNodesByName");
def_struct_primitive	( loadDocument,			blurXML,		"loadDocument");
def_struct_primitive	( newDocument,			blurXML,		"newDocument");
def_struct_primitive	( saveDocument,			blurXML,		"saveDocument");

//---------------------------------------------------------------------------------------------------
Value * addAttribute_cf( Value** arg_list, int count )
{
	check_arg_count( blurXML.addAttribute, 3, count );

	// Check for Types
	XMLNode* nodeValue = (XMLNode*) arg_list[0];
	if (!is_xmlnode(nodeValue)) return &false_value;

	// Add Attribute
	return ( nodeValue->setAttribute( arg_list[1]->to_string(), arg_list[2] ) ) ? &true_value : &false_value;
}

Value * addNode_cf( Value** arg_list, int count )
{
	check_arg_count_with_keys( blurXML.addNode, 2, count );

	// Check to see if the first argument is an XMLDocument
	XMLDocument* docValue = (XMLDocument*) arg_list[0];
	if ( is_xmldocument(docValue) ) {
		XMLNode* parentValue = (XMLNode*) (key_arg_or_default(parent, &undefined));
		if ( !is_xmlnode(parentValue) )
			parentValue = docValue->root();

		// Create New Node
		return parentValue->addNode( arg_list[1]->to_string() );
	}
	// Check to see if the first argument is an XMLNode
	XMLNode* nodeValue = (XMLNode*) arg_list[0];
	if ( is_xmlnode( nodeValue ) ) {
		return nodeValue->addNode( arg_list[1]->to_string() );
	}
	return &undefined;
}

Value * addProperty_cf( Value** arg_list, int count )
{
	check_arg_count( blurXML.addProperty, 3, count );

	// Check for Types
	XMLNode* nodeValue = (XMLNode*) arg_list[0];
	if ( !is_xmlnode(nodeValue) ) return &false_value;

	return ( nodeValue->setProperty( arg_list[1]->to_string(), arg_list[2] ) ) ? &true_value : &false_value;
}

Value * addXMLText_cf( Value** arg_list, int count )
{
	check_arg_count( blurXML.addXMLText, 3, count );

	// Check for Types
	XMLDocument* docValue = (XMLDocument*) arg_list[0];
	if ( !is_xmldocument(docValue) ) return &false_value;

	XMLNode* nodeValue = (XMLNode*) arg_list[1];
	if ( !is_xmlnode(nodeValue) ) return &false_value;

	return ( nodeValue->setText( arg_list[2]->to_string() ) ) ? &true_value : &false_value;
}

Value * getAttribute_cf( Value** arg_list, int count )
{
	check_arg_count( blurXML.getAttribute, 2, count );

	XMLNode* nodeValue = (XMLNode*) arg_list[0];
	if ( !is_xmlnode(nodeValue) )
		return &undefined;
	return nodeValue->attributeByName( arg_list[1]->to_string() );
}

Value * getAttributeNames_cf( Value** arg_list, int count )
{
	check_arg_count( blurXML.getAttributeNames, 1, count );
	XMLNode* nodeValue = (XMLNode*) arg_list[0];
	if ( !is_xmlnode(nodeValue) )
		return new Array(0);
	return ( nodeValue->attributes()->get_names() );
}

Value * getAttributeNodes_cf( Value** arg_list, int count )
{
	check_arg_count( blurXML.getAttributeNodes, 1, count );
	XMLNode* nodeValue = (XMLNode*) arg_list[0];
	if ( !is_xmlnode(nodeValue) )
		return new Array(0);
	return ( nodeValue->attributes()->get_nodes() );
}

Value * getProperty_cf( Value** arg_list, int count )
{
	check_arg_count( blurXML.getProperty, 2, count );
	XMLNode* nodeValue = (XMLNode*) arg_list[0];
	if ( !is_xmlnode(nodeValue) )
		return &undefined;
	return (nodeValue->propertyByName( arg_list[1]->to_string() ));
}

Value * getPropertyNames_cf( Value** arg_list, int count )
{
	check_arg_count( blurXML.getPropertyNames, 1, count );
	XMLNode* nodeValue = (XMLNode*) arg_list[0];
	if ( !is_xmlnode(nodeValue) )
		return new Array(0);
	return ( nodeValue->properties()->get_names() );
}

Value * getPropertyNodes_cf( Value** arg_list, int count )
{
	check_arg_count( blurXML.getPropertyNodes, 1, count );
	XMLNode* nodeValue = (XMLNode*) arg_list[0];
	if ( !is_xmlnode(nodeValue) )
		return new Array(0);
	return ( nodeValue->children()->get_nodes( TRUE ) );
}

Value * getPropertiesByName_cf( Value** arg_list, int count )
{
	check_arg_count( blurXML.getPropertiesByName, 2, count );
	XMLNode* nodeValue = (XMLNode*) arg_list[0];
	if ( !is_xmlnode(nodeValue) )
		return new Array(0);
	return ( nodeValue->propertiesByName( arg_list[1]->to_string() ) );
}

Value * getNode_cf( Value** arg_list, int count )
{
	check_arg_count( blurXML.getNode, 2, count );
	XMLNode* nodeValue = (XMLNode*) arg_list[0];
	if ( !is_xmlnode(nodeValue) )
		return &undefined;
	return ( nodeValue->childByName( arg_list[1]->to_string() ) );
}

Value * getNodes_cf( Value** arg_list, int count )
{
	check_arg_count( blurXML.getNodes, 1, count );
	XMLNode* nodeValue = (XMLNode*) arg_list[0];
	if ( !is_xmlnode(nodeValue) )
		return new Array(0);
	return ( nodeValue->children()->get_nodes( FALSE, TRUE ) );
}

Value * getNodeNames_cf( Value** arg_list, int count )
{
	check_arg_count( blurXML.getNodeNames, 1, count );
	XMLNode* nodeValue = (XMLNode*) arg_list[0];
	if ( !is_xmlnode(nodeValue) )
		return new Array(0);
	return (nodeValue->children()->get_names( FALSE, TRUE ));
}

Value * getNodesByName_cf( Value** arg_list, int count )
{
	check_arg_count( blurXML.getNodesByName, 2, count );
	XMLNode* nodeValue = (XMLNode*) arg_list[0];
	if ( !is_xmlnode(nodeValue) )
		return new Array(0);
	return ( nodeValue->childrenByName( arg_list[1]->to_string() ) );
}

Value * loadDocument_cf( Value** arg_list, int count )
{
	check_arg_count_with_keys( blurXML.loadDocument, 1, count );
	one_typed_value_local( XMLDocument* out );
	vl.out = new XMLDocument();
	if ( vl.out->loadFromFile( arg_list[0]->to_string() ) ) {
		return_value( vl.out );
	}
	pop_value_locals();
	return &undefined;
}

Value * newDocument_cf( Value** arg_list, int count )
{
	check_arg_count( blurXML.newDocument, 0, count );
	one_typed_value_local( XMLDocument* out );
	vl.out = new XMLDocument();

	if ( vl.out->isValid() )
		return_value( vl.out );

	pop_value_locals();
	return &undefined;
}

Value * saveDocument_cf( Value** arg_list, int count )
{
	check_arg_count( blurXML.saveDocument, 2, count );
	XMLDocument* docValue = (XMLDocument*) arg_list[0];
	if ( !is_xmldocument(docValue) )
		return &false_value;
	return ( docValue->saveToFile( arg_list[1]->to_string()) ) ? &true_value : &false_value;
}

//---------------------------------------------------------------------------------------------------
//										INITIALIZE BLURXML
//---------------------------------------------------------------------------------------------------
void BlurXMLInit() { }
