//*****************************************************************************
/*
	\file  ListBoxPlus.cpp

	[TITLE]
	Implementation of a ListBox as a Maxscript UI control

	[DESCRIPTION]
	

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

#include "imports.h"

#include "3dsmaxport.h"

// ============================================================================
extern HINSTANCE g_hInst;
extern void ValueToBitArray(Value* inval, BitArray &theBitArray, int maxSize, TCHAR* errmsg = _T(""), int selTypesAllowed = 0);
extern COLORREF DarkenColour(COLORREF col, double factor);
extern COLORREF LightenColour(COLORREF col, double factor);
// ============================================================================

#include "resource.h"

#define LISTBOXCTRL_WINDOWCLASS _T("LISTBOXCTRL_WINDOWCLASS")
#define TOOLTIP_ID 1


#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
#include "maxscript\macros\define_instantiation_functions.h"
#else
#include "definsfn.h"
#endif
	def_name ( multiSelection )
	def_name ( locked )
	def_name ( escpressed )
	def_name ( enterPressed )

#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
#include "maxscript\macros\define_external_functions.h"
#else
#include "defextfn.h"
#endif
	def_name ( fontName )
	def_name ( click )
	def_name ( dblclick )
	def_name ( mousedown )
	def_name ( mouseup )
	def_name ( mouseover )
	def_name ( mouseout )
	def_name ( rclick )
	def_name ( rdblclick )
	def_name ( rmousedown )
	def_name ( rmouseup )
	def_name ( hwnd )
//def_name ( additem )




// ============================================================================
visible_class(ListBoxPlusControl)

class ListBoxPlusControl : public RolloutControl
{
protected:
	HWND		m_hWnd;
	HFONT		m_hFont;
	int			m_size;
	TSTR		m_fontName;
	BOOL		m_multiSelection;
	COLORREF	m_DefaultColor;
	COLORREF	m_DefaultBkColor;
	bool		m_bMouseOver;

	Array*		m_item_array;
	Array*		m_color_array;
	int			m_lastSelection;
	BitArray*	m_selection;
	BitArray*	m_indeterminate;
	BitArray*	m_locked;
	BOOL		m_lButtonDown, m_dragging;
	POINT		m_startDragPoint, m_curPoint, m_endDragPoint;


public:
	// Static methods
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
		{ return new ListBoxPlusControl(name, caption, keyparms, keyparm_count); }

	// Property methods
	void  Invalidate();

	// Event handlers
	LRESULT LButtonDown(int xPos, int yPos, int fwKeys);
	LRESULT LButtonUp(int xPos, int yPos, int fwKeys);
	LRESULT LButtonDblClk(int xPos, int yPos, int fwKeys);
	LRESULT RButtonDown(int xPos, int yPos, int fwKeys);
	LRESULT RButtonUp(int xPos, int yPos, int fwKeys);
	LRESULT RButtonDblClk(int xPos, int yPos, int fwKeys);
	LRESULT MouseMove(int xPos, int yPos, int fwKeys);
	LRESULT MouseOut(int xPos, int yPos, int fwKeys);
	LRESULT MouseOver(int xPos, int yPos, int fwKeys);
	LRESULT EscPressed(int xPos, int yPos, int fwKeys);
	LRESULT EnterPressed(int xPos, int yPos, int fwKeys);
	LRESULT EraseBackground(HDC hDC);
	LRESULT Paint();

	void invertItem(int ItemID);
	void selectItem(int ItemID);
	void unSelectItem(int ItemID);
	int getItemId(int xPos, int yPos);

	void SetFont(const TCHAR *nameFont, float size, BOOL bold);

	// MAXScript event handlers
	ListBoxPlusControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	~ListBoxPlusControl();

	         classof_methods(ListBoxPlusControl, RolloutControl);
	void     gc_trace();
	void     collect() { delete this; }
	void     sprin1(CharStream* s) { s->printf(_T("ListBoxPlusControl:%s"), name->to_string()); }

	void     add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
	LPCTSTR  get_control_class() { return SPINNERWINDOWCLASS; }
	BOOL     handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);

	int		 num_controls() { return 1; }

	Value*   get_property(Value** arg_list, int count);
	Value*   set_property(Value** arg_list, int count);
	void     set_enable();
};

// ============================================================================
ListBoxPlusControl::ListBoxPlusControl(Value* name, Value* caption, Value** keyparms, int keyparm_count)
	: RolloutControl(name, caption, keyparms, keyparm_count)
{
	tag = class_tag(ListBoxPlusControl);

	m_hWnd			= NULL;
	m_item_array	= NULL;
	m_color_array	= NULL;
	m_selection		= new BitArray (0);
	m_indeterminate	= new BitArray (0);
	m_locked		=  new BitArray (0);
	m_multiSelection = TRUE;
	m_DefaultColor	= GetCustSysColor(COLOR_BTNTEXT);
 	m_DefaultBkColor = GetCustSysColor(COLOR_WINDOW);
	m_bMouseOver = false;

	m_hFont = NULL;
	m_size = 14;
	m_fontName = _T("Arial");

	m_lButtonDown = FALSE;
	m_dragging	  = FALSE;
}

ListBoxPlusControl::~ListBoxPlusControl()
{
	if(m_hFont) DeleteObject(m_hFont);
}


void ListBoxPlusControl::gc_trace()
{
	RolloutControl::gc_trace();

	if (m_item_array && m_item_array->is_not_marked())
		m_item_array->gc_trace();

	if (m_color_array && m_color_array->is_not_marked())
		m_color_array->gc_trace();

	/*if (m_selection && m_selection->is_not_marked())
		m_selection->gc_trace();

	if (m_indeterminate && m_indeterminate->is_not_marked())
		m_indeterminate->gc_trace();

	if (m_locked && m_locked->is_not_marked())
		m_locked->gc_trace();*/

}

