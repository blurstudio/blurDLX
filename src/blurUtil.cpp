/*!
	\file		BlurFns.h

	\remarks	Implementation file for the misc MAXScript extension functions
	
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
#include "Numbers.h"
#include "MAXclses.h"
#include "Streams.h"
#include "MSTime.h"
#include "MAXObj.h"
#include "Parser.h"
#include "LclClass.h"
#include "meshdelta.h"
#include "MeshSub.h"
#include "resource.h"
#include "OPC_TriTriOverlap.h"
#include "bbox.h"
#include "randgenerator.h"
#include "maxapi.h"
#include "ILayerProperties.h"
#include "ilayer.h"
#include "ilayermanager.h"
#include "3dmath.h"
#include "imtledit.h"

#include "maxmats.h"

#include <string>
#include <iostream>
#include <cctype>
#include <algorithm>

// define the new primitives using macros from SDK
#include "defextfn.h"
#include "definsfn.h"

// ------------------------------------------------------------------------------------------------------
//											GLOBALS DEFINES
// ------------------------------------------------------------------------------------------------------

// if this number changes, all dll's will need to be recompiled.
#define BLURDLXLIB_API MAX_API_NUM

// change this if/when all dll's need recompilation.  Also remember to set REV to 0.
#define BLURDLXLIB_VER 1

// change this if/when BlurLibs API's/functionality changes, but old dll's don't need recompile
#define BLURDLXLIB_REV 0  // ie. V1.08

// VERSION_BLURLIB should contain all the info needed to tell is the library is valid in the current system.
// This macro is used to find the version of blurlib linked against at build time.
#define VERSION_BLURDLXLIB ((BLURDLXLIB_API<<16)|(BLURDLXLIB_VER<<8)|BLURDLXLIB_REV)


// following def also in common_funcs.cpp for use by ValueToBitArray()
// following only valid for integer and float range checking
#define range_check(_val, _lowerLimit, _upperLimit, _desc)					\
	if (_val < _lowerLimit || _val > _upperLimit) {							\
		TCHAR buf[256];														\
		TCHAR buf2[128];													\
		strcpy(buf,_desc);													\
		strcat(buf,_T(" < "));												\
		_snprintf(buf2, 128, _T("%g"), EPS(_lowerLimit));					\
		strcat(buf,buf2);													\
		strcat(buf,_T(" or > "));											\
		_snprintf(buf2, 128, _T("%g"), EPS(_upperLimit));					\
		strcat(buf,buf2);													\
		strcat(buf,_T(": "));												\
		_snprintf(buf2, 128, _T("%g"), EPS(_val));							\
		strcat(buf,buf2);													\
		throw RuntimeError (buf);											\
	}

// valid values for parameter selTypesAllowed to ValueToBitArray, ValueToBitArrayM
#define MESH_VERTSEL_ALLOWED 1
#define MESH_FACESEL_ALLOWED 2
#define MESH_EDGESEL_ALLOWED 3

#define n_caption		Name::intern( "caption" )
#define n_refresh		Name::intern( "refresh" )

// uniqueId
#define BLURDLX_CLASS_ID	0x000001
#define	UNIQUE_ID_INDEX		0

namespace IdTypes { enum IdType { All, Atmos, Effect, Layer, Object, Material, Map }; };

// getDuplicateVerts 
#define EDITTRIOBJ_CLASSID GetEditTriObjDesc()->ClassID()
#define TRIOBJ_CLASSID Class_ID(TRIOBJ_CLASS_ID, 0)

#define get_valid_node(_node, _fn) _get_valid_node((MAXNode*)_node, #_fn##" requires a node")
#define get_meshForValue(_value, _accessType, _owner, _fn) _get_meshForValue(_value, _accessType, _owner, #_fn##)

// ------------------------------------------------------------------------------------------------------
//											C++ METHODS
// ------------------------------------------------------------------------------------------------------
INode*		_get_valid_node(MAXNode* _node, TCHAR* errmsg) {
	if (is_node(_node)){	
		if(deletion_check_test(_node))
			throw RuntimeError("Attempted to access to deleted object");
	}
	else
		throw RuntimeError (errmsg);

	return _node->to_node();
}

Mesh*		_get_meshForValue(Value* value, int accessType, ReferenceTarget** owner, TCHAR* fn) {
	/*!-----------------------------------------------------------
		\remarks
			if value is a node, do normal mesh access for node.
			if value is an editable mesh (baseobject), get its mesh.  Don't know the INode though
			if value is a maxscript MeshValue, get its mesh
	------------------------------------------------------------*/
	Mesh* mesh;
	if (owner) *owner = NULL;
	if (value->is_kind_of(class_tag(MAXNode))) 
	{
		get_valid_node(value, fn);
		mesh = ((MAXNode*)value)->set_up_mesh_access(accessType, owner);
	}
	else if (value->tag == class_tag(MAXObject) &&
			 ((MAXObject*)value)->obj->ClassID() == EDITTRIOBJ_CLASSID) 
	{
		mesh = &((TriObject*)((MAXObject*)value)->obj)->GetMesh();
		if (owner) *owner = ((MAXObject*)value)->obj;
	}
	else 
	{
		mesh = ((MeshValue*)value)->to_mesh();
		if (owner) *owner = ((MeshValue*)value)->obj;
	}

	return mesh;
}
COLORREF	DarkenColour(COLORREF col, double factor) {
	/*!--------------------------------------------------------------------
		\remarks
			Lightens a colour by a factor between 0 and 1.

		\param		col			COLORREF		RGB colour to darken
		\param		factor		double			number between 0 and 1 to darken the colour by

		\return		COLORREF	Resulting new color
	---------------------------------------------------------------------*/
	if (factor > 0.0 && factor <= 1.0){	
		return RGB((BYTE)(GetRValue(col) - (factor * GetRValue(col))),
				   (BYTE)(GetGValue(col) - (factor * GetGValue(col))),
				   (BYTE)(GetBValue(col) - (factor * GetBValue(col))));
	}
	return col;
}
TCHAR*		GetString(int id) {
	static TCHAR buf[256];

	if(hInstance)
		return LoadString(hInstance, id, buf, sizeof(buf)) ? buf : NULL;
	return NULL;
}
COLORREF	LightenColour(COLORREF col, double factor) {
	/*!--------------------------------------------------------------------
		\remarks
			Lightens a colour by a factor between 0 and 1.

		\param		col			COLORREF		RGB colour to lighten
		\param		factor		double			number between 0 and 1 to lighten the colour by

		\return		COLORREF	Resulting new color
	---------------------------------------------------------------------*/
	if (factor > 0.0 && factor <= 1.0){	
		return RGB((BYTE)((factor * (255 - GetRValue(col))) + GetRValue(col)),
				   (BYTE)((factor * (255 - GetGValue(col))) + GetGValue(col)),
				   (BYTE)((factor * (255 - GetBValue(col))) + GetBValue(col)));
	}
	return col;
}
void		ValueToBitArray(Value* inval, BitArray &theBitArray, int maxSize, TCHAR* errmsg = _T(""), int selTypesAllowed = 0) {
	if (inval->is_kind_of(class_tag(Array))) {
		Array* theArray = (Array*)inval;
		for (int k = 0; k < theArray->size; k++) {
			int index = theArray->data[k]->to_int();
			if (maxSize != -1) {
				range_check(index, 1, maxSize, errmsg);
			}
			else
				if (index > theBitArray.GetSize())
					theBitArray.SetSize(index, 1);
			theBitArray.Set(index-1);
		}
	}
	else if (inval->is_kind_of(class_tag(BitArrayValue))) {
		for (int k = 0; k < ((BitArrayValue*)inval)->bits.GetSize(); k++) {
			if ((((BitArrayValue*)inval)->bits)[k]) {
				if (maxSize != -1) {
					range_check(k+1, 1, maxSize, errmsg);
				}
				else
					if (k+1 > theBitArray.GetSize())
						theBitArray.SetSize(k+1, 1);
				theBitArray.Set(k);
			}
		}
	}
	else if (inval == n_all) {
		theBitArray.SetAll();
	}
	else if (inval == n_none) {
		theBitArray.ClearAll();
	}
	else if (is_number(inval)) {
		int index = inval->to_int();
		if (maxSize != -1) {
			range_check(index, 1, maxSize, errmsg);
		}
		else
			if (index > theBitArray.GetSize())
				theBitArray.SetSize(index, 1);
		theBitArray.Set(index-1);
	}
	else if ((selTypesAllowed == MESH_VERTSEL_ALLOWED && (is_vertselection(inval))) || 
	         (selTypesAllowed == MESH_FACESEL_ALLOWED && (is_faceselection(inval))) ||
	         (selTypesAllowed == MESH_EDGESEL_ALLOWED && (is_edgeselection(inval))))
			theBitArray = ((BitArrayValue*)((MeshSelection*)inval)->to_bitarrayValue())->bits;
	else 
		throw RuntimeError (_T("IDS_CANNOT_CONVERT_TO_BITARRAY"), inval);

}

