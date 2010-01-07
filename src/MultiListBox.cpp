// ============================================================================
// TreeView.cpp
// Copyright ©2005 Blur Studios Inc.
// Created by Diego Garcia
// ----------------------------------------------------------------------------


/*#include "MAXScrpt.h"
#include "MAXObj.h"
#include "meshdelta.h"
#include "Parser.h"

#include "value.h"*/

#include "MAXScrpt.h"
#include "meshdelta.h"
#include "MAXObj.h"
#include "Numbers.h"
#include "MeshSub.h"
//#include "3DMath.h"
#include "Parser.h"

extern HINSTANCE g_hInst;


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

void ValueToBitArray(Value* inval, BitArray &theBitArray, int maxSize, TCHAR* errmsg = _T(""), int selTypesAllowed = 0)
{
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

#include "resource.h"

/*#include "lclinsfn.h"		// no idea why, but these two lines HAVE to be here in order to compile...
#	include "tvnames.h"*/

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )


#include "definsfn.h"
	def_name ( multiListBoxPlus )
	def_name ( selectionEnd )


//  =================================================
//  UI Items
//  =================================================


/* -------------------- MultiListBoxControl  ------------------- */
/*
rollout test "test"
(MultiListBox mlb "MultiListBox" items:#("A","B","C") selection:#(1,3)
on mlb selected val do format "selected: % - %\n" val mlb.selection[val]
on mlb doubleclicked val do format "doubleclicked: % - %\n" val mlb.selection[val]
on mlb selectionEnd do format "selectionEnd: %\n" mlb.selection
)
rof=newrolloutfloater "tester" 200 300
addrollout test rof
test.mlb.items
test.mlb.selection=1
test.mlb.selection=#(1,3)
test.mlb.selection=#{}

*/
class MultiListBoxControl;
visible_class (MultiListBoxControl)

class MultiListBoxControl : public RolloutControl
{
public:
	Array*		item_array;
	int			lastSelection;
	BitArray*	selection;

	// Constructor
	MultiListBoxControl(Value* name, Value* caption, Value** keyparms, int keyparm_count)
			: RolloutControl(name, caption, keyparms, keyparm_count)
	{
		tag = class_tag(MultiListBoxControl); 
		item_array = NULL;
		selection = new BitArray (0);
	}
	~MultiListBoxControl() { delete selection;}

	static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
						{ return new MultiListBoxControl (name, caption, keyparms, keyparm_count); }


	classof_methods (MultiListBoxControl, RolloutControl);

	// Garbage collection
	void		collect() {delete this; }

	// Print out the internal name of the control to MXS
	void		sprin1(CharStream* s) { s->printf(_T("MultiListBoxControl:%s"), name->to_string()); }

	// Return the window class name 
	LPCTSTR		get_control_class() { return _T("MULTILISTBOX"); }
		
	int			num_controls() { return 2; }

	// Top-level call for changing rollout layout. We don't process this.
	void		compute_layout(Rollout *ro, layout_data* pos) { }

	void gc_trace()
	{
		RolloutControl::gc_trace();

		if (item_array && item_array->is_not_marked())
			item_array->gc_trace();
	}


		
	void compute_layout(Rollout *ro, layout_data* pos, int& current_y)
	{
		setup_layout(ro, pos, current_y);

		TCHAR*	label_text = caption->eval()->to_string();
		int label_height = (strlen(label_text) != 0) ? ro->text_height + SPACING_BEFORE - 2 : 0;
		Value* height_obj;
		int item_count = int_control_param(height, height_obj, 10);
		int lb_height = item_count * ro->text_height + 7;

		process_layout_params(ro, pos, current_y);
		pos->height = label_height + lb_height;
		current_y = pos->top + pos->height;
	}

