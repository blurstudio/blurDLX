//*****************************************************************************
/*
	\file  ButtonTag.cpp

	[TITLE]
	Attemp to extend a button to be able to resize it

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

#include "MAXScrpt.h"
#include "BitMaps.h"
#include "Numbers.h"
#include "ColorVal.h"
#include "3DMath.h"
#include "Parser.h"
//#include "MXSAgni.h"

extern HINSTANCE g_hInst;

#include "resource.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "defextfn.h"
	def_name ( click )
	def_name ( dblclick )
	def_name ( mousedown )
	def_name ( mouseup )
	def_name ( mouseover )
	def_name ( mouseout )
	def_name ( tooltip )
	def_name ( opacity )
	def_name ( transparent )
	def_name ( rclick )
	def_name ( rdblclick )
	def_name ( rmousedown )
	def_name ( rmouseup )
	def_name ( fontName )

#define BUTTONTAG_WINDOWCLASS _T("button")
#define TOOLTIP_ID 1
// ============================================================================
visible_class(ButtonTag)

class ButtonTag : public RolloutControl
{
protected:
	HWND     m_hWnd;
	HWND     m_hToolTip;
	TSTR     m_sToolTip;
	bool	 m_bMouseOver;
	float    m_opacity;
	COLORREF m_transparent;
	COLORREF m_color;
	HFONT    m_hFont;
	int		 m_size;
	BOOL	 m_bold;
	TCHAR*	 m_fontName;

	HBITMAP     m_hBitmap;
	Value      *m_maxBitMap;
	Value      *m_style;

	HIMAGELIST	hImageBTN;

public:
	// Static methods
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
		{ return new ButtonTag(name, caption, keyparms, keyparm_count); }

	ButtonTag(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	~ButtonTag();

	int SetBitmap(Value *val);
	void SetText(TCHAR *text);
	void SetFont(TCHAR *nameFont, float size, BOOL bold);

	TOOLINFO* GetToolInfo();
	void Invalidate();

	// Event handlers
	LRESULT ButtonClicked(int xPos, int yPos, int fwKeys);
	LRESULT ButtonRClicked(int xPos, int yPos, int fwKeys);
	LRESULT ButtonDoubleClicked(int xPos, int yPos, int fwKeys);

	LRESULT ButtonTag::EraseBackground(HDC hDC);

	         classof_methods(ButtonTag, RolloutControl);
	void     gc_trace();
	void     collect() { delete this; }
	void     sprin1(CharStream* s) { s->printf(_T("ButtonPlus:%s"), name->to_string()); }

	void     add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
	LPCTSTR  get_control_class() { return BUTTONTAG_WINDOWCLASS; }
	void     compute_layout(Rollout *ro, layout_data* pos) { }
	BOOL     handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);

	Value*   get_property(Value** arg_list, int count);
	Value*   set_property(Value** arg_list, int count);
	Value*	 get_props_vf(Value** arg_list, int count);

	void     set_enable();
};

// ============================================================================
ButtonTag::ButtonTag(Value* name, Value* caption, Value** keyparms, int keyparm_count)
	: RolloutControl(name, caption, keyparms, keyparm_count)
{
	tag = class_tag(ButtonTag);

	m_hWnd = NULL;
	m_hToolTip = NULL;
	m_sToolTip = _T("");
	m_hBitmap = NULL;
	m_maxBitMap = NULL;
	m_bMouseOver = false;
	m_style = n_bmp_stretch;
	m_opacity = 0.f;
	m_transparent = 0;
	m_color = 0;
	m_hFont = NULL;
	m_size = 14;
	m_bold = FALSE;
	m_fontName = _T("Arial");
}

// ============================================================================
ButtonTag::~ButtonTag()
{
	ImageList_Destroy(hImageBTN);
	if(m_hFont) DeleteObject(m_hFont);
	if(m_hBitmap) DeleteObject(m_hBitmap);
}

void ButtonTag::gc_trace()
{
	RolloutControl::gc_trace();
	if (m_maxBitMap && m_maxBitMap->is_not_marked())
		m_maxBitMap->gc_trace();
	if (m_style && m_style->is_not_marked())
		m_style->gc_trace();
}

// ============================================================================
TOOLINFO* ButtonTag::GetToolInfo()
{
	static TOOLINFO ti;

	memset(&ti, 0, sizeof(TOOLINFO));
	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = m_hWnd;
	ti.uId = TOOLTIP_ID;
	ti.lpszText = (LPSTR)m_sToolTip;
	GetClientRect(m_hWnd, &ti.rect);

	return &ti;
}

// ============================================================================
LRESULT ButtonTag::ButtonClicked(int xPos, int yPos, int fwKeys)
{
	run_event_handler(parent_rollout, n_click, NULL, 0);
	run_event_handler(parent_rollout, n_pressed, NULL, 0);
	//SetCapture(m_hWnd);

	return TRUE;
}

LRESULT ButtonTag::ButtonRClicked(int xPos, int yPos, int fwKeys)
{
	run_event_handler(parent_rollout, n_rclick, NULL, 0);
	//SetCapture(m_hWnd);

	return TRUE;
}

LRESULT ButtonTag::ButtonDoubleClicked(int xPos, int yPos, int fwKeys)
{
	run_event_handler(parent_rollout, n_dblclick, NULL, 0);
	//SetCapture(m_hWnd);

	return TRUE;
}



// ============================================================================
void ButtonTag::Invalidate()
{
	if (m_hWnd == NULL) return;
	RECT rect;
	GetClientRect(m_hWnd, &rect);
	MapWindowPoints(m_hWnd, parent_rollout->page, (POINT*)&rect, 2);
	InvalidateRect(parent_rollout->page, &rect, TRUE);
	InvalidateRect(m_hWnd, NULL, TRUE);
}

// ============================================================================
int ButtonTag::SetBitmap(Value *val)
{
	if(val == &undefined)
	{
		if(m_hBitmap) DeleteObject(m_hBitmap);
		m_hBitmap = NULL;
		m_maxBitMap = NULL;
	}
	else
	{
		HWND hWnd = MAXScript_interface->GetMAXHWnd();

		MAXBitMap *mbm = (MAXBitMap*)val;
		type_check(mbm, MAXBitMap, _T("set .bitmap"));
		m_maxBitMap = val;

		HDC hDC = GetDC(hWnd);
		PBITMAPINFO bmi = mbm->bm->ToDib(32);
		if(m_hBitmap) DeleteObject(m_hBitmap);
		m_hBitmap = CreateDIBitmap(hDC, &bmi->bmiHeader, CBM_INIT, bmi->bmiColors, bmi, DIB_RGB_COLORS);
		LocalFree(bmi);
		ReleaseDC(hWnd, hDC);
	}

	Invalidate();
	return 1;
}

// ============================================================================
void ButtonTag::SetFont(TCHAR *nameFont, float size, BOOL bold)
{
	LOGFONT lf;
	GetObject(m_hFont, sizeof(LOGFONT), &lf);

	// request a face name "Arial"
	strcpy(lf.lfFaceName, nameFont);

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
void ButtonTag::SetText(TCHAR *text)
{
	SetWindowText(m_hWnd, text);
	Invalidate();
}

// ============================================================================
visible_class_instance (ButtonTag, "ButtonTag")

void ButtonTag::add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y)
{
	caption = caption->eval();

	TCHAR *text = caption->eval()->to_string();
	control_ID = next_id();
	parent_rollout = ro;

	layout_data pos;
	setup_layout(ro, &pos, current_y);

	process_layout_params(ro, &pos, current_y);

	m_hWnd = CreateWindow(
		_T("button"),
		text,
		WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_NOTIFY , 
		pos.left, pos.top, pos.width, pos.height,
		parent, (HMENU)control_ID, g_hInst, this);

	SetWindowText(m_hWnd, text);
}


// ============================================================================
BOOL ButtonTag::handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_COMMAND)
	{
		switch (HIWORD(wParam))
		{
			case BN_CLICKED:
				return ButtonClicked((short)LOWORD(lParam), (short)HIWORD(lParam), wParam);

			case BN_DBLCLK:
				return ButtonDoubleClicked((short)LOWORD(lParam), (short)HIWORD(lParam), wParam);

			case WM_RBUTTONUP:
				return ButtonRClicked((short)LOWORD(lParam), (short)HIWORD(lParam), wParam);
		}
	}
	return FALSE;
}

Value* ButtonTag::get_props_vf(Value** arg_list, int count)
{
	one_typed_value_local(Array* itemsArray);
	vl.itemsArray = new Array (0);

	vl.itemsArray->append(n_width);
	vl.itemsArray->append(n_height);

	return_value(vl.itemsArray);	
}

// ============================================================================
Value* ButtonTag::get_property(Value** arg_list, int count)
{
	Value* prop = arg_list[0];

	if(prop == n_width)
	{
		if(parent_rollout && parent_rollout->page)
		{
			HWND hWnd = GetDlgItem(parent_rollout->page, control_ID);
			RECT rect;
			GetWindowRect(hWnd, &rect);
			MapWindowPoints(NULL, parent_rollout->page, (LPPOINT)&rect, 2);
			return Integer::intern(rect.right-rect.left);
		}
		else return &undefined;
	}
	else if(prop == n_height)
	{
		if(parent_rollout && parent_rollout->page)
		{
			HWND hWnd = GetDlgItem(parent_rollout->page, control_ID);
			RECT rect;
			GetWindowRect(hWnd, &rect);
			MapWindowPoints(NULL, parent_rollout->page, (LPPOINT)&rect, 2);
			return Integer::intern(rect.bottom-rect.top);
		}
		else return &undefined;
	}

	/*else if(prop == n_tooltip)
	{
		if(parent_rollout && parent_rollout->page)
			return new String(m_sToolTip);
		else
			return &undefined;
	}
	else if(prop == n_style)
	{
		if(parent_rollout && parent_rollout->page)
			return m_style;
		else
			return &undefined;
	}
	else if(prop == n_bitmap)
	{
		if(parent_rollout && parent_rollout->page)
			return m_maxBitMap ? m_maxBitMap : &undefined;
		else
			return &undefined;
	}
	else if(prop == n_transparent)
	{
		if(parent_rollout && parent_rollout->page)
			return new ColorValue(m_transparent);
		else
			return &undefined;
	}
	*/
	return RolloutControl::get_property(arg_list, count);
}


