//*****************************************************************************
/*
	\file  ImgTagPlus.cpp

	[TITLE]
	Extension of the current imgtag Maxscript UI Control

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
#include "resource.h"

extern HINSTANCE g_hInst;


#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
#include "maxscript\macros\define_instantiation_functions.h"
#else
#include "definsfn.h"
#endif

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


#define IMGTAG_WINDOWCLASS _T("IMGTAG_WINDOWCLASS")
#define TOOLTIP_ID 1
// ============================================================================
visible_class(ImgTag)

class ImgTag : public RolloutControl
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
	TSTR	 m_fontName;

	HBITMAP     m_hBitmap;
	Value      *m_maxBitMap;
	Value      *m_style;

public:
	// Static methods
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
		{ return new ImgTag(name, caption, keyparms, keyparm_count); }

	ImgTag(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	~ImgTag();

	int SetBitmap(Value *val);
	void SetText(const TCHAR *text);
	void SetFont(const TCHAR *nameFont, float size, BOOL bold);

	TOOLINFO* GetToolInfo();
	void Invalidate();

	// Event handlers
	LRESULT LButtonDown(int xPos, int yPos, int fwKeys);
	LRESULT LButtonUp(int xPos, int yPos, int fwKeys);
	LRESULT LButtonDblClk(int xPos, int yPos, int fwKeys);
	LRESULT RButtonDown(int xPos, int yPos, int fwKeys);
	LRESULT RButtonUp(int xPos, int yPos, int fwKeys);
	LRESULT RButtonDblClk(int xPos, int yPos, int fwKeys);
	LRESULT MouseMove(int xPos, int yPos, int fwKeys);
	LRESULT EraseBkgnd(HDC hDC);
	LRESULT Paint(HDC hDC);


	         classof_methods(ImgTag, RolloutControl);
	void     gc_trace();
	void     collect() { delete this; }
	void     sprin1(CharStream* s) { s->printf(_T("ImgTag:%s"), name->to_string()); }

	void     add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
	LPCTSTR  get_control_class() { return IMGTAG_WINDOWCLASS; }
	void     compute_layout(Rollout *ro, layout_data* pos) { }
	BOOL     handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);

	Value*   get_property(Value** arg_list, int count);
	Value*   set_property(Value** arg_list, int count);
	void     set_enable();
};

// ============================================================================
ImgTag::ImgTag(Value* name, Value* caption, Value** keyparms, int keyparm_count)
	: RolloutControl(name, caption, keyparms, keyparm_count)
{
	tag = class_tag(ImgTag);

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
ImgTag::~ImgTag()
{
	if(m_hFont) DeleteObject(m_hFont);
	if(m_hBitmap) DeleteObject(m_hBitmap);
}

void ImgTag::gc_trace()
{
	RolloutControl::gc_trace();
	if (m_maxBitMap && m_maxBitMap->is_not_marked())
		m_maxBitMap->gc_trace();
	if (m_style && m_style->is_not_marked())
		m_style->gc_trace();
}

// ============================================================================
TOOLINFO* ImgTag::GetToolInfo()
{
	static TOOLINFO ti;

	memset(&ti, 0, sizeof(TOOLINFO));
	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = m_hWnd;
	ti.uId = TOOLTIP_ID;
	ti.lpszText = (LPTSTR)(m_sToolTip.data());
	GetClientRect(m_hWnd, &ti.rect);

	return &ti;
}

// ============================================================================
LRESULT CALLBACK ImgTag::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	ImgTag *ctrl = DLGetWindowLongPtr<ImgTag*>(hWnd, GWLP_USERDATA);
	if(ctrl == NULL && msg != WM_CREATE)
		return DefWindowProc(hWnd, msg, wParam, lParam);

	if(ctrl && ctrl->m_hToolTip)
	{
		MSG ttmsg;
		ttmsg.lParam = lParam;
		ttmsg.wParam = wParam;
		ttmsg.message = msg;
		ttmsg.hwnd = hWnd;
		SendMessage(ctrl->m_hToolTip, TTM_RELAYEVENT, 0, (LPARAM)(LPMSG)&ttmsg);
	}

	switch(msg)
	{
	case WM_CREATE:
		{
			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			ctrl = (ImgTag*)lpcs->lpCreateParams;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (DWORD_PTR)ctrl);
			break;
		}

	case WM_KILLFOCUS:
		ctrl->m_bMouseOver = false;
		ctrl->run_event_handler(ctrl->parent_rollout, n_mouseout, NULL, 0);
		break;

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

	case WM_ERASEBKGND:
		return ctrl->EraseBkgnd((HDC)wParam);

	case WM_PAINT:
		PAINTSTRUCT ps;
		BeginPaint(hWnd,&ps);
		ctrl->Paint(ps.hdc);
		EndPaint(hWnd,&ps);
		return FALSE;

	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return FALSE;
}

// ============================================================================
LRESULT ImgTag::LButtonDown(int xPos, int yPos, int fwKeys)
{
	//SetFocus(m_hWnd);
	run_event_handler(parent_rollout, n_mousedown, NULL, 0);
	//SetCapture(m_hWnd);

	return TRUE;
}

// ============================================================================
LRESULT ImgTag::LButtonUp(int xPos, int yPos, int fwKeys)
{
	run_event_handler(parent_rollout, n_mouseup, NULL, 0);
	run_event_handler(parent_rollout, n_click, NULL, 0);
	//SetCapture(m_hWnd);

	return TRUE;
}

// ============================================================================
LRESULT ImgTag::LButtonDblClk(int xPos, int yPos, int fwKeys)
{
	//SetFocus(m_hWnd);
	run_event_handler(parent_rollout, n_dblclick, NULL, 0);
	//SetCapture(m_hWnd);

	return TRUE;
}

// ============================================================================
LRESULT ImgTag::RButtonDown(int xPos, int yPos, int fwKeys)
{
	//SetFocus(m_hWnd);
	run_event_handler(parent_rollout, n_rmousedown, NULL, 0);
	//SetCapture(m_hWnd);

	return TRUE;
}

// ============================================================================
LRESULT ImgTag::RButtonUp(int xPos, int yPos, int fwKeys)
{
	run_event_handler(parent_rollout, n_rmouseup, NULL, 0);
	run_event_handler(parent_rollout, n_rclick, NULL, 0);
	//SetCapture(m_hWnd);

	return TRUE;
}

// ============================================================================
LRESULT ImgTag::RButtonDblClk(int xPos, int yPos, int fwKeys)
{
	//SetFocus(m_hWnd);
	run_event_handler(parent_rollout, n_rdblclick, NULL, 0);
	//SetCapture(m_hWnd);

	return TRUE;
}

// ============================================================================
LRESULT ImgTag::MouseMove(int xPos, int yPos, int fwKeys)
{
	if(m_bMouseOver)
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
//		SetFocus(m_hWnd);
		run_event_handler(parent_rollout, n_mouseover, NULL, 0);
		SetCapture(m_hWnd);
	}

	return TRUE;
}

// ============================================================================
LRESULT ImgTag::EraseBkgnd(HDC hDC)
{
	return 1;
}

// ============================================================================
LRESULT ImgTag::Paint(HDC hDC)
{
	SetBkMode(hDC, TRANSPARENT);

	RECT rect;
	GetClientRect(m_hWnd, &rect);

	if(!m_maxBitMap || !m_hBitmap)
	{
		TCHAR text[256];
 		GetWindowText(m_hWnd, text, sizeof(text));

		SetTextColor(hDC, m_color);
		SelectFont(hDC, m_hFont);
 		TextOut(hDC, 0, 0, text, (int)_tcslen(text));

		return FALSE;
	}

	MAXBitMap* mbm = (MAXBitMap*)m_maxBitMap;
	int width  = mbm->bi.Width();
	int height = mbm->bi.Height();

	HDC hMemDC = CreateCompatibleDC(hDC);
	SelectObject(hMemDC, m_hBitmap);

	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = (BYTE)((1.f - m_opacity) * 255.f);
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// quick patch to fix build breakage. Need to fix..
//	bf.AlphaFormat = AC_SRC_NO_PREMULT_ALPHA|AC_DST_NO_ALPHA;
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	bf.AlphaFormat = 0x03;

	if(m_style == n_bmp_tile)
	{
		for(int x = 0; x < rect.right; x+=width)
			for(int y = 0; y < rect.bottom; y+=height) 
			{
					BitBlt(hDC, x, y, width, height, hMemDC, 0, 0, SRCCOPY);
			}
	}
	else if(m_style == n_bmp_center)
	{
		BitBlt(hDC, (rect.right-width)>>1, (rect.bottom-height)>>1,
			width, height, hMemDC, 0, 0, SRCCOPY);
	}
	else
	{
		StretchBlt(hDC, 0, 0, rect.right, rect.bottom, hMemDC, 0, 0, width, height, SRCCOPY);
	}

	DeleteDC(hMemDC);

	return FALSE;
}

// ============================================================================
void ImgTag::Invalidate()
{
	if (m_hWnd == NULL) return;
	RECT rect;
	GetClientRect(m_hWnd, &rect);
	MapWindowPoints(m_hWnd, parent_rollout->page, (POINT*)&rect, 2);
	InvalidateRect(parent_rollout->page, &rect, TRUE);
	InvalidateRect(m_hWnd, NULL, TRUE);
}

// ============================================================================
int ImgTag::SetBitmap(Value *val)
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
void ImgTag::SetFont(const TCHAR *nameFont, float size, BOOL bold)
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
void ImgTag::SetText(const TCHAR *text)
{
	SetWindowText(m_hWnd, text);
	Invalidate();
}

// ============================================================================
visible_class_instance (ImgTag, "ImgTag")

void ImgTag::add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y)
{
	caption = caption->eval();

	const TCHAR *text = caption->eval()->to_string();
	control_ID = next_id();
	parent_rollout = ro;

	Value *val = control_param(tooltip);
	if(val != &unsupplied)
		m_sToolTip = val->to_string();

	val = control_param(style);
	if(val != &unsupplied)
		m_style = val;

	val = control_param(bitmap);
	if(val != &unsupplied)
		SetBitmap(val);

	val = control_param(transparent);
	if(val != &unsupplied)
		m_transparent = val->to_colorref();
	else
		m_transparent = RGB(0,0,0);

	val = control_param(color);
	if(val != &unsupplied)
		m_color = val->to_colorref();
	else
		m_color = RGB(0,0,0);

	val = control_param(opacity);
	if(val != &unsupplied)
	{
		m_opacity = val->to_float();
		if(m_opacity < 0.f) m_opacity = 0.f;
		if(m_opacity > 1.f) m_opacity = 1.f;
	}

	val = control_param(bold);
	if(val != &unsupplied)
		m_bold= val->to_bool();
	else
		m_bold = FALSE;

	val = control_param(size);
	if(val != &unsupplied)
		m_size= val->to_int();
	else
		m_size = 12;

	layout_data pos;
	setup_layout(ro, &pos, current_y);

 	if(m_maxBitMap)
	{
		MAXBitMap *mbm = (MAXBitMap*)m_maxBitMap;
		pos.width = mbm->bi.Width();
		pos.height = mbm->bi.Height();
	}

	process_layout_params(ro, &pos, current_y);

	m_hWnd = CreateWindow(
		IMGTAG_WINDOWCLASS,
		text,
		WS_VISIBLE | WS_CHILD | WS_GROUP,
		pos.left, pos.top, pos.width, pos.height,
		parent, (HMENU)control_ID, g_hInst, this);

//	SendDlgItemMessage(parent, control_ID, WM_SETFONT, (WPARAM)ro->font, 0L);

	m_hToolTip = CreateWindow(
		TOOLTIPS_CLASS,
		TEXT(""), WS_POPUP,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		m_hWnd, (HMENU)NULL, g_hInst, NULL);

	SendMessage(m_hToolTip, TTM_ADDTOOL, 0, (LPARAM)GetToolInfo());


	LOGFONT lf;
	GetObject(ro->font, sizeof(LOGFONT), &lf);
	lf.lfHeight = m_size;						// request a 12-pixel-height font
	lf.lfWeight = m_bold ? FW_BOLD : FW_NORMAL; // bold?
	_tcscpy(lf.lfFaceName, m_fontName.data());				// request a face name "Arial"

	m_hFont = CreateFontIndirect(&lf);
	SendDlgItemMessage(parent, control_ID, WM_SETFONT, (WPARAM)m_hFont, 0L);

	SetWindowText(m_hWnd, text);
}


// ============================================================================
BOOL ImgTag::handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}


// ============================================================================
Value* ImgTag::get_property(Value** arg_list, int count)
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
	else if(prop == n_tooltip)
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
	else if(prop == n_opacity)
	{
		if(parent_rollout && parent_rollout->page)
			return Float::intern(m_opacity);
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
	else if(prop == n_size)
	{
		if(parent_rollout && parent_rollout->page)
			return Integer::intern(m_size);
		else
			return &undefined;
	}
	else if(prop == n_bold)
	{
		if(parent_rollout && parent_rollout->page)
			return (m_bold ? &true_value : &false_value);
		else
			return &undefined;
	}
	else if(prop == n_fontName)
	{
		if(parent_rollout && parent_rollout->page)
			return new String(m_fontName);
		else
			return &undefined;
	}
	else if(prop == n_color)
	{
		if(parent_rollout && parent_rollout->page)
			return new ColorValue(m_color);
		else
			return &undefined;
	}

	return RolloutControl::get_property(arg_list, count);
}


// ============================================================================
Value* ImgTag::set_property(Value** arg_list, int count)
{
	Value* val = arg_list[0];
	Value* prop = arg_list[1];

	if(prop == n_bitmap)
	{
		if(parent_rollout && parent_rollout->page)
			SetBitmap(val);
	}
	else if(prop == n_width)
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
	else if(prop == n_opacity)
	{
		if(parent_rollout && parent_rollout->page)
		{
			m_opacity = val->to_float();
			if(m_opacity < 0.f) m_opacity = 0.f;
			if(m_opacity > 1.f) m_opacity = 1.f;
			Invalidate();
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
	else if(prop == n_color)
	{
		if(parent_rollout && parent_rollout->page)
		{
			m_color = val->to_colorref();
			Invalidate();
		}
	}
	else if(prop == n_size)
	{
		if(parent_rollout && parent_rollout->page)
		{
			m_size = val->to_int();
			SetFont(_T("Arial"), m_size, m_bold);
			Invalidate();
		}
	}
	else if(prop == n_bold)
	{
		if(parent_rollout && parent_rollout->page)
		{
			m_bold = val->to_bool();
			SetFont(_T("Arial"), m_size, m_bold);
			Invalidate();
		}
	}
	else if(prop == n_fontName)
	{
		if(parent_rollout && parent_rollout->page)
		{
			const TCHAR *text = val->to_string();
			m_fontName = text;
			SetFont(m_fontName.data(), m_size, m_bold);
			Invalidate();
		}
	}

	else if (prop == n_text || prop == n_caption) // not displayed
	{
		const TCHAR *text = val->to_string(); // will throw error if not convertable
		SetText(val->to_string());
		caption = val->get_heap_ptr();
	}
	else
		return RolloutControl::set_property(arg_list, count);

	return val;
}


// ============================================================================
void ImgTag::set_enable()
{
	if(parent_rollout != NULL && parent_rollout->page != NULL)
	{
		EnableWindow(m_hWnd, enabled);
		InvalidateRect(m_hWnd, NULL, TRUE);
	}
}


// ============================================================================
void ImgTagInit()
{

#if __MAXSCRIPT_2012__ || __MAXSCRIPT_2013__
	#include "macros/define_implementations.h"
#else
	#include "defimpfn.h"
#endif
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

	static BOOL registered = FALSE;
	if(!registered)
	{
		WNDCLASSEX wcex;
		wcex.cbSize        = sizeof(WNDCLASSEX);
		wcex.style         = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc   = ImgTag::WndProc;
		wcex.cbClsExtra    = 0;
		wcex.cbWndExtra    = 0;
		wcex.hInstance     = g_hInst;
		wcex.hIcon         = NULL;
		wcex.hIconSm       = NULL;
		wcex.hCursor       = LoadCursor(0, MAKEINTRESOURCE(IDC_ARROW));
		wcex.hbrBackground = NULL;
		wcex.lpszMenuName  = NULL;
		wcex.lpszClassName  = IMGTAG_WINDOWCLASS;

		if(!RegisterClassEx(&wcex))
			return;
		registered = TRUE;
	}

	install_rollout_control(Name::intern(_T("ImgTagPlus")), ImgTag::create);
}
