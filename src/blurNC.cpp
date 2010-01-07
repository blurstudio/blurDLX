/*!
	\file		blurNC.cpp

	\remarks	Blur extension classes for Blur's naming convention
	
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
#include "Structs.h"
#include "Parser.h"
#include <string>  // stl

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )


#include "definsfn.h"
	def_name ( orderElements )

//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------




Value *GetValueProperty (Value* value, TSTR propertyName)
{
	Value** argElement = (Value**)_alloca(sizeof(Value*) * 1);
	argElement[0] = Name::intern(propertyName);

	return value->get_property(argElement, 1);
}

Value *SetValueProperty (Value* value, TSTR propertyName, Value* propValue)
{
	Value** argElement = (Value**)_alloca(sizeof(Value*) * 2);
	argElement[0] = Name::intern(propertyName);
	argElement[1] = propValue;

	return value->set_property(argElement, 2);
}

Array* nc_filterString(TSTR stringInStr, TSTR filterStr)
{
	one_typed_value_local(Array* result);
	vl.result = new Array(0);

	char *cmds = stringInStr;
	char *seps = filterStr;
	char *token;

	char*	rPtr;
	char*	ePtr = save_string(cmds);
	rPtr = ePtr;

	token = strtok( rPtr, seps );
	while( token != NULL )  
	{
		// While there are tokens in "string"
		vl.result->append(new String(token));
	    token = strtok( NULL, seps );
	}

	//free(ePtr);
	
	return_value(vl.result);

}

int nc_findStringItem (Array* arrayValue, TSTR itemStr)
{
	itemStr.toUpper();

	for (int i = 0; i < arrayValue->size; i++)
	{
		TSTR arrayItem = arrayValue->data[i]->to_string();
		if (arrayItem.isNull())
			continue;

		arrayItem.toUpper();
		if (itemStr.operator == (arrayItem) )
			return i;
	}
	return -1;
}

TSTR nc_deleteIllegalCharactersAtTheEnd (TSTR elementStrIn, int illegalChar = '-')
{
	char *pdest;
	int result;
	
	TSTR elementStrOut = elementStrIn;

	pdest = strrchr( elementStrIn, illegalChar );
	result = pdest - elementStrIn + 1;

	if (result == elementStrIn.length())
		elementStrOut = elementStrIn.Substr(0, elementStrIn.length()-1);

	return elementStrOut;
}
/*
def_struct_primitive( nc_deleteIllegalCharactersAtTheEnd,	blurNC, "deleteIllegalCharactersAtTheEnd");

Value* nc_deleteIllegalCharactersAtTheEnd_cf(Value** arg_list, int count)
{
	TSTR nameIn = arg_list[0]->to_string();
	TSTR nameOut = nc_deleteIllegalCharactersAtTheEnd(nameIn);
	return new String (nameOut);
}*/

TSTR nc_rebuildComponentName (Array* itemArrayStr, TSTR separatorStr, TSTR emptyStr = "")
{
	CharStream* out = thread_local(current_stdout); 

	TSTR nameOut;

	for (int i = (itemArrayStr->size-1); i >= 0; i--)
	{
		TSTR itemStr = itemArrayStr->data[i]->to_string();

		// empty components not allowed
		if (emptyStr.operator == (itemStr))
			continue;
		
		if (i > 0)
			nameOut = separatorStr + itemStr + nameOut;
		else
			nameOut = itemStr + nameOut;
	}

	return nameOut;
}

TSTR nc_rebuildName (Array* itemArrayStr, TSTR separatorStr, TSTR emptyStr = "X")
{
	TSTR nameOut;
	BOOL stopDeletingEmptyElements = FALSE;

	int lastElement = itemArrayStr->size;

	for (int i = (itemArrayStr->size-1); i >= 0; i--)
	{
		TSTR itemStr = itemArrayStr->data[i]->to_string();
		if (emptyStr.operator ==(itemStr) && !stopDeletingEmptyElements && i > 0 )
		{
			lastElement = i;
			continue;
		}
		else
		{
			stopDeletingEmptyElements = TRUE;
		}
		
		if (i > 0)
			nameOut = separatorStr + itemStr + nameOut;
		else
			nameOut = itemStr + nameOut ;
	}

	if (lastElement != 5)
	{
		nameOut += separatorStr;
	}

	return nameOut;
}