// ============================================================================
Value* ButtonTag::set_property(Value** arg_list, int count)
{
	Value* val = arg_list[0];
	Value* prop = arg_list[1];

	if(prop == n_width)
	{
		if(parent_rollout && parent_rollout->page)
		{
			int width = val->to_int();
			HWND	hWnd = GetDlgItem(parent_rollout->page, control_ID);
			RECT	rect;
			GetWindowRect(hWnd, &rect);
			MapWindowPoints(NULL, parent_rollout->page,	(LPPOINT)&rect, 2);
			SetWindowPos(hWnd, NULL, rect.left, rect.top, width, rect.bottom-rect.top, SWP_NOZORDER);
			SendMessage(m_hToolTip, TTM_SETTOOLINFO, 0, (LPARAM)GetToolInfo());
		}
	}
	else if(prop == n_height)
	{
		if(parent_rollout && parent_rollout->page)
		{
			int height = val->to_int();
			HWND	hWnd = GetDlgItem(parent_rollout->page, control_ID);
			RECT	rect;
			GetWindowRect(hWnd, &rect);
			MapWindowPoints(NULL, parent_rollout->page,	(LPPOINT)&rect, 2);
			SetWindowPos(hWnd, NULL, rect.left, rect.top, rect.right-rect.left, height, SWP_NOZORDER);
			SendMessage(m_hToolTip, TTM_SETTOOLINFO, 0, (LPARAM)GetToolInfo());
		}
	}
/*	else if(prop == n_bitmap)
	{
		if(parent_rollout && parent_rollout->page)
			SetBitmap(val);
	}
	else if(prop == n_tooltip)
	{
		if(parent_rollout && parent_rollout->page)
		{
			m_sToolTip = val->to_string();
			SendMessage(m_hToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM)GetToolInfo());
		}
	}
	else if(prop == n_style)
	{
		if(parent_rollout && parent_rollout->page)
		{
			if(val == n_bmp_tile ||
				val == n_bmp_stretch ||
				val == n_bmp_center)
			{
				m_style = val;
				Invalidate();
			}
		}
	}
	else if(prop == n_transparent)
	{
		if(parent_rollout && parent_rollout->page)
		{
			m_transparent = val->to_colorref();
			Invalidate();
		}
	}
	*/

	else if (prop == n_text || prop == n_caption) // not displayed
	{
		TCHAR *text = val->to_string(); // will throw error if not convertable
		SetText(val->to_string());
		caption = val->get_heap_ptr();
	}
	else
		return RolloutControl::set_property(arg_list, count);

	return val;
}


// ============================================================================
void ButtonTag::set_enable()
{
	if(parent_rollout != NULL && parent_rollout->page != NULL)
	{
		EnableWindow(m_hWnd, enabled);
		InvalidateRect(m_hWnd, NULL, TRUE);
	}
}


// ============================================================================
void ButtonTagInit()
{

	#include "defimpfn.h"
	def_name ( click )
	def_name ( dblclick )
	def_name ( mousedown )
	def_name ( mouseup )
	def_name ( mouseover )
	def_name ( mouseout )
	def_name ( tooltip )
	def_name ( opacity )
	def_name ( transparent )
	def_name ( rclick )
	def_name ( rdblclick )
	def_name ( rmousedown )
	def_name ( rmouseup )
	def_name ( fontName )

	install_rollout_control(Name::intern("SimpleButton"), ButtonTag::create);
}