// ============================================================================
void ListBoxPlusControl::Invalidate()
{
	if (m_hWnd == NULL) return;
	RECT rect;
	GetClientRect(m_hWnd, &rect);
	MapWindowPoints(m_hWnd, parent_rollout->page, (POINT*)&rect, 2);
	InvalidateRect(parent_rollout->page, &rect, TRUE);
	InvalidateRect(m_hWnd, NULL, TRUE);
}


// ============================================================================
LRESULT CALLBACK ListBoxPlusControl::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	ListBoxPlusControl *ctrl = DLGetWindowLongPtr<ListBoxPlusControl*>(hWnd, GWLP_USERDATA);
	if(ctrl == NULL && msg != WM_CREATE)
		return DefWindowProc(hWnd, msg, wParam, lParam);

	switch(msg)
	{
	case WM_CREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			ctrl = (ListBoxPlusControl*)lpcs->lpCreateParams;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (DWORD_PTR)ctrl);
			break;
		}

	case WM_LBUTTONDOWN:
		return ctrl->LButtonDown((short)LOWORD(lParam), (short)HIWORD(lParam), wParam);

	case WM_LBUTTONUP:
		return ctrl->LButtonUp((short)LOWORD(lParam), (short)HIWORD(lParam), wParam);

	case WM_LBUTTONDBLCLK:
		return ctrl->LButtonDblClk((short)LOWORD(lParam), (short)HIWORD(lParam), wParam);

	case WM_RBUTTONDOWN:
		return ctrl->RButtonDown((short)LOWORD(lParam), (short)HIWORD(lParam), wParam);

	case WM_RBUTTONUP:
		return ctrl->RButtonUp((short)LOWORD(lParam), (short)HIWORD(lParam), wParam);

	case WM_RBUTTONDBLCLK:
		return ctrl->RButtonDblClk((short)LOWORD(lParam), (short)HIWORD(lParam), wParam);

	case WM_MOUSEMOVE:
		return ctrl->MouseMove((short)LOWORD(lParam), (short)HIWORD(lParam), wParam);

	case WM_KILLFOCUS:
		ctrl->m_bMouseOver = false;
		ctrl->run_event_handler(ctrl->parent_rollout, n_mouseout, NULL, 0);
		break;

	case WM_ERASEBKGND:
		return ctrl->EraseBackground((HDC)wParam);

	case WM_PAINT:
		return ctrl->Paint();

	case WM_KEYUP:
		if ((TCHAR)wParam == VK_ESCAPE) 
			return ctrl->EscPressed((short)LOWORD(lParam), (short)HIWORD(lParam), wParam);
		if ((TCHAR)wParam == VK_RETURN) 
			return ctrl->EnterPressed((short)LOWORD(lParam), (short)HIWORD(lParam), wParam);

	case WM_KEYDOWN:
		if ((TCHAR)wParam == VK_ESCAPE) 
			return ctrl->EscPressed((short)LOWORD(lParam), (short)HIWORD(lParam), wParam);

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return FALSE;
}



