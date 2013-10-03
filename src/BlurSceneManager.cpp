
#include "imports.h"

#include "maxapi.h"

#include "ilayermanager.h"		// Contains a pure virtual class. 
#include "iLayer.h"				// Contains a pure virtual class.
#include "ILayerProperties.h"	// Contains two function published class's that 
								// are the core for the maxscript exposure

#include "resource.h"

#include <time.h>
#include <windows.h>

#define TABLEOFCONTENTS_VERSION		2.0f

#include "BlurSceneManager.h"
#include "BlurFile.h"

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

#define		n_propertiesByName		(Name::intern(_T("propertiesByName")))
#define		n_document				(Name::intern(_T("document")))
#define		n_layers				(Name::intern(_T("layers")))
#define		n_loadFromFile			(Name::intern(_T("loadFromFile")))
#define		n_maxfile				(Name::intern(_T("maxfile")))
#define		n_nodes					(Name::intern(_T("nodes")))
#define		n_nodesByProperty		(Name::intern(_T("nodesByProperty")))
#define		n_saveToFile			(Name::intern(_T("saveToFile")))

//---------------------------------------------------------------------------------------------------
//										TABLE OF CONTENTS
//---------------------------------------------------------------------------------------------------
TableOfContents::TableOfContents()
{
	this->_document		= NULL;
	this->_toc			= NULL;
	this->_version		= TABLEOFCONTENTS_VERSION;
	this->_loadVersion	= 0.0f;
}

TableOfContents::~TableOfContents()
{}

Array * TableOfContents::_nodes(XMLNode* xmlNode)
{
	Array* out = new Array(0);
	out->append( xmlNode->attributeByName( _T("name") )->value() );
	XMLNodeList* children= xmlNode->childByName( _T("children") )->children();
	int count = children->length();
	for ( int i = 0; i < count; i++ ) {
		out->join( _nodes( children->getItem(i) ) );
	}
	return out;
}

Array * TableOfContents::_nodesByProperty( TSTR propName, CComBSTR propVal, XMLNode* xmlNode )
{
	Array* out = new Array(0);
	XMLNodeList * childList = xmlNode->children();
	int childCount = childList->length();
	for ( int i = 0; i < childCount; i++ ) {
		XMLNode* objectNode = childList->getItem(i);
		XMLNode* propNode	= objectNode->propertyByName( propName );
		if ( propNode ) {
			CComBSTR propNodeVal = CComBSTR( propNode->value()->to_string() );
			propNodeVal.ToLower();

			if ( propNodeVal == propVal )
				out->append( objectNode->attributeByName( _T("name") )->value() );
		}
		out->join( _nodesByProperty( propName, propVal, objectNode->childByName( _T("children") ) ) );
	}
	return out;
}

Array * TableOfContents::_propertiesByName( TSTR propName, XMLNode* xmlNode )
{
	Array* out = new Array(0);
	XMLNodeList* childList	= xmlNode->children();
	int childCount = childList->length();
	for ( int i = 0; i < childCount; i++ ) {
		XMLNode* objectNode	= childList->getItem(i);
		XMLNode* propNode	= objectNode->propertyByName( propName );
		if ( propNode ) {
			out->append( Name::intern( propNode->value()->to_string() ) );
		}
		out->join( _propertiesByName( propName, objectNode->childByName( _T("children") ) ) );
	}
	return out;
}

Array * TableOfContents::layers()
{
	one_typed_value_local( Array* out );
	vl.out = new Array(0);

	if ( isInitialized() ) {
		XMLNode* layersNode = toc()->childByName( _T("layers") );
		if ( layersNode ) {
			XMLNodeList* layerList	= layersNode->children();
			int layerCount			= layerList->length();
			for (int i = 0; i < layerCount; i++ ) {
				vl.out->append( layerList->getItem(i)->attributeByName( _T("name") )->value() );
			}
		}
	}

	return_value(vl.out);
}