// ------------------------------------------------------------------------------------------------------
//										UNIQUEID METHODS
// ------------------------------------------------------------------------------------------------------
ulong		GenUniqueId() {
	mxs_seed( rand() );
	ulong a = mxs_rand();
	ulong b = ClassIDRandGenerator->rand();
	return (a^b);
}
ulong		GetUniqueId( ReferenceTarget* targ ) {
	AppDataChunk *ad		= targ->GetAppDataChunk(Class_ID(BLURDLX_CLASS_ID,0), CUST_ATTRIB_CLASS_ID, UNIQUE_ID_INDEX);

	// If the unique id has not been set, then set it to a new unique id
	if ( !ad ) {
		ulong newId		= GenUniqueId();
		int size		= static_cast<int>( sizeof(ulong) );
		targ->AddAppDataChunk( Class_ID(BLURDLX_CLASS_ID,0), CUST_ATTRIB_CLASS_ID, UNIQUE_ID_INDEX, size, (ulong*) new ulong(newId) );
		return newId;
	}
	// Otherwise, return the reftarget's unique id
	else {
		return (*((ulong*) ad->data));
	}
}
INode*		NodeByUniqueId( INode* parent, ulong id ) {
	int objectCount = parent->NumberOfChildren();
	for ( int i = 0; i < objectCount; i++ ) {
		INode* cNode		= parent->GetChildNode(i);

		// Check to see if this nodes id matches the inputed id
		AppDataChunk *ad	= cNode->GetAppDataChunk(Class_ID(BLURDLX_CLASS_ID,0), CUST_ATTRIB_CLASS_ID, UNIQUE_ID_INDEX);
		if ( ad && *((ulong*) ad->data) == id )
			return cNode;

		// Check to see if any child nodes have the inputed id
		INode* out = NodeByUniqueId( cNode, id );
		if ( out )
			return out;
	}
	return NULL;
}
INode*		NodeByUniqueName( INode* parent, TSTR uniqueName ) {
	int objectCount = parent->NumberOfChildren();
	for ( int i = 0; i < objectCount; i++ ) {
		INode* cNode		= parent->GetChildNode(i);
		TSTR nodeName( cNode->GetName() );
		nodeName.toLower();
		if ( nodeName == uniqueName )
			return cNode;

		// Check to see if any child nodes have the inputed id
		INode* out = NodeByUniqueName( cNode, uniqueName );
		if ( out )
			return out;
	}
	return NULL;
}
Value*		RefByUniqueId( int id, IdTypes::IdType type = IdTypes::All ) {
	Interface* ip	= GetCOREInterface();

	if ( type == IdTypes::All || type == IdTypes::Atmos ) {
		int atmosCount = ip->NumAtmospheric();
		for ( int i = 0; i < atmosCount; i++ ) {
			Atmospheric* atmos	= ip->GetAtmospheric(i);
			AppDataChunk* ad	= atmos->GetAppDataChunk(Class_ID(BLURDLX_CLASS_ID,0), CUST_ATTRIB_CLASS_ID, UNIQUE_ID_INDEX);
			if ( ad && *((ulong*) ad->data) == id )
				return MAXAtmospheric::intern( atmos );
		}
	}
	if ( type == IdTypes::All || type == IdTypes::Effect ) {
		int effectCount = ip->NumEffects();
		for ( int i = 0; i < effectCount; i++ ) {
			Effect* eff			= ip->GetEffect(i);
			AppDataChunk* ad	= eff->GetAppDataChunk(Class_ID(BLURDLX_CLASS_ID,0), CUST_ATTRIB_CLASS_ID, UNIQUE_ID_INDEX);
			if ( ad && *((ulong*) ad->data) == id )
				return MAXEffect::intern( eff );
		}
	}
	if ( type == IdTypes::All || type == IdTypes::Layer ) {
		IFPLayerManager* LM		= static_cast<IFPLayerManager*>(GetCOREInterface(LAYERMANAGER_INTERFACE));
		ILayerManager* lm		= (ILayerManager*) GetCOREInterface()->GetScenePointer()->GetReference(10);
		int layerCount			= lm->GetLayerCount();
		for ( int i = 0; i < layerCount; i++ ) {
			ILayer* layer		= lm->GetLayer( LM->getLayer(i)->getName() );
			AppDataChunk* ad	= layer->GetAppDataChunk(Class_ID(BLURDLX_CLASS_ID,0), CUST_ATTRIB_CLASS_ID, UNIQUE_ID_INDEX);
			if ( ad && *((ulong*) ad->data) == id )
				return FPMixinInterfaceValue::intern( LM->getLayer(i) );
		}
	}
	if ( type == IdTypes::All || type == IdTypes::Material || type == IdTypes::Map ) {
		MtlBaseLib* lib		= GetCOREInterface()->GetSceneMtls();
		int count			= lib->Count();
		MtlBase* mtl;
		Texmap* map;
		AppDataChunk* ad;
		/*for ( int i = 0; i < count; i++ ) {
			MtlBaseLib &sublib	= lib[i];
			int subcount		= sublib.Count();
			for ( int c = 0; c < subcount; c++ ) {
				mtl = sublib[c];
				if ( mtl ) {
					try			{ ad	= mtl->GetAppDataChunk( Class_ID( BLURDLX_CLASS_ID,0 ), CUST_ATTRIB_CLASS_ID, UNIQUE_ID_INDEX ); }
					catch (...)	{ continue; }

					if ( ad && *((ulong*) ad->data) == id )
						return MAXMaterial::intern( (Mtl*) mtl );

					else if ( type != IdTypes::Material ) {
						for ( int z = 0; z < mtl->NumSubTexmaps(); z++ ) {
							map = mtl->GetSubTexmap(z);
							if ( map ) {
								try { ad = map->GetAppDataChunk( Class_ID( BLURDLX_CLASS_ID, 0 ), CUST_ATTRIB_CLASS_ID, UNIQUE_ID_INDEX ); }
								catch (...) { continue; }

								if ( ad && *((ulong*) ad->data) == id )
									return MAXTexture::intern(map);
							}
						}
					}
				}
			}
		}*/
	}
	if ( type == IdTypes::All || type == IdTypes::Object ) {
		INode* out		= NodeByUniqueId( ip->GetRootNode(), id );		// Break out because it needs to be recursive
		if ( out )
			return MAXNode::intern(out);
	}
	return &undefined;
}
Value*		RefByUniqueName( TSTR uniqueName, IdTypes::IdType type = IdTypes::All ) {
	Interface* ip	= GetCOREInterface();

	if ( type == IdTypes::All || type == IdTypes::Atmos ) {
		int atmosCount = ip->NumAtmospheric();
		for ( int i = 0; i < atmosCount; i++ ) {
			Atmospheric* atmos	= ip->GetAtmospheric(i);
			TSTR atmosName( atmos->GetName() );
			atmosName.toLower();
			if ( atmosName == uniqueName )
				return MAXAtmospheric::intern( atmos );
		}
	}
	if ( type == IdTypes::All || type == IdTypes::Effect ) {
		int effectCount = ip->NumEffects();
		for ( int i = 0; i < effectCount; i++ ) {
			Effect* eff			= ip->GetEffect(i);
			TSTR effName( eff->GetName() );
			effName.toLower();
			if ( effName == effName )
				return MAXEffect::intern( eff );
		}
	}
	if ( type == IdTypes::All || type == IdTypes::Layer ) {
		IFPLayerManager* LM		= static_cast<IFPLayerManager*>(GetCOREInterface(LAYERMANAGER_INTERFACE));
		ILayerManager* lm		= (ILayerManager*) GetCOREInterface()->GetScenePointer()->GetReference(10);
		int layerCount			= lm->GetLayerCount();
		for ( int i = 0; i < layerCount; i++ ) {
			ILayer* layer		= lm->GetLayer( LM->getLayer(i)->getName() );
			TSTR layerName( layer->GetName() );
			layerName.toLower();
			if ( layerName == uniqueName )
				return FPMixinInterfaceValue::intern( LM->getLayer(i) );
		}
	}
	if ( type == IdTypes::All || type == IdTypes::Object ) {
		INode* out		= NodeByUniqueName( ip->GetRootNode(), uniqueName );		// Break out because it needs to be recursive
		if ( out )
			return MAXNode::intern(out);
	}
	return &undefined;
}