int ListBoxPlusControl::getItemId(int xPos, int yPos)
{
	int text_height = parent_rollout->text_height + 1;
	return (yPos / text_height);
}

void ListBoxPlusControl::invertItem(int ItemID)
{
	bool locked = (*m_locked)[ItemID] == 1;
	if (locked)
		return;

	bool selected = (*m_selection)[ItemID] == 1;
	bool indeterminate = (*m_indeterminate)[ItemID] == 1;	

	if (indeterminate) 
	{
		m_indeterminate->Set(ItemID, false);
		selected = false;
	}
	else
		selected = !selected;
		
	if (!m_multiSelection)
		m_selection->ClearAll();

	m_selection->Set(ItemID, selected);	
}

void ListBoxPlusControl::selectItem(int ItemID)
{
	bool locked = (*m_locked)[ItemID] == 1;
	if (locked)
		return;

	if (!m_multiSelection)
		m_selection->ClearAll();

	m_selection->Set(ItemID, true);	
	m_indeterminate->Set(ItemID, false);
}

void ListBoxPlusControl::unSelectItem(int ItemID)
{
	bool locked = (*m_locked)[ItemID] == 1;
	if (locked)
		return;

	m_selection->Set(ItemID, false);	
}



// ============================================================================
LRESULT ListBoxPlusControl::LButtonDown(int xPos, int yPos, int fwKeys)
{
	m_lButtonDown = TRUE;

	m_startDragPoint.x = xPos;
	m_startDragPoint.y = yPos;

	init_thread_locals();
	push_alloc_frame();
	one_value_local(arg);
	
	SetFocus(m_hWnd);
	int idSelected = getItemId(xPos, yPos);
	vl.arg = Integer::intern((int)idSelected+1);
	run_event_handler(parent_rollout, n_mousedown, &vl.arg, 1);
	SetCapture(m_hWnd);

	pop_value_locals();
	pop_alloc_frame();

	return 0;
}


// ============================================================================
LRESULT ListBoxPlusControl::LButtonUp(int xPos, int yPos, int fwKeys)
{
	if (!m_dragging)
	{
		m_endDragPoint.x = xPos;
		m_endDragPoint.y = yPos;
		m_startDragPoint.x = xPos;
		m_startDragPoint.y = yPos;
	}
	ReleaseCapture();

	m_lButtonDown = FALSE;
	m_dragging = FALSE;

	int startSelId = getItemId(m_startDragPoint.x, m_startDragPoint.y);
	int endSelId = getItemId(m_endDragPoint.x, m_endDragPoint.y);
	
	if (!m_multiSelection)
	{
		startSelId = endSelId;
	}

	if (startSelId > endSelId)
	{
		int tempId = startSelId;
		startSelId = endSelId;
		endSelId = tempId;
	}

	init_thread_locals();
	push_alloc_frame();
	one_value_local(arg);

	for (int idSelected = startSelId; idSelected <= endSelId ; ++idSelected)
	{
		invertItem(idSelected);

		vl.arg = Integer::intern((int)idSelected+1);
		run_event_handler(parent_rollout, n_mouseup, &vl.arg, 1);
		run_event_handler(parent_rollout, n_click, &vl.arg, 1);
	}

	pop_value_locals();
	pop_alloc_frame();
		
	Invalidate();



	return 0;
}