	// Add the control itself to a rollout window
	void add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y)
	{
		HWND	label, list_box;
		int		left, top, width, height;
		SIZE	size;
		TCHAR*	label_text = caption->eval()->to_string();

		// add 2 controls for a list box: a static label & the list
		parent_rollout = ro;
		control_ID = next_id();
		WORD label_id = next_id();
		WORD list_box_id = control_ID;

		int label_height = (strlen(label_text) != 0) ? ro->text_height + SPACING_BEFORE - 2 : 0;
		Value* height_obj;
		int item_count = int_control_param(height, height_obj, 10);
		int lb_height = item_count * ro->text_height + 7;

		layout_data pos;
		compute_layout(ro, &pos, current_y);

		// place optional label
		left = pos.left;
		// LAM - defect 298613 - not creating the caption HWND was causing problems (whole screen redrawn
		// when control moved, setting caption text set wrong HWND). Now always create.
//		if (label_height != 0)
//		{
			GetTextExtentPoint32(ro->rollout_dc, label_text, strlen(label_text), &size); 	
			width = min(size.cx, pos.width); height = ro->text_height;
			top = pos.top; 
			label = CreateWindow(_T("STATIC"),
									label_text,
									WS_VISIBLE | WS_CHILD | WS_GROUP,
									left, top, width, height,    
									parent, (HMENU)label_id, hInstance, NULL);
//		}

		// place list box
		top = pos.top + label_height;
		width = pos.width; height = lb_height;
		list_box = CreateWindowEx(WS_EX_TOOLWINDOW, 
								_T("LISTBOX"),
								_T(""),
								//LBS_OWNERDRAWFIXED | 
								LBS_HASSTRINGS | LBS_EXTENDEDSEL | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY | WS_BORDER | WS_VSCROLL | WS_VISIBLE | WS_CHILD | WS_TABSTOP, // LBS_OWNERDRAWFIXED | 
								left, top, width, height,    
								parent, (HMENU)list_box_id, hInstance, NULL);

		SendMessage(label, WM_SETFONT, (WPARAM)ro->font, 0L);
		SendMessage(list_box, WM_SETFONT, (WPARAM)ro->font, 0L);

		// fill up the list
		if (ro->init_values)
		{
			one_value_local(selection);
			item_array = (Array*)control_param(items);
			if (item_array == (Array*)&unsupplied)
				item_array = new Array (0);
			type_check(item_array, Array, _T("items:"));
			vl.selection = control_param(selection);
			if (vl.selection != &unsupplied) 
				ValueToBitArray(vl.selection, *selection, -1);
			selection->SetSize(item_array->size,1);
			pop_value_locals();
		}

		// add items from array to combobox and compute widest entry & set minwidth
		int max_width = 0;
		SendMessage(list_box, LB_RESETCONTENT, 0, 0);
		for (int i = 0; i < item_array->size; i++)
		{
			TCHAR* item = item_array->data[i]->to_string();
			SendMessage(list_box, LB_ADDSTRING, 0, (LPARAM)item);
			GetTextExtentPoint32(ro->rollout_dc, item, strlen(item), &size); 	
			if (size.cx > max_width) max_width = size.cx;
			SendMessage(list_box, LB_SETSEL, (*selection)[i], i);
		}
	}

	BOOL handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam)
	{
		int i = message;
		CharStream* out = thread_local(current_stdout); 
		out->printf(_T("Message ID: %i\n"), i);

		
		if (message == WM_MEASUREITEM)
		{
			out->printf(_T("WM_MEASUREITEM ID: %d\n"), i);
			int i = 0;
		}
		if (message == WM_DRAWITEM)
		{
			out->printf(_T("WM_DRAWITEM ID: %d\n"), i);
			int i = 0;
		}

		if (message == WM_PAINT)
		{
			out->printf(_T("WM_PAINT ID: %d\n"), i);
			int i = 0;
		}

		if (message == WM_COMMAND)
		{
			if (HIWORD(wParam) == LBN_SELCHANGE)
			{	int i;
				HWND list_box = GetDlgItem(parent_rollout->page, control_ID);
				int numSel = SendMessage(list_box,LB_GETSELCOUNT,0,0);
				int* selArray = new int[numSel];
				SendMessage(list_box,LB_GETSELITEMS,numSel,(LPARAM)selArray);
				lastSelection = SendMessage(list_box,LB_GETCURSEL,0,0);
				BitArray newSelection (item_array->size);
				for (i = 0; i < numSel; i++) 
					newSelection.Set(selArray[i]);
				one_value_local(sel);
				BOOL selChanged = FALSE;
				for (i = 0; i < item_array->size; i++) {
					if (newSelection[i] != (*selection)[i]) {
						selection->Set(i,newSelection[i]);
						vl.sel = Integer::intern(i + 1);
						call_event_handler(ro, n_selected, &vl.sel, 1);
						selChanged = TRUE;
					}
				}
				pop_value_locals();
				delete [] selArray;
				if (selChanged) call_event_handler(ro, n_selectionEnd, NULL, 0);
			}
			else if (HIWORD(wParam) == LBN_DBLCLK)
			{
				one_value_local(sel);
				vl.sel = Integer::intern(lastSelection + 1);
				call_event_handler(ro, n_doubleClicked, &vl.sel, 1); 
				pop_value_locals();
			}
			return TRUE;
		}
		return FALSE;
	}

	Value* set_property(Value** arg_list, int count)
	{
		Value* val = arg_list[0];
		Value* prop = arg_list[1];

		if (prop == n_text || prop == n_caption)
		{
			TCHAR* text = val->to_string();
			caption = val->get_heap_ptr();
			if (parent_rollout != NULL && parent_rollout->page != NULL)
				set_text(text, GetDlgItem(parent_rollout->page, control_ID + 1), n_left);
		}
		else if (prop == n_items)
		{
			type_check(val, Array, _T("items:"));
			item_array = (Array*)val;
			selection->SetSize(item_array->size,1);
			if (parent_rollout != NULL && parent_rollout->page != NULL)
			{
				HWND list_box = GetDlgItem(parent_rollout->page, control_ID);
				SendMessage(list_box, LB_RESETCONTENT, 0, 0);
				for (int i = 0; i < item_array->size; i++) {
					SendMessage(list_box, LB_ADDSTRING, 0, (LPARAM)item_array->data[i]->to_string());
					SendMessage(list_box, LB_SETSEL, (*selection)[i], i);
				}

			}
		}
		else if (prop == n_selection)
		{
			selection->ClearAll();
			ValueToBitArray(val, *selection, -1);
			selection->SetSize(item_array->size,1);
			if (parent_rollout != NULL && parent_rollout->page != NULL)
			{
				HWND list_box = GetDlgItem(parent_rollout->page, control_ID);
				for (int i = 0; i < item_array->size; i++) 
					SendMessage(list_box, LB_SETSEL, (*selection)[i], i);
			}
		}
		else
			return RolloutControl::set_property(arg_list, count);

		return val;
	}

	Value* get_property(Value** arg_list, int count)
	{
		Value* prop = arg_list[0];

		if (prop == n_items)
			return item_array;
		else if (prop == n_selection)
			return new BitArrayValue(*selection);
		else if (prop == n_doubleClicked || prop == n_selected || prop == n_selectionEnd)
			return get_event_handler(prop);
		else
			return RolloutControl::get_property(arg_list, count);
	}

	void set_enable()
	{
		if (parent_rollout != NULL && parent_rollout->page != NULL)
		{
			// set combox enable
			EnableWindow(GetDlgItem(parent_rollout->page, control_ID), enabled);
			// set caption enable
			HWND ctrl = GetDlgItem(parent_rollout->page, control_ID + 1);
			if (ctrl)
				EnableWindow(ctrl, enabled);
		}
	}
};

visible_class_instance (MultiListBoxControl, "MultiListBoxControl");


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
void MultiListBoxInit()
{
	#include "defimpfn.h"
	def_name ( multiListBoxPlus )
	def_name ( selectionEnd )

	// initialize variables to hold names used as keyword parameters
	install_rollout_control(n_multiListBoxPlus,		MultiListBoxControl::create);	
}