// ------------------------------------------------------------------------------------------------------
//										BLURUTIL STRUCT METHODS
// ------------------------------------------------------------------------------------------------------

// Misc Methods
def_struct_primitive( getDuplicateVerts,	blurUtil,		"getDuplicateVerts");
def_struct_primitive( getReferenceTarget,	blurUtil,		"getReferenceTarget" );
def_struct_primitive( intersectTriTri,		blurUtil,		"intersectTriTri" );
def_struct_primitive( searchAndReplace,		blurUtil,		"searchAndReplace");
def_struct_primitive( setMaterials,			blurUtil,		"setMaterials" );
def_struct_primitive( setNames,				blurUtil,		"setNames" );

// Gui Methods
def_struct_primitive( controlSize,			blurUtil,		"controlSize" );
def_struct_primitive( setSubRolloutSize,	blurUtil,		"setSubRolloutSize" );
def_struct_primitive( setControlSize,		blurUtil,		"setControlSize" );

// UniqueId Methods
def_struct_primitive( genUniqueId,			blurUtil,		"genUniqueId" );
def_struct_primitive( refByUniqueId,		blurUtil,		"refByUniqueId" );
def_struct_primitive( refByUniqueName,		blurUtil,		"refByUniqueName" );
def_struct_primitive( uniqueId,				blurUtil,		"uniqueId" );