// ============================================================================
LRESULT ListBoxPlusControl::LButtonDblClk(int xPos, int yPos, int fwKeys)
{
	init_thread_locals();
	push_alloc_frame();
	one_value_local(arg);
	
	int idSelected = getItemId(xPos, yPos);
	vl.arg = Integer::intern((int)idSelected+1);

	SetFocus(m_hWnd);
	run_event_handler(parent_rollout, n_dblclick, &vl.arg, 1);
	SetCapture(m_hWnd);

	pop_value_locals();
	pop_alloc_frame();


	return 0;
}

// ============================================================================
LRESULT ListBoxPlusControl::RButtonDown(int xPos, int yPos, int fwKeys)
{
	init_thread_locals();
	push_alloc_frame();
	one_value_local(arg);
	
	int idSelected = getItemId(xPos, yPos);
	vl.arg = Integer::intern((int)idSelected+1);

	SetFocus(m_hWnd);
	run_event_handler(parent_rollout, n_rmousedown, &vl.arg, 1);
	SetCapture(m_hWnd);

	pop_value_locals();
	pop_alloc_frame();

	return 0;
}

// ============================================================================
LRESULT ListBoxPlusControl::RButtonUp(int xPos, int yPos, int fwKeys)
{
	run_event_handler(parent_rollout, n_rmouseup, NULL, 0);
	run_event_handler(parent_rollout, n_rclick, NULL, 0);

	return 0;
}

// ============================================================================
LRESULT ListBoxPlusControl::RButtonDblClk(int xPos, int yPos, int fwKeys)
{
	SetFocus(m_hWnd);
	run_event_handler(parent_rollout, n_rdblclick, NULL, 0);
	SetCapture(m_hWnd);

	return 0;
}

// ============================================================================
LRESULT ListBoxPlusControl::MouseMove(int xPos, int yPos, int fwKeys)
{
	// detect if the mouse is inside the list
	/*if(m_bMouseOver)
	{
		RECT rect;
		POINT pnt = {xPos,yPos};
		GetClientRect(m_hWnd, &rect);
		if(!PtInRect(&rect, pnt))
		{
			m_bMouseOver = false;
			ReleaseCapture();
			run_event_handler(parent_rollout, n_mouseout, NULL, 0);
		}
	}
	else
	{
		m_bMouseOver = true;
		run_event_handler(parent_rollout, n_mouseover, NULL, 0);
		SetCapture(m_hWnd);
	}*/
	
	// detects if the mouse is On an item of the list
	init_thread_locals();
	push_alloc_frame();
	one_value_local(arg);
	
	int idSelected = getItemId(xPos, yPos);
	vl.arg = Integer::intern((int)idSelected+1);

	run_event_handler(parent_rollout, n_mousemove, &vl.arg, 1);

	pop_value_locals();
	pop_alloc_frame();
	
	run_event_handler(parent_rollout, n_mouseover, NULL, 0);

	//run_event_handler(parent_rollout, n_mousemove, NULL, 0);

	return 0;
}

LRESULT ListBoxPlusControl::MouseOut(int xPos, int yPos, int fwKeys)
{
	run_event_handler(parent_rollout, n_mouseout, NULL, 0);
	return 0;
}

LRESULT ListBoxPlusControl::EscPressed(int xPos, int yPos, int fwKeys)
{
	run_event_handler(parent_rollout, n_escpressed, NULL, 0);

	return 0;	
}

LRESULT ListBoxPlusControl::EnterPressed(int xPos, int yPos, int fwKeys)
{
	run_event_handler(parent_rollout, n_enterPressed, NULL, 0);

	return 0;	
}




// ============================================================================
LRESULT ListBoxPlusControl::EraseBackground(HDC hDC)
{
	return DefWindowProc(m_hWnd, WM_ERASEBKGND, (WPARAM)hDC, 0);
}

