#include "MAXScrpt.h"
#include "MAXObj.h"
#include "maxapi.h"
#include "Numbers.h"
#include "Strings.h"

#include "ilayermanager.h"		// Contains a pure virtual class. 
#include "iLayer.h"				// Contains a pure virtual class.
#include "ILayerProperties.h"	// Contains two function published class's that 
								// are the core for the maxscript exposure

#include "resource.h"

#include <time.h>
#include <windows.h>

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#define			TABLEOFCONTENTS_VERSION		2.0f

#include "BlurSceneManager.h"
#include "BlurFile.h"

#include "defextfn.h"
#include "definsfn.h"

//---------------------------------------------------------------------------------------------------
//										GLOBAL NAMES
//---------------------------------------------------------------------------------------------------

#define			n_propertiesByName		(Name::intern(_T("propertiesByName")))
#define			n_document				(Name::intern(_T("document")))
#define			n_layers				(Name::intern(_T("layers")))
#define			n_loadFromFile			(Name::intern(_T("loadFromFile")))
#define			n_maxfile				(Name::intern(_T("maxfile")))
#define			n_nodes					(Name::intern(_T("nodes")))
#define			n_nodesByProperty		(Name::intern(_T("nodesByProperty")))
#define			n_saveToFile			(Name::intern(_T("saveToFile")))