// Windows Methods
def_struct_primitive( getWindowText,		blurUtil,		"getWindowText" );
def_struct_primitive( maximizeWindow,		blurUtil,		"maximizeWindow" );
def_struct_primitive( minimizeWindow,		blurUtil,		"minimizeWindow" );
def_struct_primitive( restoreWindow,		blurUtil,		"restoreWindow" );
def_struct_primitive( setClipboardData,		blurUtil,		"setClipboardData");
def_struct_primitive( setWindowOnTop,		blurUtil,		"setWindowOnTop");
def_struct_primitive( setWindowText,		blurUtil,		"setWindowText" );
def_struct_primitive( showWindow,			blurUtil,		"showWindow" );

// -----------------------------------------------------------------------------------------------------
// Misc Methods
Value*		getDuplicateVerts_cf( Value** arg_list, int arg_count ) {
	/*!---------------------------------------------------------
		\remarks
			Gets the duplicate vertices of a mesh based on a float tolerance
		
		\param		mesh		Value* (node,editable mesh)
		\param		tolerance	float

		\returns
			<BitArray>
	----------------------------------------------------------*/
	check_arg_count(getDuplicateVerts, 2, arg_count);
	
	ReferenceTarget *owner;
	Mesh* mesh = get_meshForValue(arg_list[0], MESH_BASE_OBJ, &owner, weldVertsByThreshold);
	float tol = arg_list[1]->to_float();

	int nVerts=mesh->getNumVerts();

	BitArray whichVerts (nVerts);
	whichVerts.SetAll();
	
	MeshDelta tmd (*mesh);
	BOOL found = tmd.WeldByThreshold (*mesh, whichVerts, tol);
	int nv = whichVerts.NumberSet();
	
	BitArray dupVerts(nVerts);
	
	if (found) 
		dupVerts = tmd.vDelete;

	return (new BitArrayValue (dupVerts));
}
Value*		getReferenceTarget_cf( Value** arg_list, int count ) {
	check_arg_count (getReferenceTarget, 1, count);

	ReferenceMaker* target = arg_list[0]->to_reftarg();

	Value *item;

	Class_ID cid = target->ClassID();
	SClass_ID sid = target->SuperClassID();

	MAXClass* cls = lookup_MAXClass(&cid, sid);

	if (cls == &inode_object)
		item = MAXNode::intern((INode*)target);
	else if (cls->superclass->superclass == &node_class)
		item = MAXObject::intern((Object*)target);
	else
		item = (*cls->superclass->maker)(cls, (ReferenceTarget*)target, NULL, 0);

	return item;
}
Value*		intersectTriTri_cf( Value** arg_list, int count ) {
	check_arg_count (blurUtil.intersectTriTri, 6, count);
	
	Point3 V0 = arg_list[0]->to_point3();
	Point3 V1 = arg_list[1]->to_point3();
	Point3 V2 = arg_list[2]->to_point3();
	Point3 U0 = arg_list[3]->to_point3();
	Point3 U1 = arg_list[4]->to_point3();
	Point3 U2 = arg_list[5]->to_point3();

	BOOL intersect = TriTriOverlap(V0, V1, V2, U0, U1, U2);

	return intersect ? &true_value: &false_value;
}
Value*		searchAndReplace_cf(Value** arg_list, int count) {
	extern Value* replace_cf( Value** arg_list, int count );
	return replace_cf( arg_list, count );						// moved into blurString library - see blurString.cpp - EKH 9/18/07
}
Value*		setMaterials_cf( Value** arg_list, int count ) {
	check_arg_count_with_keys (SetNames, 2, count);

	Value* nodes = arg_list[0]->eval();
	Value* mtls = arg_list[1]->eval();

	if ( !(is_collection(nodes) && is_collection(mtls)) )
		throw RuntimeError (_T("blurUtil.setMaterials expects 2 arrays, of nodes, and of materials." ));
	
	Array* narray = (Array*)nodes;
	Array* marray = (Array*)mtls;

	if ( narray->size != marray->size )
		throw RuntimeError (_T("blurUtil.setMaterials requires both arrays to be the same size"));
	
	int numitems	= narray->size;
	int success		= 0;

	theHold.Suspend();
	theHold.DisableUndo();

	Interface* ip = GetCOREInterface();

	Value *n = NULL;
	Value *m = NULL;
	for (int j = 0; j < numitems; j++) {
		n = narray->data[j];
		m = marray->data[j];
		if ( !(n->is_kind_of(class_tag(MAXNode)) && m->is_kind_of(class_tag(MAXMaterial))) )
			continue;

		n->to_node()->SetMtl( m->to_mtl() );
		success += 1;
	}

	theHold.EnableUndo();
	theHold.Resume();

	return Integer::intern(success);
}
Value*		setNames_cf( Value** arg_list, int count ) {
	check_arg_count_with_keys (SetNames, 2, count);

	if (!arg_list[0]->is_kind_of(class_tag(Array)))
		throw RuntimeError (_T("First argument invalid, cannot convert to array"));

	if (!arg_list[1]->is_kind_of(class_tag(Array)))
		throw RuntimeError (_T("Second argument invalid, cannot convert to array"));

	Array* nodeArray = (Array*)arg_list[0];
	Array* nameArray = (Array*)arg_list[1];

	if (nodeArray->size != nameArray->size)
		throw RuntimeError (_T("Argument size invalid, both arrays should contain the same number of elements"));

	for (int j = 0; j < nodeArray->size; j++) {
		if ( !nodeArray->data[j]->is_kind_of(class_tag(MAXNode)) )
			continue;
		if ( !nameArray->data[j]->is_kind_of(class_tag(String)) )
			continue;

		INode* node = nodeArray->data[j]->to_node();
		TCHAR* newName = nameArray->data[j]->to_string();
        
		node->SetName(newName);		
	}

	return nodeArray;
}