def_struct_primitive( nc_rebuildName,	blurNC, "rebuildName");
Value* nc_rebuildName_cf(Value** arg_list, int count)
{
	// do the checks!
	Struct* NCDefStruct = (Struct*) arg_list[0];
	TSTR nameInStr = arg_list[1]->to_string();

	// elementSpacer, componentSpacer, componentEmpty, elementEmpty, specialCharacterArray
	TSTR elementSpacer = GetValueProperty(NCDefStruct, "elementSpacer")->to_string();
	TSTR elementEmpty = GetValueProperty(NCDefStruct, "elementEmpty")->to_string();

	Array* elementArray = (Array*) nc_filterString(nameInStr, elementSpacer);
	TSTR nameOut = nc_rebuildName (elementArray, elementSpacer, elementEmpty);

	return new String (nameOut);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------
#include "iLayer.h"
/*
a = layermanager.getlayer 5
blurNC.GetElement _blurName a #location

*/
TSTR nc_getName (Value* valueIn)
{
	if (valueIn == &undefined)
		return _T("");

	TSTR nameOut;

	// get the name of the item to check
	if (is_node(valueIn))
	{
		INode* node = valueIn->to_node();
		nameOut = node->GetName();
	}
	else if (is_string(valueIn))
	{
		nameOut = valueIn->to_string();
	}
	else if (is_fpmixininterface(valueIn)) // TBR: layers in this case, but what about other mixininterfaces??
	{
		Value** argElement = (Value**)_alloca(sizeof(Value*) * 1);
		argElement[0] = new String("layerAsRefTarg");	
		Value* layerAsRefTargValue = valueIn->get_property(argElement, 1);

		ILayer* layerAsRefTarg = (ILayer*) layerAsRefTargValue->to_reftarg();
        if (layerAsRefTarg)
			nameOut = layerAsRefTarg->GetName();
	}
	else 
	{
		Value* nameValue = valueIn->_get_property(n_name);

		if (is_string(nameValue))
			nameOut = nameValue->to_string();
		else
			return _T("");
	}

	return nameOut;
}

def_struct_primitive( nc_getName,	blurNC, "getName");

Value* nc_getName_cf(Value** arg_list, int count)
{
	check_arg_count(nc_getName, 2, count);
	Struct* NCDefStruct = (Struct*) arg_list[0];
	TSTR newNameStr = nc_getName(arg_list[1]);

	return new String (newNameStr);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------
#define LAYERPROPERTIES_INTERFACE Interface_ID(0x202e0798, 0x3589129b)


TSTR nc_setName (Value* valueIn, TSTR newName)
{
	if (valueIn == &undefined)
		return _T("");

	// get the name of the item to check
	if (is_node(valueIn))
	{
		INode* node = valueIn->to_node();
		node->SetName(newName);
		return newName;
	}
	else if (is_string(valueIn))
	{
		valueIn = new String (newName);
		return newName;
	}
	else if (is_fpmixininterface(valueIn)) // TBR: layers in this case, but what about other mixininterfaces??
	{
		Value** argElement = (Value**)_alloca(sizeof(Value*) * 1);
		argElement[0] = new String("layerAsRefTarg");	
		Value* layerAsRefTargValue = valueIn->get_property(argElement, 1);

		ILayer* layerAsRefTarg = (ILayer*) layerAsRefTargValue->to_reftarg();
        if (layerAsRefTarg)
		{
			layerAsRefTarg->SetName(newName);
			return layerAsRefTarg->GetName();
		}

		return newName;
	}
	
	Value* nameValue = valueIn->_set_property(n_name, new String(newName));
	return newName;
}


/*a = layermanager.getlayer 1
blurnc.setName _blurname a "NewNAme"*/

def_struct_primitive( nc_setName,	blurNC, "setName");

Value* nc_setName_cf(Value** arg_list, int count)
{
	check_arg_count(nc_setName, 3, count);
	Struct* NCDefStruct = (Struct*) arg_list[0];
	TSTR newNameInStr = arg_list[2]->to_string();
	TSTR newNameStr = nc_setName(arg_list[1], newNameInStr);

	return new String (newNameStr);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------
int nc_getTypeIndex (TSTR typeInStr, Struct* NCDefStruct)
{
	// get the elements definition array
	Array* elementDefArray = (Array*) GetValueProperty(NCDefStruct, "elementDefArray");
	
	for (int i = 0; i < elementDefArray->size; i++)
	{
		// this is the element definition
		Struct* elementDef = (Struct*)elementDefArray->data[i];

		// get the first elementType
		Value* elementType = GetValueProperty(elementDef, "type");
		TSTR elementTypeStr = elementType->to_string();
		if (typeInStr.operator ==(elementTypeStr))
			return i;
	}

	return -1;
}

TSTR nc_getElementClassFromType (TSTR typeInStr, Struct* NCDefStruct)
{
	int typeIndex = nc_getTypeIndex(typeInStr, NCDefStruct);
	Array* elementDefArray = (Array*) GetValueProperty(NCDefStruct, "elementDefArray");
	Struct* elementDef = (Struct*)elementDefArray->data[typeIndex];
	Value* elementClass = GetValueProperty(elementDef, "class");
	TSTR elementClassStr = elementClass->to_string();

	return elementClassStr;
}

def_struct_primitive( nc_getTypeIndex,	blurNC, "getTypeIndex");

Value* nc_getTypeIndex_cf(Value** arg_list, int count)
{
	check_arg_count(nc_getTypeIndex, 2, count);
	Struct* NCDefStruct = (Struct*) arg_list[0];
	TSTR typeInStr = arg_list[1]->to_string();

	int typeIndex = nc_getTypeIndex(typeInStr, NCDefStruct);
	
	if (typeIndex != -1)
		return Integer::intern(typeIndex+1);

	return Integer::intern(0);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------
def_struct_primitive( nc_isValidType,	blurNC, "isValidType");

Value* nc_isValidType_cf(Value** arg_list, int count)
{
	check_arg_count(nc_isValidType, 2, count);
	Struct* NCDefStruct = (Struct*) arg_list[0];
	TSTR typeInStr = arg_list[1]->to_string();

	int typeIndex = nc_getTypeIndex(typeInStr, NCDefStruct);
	
	if (typeIndex != -1)
		return &true_value;

	return &false_value;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------

TSTR nc_GetValidElementText (TSTR componentIn, Struct* NCDefStruct  )
{
	using namespace std;

	// get the specialCharacterArray
	string specialCharacters = GetValueProperty(NCDefStruct, "specialCharacters")->to_string() ;
	string componentSpacer = GetValueProperty(NCDefStruct, "componentSpacer")->to_string();
	string elementSpacer = GetValueProperty(NCDefStruct, "elementSpacer")->to_string();
	TSTR elementEmpty = GetValueProperty(NCDefStruct, "elementEmpty")->to_string();

	string spacerChars = " " + elementSpacer;
    
	string elementStr = componentIn;
	string elementOutStr = "";

	string::const_iterator it1 = elementStr.begin();

	while ( it1 != elementStr.end() )
	{
		if (specialCharacters.find(*it1) != -1)
		{
            it1++;
			continue;
		}

		if (spacerChars.find(*it1) != -1)
		{
			it1++;
			elementOutStr += componentSpacer;
			continue;
		}
		
		elementOutStr += *it1;
		it1++;
	}

	TSTR elementOut = elementOutStr.c_str();
	if (elementOut.isNull())
		elementOut = elementEmpty;

	return elementOut;
}

def_struct_primitive( nc_GetValidElementText,	blurNC, "getValidElementText");

Value* nc_GetValidElementText_cf(Value** arg_list, int count)
{
	check_arg_count(nc_GetValidElementText, 2, count);

	Struct* NCDefStruct = (Struct*) arg_list[0];
	TSTR componentInStr = arg_list[1]->to_string();

	TSTR validElementText = nc_GetValidElementText (componentInStr, NCDefStruct);
	return new String(validElementText);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------

BOOL nc_IsValidElementText (TSTR componentIn, Struct* NCDefStruct  )
{
	if (componentIn.isNull())
		return FALSE;

	using namespace std;

	// get the specialCharacterArray
	string specialCharacters = GetValueProperty(NCDefStruct, "specialCharacters")->to_string() ;
	string elementStr = componentIn;

	string::const_iterator it1 = elementStr.begin();

	while ( it1 != elementStr.end() )
	{
		if (specialCharacters.find(*it1) != -1)
			return FALSE;

		it1++;
	}
	
	return TRUE;
}


def_struct_primitive( nc_IsValidElementText,	blurNC, "isValidElementText");

Value* nc_IsValidElementText_cf(Value** arg_list, int count)
{
	check_arg_count(nc_IsValidElementText, 2, count);

	Struct* NCDefStruct = (Struct*) arg_list[0];
	TSTR componentInStr = arg_list[1]->to_string();

	return nc_IsValidElementText(componentInStr, NCDefStruct) ? &true_value : &false_value;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------

BOOL nc_IsValidComponent (TSTR componentInStr, TSTR typeInStr, Struct* NCDefStruct)
{
	TSTR componentEmpty = GetValueProperty(NCDefStruct, "componentEmpty")->to_string();
	TSTR elementEmpty = GetValueProperty(NCDefStruct, "elementEmpty")->to_string();
	TSTR componentSpacer = GetValueProperty(NCDefStruct, "componentSpacer")->to_string();

	if (componentInStr.operator ==(componentEmpty))
		return FALSE;

	if (componentInStr.operator ==(elementEmpty))
		return TRUE;

	int typeIndex = nc_getTypeIndex(typeInStr, NCDefStruct);
	Array* elementDefArray = (Array*) GetValueProperty(NCDefStruct, "elementDefArray");
	Struct* elementDef = (Struct*)elementDefArray->data[typeIndex];
	TSTR elementClassStr = GetValueProperty(elementDef, "class")->to_string();

	if (elementClassStr.operator ==(_T("text")))
	{
		BOOL isValidElementText = nc_IsValidElementText(componentInStr, NCDefStruct);
		return isValidElementText;
	}
	else 
	{
		Array* componentCheckList = (Array*)GetValueProperty(elementDef, "componentCheckList");
		componentInStr.toUpper();

		Array* componentArray = nc_filterString(componentInStr, componentSpacer);

		for (int i = 0; i < componentArray->size; i++)
		{
			if (nc_findStringItem(componentCheckList, componentArray->data[i]->to_string()) < 0)
				return FALSE;

		}
		return TRUE;
			
	}
	return FALSE;
}

def_struct_primitive( nc_IsValidComponent,	blurNC, "isValidComponent");

Value* nc_IsValidComponent_cf(Value** arg_list, int count)
{
	check_arg_count(nc_IsValidComponent, 3, count);

	Struct* NCDefStruct = (Struct*) arg_list[0];
	TSTR componentInStr = arg_list[1]->to_string();
	TSTR typeInStr = arg_list[2]->to_string();

	return nc_IsValidComponent(componentInStr, typeInStr, NCDefStruct) ? &true_value : &false_value;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------
BOOL nc_IsValidMultipleElement (TSTR elementInStr, TSTR typeInStr, Struct* NCDefStruct  )
{
	TSTR componentEmpty = GetValueProperty(NCDefStruct, "componentEmpty")->to_string();
	TSTR elementEmpty = GetValueProperty(NCDefStruct, "elementEmpty")->to_string();
	TSTR componentSpacer = GetValueProperty(NCDefStruct, "componentSpacer")->to_string();

	if (elementInStr.operator ==(componentEmpty))
		return FALSE;

	if (elementInStr.operator ==(elementEmpty))
		return TRUE;

	int typeIndex = nc_getTypeIndex(typeInStr, NCDefStruct);
	Array* elementDefArray = (Array*) GetValueProperty(NCDefStruct, "elementDefArray");
	Struct* elementDef = (Struct*)elementDefArray->data[typeIndex];
	Array* elementCheckList = (Array*)GetValueProperty(elementDef, "elementCheckList");

	elementInStr.toUpper();

	if (nc_findStringItem(elementCheckList, elementInStr) != -1)
		return TRUE;
	
	return FALSE;
}

BOOL nc_IsValidElement(TSTR elementInStr, TSTR typeInStr, Struct* NCDefStruct)
{
	using namespace std;

	TSTR componentStr = GetValueProperty(NCDefStruct, "componentSpacer")->to_string();
	TSTR elementClassStr = nc_getElementClassFromType(typeInStr, NCDefStruct);

	if (elementClassStr.operator ==(_T("text")))
		return nc_IsValidElementText(elementInStr, NCDefStruct);
	else if (elementClassStr.operator ==(_T("element")))
		return nc_IsValidMultipleElement(elementInStr, typeInStr, NCDefStruct);
	else if (elementClassStr.operator ==(_T("component")))
		return nc_IsValidComponent(elementInStr,typeInStr, NCDefStruct);
	
	return FALSE;
}

//blurNC.isValidElement _blurname "L" #location 
def_struct_primitive( nc_IsValidElement,	blurNC, "isValidElement");

Value* nc_IsValidElement_cf(Value** arg_list, int count)
{
	check_arg_count(nc_IsValidElement, 3, count);

	Struct* NCDefStruct = (Struct*) arg_list[0];
	TSTR elementInStr = arg_list[1]->to_string();
	TSTR typeInStr = arg_list[2]->to_string();

	return nc_IsValidElement(elementInStr, typeInStr, NCDefStruct) ? &true_value : &false_value;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------


TSTR nc_GetValidElement(TSTR elementInStr, TSTR typeInStr, Struct* NCDefStruct)
{
	TSTR componentStr = GetValueProperty(NCDefStruct, "componentSpacer")->to_string();
	
	Array* componentArray = (Array*) nc_filterString(elementInStr, componentStr);
	TSTR componentSpacer = GetValueProperty(NCDefStruct, "componentSpacer")->to_string();
	TSTR elementEmpty = GetValueProperty(NCDefStruct, "elementEmpty")->to_string();

	TSTR elementClassStr = nc_getElementClassFromType(typeInStr, NCDefStruct);

	if (elementClassStr.operator ==(_T("text")))
	{
		return nc_GetValidElementText (elementInStr, NCDefStruct );
	}
	else //if (elementClassStr.operator ==(_T("element")) || elementClassStr.operator ==(_T("component")))
	{
		TSTR validElementStr = _T("");
		Array* componentOutArray = new Array(0);

		for (int i = 0; i < componentArray->size; i++)
		{
			if (!nc_IsValidComponent(componentArray->data[i]->to_string(),typeInStr, NCDefStruct))
				continue;
			
			TSTR component = componentArray->data[i]->to_string();
		
			// check if we already used this component
			if (nc_findStringItem(componentOutArray, component) != -1)
				continue;

			componentOutArray->append(componentArray->data[i]);
			validElementStr += component;

			if (i < componentArray->size-1)
				validElementStr += componentSpacer;
		}
		if (validElementStr.isNull())
			validElementStr = elementEmpty;

		return validElementStr;
	}
}

def_struct_primitive( nc_GetValidElement,	blurNC, "getValidElement");

Value* nc_GetValidElement_cf(Value** arg_list, int count)
{
	check_arg_count(nc_GetValidElement, 3, count);

	Struct* NCDefStruct = (Struct*) arg_list[0];
	TSTR elementInStr = arg_list[1]->to_string();
	TSTR typeInStr = arg_list[2]->to_string();
	TSTR validElementStr = nc_GetValidElement(elementInStr, typeInStr, NCDefStruct);

	return new String (validElementStr);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------
TSTR nc_GetElement (TSTR nameInStr, TSTR typeInStr, Struct* NCDefStruct)
{
	TSTR elementSpacer = GetValueProperty(NCDefStruct, "elementSpacer")->to_string();
	int typeIndex = nc_getTypeIndex(typeInStr, NCDefStruct);

	Array* elementArray = (Array*) nc_filterString(nameInStr, elementSpacer);
	
	if (typeIndex >= elementArray->size)
		return "";

	return elementArray->data[typeIndex]->to_string();
}

def_struct_primitive( nc_GetElement, blurNC, "getElement");

Value* nc_GetElement_cf(Value** arg_list, int count)
{
	check_arg_count(nc_GetElement, 3, count);

	Struct* NCDefStruct = (Struct*) arg_list[0];
	
	TSTR nameInStr = nc_getName(arg_list[1]);
	if (nameInStr.isNull())
		return &undefined;

	TSTR typeInStr = arg_list[2]->to_string();
	TSTR elementOut = nc_GetElement(nameInStr,typeInStr, NCDefStruct);
	
	if (elementOut.isNull())
		return &undefined;

	return new String(elementOut); 
}


//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------

BOOL nc_HasValidElement (TSTR nameInStr, TSTR typeInStr, Struct* NCDefStruct)
{
	TSTR elementIn = nc_GetElement(nameInStr,typeInStr, NCDefStruct);
	TSTR elementEmpty = GetValueProperty(NCDefStruct, "elementEmpty")->to_string();

	if (elementIn.isNull())
		return FALSE;

	if (elementIn.operator ==(elementEmpty))
		return TRUE;
    
	return nc_IsValidElement(elementIn, typeInStr, NCDefStruct);
}

//blurNC.hasvalidElement _blurname "C_entity_usage_location_part" #location
def_struct_primitive( nc_HasValidElement, blurNC, "hasValidElement");

Value* nc_HasValidElement_cf(Value** arg_list, int count)
{
	check_arg_count(nc_HasValidElement, 3, count);

	Struct* NCDefStruct = (Struct*) arg_list[0];
	
	TSTR nameInStr = nc_getName(arg_list[1]);
	if (nameInStr.isNull())
		return &false_value;

	TSTR typeInStr = arg_list[2]->to_string();
	TSTR elementIn = nc_GetElement(nameInStr,typeInStr, NCDefStruct);
	
	if (elementIn.isNull())
		return &false_value;

	return (nc_HasValidElement(nameInStr, typeInStr, NCDefStruct)) ? &true_value : &false_value;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------

BOOL nc_HasEmptyElement (TSTR nameInStr, TSTR typeInStr, Struct* NCDefStruct)
{
	TSTR elementIn = nc_GetElement(nameInStr,typeInStr, NCDefStruct);
	TSTR elementEmpty = GetValueProperty(NCDefStruct, "elementEmpty")->to_string();

	if (elementIn.operator ==(elementEmpty))
		return TRUE;

    return FALSE;
}

def_struct_primitive( nc_HasEmptyElement, blurNC, "hasEmptyElement");

Value* nc_HasEmptyElement_cf(Value** arg_list, int count)
{
	check_arg_count(nc_HasEmptyElement, 3, count);

	Struct* NCDefStruct = (Struct*) arg_list[0];

	TSTR nameInStr = nc_getName(arg_list[1]);
	if (nameInStr.isNull())
		return &true_value;

	TSTR typeInStr = arg_list[2]->to_string();
	TSTR elementIn = nc_GetElement(nameInStr,typeInStr, NCDefStruct);
	
	if (elementIn.isNull())
		return &true_value;

	return (nc_HasEmptyElement(nameInStr, typeInStr, NCDefStruct)) ? &true_value : &false_value;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------
TSTR nc_MakeEmptyElement (TSTR nameInStr, TSTR typeInStr, Struct* NCDefStruct)
{
	TSTR elementSpacer = GetValueProperty(NCDefStruct, "elementSpacer")->to_string();
	TSTR elementEmpty = GetValueProperty(NCDefStruct, "elementEmpty")->to_string();
	int typeIndex = nc_getTypeIndex(typeInStr, NCDefStruct);
	
	Array* elementArray = (Array*) nc_filterString(nameInStr, elementSpacer);

	TSTR nameOutStr = nameInStr;
	if (typeIndex >= elementArray->size)
	{
		for (int i = 0; i <= (typeIndex - elementArray->size); i ++)
			nameOutStr += elementSpacer + elementEmpty;
	}
	else
	{
		elementArray->data[typeIndex] = new String (elementEmpty);
		nameOutStr = nc_rebuildName(elementArray, elementSpacer, elementEmpty);
	}

	return nameOutStr;
}

// blurnc.makeEmptyElement _blurname "C_entity_Mesh_L-R_Part" #modrig 
def_struct_primitive( nc_MakeEmptyElement, blurNC, "makeEmptyElement");

Value* nc_MakeEmptyElement_cf(Value** arg_list, int count)
{
	check_arg_count(nc_MakeEmptyElement, 3, count);

	Struct* NCDefStruct = (Struct*) arg_list[0];

	TSTR nameInStr = nc_getName(arg_list[1]);
	if (nameInStr.isNull())
		return arg_list[1];

	TSTR typeInStr = arg_list[2]->to_string();

	TSTR nameOut = nc_MakeEmptyElement (nameInStr, typeInStr, NCDefStruct);

	nameOut = nc_setName(arg_list[1], nameOut);

	return new String (nameOut);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------

def_struct_primitive( nc_ClearElement, blurNC, "clearElement");

Value* nc_ClearElement_cf(Value** arg_list, int count)
{
	check_arg_count(nc_ClearElement, 3, count);

	Struct* NCDefStruct = (Struct*) arg_list[0];

	TSTR nameInStr = nc_getName(arg_list[1]);
	if (nameInStr.isNull())
		return arg_list[1];

	TSTR typeInStr = arg_list[2]->to_string();

	TSTR nameOut = nc_MakeEmptyElement (nameInStr, typeInStr, NCDefStruct);
	
	nameOut = nc_setName(arg_list[1], nameOut);

	return new String (nameOut);
}



//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------
TSTR nc_getBeautyComponentName (TSTR componentInStr, TSTR typeInStr, Struct* NCDefStruct)
{
	int typeIndex = nc_getTypeIndex(typeInStr, NCDefStruct);
	Array* elementDefArray = (Array*) GetValueProperty(NCDefStruct, "elementDefArray");
	Struct* elementDef = (Struct*)elementDefArray->data[typeIndex];
	Array* componentList = (Array*)GetValueProperty(elementDef, "componentList");
	Array* componentCheckList = (Array*)GetValueProperty(elementDef, "componentCheckList");
	componentInStr.toUpper();

	int componentIndex = nc_findStringItem(componentList, componentInStr);
    if 	(componentIndex != -1)
		return componentList->data[componentIndex]->to_string();

	return componentInStr;
}

TSTR nc_AddComponent (TSTR nameInStr, TSTR typeInStr, TSTR componentInStr, Struct* NCDefStruct)
{
	int typeIndex = nc_getTypeIndex(typeInStr, NCDefStruct);
	TSTR validElementStr = nc_GetValidElement(componentInStr, typeInStr,NCDefStruct);
	TSTR elementEmpty = GetValueProperty(NCDefStruct, "elementEmpty")->to_string();
	TSTR elementSpacer = GetValueProperty(NCDefStruct, "elementSpacer")->to_string();
	TSTR componentSpacer = GetValueProperty(NCDefStruct, "componentSpacer")->to_string();

	TSTR elementStr = nc_GetElement(nameInStr,typeInStr, NCDefStruct);
	
	// if the element was empty just fill it with the validelementInStr
	if (elementStr.operator ==(elementEmpty))
	{
		elementStr = validElementStr;
	}
	// has no element for that type, create the empty one
	else if (elementStr.isNull())
	{
		nameInStr = nc_MakeEmptyElement(nameInStr, typeInStr, NCDefStruct);
		elementStr = validElementStr;
	}
	// add only the components that are not already present
	else
	{
		Array* componentInArray = (Array*) nc_filterString(elementStr, componentSpacer);
		Array* validComponentArray = (Array*) nc_filterString(validElementStr, componentSpacer);

		for (int i = 0; i < validComponentArray->size; i++)        
		{
			TSTR component = validComponentArray->data[i]->to_string();
			if (!nc_IsValidComponent(component,typeInStr, NCDefStruct))
				continue;

			if (nc_findStringItem(componentInArray, component) < 0)
				elementStr += componentSpacer + nc_getBeautyComponentName(component,typeInStr, NCDefStruct);
		}
	}
	// make sure that our element is at least the empty element
	if (elementStr.isNull())
		elementStr = elementEmpty; //validElementStr;

	// rebuild the name
	Array* elementArray = (Array*) nc_filterString(nameInStr, elementSpacer);
	elementArray->data[typeIndex] = new String(elementStr);

	TSTR nameOut = nc_rebuildName(elementArray, elementSpacer, elementEmpty);
	return nameOut;
}
// blurNC.addComponent _blurName "C_Sonic_Ctrl_F_" #location "R"
// blurnc.addComponent _blurname "C_entity_Mesh_L-R_Part" #location "L-T-W-T"
def_struct_primitive( nc_AddComponent, blurNC, "addComponent");

Value* nc_AddComponent_cf(Value** arg_list, int count)
{
	check_arg_count(nc_AddComponent, 4, count);

	Struct* NCDefStruct = (Struct*) arg_list[0];

	TSTR nameInStr = nc_getName(arg_list[1]);
	if (nameInStr.isNull())
		return arg_list[1];

	TSTR typeInStr = arg_list[2]->to_string();
	TSTR componentInStr = arg_list[3]->to_string();

	TSTR nameOut = nc_AddComponent (nameInStr, typeInStr, componentInStr, NCDefStruct);
	
	nameOut = nc_setName(arg_list[1], nameOut);

	return new String (nameOut);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------
def_struct_primitive( nc_SetElement, blurNC, "setElement");

Value* nc_SetElement_cf(Value** arg_list, int count)
{
	check_arg_count(nc_SetElement, 4, count);

	Struct* NCDefStruct = (Struct*) arg_list[0];

	TSTR nameInStr = nc_getName(arg_list[1]);
	if (nameInStr.isNull())
		return arg_list[1];

	TSTR typeInStr = arg_list[2]->to_string();
	TSTR elementInStr = arg_list[3]->to_string();

	TSTR nameOut = nc_MakeEmptyElement (nameInStr, typeInStr, NCDefStruct);
	nameOut = nc_AddComponent(nameOut,typeInStr, elementInStr, NCDefStruct); 
	
	nameOut = nc_setName(arg_list[1], nameOut);

	return new String (nameOut);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------
TSTR nc_RemoveComponent (TSTR nameInStr, TSTR typeInStr, TSTR componentInStr, Struct* NCDefStruct)
{
	CharStream* out = thread_local(current_stdout); 

	int typeIndex = nc_getTypeIndex(typeInStr, NCDefStruct);
	TSTR elementEmpty = GetValueProperty(NCDefStruct, "elementEmpty")->to_string();
	TSTR elementSpacer = GetValueProperty(NCDefStruct, "elementSpacer")->to_string();
	TSTR componentSpacer = GetValueProperty(NCDefStruct, "componentSpacer")->to_string();

	TSTR validElementStr = nc_GetValidElement(componentInStr, typeInStr,NCDefStruct);

	TSTR elementStr = nc_GetElement(nameInStr,typeInStr, NCDefStruct);

	// if the element was empty we have finished already!
	if (elementStr.operator ==(elementEmpty) || elementStr.isNull())
		return nameInStr;

	Array* componentInArray = (Array*) nc_filterString(elementStr, componentSpacer);
	Array* validComponentToRemoveArray = (Array*) nc_filterString(validElementStr, componentSpacer);
	
	Array* componentOutArray = new Array(0);
	
	for (int i = 0; i < componentInArray->size; i++)        
	{
		TSTR componentIn = componentInArray->data[i]->to_string();

		if (nc_findStringItem(validComponentToRemoveArray, componentIn) < 0)
			componentOutArray->append(componentInArray->data[i]);
	}	
	
	if (componentOutArray->size == 0)
		elementStr = elementEmpty;
	else
		elementStr = nc_rebuildComponentName (componentOutArray, componentSpacer, elementEmpty);
		//elementStr = nc_rebuildName (componentOutArray, componentSpacer, elementEmpty);

	// rebuild the name
	Array* elementArray = (Array*) nc_filterString(nameInStr, elementSpacer);
	elementArray->data[typeIndex] = new String(elementStr);

	TSTR nameOut = nc_rebuildName(elementArray, elementSpacer, elementEmpty);
	return nameOut;
}
/*
a = "C_Sonic_Mesh_L_Pupil_Rndr-Pc_H-3-Facial_"
blurNc.RemoveComponent _blurname a #Usage "Mesh" 
blurNc.getvalidelement _blurname "Mesh" #Usage 
blurNc.getelement _blurname a #Usage 
*/
// blurnc.removeComponent _blurname "C_entity_Mesh_L-R_Part" #location "L-T-W-T"
def_struct_primitive( nc_RemoveComponent, blurNC, "removeComponent");

Value* nc_RemoveComponent_cf(Value** arg_list, int count)
{
	check_arg_count(nc_RemoveComponent, 4, count);

	Struct* NCDefStruct = (Struct*) arg_list[0];

	TSTR nameInStr = nc_getName(arg_list[1]);
	if (nameInStr.isNull())
		return arg_list[1];

	TSTR typeInStr = arg_list[2]->to_string();
	TSTR componentInStr = arg_list[3]->to_string();

	TSTR nameOut = nc_RemoveComponent (nameInStr, typeInStr, componentInStr, NCDefStruct);
	
	nameOut = nc_setName(arg_list[1], nameOut);

	return new String (nameOut);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------
def_struct_primitive( nc_GetNameAsElementArray, blurNC, "getNameAsElementArray");

Value* nc_GetNameAsElementArray_cf(Value** arg_list, int count)
{
	check_arg_count(nc_GetNameAsElementArray, 2, count);

	Struct* NCDefStruct = (Struct*) arg_list[0];
	
	one_typed_value_local(Array* result);

	TSTR nameInStr = nc_getName(arg_list[1]);
	if (nameInStr.isNull())
	{
		vl.result = new Array(0);
		return_value(vl.result);
	}

	TSTR elementSpacer = GetValueProperty(NCDefStruct, "elementSpacer")->to_string();

	vl.result = nc_filterString(nameInStr, elementSpacer);

	return_value(vl.result);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------
def_struct_primitive( nc_GetNameFromElementArray, blurNC, "getNameFromElementArray");

Value* nc_GetNameFromElementArray_cf(Value** arg_list, int count)
{
	check_arg_count(nc_GetNameFromElementArray, 2, count);

	Struct* NCDefStruct = (Struct*) arg_list[0];
	Array* elementArray = (Array*)arg_list[1];
	TSTR elementEmpty = GetValueProperty(NCDefStruct, "elementEmpty")->to_string();
	TSTR elementSpacer = GetValueProperty(NCDefStruct, "elementSpacer")->to_string();
	TSTR nameOut = nc_rebuildName(elementArray, elementSpacer, elementEmpty);

	return new String(nameOut);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------
def_struct_primitive( nc_GetElementFromComponentArray , blurNC, "getElementFromComponentArray");

Value* nc_GetElementFromComponentArray_cf(Value** arg_list, int count)
{
	check_arg_count(nc_GetElementFromComponentArray, 2, count);

	Struct* NCDefStruct = (Struct*) arg_list[0];
	Array* componentArray = (Array*)arg_list[1];
	TSTR elementEmpty = GetValueProperty(NCDefStruct, "elementEmpty")->to_string();
	TSTR componentSpacer = GetValueProperty(NCDefStruct, "componentSpacer")->to_string();

//	TSTR nameOut = nc_rebuildName(componentArray, componentSpacer, elementEmpty);
	TSTR nameOut = nc_rebuildComponentName(componentArray, componentSpacer, elementEmpty);

	return new String(nameOut);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------
def_struct_primitive( nc_GetElementAsComponentArray , blurNC, "getElementAsComponentArray");

Value* nc_GetElementAsComponentArray_cf(Value** arg_list, int count)
{
	check_arg_count(nc_GetElementAsComponentArray, 2, count);

	Struct* NCDefStruct = (Struct*) arg_list[0];
	TSTR elementInStr = arg_list[1]->to_string();
	TSTR componentSpacer = GetValueProperty(NCDefStruct, "componentSpacer")->to_string();

	one_typed_value_local(Array* result);

	vl.result = nc_filterString(elementInStr , componentSpacer);

	return_value(vl.result);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------
BOOL nc_CheckNamingConvention ( TSTR nameInStr, Struct* NCDefStruct )
{
	TSTR elementSpacer = GetValueProperty(NCDefStruct, "elementSpacer")->to_string();
	TSTR componentSpacer = GetValueProperty(NCDefStruct, "componentSpacer")->to_string();

	Array* elementDefArray = (Array*) GetValueProperty(NCDefStruct, "elementDefArray");
	Array* elementArray = nc_filterString(nameInStr, elementSpacer);

	if (elementArray->size == 0)
		return FALSE;

	if (elementArray->size > elementDefArray->size)
		return FALSE;

	for (int i = 0; i < elementArray->size; i++)
	{
		TSTR typeInStr = GetValueProperty(elementDefArray->data[i], "type")->to_string();
		TSTR classInStr = GetValueProperty(elementDefArray->data[i], "class")->to_string();
		TSTR elementStr = elementArray->data[i]->to_string();
		
		if (classInStr.operator == (_T("element")) ) 
		{
			if (!nc_IsValidElement(elementStr,typeInStr, NCDefStruct))
				return FALSE;
		}
		else if ( classInStr.operator == (_T("text")) )
		{
			if (!nc_IsValidComponent(elementStr,typeInStr, NCDefStruct))
				return FALSE;
		}
		else
		{
			Array* componentArray = (Array*) nc_filterString(elementStr, componentSpacer);
			for (int j = 0; j < componentArray->size; j++)
				if (!nc_IsValidComponent(componentArray->data[j]->to_string(),typeInStr, NCDefStruct))
					return FALSE;
		}
	}

	return TRUE;
}
/*
blurnc.checknamingconvention _blurname "C_Aeon_Hairfx_L_Head-01_Mha_H-4-Export"

for o in objects do
(
	local isValid = blurnc.checkNamingConvention _blurname o
	if (isValid == false) then
		format "%\n" o.name
)

*/
def_struct_primitive( nc_CheckNamingConvention , blurNC, "checkNamingConvention");

Value* nc_CheckNamingConvention_cf(Value** arg_list, int count)
{
	check_arg_count(nc_CheckNamingConvention, 2, count);

	Struct* NCDefStruct = (Struct*) arg_list[0];
	
	BOOL isValid = FALSE;

	if (arg_list[1]->is_kind_of(class_tag(Array)))
	{
		Array* itemArray = (Array*)arg_list[1];
		for (int i =0; i < itemArray->size; i++)
		{
			TSTR nameInStr = nc_getName(itemArray->data[i]);
			if (nameInStr.isNull())
				return &false_value;

			isValid = nc_CheckNamingConvention ( nameInStr, NCDefStruct );
			if (!isValid) 
				return &false_value;
		}
	}
	else
	{
		TSTR nameInStr = nc_getName(arg_list[1]);
		if (nameInStr.isNull())
			return &false_value;

		isValid = nc_CheckNamingConvention ( nameInStr, NCDefStruct );
	}

	return isValid ? &true_value : &false_value;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------
TSTR nc_MakeNamingConventionName ( TSTR nameInStr, Struct* NCDefStruct )
{
	TSTR elementSpacer = GetValueProperty(NCDefStruct, "elementSpacer")->to_string();
	TSTR componentSpacer = GetValueProperty(NCDefStruct, "componentSpacer")->to_string();
	TSTR elementEmpty = GetValueProperty(NCDefStruct, "elementEmpty")->to_string();

	Array* elementDefArray = (Array*) GetValueProperty(NCDefStruct, "elementDefArray");
	Array* elementArray = nc_filterString(nameInStr, elementSpacer);

	int elementCount = elementArray->size;

	if (elementCount <= 0)
		return _T("X");

	if (elementCount == 1)
	{
		TSTR nameOut = "X_X_X_X_";
		nameOut += nc_GetValidElementText(nameInStr, NCDefStruct);
		return nameOut;
	}
	
	if (elementCount > elementDefArray->size)
		elementCount = elementDefArray->size;

	Array* newElementArray = new Array(0);

	for (int i = 0; i < elementCount; i++)
	{
		TSTR typeInStr = GetValueProperty(elementDefArray->data[i], "type")->to_string();
		TSTR classInStr = GetValueProperty(elementDefArray->data[i], "class")->to_string();
		TSTR elementStr = elementArray->data[i]->to_string();
		
		
		if (classInStr.operator ==(_T("element")))
		{
			if (!nc_IsValidComponent(elementStr,typeInStr, NCDefStruct))
				elementStr = nc_GetValidElement(elementStr, typeInStr, NCDefStruct);
			
			if (elementStr.isNull())
				elementStr = elementEmpty;
		}else if (classInStr.operator ==(_T("text")))
		{
			if (!nc_IsValidComponent(elementStr,typeInStr, NCDefStruct))
				elementStr = nc_GetValidElementText(elementStr, NCDefStruct);
			
			if (elementStr.isNull())
				elementStr = elementEmpty;						
		}
		else
		{
			Array* componentArray = (Array*) nc_filterString(elementStr, componentSpacer);
			Array* newComponentArray = new Array(0);

			for (int j = 0; j < componentArray->size; j++)
			{
				TSTR component = componentArray->data[j]->to_string();
				if (nc_IsValidComponent(component,typeInStr, NCDefStruct))
					newComponentArray->append(componentArray->data[j]);
			}

			if (newComponentArray->size == 0)
				newComponentArray->append(new String(elementEmpty));
			
//			elementStr = nc_rebuildName(newComponentArray, componentSpacer, "");
			elementStr = nc_rebuildComponentName(newComponentArray, componentSpacer, "");
		}
		
		newElementArray->append(new String (elementStr));
	}
	
	TSTR nameOut = nc_rebuildName(newElementArray, elementSpacer);
	return nameOut;
}

def_struct_primitive( nc_MakeNamingConventionName , blurNC, "makeNamingConventionName");

Value* nc_MakeNamingConventionName_cf(Value** arg_list, int count)
{
	check_arg_count(nc_MakeNamingConventionName, 2, count);

	Struct* NCDefStruct = (Struct*) arg_list[0];

	TSTR nameInStr = nc_getName(arg_list[1]);
	if (nameInStr.isNull())
		return &false_value;

	TSTR nameOut = nc_MakeNamingConventionName ( nameInStr, NCDefStruct );
	
	nameOut = nc_setName(arg_list[1], nameOut);

	return new String (nameOut);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------
Value* filterstring_cf(Value** arg_list, int count)
{
	check_arg_count(FilterString, 2, count);
	type_check(arg_list[0], String, "FilterString [String to filter] [Tokens]");
	type_check(arg_list[1], String, "FilterString [String to filter] [Tokens]");

	one_typed_value_local(Array* result);
	vl.result = new Array(0);

	char *cmds = arg_list[0]->to_string();
	char *seps = arg_list[1]->to_string();
	char *token;

	char*	rPtr;
	char*	ePtr = save_string(cmds);
	rPtr = ePtr;

	token = strtok( rPtr, seps );
	while( token != NULL )  
	{
		// While there are tokens in "string"
		vl.result->append(new String(token));
	    token = strtok( NULL, seps );
	}

	//free(ePtr);

	return_value (vl.result); // LAM - 5/18/01 - was: return vl.result;
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------



Array* nc_GetNamingConventionAsIndexes(TSTR nameInStr, Value* orderElements, Struct* NCDefStruct)
{
	// get the elements definition array
	Array* elementDefArray = (Array*) GetValueProperty(NCDefStruct, "elementDefArray");
	TSTR elementSpacer = GetValueProperty(NCDefStruct, "elementSpacer")->to_string();
	TSTR componentSpacer = GetValueProperty(NCDefStruct, "componentSpacer")->to_string();
	TSTR elementEmpty = GetValueProperty(NCDefStruct, "elementEmpty")->to_string();

	// the result naming convention array
	Array* ncArray = new Array(elementDefArray->size);	

	// get the elements in the name filtering with the elementSpacer
	Array* elementArray = nc_filterString(nameInStr, elementSpacer);

	// it maps the custom element order array to the one in the NC
	// if not orderlements array has been specified then it is exactly the same as the one in the NC
	Array* mapOrderArray = new Array(elementDefArray->size);

	if (orderElements == &unsupplied)
	{
		for (int i = 0; i < elementDefArray->size; i++)
		{
			mapOrderArray->append(Integer::intern(i));
			ncArray->append(&undefined);
		}
	}
	else
	{
		Array* orderElementArray = (Array*) orderElements;
		for (int i = 0; i < orderElementArray->size; i++)
			ncArray->append(&undefined);

		for (int i = 0; i < elementDefArray->size; i++)
		{
			// this is the first element definition
			Struct* elementDef = (Struct*)elementDefArray->data[i];

			// get the first elementType
			Value* elementType = GetValueProperty(elementDef, "type");
			TSTR elementTypeStr = elementType->to_string();
			
			mapOrderArray->append(&undefined);
			for (int j = 0; j < orderElementArray->size; j++)
			{
				// get the second elementType
				TSTR element2TypeStr = orderElementArray->data[j]->to_string();
				
				if (element2TypeStr.operator ==(elementTypeStr))
					mapOrderArray->data[i] = (Integer::intern(j));
			}
		}

	}
	int i = mapOrderArray->size;

	// fills the nc array with the default values
	Array* elementDefClassArray = new Array(elementDefArray->size);
 	for (int i = 0; i < elementDefArray->size; i++)
	{
		if (mapOrderArray->data[i] == &undefined)
			continue;

		int dstIndex = mapOrderArray->data[i]->to_int();

		// this is the element definition
		Struct* elementDef = (Struct*)elementDefArray->data[i];

		// get the class and type of the elementDef
		Value* elementClass = GetValueProperty(elementDef, "class");
		TSTR elementClassStr = elementClass->to_string();

		elementDefClassArray->append(elementClass);

		if (elementClassStr.operator ==(_T("text")))
			ncArray->data[dstIndex] = new String("");
		else
			ncArray->data[dstIndex] = new BitArrayValue();
	}

	// there is no elements, so it doesn't follow the naming convention!
	if (elementArray->size == 0)
		return ncArray;

	// check every single element according to its class
	for (int i = 0; i < elementArray->size; i++)
	{
		if (i > mapOrderArray->size-1)
			continue;

		// mapping with the desired element order
		if (mapOrderArray->data[i] == &undefined)
			continue;

		int dstIndex = mapOrderArray->data[i]->to_int();

		// element string
		Value* element = elementArray->data[i];
		TSTR elementStr = element->to_string();

		if (elementStr.operator == (elementEmpty) )
			continue;

		// this is the element definition
		Struct* elementDef = (Struct*)elementDefArray->data[i];
		
		// get the class and type of the elementDef
		Value* elementClass = GetValueProperty(elementDef, "class");
		TSTR elementClassStr = elementClass->to_string();

		// depending of the class of the element, proceed according to the NC
		// the text elements should not contain any special character
		if (elementClassStr.operator ==(_T("text")))
		{
			ncArray->data[dstIndex] = element;
		}
		// component elements are checked against all the components in the element definition
		else if (elementClassStr.operator ==(_T("component")))
		{
			Array* componentDefList = (Array*) GetValueProperty(elementDef, "componentList");

			BitArrayValue* indexArray = (BitArrayValue*) ncArray->data[dstIndex];
			indexArray->bits.SetSize(componentDefList->size,1);

			Array* componentArray = (Array*)nc_filterString(elementStr, componentSpacer);

			// name components
			for (int k = 0; k < componentArray->size; k++)
			{
				TSTR componentStr = componentArray->data[k]->to_string();
				
				if (componentStr.operator == (elementEmpty) )
					continue;
				
				// look for the component in the component list
				int componentIndex = nc_findStringItem(componentDefList, componentStr);
				if (componentIndex != -1)
					indexArray->bits.Set(componentIndex);
			}
		}
		// element type elements are checked against all the elements in the element definition
		else if (elementClassStr.operator ==(_T("element")))
		{
			Array* elementDefList = (Array*) GetValueProperty(elementDef, "elementList");
			
			BitArrayValue* indexArray = (BitArrayValue*) ncArray->data[dstIndex];
			indexArray->bits.SetSize(elementDefList ->size,1);
			
			int elementIndex = nc_findStringItem(elementDefList, elementStr);
			if (elementIndex != -1)
				indexArray->bits.Set(elementIndex);
		}

	}

	return ncArray;
}

def_struct_primitive( nc_GetNamingConventionAsIndexes,	blurNC, "getNamingConventionAsIndexes");

Value *nc_GetNamingConventionAsIndexes_cf(Value** arg_list, int count)
{
	Struct* NCDefStruct = (Struct*) arg_list[0];

	TSTR nameInStr = nc_getName(arg_list[1]);

	Value* orderElements = key_arg(orderElements);

	one_typed_value_local(Array* result);
	vl.result = new Array(0);

	if (nameInStr.isNull())
		return_value (vl.result);

	vl.result = nc_GetNamingConventionAsIndexes(nameInStr, orderElements, NCDefStruct);

	return_value (vl.result);
}
/*
Value *nc_GetNamingConventionAsIndexes_cf(Value** arg_list, int count)
{
	TSTR name;

	// get the name of the item to check
	if (is_node(arg_list[0]))
	{
		INode* node = arg_list[0]->to_node();
		name = node->GetName();
	}
	else if (is_string(arg_list[0]))
	{
		name = arg_list[0]->to_string();
	}
	else 
	{
		Value** argElement = (Value**)_alloca(sizeof(Value*) * 2);
		argElement[0] = new String("name");	
		
		Value* nameValue = arg_list[0]->get_property(argElement, 1);

		if (is_string(nameValue))
			name = nameValue->to_string();
		else
			return &false_value;
	}

	// do the checks!
	Struct* NCDefStruct = (Struct*) arg_list[1];

	// get the elements definition array
	Array* elementDefArray = (Array*) GetValueProperty(NCDefStruct, "elementDefArray");

	// elementSpacer, componentSpacer, componentEmpty, elementEmpty, specialCharacterArray
	Value* elementSpacer		=  GetValueProperty(NCDefStruct, "elementSpacer");
	Value* componentSpacer		= GetValueProperty(NCDefStruct, "componentSpacer");
	Value* componentEmpty		= GetValueProperty(NCDefStruct, "componentEmpty");
	Value* elementEmpty			= GetValueProperty(NCDefStruct, "elementEmpty");
	Array* specialCharacterArray= (Array*) GetValueProperty(NCDefStruct, "specialCharacterArray");

	TSTR elementSpacerStr		= elementSpacer->to_string();
	TSTR componentSpacerStr		= componentSpacer->to_string();
	TSTR componentEmptyStr		= componentEmpty->to_string();
	TSTR elementEmptyStr		= elementEmpty->to_string();

	// the result naming convention array
	Array* ncArray = new Array(elementDefArray->size);

	// get the elements in the name filtering with the elementSpacer
	Value** argElement = (Value**)_alloca(sizeof(Value*) * 2);
	argElement[0] = new String(name);	
	argElement[1] = elementSpacer;	
	Array* elementArray = (Array*)filterstring_cf(argElement, 2);

	// it maps the custom element order array to the one in the NC
	// if not orderlements array has been specified then it is exactly the same as the one in the NC
	Array* mapOrderArray = new Array(elementDefArray->size);

	Value* val = key_arg(orderElements);
	if (val != &unsupplied)
	{
		Array* orderElementArray = (Array*) val;
		for (int i = 0; i < orderElementArray->size; i++)
			ncArray->append(&undefined);

		for (int i = 0; i < elementDefArray->size; i++)
		{
			// this is the first element definition
			Struct* elementDef = (Struct*)elementDefArray->data[i];

			// get the first elementType
			Value* elementType = GetValueProperty(elementDef, "type");
			TSTR elementTypeStr = elementType->to_string();
			
			mapOrderArray->append(&undefined);
			for (int j = 0; j < orderElementArray->size; j++)
			{
				// get the second elementType
				TSTR element2TypeStr = orderElementArray->data[j]->to_string();
				
				if (element2TypeStr.operator ==(elementTypeStr))
					mapOrderArray->data[i] = (Integer::intern(j));
			}
		}

	}
	else
	{
		for (int i = 0; i < elementDefArray->size; i++)
		{
			mapOrderArray->append(Integer::intern(i));
			ncArray->append(&undefined);
		}
	}
	
	// fills the nc array with the default values
	Array* elementDefClassArray = new Array(elementDefArray->size);
	for (int i = 0; i < elementDefArray->size; i++)
	{
		if (mapOrderArray->data[i] == &undefined)
			continue;
		
		int dstIndex = mapOrderArray->data[i]->to_int();

		// this is the element definition
		Struct* elementDef = (Struct*)elementDefArray->data[i];

		// get the class and type of the elementDef
		Value** argElement = (Value**)_alloca(sizeof(Value*) * 1);

		argElement[0] = Name::intern("class");
		Value* elementClass = elementDef->get_property(argElement, 1);
		TSTR elementClassStr = elementClass->to_string();

		elementDefClassArray->append(elementClass);

		if (elementClassStr.operator ==(_T("text")))
			ncArray->data[dstIndex] = new String("");
		else
			ncArray->data[dstIndex] = new BitArrayValue();
	}

	// there is no elements, so it doesn't follow the naming convention!
	if (elementArray->size == 0)
		return ncArray;
	
	// check every single element according to its class
	for (int i = 0; i < elementArray->size; i++)
	{
		// mapping with the desired element order
		if (mapOrderArray->data[i] == &undefined)
			continue;

		int dstIndex = mapOrderArray->data[i]->to_int();

		// element string
		Value* element = elementArray->data[i];
		TSTR elementStr = element->to_string();

		if (elementStr.operator == (elementEmptyStr) )
			continue;

		// this is the element definition
		Struct* elementDef = (Struct*)elementDefArray->data[i];
		
		// get the class and type of the elementDef
		Value* elementClass =  GetValueProperty(elementDef, "class");//elementDefClassArray->data[i];
		TSTR elementClassStr = elementClass->to_string();

		// depending of the class of the element, proceed according to the NC
		// the text elements should not contain any special character
		if (elementClassStr.operator ==(_T("text")))
		{
			ncArray->data[dstIndex] = element;
		}
		// component elements are checked against all the components in the element definition
		else if (elementClassStr.operator ==(_T("component")))
		{
			argElement[0] = Name::intern("componentList");
			Array* componentDefList = (Array*) elementDef->get_property(argElement, 1);

			BitArrayValue* indexArray = (BitArrayValue*) ncArray->data[dstIndex];
			indexArray->bits.SetSize(componentDefList->size,1);

			Value** argElement = (Value**)_alloca(sizeof(Value*) * 2);
			argElement[0] = element;	
			argElement[1] = componentSpacer;	
			Array* componentArray = (Array*)filterstring_cf(argElement, 2);

			// name components
			for (int k = 0; k < componentArray->size; k++)
			{
				Value *componentValue = componentArray->data[k];
				TSTR componentStr = componentValue->to_string();
				
				if (componentStr.operator == (elementEmptyStr) )
					continue;

				// nc components
				for (int j = 0; j < componentDefList->size; j++)
				{
					Value *componentDefValue = componentDefList->data[j];
					TSTR componentDefStr = componentDefValue->to_string();
				
					if (componentDefStr.isNull())
						continue;

					if (componentDefStr.operator ==( componentStr ))
					{
						indexArray->bits.Set(j);
						break;
					}
				}
			}
		}
		// element type elements are checked against all the elements in the element definition
		else if (elementClassStr.operator ==(_T("element")))
		{
			argElement[0] = Name::intern("elementList");
			Array* elementDefList = (Array*)elementDef->get_property(argElement, 1);
			
			BitArrayValue* indexArray = (BitArrayValue*) ncArray->data[dstIndex];
			indexArray->bits.SetSize(elementDefList ->size,1);

			for (int j = 0; j < elementDefList ->size; j++)
			{
				Value *elementDefValue = elementDefList ->data[j];
				TSTR elementDefStr = elementDefValue->to_string();

				if (elementDefStr.isNull())
					continue;

				if (elementDefStr.operator ==( elementStr ))
					indexArray->bits.Set(j);
			}
		}

	}

	return ncArray;
}
*/
//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------
def_struct_primitive( nc_GetNamingConvention, blurNC, "GetNamingConvention");

Value* nc_GetNamingConvention_cf(Value** arg_list, int count)
{
	TSTR name;

	// get the name of the item to check
	if (is_node(arg_list[0]))
	{
		INode* node = arg_list[0]->to_node();
		name = node->GetName();
	}
	else if (is_string(arg_list[0]))
	{
		name = arg_list[0]->to_string();
	}
	else 
	{
		Value** argElement = (Value**)_alloca(sizeof(Value*) * 2);
		argElement[0] = new String("name");	
		
		Value* nameValue = arg_list[0]->get_property(argElement, 1);

		if (is_string(nameValue))
			name = nameValue->to_string();
		else
			return &false_value;
	}

	// do the checks!
	Struct* NCDefStruct = (Struct*) arg_list[1];

	// get the elements definition array
	Array* elementDefArray = (Array*) GetValueProperty(NCDefStruct, "elementDefArray");

	// elementSpacer, componentSpacer, componentEmpty, elementEmpty, specialCharacterArray
	Value* elementSpacer		=  GetValueProperty(NCDefStruct, "elementSpacer");
	Value* componentSpacer		= GetValueProperty(NCDefStruct, "componentSpacer");
	Value* componentEmpty		= GetValueProperty(NCDefStruct, "componentEmpty");
	Value* elementEmpty			= GetValueProperty(NCDefStruct, "elementEmpty");
	Array* specialCharacterArray= (Array*) GetValueProperty(NCDefStruct, "specialCharacterArray");

	TSTR elementSpacerStr		= elementSpacer->to_string();
	TSTR componentSpacerStr		= componentSpacer->to_string();
	TSTR componentEmptyStr		= componentEmpty->to_string();
	TSTR elementEmptyStr		= elementEmpty->to_string();

	// the result naming convention array
	Array* ncArray = new Array(elementDefArray->size);

	// get the elements in the name filtering with the elementSpacer
	Value** argElement = (Value**)_alloca(sizeof(Value*) * 2);
	argElement[0] = new String(name);	
	argElement[1] = elementSpacer;	
	Array* elementArray = (Array*)filterstring_cf(argElement, 2);

	// it maps the custom element order array to the one in the NC
	// if not orderlements array has been specified then it is exactly the same as the one in the NC
	Array* mapOrderArray = new Array(elementDefArray->size);

	Value* val = key_arg(orderElements);
	if (val != &unsupplied)
	{
		Array* orderElementArray = (Array*) val;
		for (int i = 0; i < orderElementArray->size; i++)
			ncArray->append(&undefined);

		for (int i = 0; i < elementDefArray->size; i++)
		{
			// this is the first element definition
			Struct* elementDef = (Struct*)elementDefArray->data[i];

			// get the first elementType
			Value* elementType = GetValueProperty(elementDef, "type");
			TSTR elementTypeStr = elementType->to_string();
			
			mapOrderArray->append(&undefined);
			for (int j = 0; j < orderElementArray->size; j++)
			{
				// get the second elementType
				TSTR element2TypeStr = orderElementArray->data[j]->to_string();
				
				if (element2TypeStr.operator ==(elementTypeStr))
					mapOrderArray->data[i] = (Integer::intern(j));
			}
		}

	}
	else
	{
		for (int i = 0; i < elementDefArray->size; i++)
		{
			mapOrderArray->append(Integer::intern(i));
			ncArray->append(&undefined);
		}
	}
	
	// fills the nc array with the default values
	Array* elementDefClassArray = new Array(elementDefArray->size);
	for (int i = 0; i < elementDefArray->size; i++)
	{
		if (mapOrderArray->data[i] == &undefined)
			continue;
		
		int dstIndex = mapOrderArray->data[i]->to_int();

		// this is the element definition
		Struct* elementDef = (Struct*)elementDefArray->data[i];

		// get the class and type of the elementDef
		Value** argElement = (Value**)_alloca(sizeof(Value*) * 1);

		argElement[0] = Name::intern("class");
		Value* elementClass = elementDef->get_property(argElement, 1);
		TSTR elementClassStr = elementClass->to_string();

		elementDefClassArray->append(elementClass);
		ncArray->data[dstIndex] = new Array(0);

		/*if (elementClassStr.operator ==(_T("text")))
			ncArray->data[dstIndex] = new String("X");
		else
			ncArray->data[dstIndex] = new String("X");*/
	}

	// there is no elements, so it doesn't follow the naming convention!
	if (elementArray->size == 0)
		return ncArray;
	
	// check every single element according to its class
	for (int i = 0; i < elementArray->size; i++)
	{
		// mapping with the desired element order
		if (mapOrderArray->data[i] == &undefined)
			continue;

		int dstIndex = mapOrderArray->data[i]->to_int();

		// element string
		Value* element = elementArray->data[i];
		TSTR elementStr = element->to_string();

		if (elementStr.operator == (elementEmptyStr) )
			continue;

		// this is the element definition
		Struct* elementDef = (Struct*)elementDefArray->data[i];
		
		// get the class and type of the elementDef
		Value* elementClass =  GetValueProperty(elementDef, "class");//elementDefClassArray->data[i];
		TSTR elementClassStr = elementClass->to_string();

		// depending of the class of the element, proceed according to the NC
		// the text elements should not contain any special character
		if (elementClassStr.operator ==(_T("text")))
		{
			ncArray->data[dstIndex] = element;
		}
		// component elements are checked against all the components in the element definition
		else if (elementClassStr.operator ==(_T("component")))
		{
			argElement[0] = Name::intern("componentList");
			Array* componentDefList = (Array*)elementDef->get_property(argElement, 1);

			Array* namesArray = (Array*) ncArray->data[dstIndex];

			Value** argElement = (Value**)_alloca(sizeof(Value*) * 2);
			argElement[0] = element;	
			argElement[1] = componentSpacer;	
			Array* componentArray = (Array*)filterstring_cf(argElement, 2);

			// name components
			for (int k = 0; k < componentArray->size; k++)
			{
				Value *componentValue = componentArray->data[k];
				TSTR componentStr = componentValue->to_string();
				
				if (componentStr.operator == (elementEmptyStr) )
					continue;

				// nc components
				for (int j = 0; j < componentDefList->size; j++)
				{
					Value *componentDefValue = componentDefList->data[j];
					TSTR componentDefStr = componentDefValue->to_string();
				
					if (componentDefStr.isNull())
						continue;

					if (componentDefStr.operator ==( componentStr ))
					{
						namesArray->append(componentValue);
						break;
					}
				}
			}
		}
		// element type elements are checked against all the elements in the element definition
		else if (elementClassStr.operator ==(_T("element")))
		{
			argElement[0] = Name::intern("elementList");
			Array* elementDefList = (Array*)elementDef->get_property(argElement, 1);

			Array* namesArray = (Array*) ncArray->data[dstIndex];

			for (int j = 0; j < elementDefList ->size; j++)
			{
				Value *elementDefValue = elementDefList ->data[j];
				TSTR elementDefStr = elementDefValue->to_string();

				if (elementDefStr.isNull())
					continue;

				if (elementDefStr.operator ==( elementStr ))
					namesArray->append(element);
			}
		}

	}

	TSTR finalName = _T("");

	for (int i = 0; i < ncArray->size; i++)
	{
		if (is_array(ncArray->data[i]))
		{
			Array* elementArray = (Array*) ncArray->data[i];
			if (elementArray->size == 0)
				finalName += elementEmptyStr;
			else if (elementArray->size == 1)
				finalName += elementArray->data[0]->to_string();
			else
			{
				for (int j = 0; j < elementArray->size; j++)
				{
					finalName += elementArray->data[j]->to_string();
					if (j != elementArray->size-1)
						finalName += componentSpacerStr;
				}
			}
		}
		else if (is_string(ncArray->data[i]))
		{
			finalName += ncArray->data[i]->to_string();
		}
		
		if (i != ncArray->size-1)
			finalName += elementSpacerStr;
	}

	return new String(finalName);
}

void blurNCInit()
{
	#include "defimpfn.h"
		def_name ( orderElements )
}