// ============================================================================
LRESULT ListBoxPlusControl::Paint()
{
	PAINTSTRUCT ps;
	HDC hDC = BeginPaint(m_hWnd, &ps);
	SetBkMode(hDC, TRANSPARENT);

	RECT rect;
	GetClientRect(m_hWnd, &rect);
	
	int text_height = parent_rollout->text_height + 1;

	// background
	HBRUSH sBrush = CreateSolidBrush(m_DefaultBkColor);
	SelectObject(hDC,sBrush);
	Rectangle(hDC, rect.left, rect.top, rect.right, rect.bottom); 
	DeleteObject(sBrush);

	for (int i = 0; i < m_item_array->size; ++i)
	{
		RECT itemRect;
		itemRect.left = rect.left + 1;
		itemRect.right = rect.right - 1;
		itemRect.top = (text_height * i) + 1;
		itemRect.bottom = (text_height * (i+1));

		const TCHAR* text = m_item_array->data[i]->to_string();
		bool selected = (*m_selection)[i] == 1;
		bool indeterminate = (*m_indeterminate)[i] == 1;
		
		COLORREF m_color = m_DefaultColor;

		if (i < m_color_array->size)
			m_color = m_color_array->data[i]->to_colorref();

		COLORREF m_BkColor = LightenColour(m_color, 0.75f);
		HPEN hpen = CreatePen(PS_SOLID, 1, DarkenColour(m_BkColor, 0.25f));
		HBRUSH bkBrush = indeterminate ? CreateHatchBrush(HS_DIAGCROSS, m_BkColor) : CreateSolidBrush(m_BkColor) ;
		
		if (selected || indeterminate)
		{
			SelectObject(hDC, hpen);
			SelectObject(hDC, bkBrush);
	 		Rectangle(hDC, itemRect.left, itemRect.top, itemRect.right, itemRect.bottom);
		}
		
		SelectFont(hDC, m_hFont);
		SetTextColor(hDC, m_color);

		DrawText(hDC, text, (int)_tcslen(text), &itemRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

		DeleteObject(hpen);
		DeleteObject(bkBrush);
	}
	
	if (m_dragging)
	{
		RECT selectionRect;
		selectionRect.left	 = min (m_startDragPoint.x, m_endDragPoint.x);
		selectionRect.top	 = min (m_startDragPoint.y, m_endDragPoint.y);
		selectionRect.right  = max (m_startDragPoint.x, m_endDragPoint.x);
		selectionRect.bottom = max (m_startDragPoint.y, m_endDragPoint.y);
		
		DrawFocusRect(hDC, &selectionRect);
	}

	EndPaint(m_hWnd, &ps);
	return 0;
}


// ============================================================================
visible_class_instance (ListBoxPlusControl, "ListBoxPlusControl")

void ListBoxPlusControl::add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y)
{
	caption = caption->eval();

/*	HWND	label;
	int		left, top, width, height;
	SIZE	size;*/
	const TCHAR *label_text = caption->eval()->to_string();

	parent_rollout = ro;
	control_ID = next_id();

	layout_data pos;
	process_layout_params(ro, &pos, current_y);
	//compute_layout(ro, &pos, current_y);

	Value *val = control_param(size);
	if(val != &unsupplied)
		m_size = val->to_int();
	else
		m_size = 14;

	m_hWnd = CreateWindow(
		LISTBOXCTRL_WINDOWCLASS,
		TEXT(""),
		WS_VISIBLE | WS_CHILD | WS_GROUP , //| WS_OVERLAPPED , // dgh 22 nov 05 - testing window on top
		pos.left, pos.top, pos.width, pos.height,    
		parent, (HMENU)control_ID, g_hInst, this);
	
	LOGFONT lf;
	GetObject(ro->font, sizeof(LOGFONT), &lf);
	lf.lfHeight = m_size;					
	_tcscpy(lf.lfFaceName, m_fontName.data());
	m_hFont = CreateFontIndirect(&lf);

	SendDlgItemMessage(parent, control_ID, WM_SETFONT, (WPARAM)m_hFont, 0L);
	SendDlgItemMessage(m_hWnd, control_ID, WM_SETFONT, (WPARAM)m_hFont, 0L);

	// fill up the list
	if (ro->init_values)
	{
		three_value_locals(selection, indeterminate, locked);
		m_item_array = (Array*)control_param(items);
		if (m_item_array == (Array*)&unsupplied)
			m_item_array = new Array (0);
		type_check(m_item_array, Array, _T("items:"));

		m_color_array = (Array*)control_param(color);
		if (m_color_array == (Array*)&unsupplied)
			m_color_array = new Array (0);
		type_check(m_color_array, Array, _T("color:"));

		vl.selection = control_param(selection);
		if (vl.selection != &unsupplied) 
			ValueToBitArray(vl.selection, *m_selection, -1);
		m_selection->SetSize(m_item_array->size,1);

		vl.indeterminate = control_param(indeterminate);
		if (vl.indeterminate != &unsupplied) 
			ValueToBitArray(vl.indeterminate, *m_indeterminate, -1);
		m_indeterminate->SetSize(m_item_array->size,1);

		vl.locked = control_param(locked);
		if (vl.locked != &unsupplied) 
			ValueToBitArray(vl.locked, *m_locked, -1);
		m_locked->SetSize(m_item_array->size,1);

		pop_value_locals();
	}

	// resize the window if needed
	RECT rect;
	GetClientRect(m_hWnd, &rect);
	
	int text_height = parent_rollout->text_height + 1;
	rect.bottom = max(rect.bottom, text_height * m_item_array->size + 1);
	
	POINT pnt = {0,0};
	MapWindowPoints(m_hWnd, parent_rollout->page, &pnt, 1);
	MoveWindow(m_hWnd, pnt.x, pnt.y, rect.right - rect.left, rect.bottom - rect.top, TRUE);
	InvalidateRect(m_hWnd, NULL,TRUE);
	
	//SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

	UpdateWindow(m_hWnd);

}