Value*		controlSize_cf( Value** arg_list, int count ) {
	check_arg_count( controlSize, 1, count );

	if ( !is_rolloutcontrol(arg_list[0]->eval()) )
		throw RuntimeError (_T("First argument of controlSize needs to be a rollout control."));

	RolloutControl* control	= (RolloutControl*) arg_list[0]->eval();
	Point2 point;
	point.x = 0;
	point.y = 0;

	if ( control->parent_rollout ) {
		HWND hWnd	= GetDlgItem( control->parent_rollout->page, control->control_ID );
		RECT rect;
		GetWindowRect( hWnd, &rect );
		MapWindowPoints( NULL, control->parent_rollout->page, (LPPOINT)&rect, 2 );
		point.x	= rect.right - rect.left;
		point.y	= rect.bottom - rect.top;
	}
	one_typed_value_local( Point2Value* out );
	vl.out = new Point2Value( point );
	return_value( vl.out );
}
Value*		setSubRolloutSize_cf( Value** arg_list, int count ) {
	check_arg_count( blurUtil.setRolloutSize, 2, count );
	if ( !is_rollout( arg_list[0]->eval() ) )
		throw RuntimeError( _T( "First argument of blurUtil.setRolloutSize needs to be a rollout control." ) );
	if ( !is_point2( arg_list[1]->eval() ) )
		throw RuntimeError( _T( "Second argument of blurUtil.setRolloutSize needs to be a Point2 value." ) );

	Rollout* control	= (Rollout*) arg_list[0]->eval();
	Point2Value* size	= (Point2Value*) arg_list[1]->eval();
	int width			= (int) size->p.x;
	int height			= (int) size->p.y;
	
	HWND hContainer		= GetParent( control->page );
	HWND hParent		= GetParent( hContainer );

	if ( hContainer && hParent ) {
		// Update Rollout Sizes
		control->rollout_width	= width - 12;

		RECT rect;
		RECT childRect;

		//---------------------------------------------------------------
		// Update Container Window

		GetWindowRect( hContainer, &rect );
		MapWindowPoints( NULL, hParent, (LPPOINT) &rect, 2 );
		SetWindowPos( hContainer, NULL, rect.left, rect.top, width, rect.bottom - rect.top, SWP_NOZORDER );

		//---------------------------------------------------------------
		// Update Button

		HWND hBtn	= GetDlgItem( hContainer, 0 );
		GetWindowRect( hBtn, &childRect );
		MapWindowPoints( NULL, hContainer, (LPPOINT) &childRect, 2 );
		SetWindowPos( hBtn, NULL, childRect.left, childRect.top, width - 12, childRect.bottom - childRect.top, SWP_NOZORDER );

		//---------------------------------------------------------------
		// Update Rollout Control

		GetWindowRect( control->page, &childRect );
		MapWindowPoints( NULL, hContainer, (LPPOINT)&childRect, 2 );
		SetWindowPos( control->page, NULL, childRect.left, childRect.top, width - 12, childRect.bottom - childRect.top, SWP_NOZORDER );
		InvalidateRect( control->page, &childRect, TRUE );

		return &true_value;
	}
	return &false_value;
}
Value*		setControlSize_cf( Value** arg_list, int count ) {
	check_arg_count_with_keys( setControlSize, 2, count );

	if ( is_rollout( arg_list[0]->eval() ) )
		return setSubRolloutSize_cf( arg_list, count );

	if ( !is_rolloutcontrol(arg_list[0]->eval()) )
		throw RuntimeError (_T("First argument of setControlSize needs to be a rollout control."));
	if ( !is_point2(arg_list[1]->eval()) )
		throw RuntimeError (_T("Second argument of setControlSize needs to be a Point2 value."));
	
	RolloutControl* control = (RolloutControl*) arg_list[0]->eval();
	Point2Value* size		= (Point2Value*) arg_list[1]->eval();
	int width				= (int) size->p.x;
	int height				= (int) size->p.y;

	if ( control->parent_rollout ) {
		HWND hWnd	= GetDlgItem( control->parent_rollout->page, control->control_ID );
		RECT rect;
		GetWindowRect( hWnd, &rect );
		MapWindowPoints( NULL, control->parent_rollout->page, (LPPOINT)&rect, 2 );
		SetWindowPos( hWnd, NULL, rect.left, rect.top, width, height, SWP_NOZORDER );
		if ( key_arg_or_default( refresh, &true_value ) == &true_value ) {
			InvalidateRect( control->parent_rollout->page, &rect, TRUE );
		}
		return &true_value;
	}
	return &false_value;
}