bool TableOfContents::loadFromFile( TSTR fileName )
{
	_toc			= NULL;
	_document		= new XMLDocument();
	if ( _document->loadFromFile( fileName ) ) {
		_toc = _document->root()->childByName( _T("toc") );
		if ( _toc ) {
			// Check Version Compatibility

			XMLNode* versionAttr = _toc->attributeByName( _T("version") );
			if ( versionAttr != (XMLNode*) &undefined ) {
				_loadVersion	= (float) _tstof( versionAttr->value()->to_string() );
				if ( _loadVersion >= _version ) {
					return true;
				}
				else {
					// Support Previous Versions
					mprintf( _T("TOC Version is not supported (fileVersion:%0.1f currVersion:%0.1f)\n"), _loadVersion, _version );
				}
			}
			else {
				mprintf( _T("TOC Version is not found, and is not supported.\n") );
			}
		}
	}
	_document = NULL;
	_toc = NULL;
	return false;
}

Array * TableOfContents::nodes() {
	one_typed_value_local( Array* out );
	vl.out = new Array(0);
	if ( isInitialized() ) {
		XMLNode* rootNode = toc()->childByName( _T("objects") );
		if ( rootNode ) {
			XMLNode* sceneRoot = rootNode->childByName( _T("object") );
			if ( sceneRoot ) {
				XMLNodeList* children = sceneRoot->childByName( _T("children") )->children();
				int count = children->length();
				for ( int i = 0; i < count; i++ ) {
					vl.out->join( _nodes( children->getItem(i) ) );
				}
			}
		}
	}
	return_value( vl.out );
}

Array * TableOfContents::nodesByProperty( TSTR propName, TSTR propValue )
{
	one_typed_value_local( Array* out );
	vl.out = new Array(0);

	if ( isInitialized() ) {
		CComBSTR propVal = CComBSTR(propValue);
		propVal.ToLower();
		push_alloc_frame();
		XMLNode* objectsNode = toc()->childByName( _T("objects") );
		if ( objectsNode )
			vl.out = _nodesByProperty( propName, propVal, objectsNode );
		pop_alloc_frame();
	}

	return_value( vl.out );
}

Array * TableOfContents::propertiesByName( TSTR propName ) {
	one_typed_value_local( Array* out );
	vl.out = new Array(0);

	// Collect Properties
	if ( isInitialized() ) {
		push_alloc_frame();
		XMLNode* objectsNode = toc()->childByName( _T("objects") );
		if ( objectsNode ) {
			XMLNode* rootNode = objectsNode->childByName( _T("object") );

			// Ignore the Root Max Node
			if ( rootNode ) {
				objectsNode		= rootNode->childByName( _T("children") );
				// Remove Duplicate Properties
				Array* dupProps = _propertiesByName( propName, objectsNode );

				bool found = false;
				for (int i = 0; i < dupProps->size; i++ ) {
					Value* prop = dupProps->data[i];
					found = false;
					for (int c = 0; c < vl.out->size; c++ ) {
						if ( vl.out->data[c] == prop ) {
							found = true;
							break;
						}
					}

					if ( !found )
						vl.out->append( prop );
				}
			}
		}
		pop_alloc_frame();
	}

	return_value( vl.out );
}

void TableOfContents::recordClassInfo( INode* node, XMLNode* xmlNode ) {
	Value * tempNode = MAXNode::intern( node );
	StringStream* classStr = new StringStream();
	StringStream* superClassStr = new StringStream();
	
	tempNode->classOf_vf(NULL,0)->sprin1(classStr);
	tempNode->superClassOf_vf(NULL,0)->sprin1(superClassStr);

	xmlNode->setProperty( _T("class"), TSTR( classStr->to_string() ) );
	xmlNode->setProperty( _T("superclass"), TSTR( superClassStr->to_string() ) );

	DeleteObject( classStr );
	DeleteObject( superClassStr );
}