//---------------------------------------------------------------------------------------------------
//										TABLE OF CONTENTS
//---------------------------------------------------------------------------------------------------
TableOfContents::TableOfContents() {
	this->_document		= NULL;
	this->_toc			= NULL;
	this->_version		= TABLEOFCONTENTS_VERSION;
	this->_loadVersion	= 0.0f;
}
TableOfContents::~TableOfContents() {
}
Array*		TableOfContents::_nodes(XMLNode* xmlNode) {
	Array* out				= new Array(0);
	out->append( xmlNode->attributeByName( "name" )->value() );
	XMLNodeList* children	= xmlNode->childByName( "children" )->children();
	int count				= children->length();
	for ( int i = 0; i < count; i++ ) {
		out->join( this->_nodes( children->getItem(i) ) );
	}
	return out;
}
Array*		TableOfContents::_nodesByProperty( TSTR propName, CComBSTR propVal, XMLNode* xmlNode ) {
	Array* out				= new Array(0);
	XMLNodeList* childList	= xmlNode->children();
	int childCount			= childList->length();
	for ( int i = 0; i < childCount; i++ ) {
		XMLNode* objectNode = childList->getItem(i);
		XMLNode* propNode	= objectNode->propertyByName( propName );
		if ( propNode ) {
			
			CComBSTR propNodeVal = CComBSTR( propNode->value()->to_string() );
			propNodeVal.ToLower();

			if ( propNodeVal == propVal )
				out->append( objectNode->attributeByName( "name" )->value() );
		}
		out->join( this->_nodesByProperty( propName, propVal, objectNode->childByName( "children" ) ) );
	}
	return out;
}
Array*		TableOfContents::_propertiesByName( TSTR propName, XMLNode* xmlNode ) {
	Array* out				= new Array(0);
	XMLNodeList* childList	= xmlNode->children();
	int childCount			= childList->length();
	for ( int i = 0; i < childCount; i++ ) {
		XMLNode* objectNode	= childList->getItem(i);
		XMLNode* propNode	= objectNode->propertyByName( propName );
		if ( propNode ) {
			out->append( Name::intern( propNode->value()->to_string() ) );
		}
		out->join( this->_propertiesByName( propName, objectNode->childByName( "children" ) ) );
	}
	return out;
}
Array*		TableOfContents::layers() {
	one_typed_value_local( Array* out );
	vl.out = new Array(0);

	if ( this->isInitialized() ) {
		XMLNode* layersNode = this->toc()->childByName( "layers" );
		if ( layersNode ) {
			XMLNodeList* layerList	= layersNode->children();
			int layerCount			= layerList->length();
			for (int i = 0; i < layerCount; i++ ) {
				vl.out->append( layerList->getItem(i)->attributeByName( "name" )->value() );
			}
		}
	}

	return_value(vl.out);
}
bool		TableOfContents::loadFromFile( TSTR fileName ) {
	this->_document		= NULL;
	this->_toc			= NULL;

	this->_document		= new XMLDocument();
	if ( this->_document->loadFromFile( fileName ) ) {
		this->_toc		= this->_document->root()->childByName( "toc" );
		if ( this->_toc ) { 
			// Check Version Compatibility

			XMLNode* versionAttr	= this->_toc->attributeByName( "version" );
			if ( versionAttr != (XMLNode*) &undefined ) {
				this->_loadVersion	= (float) atof( versionAttr->value()->to_string() );
				if ( this->_loadVersion >= this->_version ) {
					return true;
				}
				else {
					// Support Previous Versions
					mprintf( "TOC Version is not supported (fileVersion:%0.1f currVersion:%0.1f)\n", this->_loadVersion, this->_version );
				}
			}
			else {
				mprintf( "TOC Version is not found, and is not supported.\n" );
			}
		}
	}
	this->_document		= NULL;
	this->_toc			= NULL;
	return false;
}
Array*		TableOfContents::nodes() {
	one_typed_value_local( Array* out );
	vl.out = new Array(0);
	if ( this->isInitialized() ) {
		XMLNode* rootNode = this->toc()->childByName( "objects" );
		if ( rootNode ) {
			XMLNode* sceneRoot = rootNode->childByName( "object" );
			if ( sceneRoot ) {
				XMLNodeList* children	= sceneRoot->childByName( "children" )->children();
				int count				= children->length();
				for ( int i = 0; i < count; i++ ) {
					vl.out->join( this->_nodes( children->getItem(i) ) );
				}
			}
		}
	}
	return_value( vl.out );
}
Array*		TableOfContents::nodesByProperty( TSTR propName, TSTR propValue ) {
	one_typed_value_local( Array* out );
	vl.out = new Array(0);

	if ( this->isInitialized() ) {
		CComBSTR propVal = CComBSTR(propValue);
		propVal.ToLower();
		push_alloc_frame();
		XMLNode* objectsNode = this->toc()->childByName( "objects" );
		if ( objectsNode )
			vl.out = this->_nodesByProperty( propName, propVal, objectsNode );
		pop_alloc_frame();
	}

	return_value( vl.out );
}
Array*		TableOfContents::propertiesByName( TSTR propName ) {
	one_typed_value_local( Array* out );
	vl.out = new Array(0);

	// Collect Properties
	if ( this->isInitialized() ) {
		push_alloc_frame();
		XMLNode* objectsNode = this->toc()->childByName( "objects" );
		if ( objectsNode ) {
			XMLNode* rootNode = objectsNode->childByName( "object" );

			// Ignore the Root Max Node
			if ( rootNode ) {
				objectsNode		= rootNode->childByName( "children" );
				// Remove Duplicate Properties
				Array* dupProps = this->_propertiesByName( propName, objectsNode );

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
void		TableOfContents::recordClassInfo( INode* node, XMLNode* xmlNode ) {
	Value* tempNode				= MAXNode::intern( node );
	StringStream* classStr		= new StringStream();
	StringStream* superClassStr	= new StringStream();
	
	tempNode->classOf_vf(NULL,0)->sprin1(classStr);
	tempNode->superClassOf_vf(NULL,0)->sprin1(superClassStr);

	xmlNode->setProperty( "class",		TSTR( classStr->to_string() ) );
	xmlNode->setProperty( "superclass", TSTR( superClassStr->to_string() ) );

	DeleteObject( classStr );
	DeleteObject( superClassStr );
}
void		TableOfContents::recordLayerInfo( TimeValue atTime ) {
	push_alloc_frame();

	IFPLayerManager* lm = static_cast<IFPLayerManager*>(GetCOREInterface(LAYERMANAGER_INTERFACE));
	ILayerManager* LM	= (ILayerManager*) GetCOREInterface()->GetScenePointer()->GetReference(10);

	int layerCount		= lm->getCount();

	// Create Layers Parent Node
	XMLNode* layersNode	= this->toc()->addNode( "layers" );
	layersNode->setAttribute( "count", layerCount );

	for ( int i = 0; i < layerCount; i ++ ) {
		// Get a pointer to the layer
		ILayerProperties*	hLayer		= lm->getLayer(i);
		ILayer*				layer		= LM->GetLayer(hLayer->getName());

		XMLNode*			layerNode	= layersNode->addNode( "layer" );

		// Record Layer Attribute Info
		layerNode->setAttribute( "name",	TSTR( hLayer->getName() ) );
		layerNode->setAttribute( "index",	i );

		// Record Layer Property Info
		layerNode->setProperty( "on",							(hLayer->getOn())			? &true_value : &false_value );
		layerNode->setProperty( "lock",							(hLayer->getLock())			? &true_value : &false_value );
		layerNode->setProperty( "current",						(hLayer->getCurrent())		? &true_value : &false_value );
		layerNode->setProperty( "wirecolor",					hLayer->getWireColor() );
		layerNode->setProperty( "isGIExcluded",					(hLayer->getGIIsExcluded()) ? &true_value : &false_value );
		layerNode->setProperty( "renderable",					layer->Renderable() );
		layerNode->setProperty( "inheritVisibility",			layer->GetInheritVisibility() );
		layerNode->setProperty( "primaryVisibilitY",			layer->GetPrimaryVisibility() );
		layerNode->setProperty( "secondaryVisibility",			layer->GetSecondaryVisibility() );
		layerNode->setProperty( "recieveShadows",				layer->RcvShadows() );
		layerNode->setProperty( "castShadows",					layer->CastShadows() );
		layerNode->setProperty( "applyAtmopherics",				layer->ApplyAtmospherics() );
		layerNode->setProperty( "renderOccluded",				(layer->GetRenderOccluded()) ? &true_value : &false_value );
		layerNode->setProperty( "ishidden",						layer->IsHidden() );
		layerNode->setProperty( "isfrozen",						layer->IsFrozen() );
		layerNode->setProperty( "boxmode",						layer->GetBoxMode() );
		layerNode->setProperty( "backfacecull",					layer->GetBackCull() );
		layerNode->setProperty( "alledges",						layer->GetAllEdges() );
		layerNode->setProperty( "vertexTicks",					layer->GetVertTicks() );
		layerNode->setProperty( "showTrajectory",				layer->GetTrajectory() );
		layerNode->setProperty( "xray",							layer->HasXRayMtl() );
		layerNode->setProperty( "ignoreExtents",				layer->GetIgnoreExtents() );
		layerNode->setProperty( "showFrozenInGray",				(layer->ShowFrozenWithMtl()) ? &false_value : &true_value );
		layerNode->setProperty( "vertexColorsShaded",			layer->GetShadeCVerts() );
		layerNode->setProperty( "visibility",					layer->GetVisibility(atTime) );
		layerNode->setProperty( "imageMotionBlurMultiplier",	layer->GetImageBlurMultiplier(atTime) );
		layerNode->setProperty( "motionBlurOn",					layer->GetMotBlurOnOff(atTime) );
		layerNode->setProperty( "motionblur",					layer->MotBlur() );
		layerNode->setProperty( "display",						layer->GetDisplayFlags() );
	}


	pop_alloc_frame();
}
void		TableOfContents::recordNodeInfo( INode* node, XMLNode* xmlParent, TimeValue atTime ) {
	XMLNode* xmlNode = xmlParent->addNode( "object" );
	xmlNode->setAttribute( "name",		TSTR( node->GetName() ) );

	// Object Class and SuperClass
	this->recordClassInfo( node, xmlNode );

	// Object Layer
	ILayer* nodeLayer = (ILayer*) node->GetReference(NODE_LAYER_REF);
	xmlNode->setProperty( "layer", (nodeLayer) ? TSTR( nodeLayer->GetName() ) : TSTR( "undefined" ) );

	// Object Transform
	TSTR transformStr = _T( "(matrix3 ");

	Matrix3 nodeTM = node->GetNodeTM(atTime);
	for ( int i = 0; i < 4; i++ ) {
		TSTR rowStr = _T("");
		Point3 row = nodeTM.GetRow(i);

		rowStr.printf( "[%f, %f, %f] ", row.x, row.y, row.z );
		transformStr += rowStr;
	}
	transformStr += _T( ")" );
	xmlNode->setProperty( "transform", transformStr );

	//------------------------------------------------------------------------------------

	// Recurse through children nodes
	XMLNode* childrenXMLNode = xmlNode->addNode( "children" );
	int numChildren = node->NumberOfChildren();
	for ( int i = 0; i < numChildren; i++ )
		this->recordNodeInfo( node->GetChildNode(i), childrenXMLNode );
}
void		TableOfContents::recordUserInfo() {
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
	this->toc()->setProperty( "user", TSTR(username) );

	// Record ComputerName
	TCHAR computername[ MAX_COMPUTERNAME_LENGTH+1 ];
	namesize = MAX_COMPUTERNAME_LENGTH+1;
	GetComputerName( computername, &namesize );
	this->toc()->setProperty( "computer", TSTR(computername) );

	// Record Localtime
	this->toc()->setProperty( "localtime", today );

	// Record MAXVersion
	TCHAR maxversion[MAX_PATH];
	sprintf( maxversion, "%d", Get3DSMAXVersion() );
	this->toc()->setProperty( "maxversion", TSTR(maxversion) );
}
bool		TableOfContents::saveToFile( TSTR fileName ) {
	this->_document		= NULL;
	this->_toc			= NULL;

	if ( fileName.length() > 0 ) {
		this->_document		= new XMLDocument();
		if ( this->_document && this->_document->isValid() ) {
			// Record how long it takes to save the TableOfContents
			time_t	start, finish;
			int		elapsed_time;
			time( &start );

			// Record the root node
			this->_toc			= this->_document->addNode( "toc" );

			Interface* ip		= GetCOREInterface();
			this->toc()->setAttribute( "filename", ip->GetCurFilePath() + ip->GetCurFileName() );
			this->toc()->setAttribute( "version", this->_version );

			TimeValue currTime = GetCOREInterface()->GetTime();
			this->toc()->setProperty( "currentFrame", currTime );

			//-----------------			Record Data		------------------------
			this->recordUserInfo();
			this->recordLayerInfo();

			XMLNode* nodesNode	= this->toc()->addNode( "objects" );
			this->recordNodeInfo( GetCOREInterface()->GetRootNode(), nodesNode, currTime );

			//-----------------							------------------------

			time( &finish );
			elapsed_time = difftime( finish, start );
			this->toc()->setProperty( "timeToCalculateTOC", elapsed_time );

			// Save to File
			return this->_document->saveToFile( fileName );
		}
	}
	return false;
}
//------------------------------------	MAXScript Methods	-----------------------------------------
void		TableOfContents::sprin1(CharStream* s)		{ s->puts( "(TableOfContents())" ); }
void		TableOfContents::gc_trace()				{
	Value::gc_trace();

	if ( this->_document	&& this->_document->is_not_marked() )	this->_document->gc_trace();
	if ( this->_toc			&& this->_toc->is_not_marked() )		this->_toc->gc_trace();
}
Value*		TableOfContents::applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc ) {
	if		( methodID == n_propertiesByName ) {
		check_arg_count( propertiesByName, 1, count );
		return this->propertiesByName( arg_list[0]->eval()->to_string() );
	}
	else if ( methodID == n_loadFromFile ) {
		check_arg_count( loadFromFile, 1, count );
		return ( this->loadFromFile( arg_list[0]->eval()->to_string() ) ) ? &true_value : &false_value;
	}
	else if ( methodID == n_nodesByProperty ) {
		check_arg_count( nodesByProperty, 2, count );
		return ( this->nodesByProperty( arg_list[0]->eval()->to_string(),arg_list[1]->eval()->to_string() ) );
	}
	else if ( methodID == n_saveToFile ) {
		check_arg_count( saveToFile, 1, count );
		return ( this->saveToFile( arg_list[0]->eval()->to_string() ) ) ? &true_value : &false_value;
	}
	return &ok;
}
Value*		TableOfContents::get_property( Value **arg_list, int count ) {
	Value* prop = arg_list[0];

	if		( prop == n_propertiesByName )	{ return NEW_GENERIC_METHOD( propertiesByName ); }
	else if	( prop == n_document )			{ return this->_document; }
	else if	( prop == n_layers )			{ return this->layers(); }
	else if	( prop == n_loadFromFile )		{ return NEW_GENERIC_METHOD( loadFromFile ); }
	else if ( prop == n_nodes )				{ return this->nodes(); }
	else if ( prop == n_nodesByProperty )	{ return NEW_GENERIC_METHOD( nodesByProperty ); }
	else if	( prop == n_saveToFile )		{ return NEW_GENERIC_METHOD( saveToFile ); }
	else if ( prop == n_version )			{ return Float::intern( this->_version ); }

	return Value::get_property( arg_list, count );
}
Value*		TableOfContents::set_property( Value** arg_list, int count ) {
	return Value::set_property( arg_list, count );
}

//---------------------------------------------------------------------------------------------------
//										  C++ METHODS
//---------------------------------------------------------------------------------------------------

std::string	TocFileFromMaxFile( std::string maxfilename ) {
	std::string out("");
	if ( maxfilename.length() > 0 ) {
		Array* pathList		= splitpath( maxfilename );
		std::string	pathPart( pathList->data[0]->to_string() );
		std::string filePart( pathList->data[1]->to_string() );
		out					= pathPart + ".meta/" + filePart + ".toc";
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

Value*		loadToc_cf( Value** arg_list, int count ) {
	check_arg_count_with_keys( blurSceneManager.loadToc, 0, count );

	Value* fileName		= key_arg( maxfile );
	std::string tocFile( "" );
	if ( fileName != &unsupplied )
		tocFile			= TocFileFromMaxFile( std::string( fileName->eval()->to_string() ) );
	else
		tocFile			= TocFileFromMaxFile( std::string( GetCOREInterface()->GetCurFilePath() ) );

	one_typed_value_local( TableOfContents* out );
	vl.out				= new TableOfContents();

	if ( vl.out->loadFromFile( tocFile.c_str() ) )
		return_value( vl.out );

	pop_value_locals();
	return &undefined;
}
Value*		maxTocFile_cf( Value** arg_list, int count ) {
	check_arg_count_with_keys( blurSceneManager.tocFileName, 0, count );

	Value* fileName = key_arg( maxfile );
	if ( fileName != &unsupplied )
		return new String( TocFileFromMaxFile( std::string( fileName->eval()->to_string() ) ).c_str() );
	else
		return new String( TocFileFromMaxFile( std::string( GetCOREInterface()->GetCurFilePath() ) ).c_str() );
}
Value*		saveToc_cf( Value** arg_list, int count ) {
	check_arg_count( blurSceneManager.saveTOC, 0, count );

	Value* result = &false_value;
	if ( createMetapath() ) {
		one_typed_value_local( TableOfContents* temp );
		vl.temp = new TableOfContents();
		result = (vl.temp->saveToFile( TocFileFromMaxFile( std::string( GetCOREInterface()->GetCurFilePath() ) ).c_str() )) ? &true_value : &false_value;
		pop_value_locals();
	}
	return result;
}
Value*		toc_cf( Value** arg_list, int count ) {
	check_arg_count( blurSceneManager.toc, 0, count );
	one_value_local( out );
	vl.out = new TableOfContents();
	return_value( vl.out );
}

//---------------------------------------------------------------------------------------------------

void BlurSceneManagerInit() {}