// Unique Id Methods
Value*		genUniqueId_cf( Value** arg_list, int count ) {
	check_arg_count( blurUtil.genUniqueId, 0, count );
	return Integer::intern( GenUniqueId() );
}
Value*		refByUniqueId_cf( Value** arg_list, int count ) {
	check_arg_count_with_keys( blurUtil.refByUniqueId, 1, count );
	Value* typeValue = key_arg( type );

	IdTypes::IdType type = IdTypes::All;

	if ( typeValue == Name::intern( _T( "atmos" ) ) )			type = IdTypes::Atmos;
	else if ( typeValue == Name::intern( _T( "effect" ) ) )		type = IdTypes::Effect;
	else if ( typeValue == Name::intern( _T( "layer" ) ) )		type = IdTypes::Layer;
	else if ( typeValue == Name::intern( _T( "object" ) ) )		type = IdTypes::Object;
	else if ( typeValue == Name::intern( _T( "material" ) ) )	type = IdTypes::Material;
	else if ( typeValue == Name::intern( _T( "map" ) ) )		type = IdTypes::Map;

	int id			= arg_list[0]->eval()->to_int();
	if ( id == 0 )
		return &undefined;

	return RefByUniqueId( id, type );
}
Value*		refByUniqueName_cf( Value** arg_list, int count ) {
	check_arg_count_with_keys( blurUtil.refByUniqueName, 1, count );
	Value* typeValue = key_arg( type );

	IdTypes::IdType type = IdTypes::All;

	if ( typeValue == Name::intern( _T( "atmos" ) ) )			type = IdTypes::Atmos;
	else if ( typeValue == Name::intern( _T( "effect" ) ) )		type = IdTypes::Effect;
	else if ( typeValue == Name::intern( _T( "layer" ) ) )		type = IdTypes::Layer;
	else if ( typeValue == Name::intern( _T( "object" ) ) )		type = IdTypes::Object;
	else if ( typeValue == Name::intern( _T( "material" ) ) )	type = IdTypes::Material;
	else if ( typeValue == Name::intern( _T( "map" ) ) )		type = IdTypes::Map;

	TSTR uniqueName( arg_list[0]->eval()->to_string() );
	uniqueName.toLower();
	
	return RefByUniqueName( uniqueName, type );
}
Value*		uniqueId_cf( Value** arg_list, int count ) {
	check_arg_count_with_keys( blurUtil.unqiueId, 1, count );
	ReferenceTarget* targ;

	if ( arg_list[0] == &undefined)
		return Integer::intern(0);

	if ( is_node(arg_list[0]) )
		targ = arg_list[0]->eval()->to_node();
	else if ( arg_list[0]->is_kind_of( class_tag(MAXMaterial) ) )
		targ = ((MAXMaterial*) arg_list[0]->eval())->mat;
	else if ( arg_list[0]->is_kind_of( class_tag(MAXTexture) ) )
		targ = ((MAXTexture*) arg_list[0]->eval())->map;
	else
		targ = arg_list[0]->eval()->to_reftarg();

	return Integer::intern( GetUniqueId( targ ) );
}
// Windows Methods
Value*		getWindowText_cf( Value** arg_list, int count ) {
	check_arg_count (getWindowText, 1, count);

	HWND hWnd = (HWND)arg_list[0]->to_intptr();
	TSTR windowText;

	int len = SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0);
	windowText.Resize(len+1);

	SendMessage(hWnd, WM_GETTEXT, len+1, (LPARAM)windowText.data());

	return new String(windowText);
}
Value*		maximizeWindow_cf( Value** arg_list, int count ) {
	check_arg_count (maximizeWindow, 1, count);

	HWND windowHandle = (HWND)arg_list[0]->to_intptr();
	BOOL result = ShowWindow(windowHandle, SW_MAXIMIZE); 
	
	return result ? &true_value : &false_value;
}
Value*		minimizeWindow_cf( Value** arg_list, int count ) {
	check_arg_count (minimizeWindow, 1, count);

	HWND windowHandle = (HWND)arg_list[0]->to_intptr();
	BOOL result = ShowWindow(windowHandle, SW_MINIMIZE); 
	
	return result ? &true_value : &false_value;
}
Value*		restoreWindow_cf( Value** arg_list, int count ) {
	check_arg_count (restoreWindow, 1, count);

	HWND windowHandle = (HWND)arg_list[0]->to_intptr();
	BOOL result = ShowWindow(windowHandle, SW_RESTORE); 
	
	return result ? &true_value : &false_value;
}
Value*		setClipboardData_cf(Value** arg_list, int count) {
	check_arg_count (setClipboardData, 1, count);	
	
	TCHAR *str = arg_list[0]->to_string();

	HGLOBAL h;
	LPSTR p;

    if (!OpenClipboard( GetCOREInterface()->GetMAXHWnd())) 
        return &false_value; 

	__try
	{
		h = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (lstrlen(str) +1) * sizeof(TCHAR) );
		
		if ( h == NULL )
			return &false_value;

		p = (LPTSTR) GlobalLock(h);
		if ( p == NULL )
		{
			// lock failed
			return &false_value;
		}

		lstrcpy(p, str);
		GlobalUnlock(h);
		EmptyClipboard();
		SetClipboardData( CF_TEXT, h);
	}
	__finally
	{
		CloseClipboard();
	}

	return &true_value;
}
Value*		setWindowOnTop_cf( Value** arg_list, int count ) {
	check_arg_count (setWindowOnTop, 2, count);
	HWND windowHandle = (HWND)arg_list[0]->to_intptr();
	BOOL putItOnTop = arg_list[1]->to_bool();

	BOOL result = FALSE;

	if (putItOnTop)
		result = SetWindowPos( windowHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );
	else
		result = SetWindowPos( windowHandle, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW );

	return result ? &true_value : &false_value;
}
Value*		setWindowText_cf( Value** arg_list, int count ) {
	check_arg_count (setWindowText, 2, count);

	HWND windowHandle = (HWND)arg_list[0]->to_intptr();
	TSTR windowText = arg_list[1]->to_string();
	
	SendMessage(windowHandle, WM_SETTEXT , windowText.Length(), (LPARAM)(windowText.data()));
	
	return &true_value;
}
Value*		showWindow_cf( Value** arg_list, int count ) {
	check_arg_count (showWindow, 1, count);

	HWND windowHandle = (HWND)arg_list[0]->to_intptr();
	BOOL result = ShowWindow(windowHandle, SW_SHOW); 
	
	return result ? &true_value : &false_value;
}