void TableOfContents::recordLayerInfo( TimeValue atTime )
{
	push_alloc_frame();

	IFPLayerManager* lm = static_cast<IFPLayerManager*>(GetCOREInterface(LAYERMANAGER_INTERFACE));
	ILayerManager* LM	= (ILayerManager*) GetCOREInterface()->GetScenePointer()->GetReference(10);

	int layerCount		= lm->getCount();

	// Create Layers Parent Node
	XMLNode* layersNode = toc()->addNode( _T("layers") );
	layersNode->setAttribute( _T("count"), layerCount );

	for ( int i = 0; i < layerCount; i ++ ) {
		// Get a pointer to the layer
		ILayerProperties * hLayer = lm->getLayer(i);
		ILayer * layer = LM->GetLayer(hLayer->getName());

		XMLNode * layerNode = layersNode->addNode( _T("layer") );

		// Record Layer Attribute Info
		layerNode->setAttribute( _T("name"), TSTR(hLayer->getName()) );
		layerNode->setAttribute( _T("index"), i );

		// Record Layer Property Info
		layerNode->setProperty( _T("on"), hLayer->getOn() ? &true_value : &false_value );
		layerNode->setProperty( _T("lock"), hLayer->getLock() ? &true_value : &false_value );
		layerNode->setProperty( _T("current"), hLayer->getCurrent() ? &true_value : &false_value );
		layerNode->setProperty( _T("wirecolor"), hLayer->getWireColor() );
		layerNode->setProperty( _T("isGIExcluded"), hLayer->getGIIsExcluded() ? &true_value : &false_value );
		layerNode->setProperty( _T("renderable"), layer->Renderable() );
		layerNode->setProperty( _T("inheritVisibility"), layer->GetInheritVisibility() );
		layerNode->setProperty( _T("primaryVisibilitY"), layer->GetPrimaryVisibility() );
		layerNode->setProperty( _T("secondaryVisibility"), layer->GetSecondaryVisibility() );
		layerNode->setProperty( _T("recieveShadows"), layer->RcvShadows() );
		layerNode->setProperty( _T("castShadows"), layer->CastShadows() );
		layerNode->setProperty( _T("applyAtmopherics"), layer->ApplyAtmospherics() );
		layerNode->setProperty( _T("renderOccluded"), layer->GetRenderOccluded() ? &true_value : &false_value );
		layerNode->setProperty( _T("ishidden"), layer->IsHidden() );
		layerNode->setProperty( _T("isfrozen"), layer->IsFrozen() );
		layerNode->setProperty( _T("boxmode"), layer->GetBoxMode() );
		layerNode->setProperty( _T("backfacecull"), layer->GetBackCull() );
		layerNode->setProperty( _T("alledges"), layer->GetAllEdges() );
		layerNode->setProperty( _T("vertexTicks"), layer->GetVertTicks() );
		layerNode->setProperty( _T("showTrajectory"), layer->GetTrajectory() );
		layerNode->setProperty( _T("xray"), layer->HasXRayMtl() );
		layerNode->setProperty( _T("ignoreExtents"), layer->GetIgnoreExtents() );
		layerNode->setProperty( _T("showFrozenInGray"), layer->ShowFrozenWithMtl() ? &false_value : &true_value );
		layerNode->setProperty( _T("vertexColorsShaded"), layer->GetShadeCVerts() );
		layerNode->setProperty( _T("visibility"), layer->GetVisibility(atTime) );
		layerNode->setProperty( _T("imageMotionBlurMultiplier"), layer->GetImageBlurMultiplier(atTime) );
		layerNode->setProperty( _T("motionBlurOn"), layer->GetMotBlurOnOff(atTime) );
		layerNode->setProperty( _T("motionblur"), layer->MotBlur() );
		layerNode->setProperty( _T("display"), layer->GetDisplayFlags() );
	}

	pop_alloc_frame();
}

void TableOfContents::recordNodeInfo( INode* node, XMLNode* xmlParent, TimeValue atTime )
{
	XMLNode* xmlNode = xmlParent->addNode( _T("object") );
	xmlNode->setAttribute( _T("name"),TSTR(node->GetName()) );

	// Object Class and SuperClass
	recordClassInfo( node, xmlNode );

	// Object Layer
	ILayer* nodeLayer = (ILayer*) node->GetReference(NODE_LAYER_REF);
	xmlNode->setProperty( _T("layer"), nodeLayer ? TSTR(nodeLayer->GetName()) : TSTR(_T("undefined")) );

	// Object Transform
	TSTR transformStr = _T("(matrix3 ");

	Matrix3 nodeTM = node->GetNodeTM(atTime);
	for ( int i = 0; i < 4; i++ ) {
		TSTR rowStr = _T("");
		Point3 row = nodeTM.GetRow(i);

		rowStr.printf( _T("[%f, %f, %f] "), row.x, row.y, row.z );
		transformStr += rowStr;
	}
	transformStr += _T( ")" );
	xmlNode->setProperty( _T("transform"), transformStr );

	//------------------------------------------------------------------------------------

	// Recurse through children nodes
	XMLNode* childrenXMLNode = xmlNode->addNode( _T("children") );
	int numChildren = node->NumberOfChildren();
	for ( int i = 0; i < numChildren; i++ )
		recordNodeInfo( node->GetChildNode(i), childrenXMLNode );
}

