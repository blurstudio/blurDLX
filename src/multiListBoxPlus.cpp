//*****************************************************************************
/*
	\file  MultiListBoxPlus.cpp

	[TITLE]
	Implementation of a multiListBox as a Maxscript UI control

	[DESCRIPTION]
	 adds width & height properties to the normal multilistbox

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

// ----------------------------------------------------------------------------


#include "imports.h"

// ============================================================================
extern HINSTANCE g_hInst;
extern void ValueToBitArray(Value* inval, BitArray &theBitArray, int maxSize, TCHAR* errmsg = _T(""), int selTypesAllowed = 0);


#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
#include "macros/define_instantiation_functions.h"
#else
#include "definsfn.h"
#endif
	def_name ( multiListBoxPlus )
	def_name ( selectionEnd )
	//def_name ( multiSelection )

#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
#include "macros/define_external_functions.h"
#else
#include "defextfn.h"
#endif
	//def_name ( additem )

class MultiListBoxPlusControl;
visible_class (MultiListBoxPlusControl)

class MultiListBoxPlusControl : public RolloutControl
{
public:
	Array*		item_array;
	int			lastSelection;
	BitArray*	selection;
	int			m_width, m_height;

	// Constructor
	MultiListBoxPlusControl(Value* name, Value* caption, Value** keyparms, int keyparm_count)
			: RolloutControl(name, caption, keyparms, keyparm_count)
	{
		tag = class_tag(MultiListBoxPlusControl); 
		item_array = NULL;
		selection = new BitArray (0);
		m_width = 0;
	}
	~MultiListBoxPlusControl() { delete selection;}

	static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
						{ return new MultiListBoxPlusControl (name, caption, keyparms, keyparm_count); }


	classof_methods (MultiListBoxPlusControl, RolloutControl);

	// Garbage collection
	void		collect() {delete this; }

	// Print out the internal name of the control to MXS
	void		sprin1(CharStream* s) { s->printf(_T("MultiListBoxPlusControl:%s"), name->to_string()); }

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

		const TCHAR * label_text = caption->eval()->to_string();
		int label_height = (_tcslen(label_text) != 0) ? ro->text_height + SPACING_BEFORE - 2 : 0;
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
		const TCHAR * label_text = caption->eval()->to_string();

		// add 2 controls for a list box: a static label & the list
		parent_rollout = ro;
		control_ID = next_id();
		WORD label_id = next_id();
		WORD list_box_id = control_ID;

		int label_height = (_tcslen(label_text) != 0) ? ro->text_height + SPACING_BEFORE - 2 : 0;
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
			GetTextExtentPoint32(ro->rollout_dc, label_text, (int)_tcslen(label_text), &size);
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
		list_box = CreateWindowEx(WS_EX_CLIENTEDGE, 
								_T("LISTBOX"),
								_T(""),
								LBS_EXTENDEDSEL | LBS_NOINTEGRALHEIGHT | LBS_NOTIFY | WS_BORDER | WS_VSCROLL | WS_VISIBLE | WS_CHILD | WS_TABSTOP,
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
			const TCHAR* item = item_array->data[i]->to_string();
			SendMessage(list_box, LB_ADDSTRING, 0, (LPARAM)item);
			GetTextExtentPoint32(ro->rollout_dc, item, (int)_tcslen(item), &size); 	
			if (size.cx > max_width) max_width = size.cx;
			SendMessage(list_box, LB_SETSEL, (*selection)[i], i);
		}
		m_width = width;
		m_height = height;
	}

	BOOL handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam)
	{
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
			const TCHAR * text = val->to_string();
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
		else if (prop == n_width)
		{
			m_width = val->to_int();

			HWND list_box = GetDlgItem(parent_rollout->page, control_ID);
			RECT	rect;
			GetWindowRect(list_box, &rect);
			MapWindowPoints(NULL, parent_rollout->page,	(LPPOINT)&rect, 2);
			SetWindowPos(list_box, NULL, rect.left, rect.top, m_width, rect.bottom-rect.top, SWP_NOZORDER);
		}
		else if (prop == n_height)
		{
			m_height = val->to_int();

			HWND list_box = GetDlgItem(parent_rollout->page, control_ID);
			RECT	rect;
			GetWindowRect(list_box, &rect);
			MapWindowPoints(NULL, parent_rollout->page,	(LPPOINT)&rect, 2);
			SetWindowPos(list_box, NULL, rect.left, rect.top, rect.right-rect.left, m_height, SWP_NOZORDER);
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
		else if (prop == n_width)
			return Integer::intern(m_width);
		else if (prop == n_height)
			return Integer::intern(m_height);
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

visible_class_instance (MultiListBoxPlusControl, "MultiListBoxPlusControl");

// ============================================================================
void MultiListBoxPlusCtrlInit()
{
#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
	#include "macros/define_implementations.h"
#else
	#include "defimpfn.h"
#endif
		def_name ( multiListBoxPlus )
		def_name ( selectionEnd )

	install_rollout_control(n_multiListBoxPlus, MultiListBoxPlusControl::create);
}