// ============================================================================
BOOL ListBoxPlusControl::handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}


// ============================================================================
Value* ListBoxPlusControl::get_property(Value** arg_list, int count)
{
	Value* prop = arg_list[0];

	if (prop == n_items)
		return m_item_array;
	else if (prop == n_selection)
	{
		return_protected (new BitArrayValue(*m_selection));
	}
	else if (prop == n_indeterminate)
	{
		return_protected (new BitArrayValue(*m_indeterminate));
	}
	else if (prop == n_locked)
	{
		return_protected (new BitArrayValue(*m_locked));
	}
	else if (prop == n_multiSelection)
		return m_multiSelection ? &true_value : &false_value;
	else if(prop == n_hwnd)
	{
		if(parent_rollout && parent_rollout->page)
			return IntegerPtr::intern(reinterpret_cast<INT64>(m_hWnd));
		else
			return &undefined;
	}
	else
		return RolloutControl::get_property(arg_list, count);
	
}


// ============================================================================
Value* ListBoxPlusControl::set_property(Value** arg_list, int count)
{
	Value* val = arg_list[0];
	Value* prop = arg_list[1];

	if (prop == n_items)
	{
		type_check(val, Array, _T("items:"));
		m_item_array = (Array*)val;
		m_selection->SetSize(m_item_array->size,1);
		m_indeterminate->SetSize(m_item_array->size,1);
		if (parent_rollout != NULL && parent_rollout->page != NULL)
		{
			// resize the window if needed
			RECT rect;
			GetClientRect(m_hWnd, &rect);
			
			int text_height = parent_rollout->text_height + 1;
			rect.bottom = max(rect.bottom, text_height * m_item_array->size + 1);
			
			POINT pnt = {0,0};
			MapWindowPoints(m_hWnd, parent_rollout->page, &pnt, 1);
			MoveWindow(m_hWnd, pnt.x, pnt.y, rect.right - rect.left, rect.bottom - rect.top, TRUE);

			Invalidate();
		}
	}
	else if (prop == n_indeterminate)
	{
		m_indeterminate->ClearAll();
		ValueToBitArray(val, *m_indeterminate, -1);
		m_indeterminate->SetSize(m_item_array->size,1);
	
		if (parent_rollout != NULL && parent_rollout->page != NULL)
			Invalidate();
	}
	else if (prop == n_locked)
	{
		m_locked->ClearAll();
		ValueToBitArray(val, *m_locked, -1);
		m_locked->SetSize(m_item_array->size,1);
	
		if (parent_rollout != NULL && parent_rollout->page != NULL)
			Invalidate();
	}
	else if (prop == n_selection)
	{
		m_selection->ClearAll();
		m_selection->SetSize(m_item_array->size,1);

		if (m_multiSelection)
			ValueToBitArray(val, *m_selection, -1);
		else
		{
			if (val->is_kind_of(class_tag(Array))) 
			{
				Array* theArray = (Array*)val;
				m_selection->Set(theArray->size);
			}
			else if (val->is_kind_of(class_tag(BitArrayValue))) 
			{
				ValueToBitArray(val, *m_selection, m_item_array->size);
				//BitArrayValue* theBitArray = (BitArrayValue*) val;
				//m_selection = (BitArray*) val->to_bitarray();
				//m_selection->Set(theBitArray->bits.GetSize());
			}
			else if (is_number(val)) 
			{
				int index = val->to_int();
				if (index > 0 && index <= m_item_array->size)
					m_selection->Set(index);
			}
		}
		

		if (parent_rollout != NULL && parent_rollout->page != NULL)
		{
		/*	HWND list_box = GetDlgItem(parent_rollout->page, control_ID);
			for (int i = 0; i < item_array->size; i++) 
				SendMessage(list_box, LB_SETSEL, (*selection)[i], i);*/
			Invalidate();
		}
	}
	else if(prop == n_size)
	{
		if(parent_rollout && parent_rollout->page)
		{
			m_size = val->to_int();
			SetFont(m_fontName.data(), m_size, false);
			Invalidate();
		}
	}
	else if(prop == n_fontName)
	{
 		if(parent_rollout && parent_rollout->page)
		{
			const TCHAR *text = val->to_string();
			m_fontName = text;
			SetFont(m_fontName.data(), m_size, false);
			Invalidate();
		}
	}
	else if(prop == n_color)
	{
		type_check(val, Array, _T("color:"));
		m_color_array = (Array*)val;
		if (parent_rollout != NULL && parent_rollout->page != NULL)
		{
			Invalidate();
		}
	}
	else if(prop == n_multiSelection)
	{
		m_multiSelection = val->to_bool();

		if (parent_rollout != NULL && parent_rollout->page != NULL)
		{
			Invalidate();
		}
	}
	else
		return RolloutControl::set_property(arg_list, count);

	return val;
}