// ------------------------------------------------------------------------------------------------------
//										SYSTEM GLOBALS
// ------------------------------------------------------------------------------------------------------

// establish a new system global variable, first are the getter & stter fns
Value*		get_blurdlx_version() {
	one_typed_value_local(Array* result); 
	vl.result = new Array(4);
	vl.result->append(Integer::intern(MAX_RELEASE));
	vl.result->append(Integer::intern(BLURDLXLIB_API));
	vl.result->append(Integer::intern(BLURDLXLIB_VER));
	vl.result->append(Integer::intern(BLURDLXLIB_REV));
	return_value(vl.result);
}
Value*		get_blurdlx_loaded()				{ return &true_value; }
Value*		set_blurdlx_version(Value* val)		{ return get_blurdlx_version(); }
Value*		set_blurdlx_loaded(Value* val)		{ return get_blurdlx_loaded(); }

// ------------------------------------------------------------------------------------------------------
//										BLURUTIL INIT
// ------------------------------------------------------------------------------------------------------
void BlurUtilInit() {
	define_system_global("BlurDlxLibVer", get_blurdlx_version, set_blurdlx_version);
	define_system_global("BlurDlxLibLoaded", get_blurdlx_loaded, set_blurdlx_loaded);

	CharStream* out = thread_local(current_stdout);
	out->puts(_T("--* BlurDlx.dlx loaded *--\n"));
}