void TableOfContents::recordUserInfo()
{
	//-------------------------------		Calc Local Time			----------------------------
	// time
	struct tm *newtime;
	time_t aclock;

	time( &aclock );
	newtime = localtime(&aclock);

	// date
	TSTR today = _tasctime(newtime);		// The data string has a \n appended
	today.remove(today.length()-1);			// Remove the \n

	//-------------------------------	Record User Information		----------------------------
	// Record UserName
	TCHAR username[MAX_PATH];
	DWORD namesize = MAX_PATH;
	GetUserName( username, &namesize );
	toc()->setProperty( _T("user"), TSTR(username) );

	// Record ComputerName
	TCHAR computername[ MAX_COMPUTERNAME_LENGTH+1 ];
	namesize = MAX_COMPUTERNAME_LENGTH+1;
	GetComputerName( computername, &namesize );
	toc()->setProperty( _T("computer"), TSTR(computername) );

	// Record Localtime
	toc()->setProperty( _T("localtime"), today );

	// Record MAXVersion
	TCHAR maxversion[MAX_PATH];
	_tprintf( maxversion, _T("%d"), Get3DSMAXVersion() );
	toc()->setProperty( _T("maxversion"), TSTR(maxversion) );
}

bool TableOfContents::saveToFile( TSTR fileName ) {
	_document = NULL;
	_toc = NULL;

	if ( fileName.length() > 0 ) {
		_document = new XMLDocument();
		if ( _document && _document->isValid() ) {
			// Record how long it takes to save the TableOfContents
			time_t start, finish;
			int elapsed_time;
			time( &start );

			// Record the root node
			_toc = _document->addNode( _T("toc") );

			Interface* ip		= GetCOREInterface();
			toc()->setAttribute( _T("filename"), ip->GetCurFilePath() + ip->GetCurFileName() );
			toc()->setAttribute( _T("version"), _version );

			TimeValue currTime = GetCOREInterface()->GetTime();
			toc()->setProperty( _T("currentFrame"), currTime );

			//-----------------			Record Data		------------------------
			recordUserInfo();
			recordLayerInfo();

			XMLNode* nodesNode = toc()->addNode( _T("objects") );
			recordNodeInfo( GetCOREInterface()->GetRootNode(), nodesNode, currTime );

			//-----------------							------------------------

			time( &finish );
			elapsed_time = difftime( finish, start );
			toc()->setProperty( _T("timeToCalculateTOC"), elapsed_time );

			// Save to File
			return _document->saveToFile( fileName );
		}
	}
	return false;
}

//------------------------------------	MAXScript Methods	-----------------------------------------
void TableOfContents::sprin1(CharStream* s)
{
	s->puts( _T("(TableOfContents())") );
}

void TableOfContents::gc_trace()
{
	Value::gc_trace();

	if ( _document && _document->is_not_marked() )
		_document->gc_trace();
	if ( _toc && _toc->is_not_marked() )
		_toc->gc_trace();
}

Value * TableOfContents::applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc )
{
	if ( methodID == n_propertiesByName ) {
		check_arg_count( propertiesByName, 1, count );
		return propertiesByName( arg_list[0]->eval()->to_string() );
	}
	else if ( methodID == n_loadFromFile ) {
		check_arg_count( loadFromFile, 1, count );
		return loadFromFile( arg_list[0]->eval()->to_string() ) ? &true_value : &false_value;
	}
	else if ( methodID == n_nodesByProperty ) {
		check_arg_count( nodesByProperty, 2, count );
		return nodesByProperty( arg_list[0]->eval()->to_string(),arg_list[1]->eval()->to_string() );
	}
	else if ( methodID == n_saveToFile ) {
		check_arg_count( saveToFile, 1, count );
		return saveToFile( arg_list[0]->eval()->to_string() ) ? &true_value : &false_value;
	}
	return &ok;
}