// ============================================================================
void ListBoxPlusControl::set_enable()
{
	if(parent_rollout != NULL && parent_rollout->page != NULL)
	{
		EnableWindow(m_hWnd, enabled);
		Invalidate();
	}
}


// ============================================================================
void ListBoxPlusControl::SetFont(const TCHAR *nameFont, float size, BOOL bold)
{
	LOGFONT lf;
	GetObject(m_hFont, sizeof(LOGFONT), &lf);

	// request a face name "Arial"
	_tcscpy(lf.lfFaceName, nameFont);

	// request pixel-height font
	lf.lfHeight = size; 

	// set the style of the font
	if (!bold)
		lf.lfWeight = FW_BOLD;
	else 
		lf.lfWeight = FW_NORMAL;

	DeleteObject(m_hFont);
	m_hFont = CreateFontIndirect(&lf);
}


// ============================================================================
void ListBoxPlusCtrlInit()
{
#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
	#include "macros/define_implementations.h"
#else
	#include "defimpfn.h"
#endif
		def_name ( multiSelection )
		def_name ( locked )
		def_name ( escpressed )
		def_name ( enterPressed )

		//def_name ( fontName )
	//def_name ( additem )

	static BOOL registered = FALSE;
	if(!registered)
	{
		WNDCLASSEX wcex;
		wcex.style         = CS_HREDRAW | CS_VREDRAW;
		wcex.hInstance     = g_hInst;
		wcex.hIcon         = NULL;
		wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)GetStockObject(HOLLOW_BRUSH);
		wcex.lpszMenuName  = NULL;
		wcex.cbClsExtra    = 0;
		wcex.cbWndExtra    = 0;
		wcex.lpfnWndProc   = ListBoxPlusControl::WndProc;
		wcex.lpszClassName = LISTBOXCTRL_WINDOWCLASS;
		wcex.cbSize        = sizeof(WNDCLASSEX);
		wcex.hIconSm       = NULL;

		if(!RegisterClassEx(&wcex))
			return;
		registered = TRUE;
	}

	install_rollout_control(Name::intern(_T("ListBoxPlus")), ListBoxPlusControl::create);
}