Value * TableOfContents::get_property( Value **arg_list, int count )
{
	Value* prop = arg_list[0];

	if ( prop == n_propertiesByName )
		return NEW_GENERIC_METHOD( propertiesByName );
	else if ( prop == n_document )
		return _document;
	else if ( prop == n_layers )
		return layers();
	else if ( prop == n_loadFromFile )
		return NEW_GENERIC_METHOD( loadFromFile );
	else if ( prop == n_nodes )
		return nodes();
	else if ( prop == n_nodesByProperty )
		return NEW_GENERIC_METHOD( nodesByProperty );
	else if ( prop == n_saveToFile )
		return NEW_GENERIC_METHOD( saveToFile );
	else if ( prop == n_version )
		return Float::intern( _version );

	return Value::get_property( arg_list, count );
}

Value * TableOfContents::set_property( Value** arg_list, int count )
{
	return Value::set_property( arg_list, count );
}

//---------------------------------------------------------------------------------------------------
//										  C++ METHODS
//---------------------------------------------------------------------------------------------------

TSTR TocFileFromMaxFile( const TSTR & maxfilename )
{
	TSTR out;
	if ( maxfilename.Length() > 0 ) {
		Array* pathList		= splitpath( maxfilename );
		TSTR pathPart( pathList->data[0]->to_string() );
		TSTR filePart( pathList->data[1]->to_string() );
		out = pathPart + _T(".meta/") + filePart + _T(".toc");
	}
	return out;
}

//---------------------------------------------------------------------------------------------------
//									BLURSCENEMANAGER LIBRARY
//---------------------------------------------------------------------------------------------------

def_struct_primitive(	loadToc,		blurSceneManager,		"loadToc" );
def_struct_primitive(	maxTocFile,		blurSceneManager,		"maxTocFile" );
def_struct_primitive(	saveToc,		blurSceneManager,		"saveToc" );
def_struct_primitive(	toc,			blurSceneManager,		"toc" );

//---------------------------------------------------------------------------------------------------

Value * loadToc_cf( Value** arg_list, int count )
{
	check_arg_count_with_keys( blurSceneManager.loadToc, 0, count );

	Value * fileName = key_arg( maxfile );
	TSTR tocFile;
	if ( fileName != &unsupplied )
		tocFile = TocFileFromMaxFile( fileName->eval()->to_string() );
	else
		tocFile = TocFileFromMaxFile( GetCOREInterface()->GetCurFilePath() );

	one_typed_value_local( TableOfContents* out );
	vl.out = new TableOfContents();

	if ( vl.out->loadFromFile( tocFile ) )
		return_value( vl.out );

	pop_value_locals();
	return &undefined;
}

Value * maxTocFile_cf( Value** arg_list, int count )
{
	check_arg_count_with_keys( blurSceneManager.tocFileName, 0, count );

	Value* fileName = key_arg( maxfile );
	if ( fileName != &unsupplied )
		return new String( TocFileFromMaxFile( fileName->eval()->to_string() ) );
	else
		return new String( TocFileFromMaxFile( GetCOREInterface()->GetCurFilePath() ) );
}

Value * saveToc_cf( Value** arg_list, int count )
{
	check_arg_count( blurSceneManager.saveTOC, 0, count );

	Value* result = &false_value;
	if ( createMetapath() ) {
		one_typed_value_local( TableOfContents* temp );
		vl.temp = new TableOfContents();
		result = vl.temp->saveToFile( TocFileFromMaxFile( GetCOREInterface()->GetCurFilePath() ) ) ? &true_value : &false_value;
		pop_value_locals();
	}
	return result;
}

Value * toc_cf( Value** arg_list, int count )
{
	check_arg_count( blurSceneManager.toc, 0, count );
	one_value_local( out );
	vl.out = new TableOfContents();
	return_value( vl.out );
}

//---------------------------------------------------------------------------------------------------

void BlurSceneManagerInit() {}
