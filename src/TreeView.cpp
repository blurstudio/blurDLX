// ============================================================================
// TreeView.cpp
// Copyright ©2005 Blur Studios Inc.
// Created by Diego Garcia
// ----------------------------------------------------------------------------


#include "MAXScrpt.h"
#include "MAXObj.h"
#include "Numbers.h"
#include "ColorVal.h"
#include "Structs.h"
#include "Parser.h"

#include "rollouts.h"
#include "MaxIcon.h"

#include "MAXMats.h"


extern HINSTANCE g_hInst;
extern COLORREF DarkenColour(COLORREF col, double factor);
extern COLORREF LightenColour(COLORREF col, double factor);
extern INode* _get_valid_node(MAXNode* _node, TCHAR* errmsg);

#include "resource.h"

//#include "lclinsfn.h"		// no idea why, but these two lines HAVE to be here in order to compile...
//#	include "tvnames.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#define GAP_SIZE         5
#define ICON_SIZE		16
#define TREECTRL_WINDOWCLASS _T("TREECTRL_WINDOWCLASS")

#include "GenericMethod.h"
#include "TreeView.h"
static WNDPROC lpfnWndProc = NULL; // original wndproc for the tree view 


#include "definsfn.h"
	def_name ( hwnd )
	def_name ( additem )
	def_name ( deleteitem )
	def_name ( deleteAllItems )
	def_name ( expand )
	def_name ( collapse )
	def_name ( toggleExpand )
	def_name ( ensureVisible )
	def_name ( showTree )
	def_name ( beginDrag )
	def_name ( endDrag )
	def_name ( imageList )
	def_name ( treeViewControl )
	def_name ( iconSelectedImage )
	def_name ( iconImage )
	def_name ( iconState )
	def_name ( overlayList )
	def_name ( multiselection )
	def_name ( dragEnabled )
		
	def_name ( iconPressed )
	def_name ( buttonPressed )
	def_name ( indentPressed )
	def_name ( stateIconPressed )
	def_name ( rightAreaPressed )

	def_name ( sortBy )
	def_name ( flat2D )
	def_name ( bkColor )
	def_name ( selectedColor )
	def_name ( useSelectedColor )
	def_name ( editLabels )

	def_name ( preBeginEditLabel )
	def_name ( beginEditLabel )
	def_name ( endEditLabel )
	def_name ( focusChanged )

	def_name ( data )
	def_name ( extraData )
	
	def_name ( syncSelection )
	def_name ( recursive )
	
	def_name ( decrease )
	def_name ( rootNodes )
	def_name ( customtooltips )

	def_name ( siblings )
	def_name ( isCollapsed )
	def_name ( editLabel )
	
	
#include "defextfn.h"
	def_name ( mousedown )
	def_name ( mouseup )
	def_name ( rclick )
	def_name ( dblclick )
	def_name ( rdblclick )
	def_name ( tooltip )
	




// ============================================================================
visible_class_instance (TreeViewControl, s_treeview)


Value *tv_addItem(TreeViewControl *tvc, Value* val, Value* parent = &undefined)
{
	TreeViewItem *tvItem = new TreeViewItem();
	if(!tvItem ) return &false_value;
	
	// set treeviewControl property
	tvItem->m_tvControl = tvc;

	// set parent property
	if (parent != &unsupplied && parent != &undefined)
	{
		TreeViewItem *pParentTreeviewItem = (TreeViewItem*) parent;
		if (pParentTreeviewItem)
			tvItem->m_parent = pParentTreeviewItem;
		else
		{
			tvItem->m_tvi = TVI_ROOT;
			tvItem->m_parent = NULL;
		}
	}
	else
	{
		tvItem->m_tvi = TVI_ROOT;
		tvItem->m_parent = NULL;
	}
	
	// create the item depending the value specified
	if (val->is_kind_of(class_tag(MAXNode)))
	{
		INode *node = val->to_node();

		tvItem->m_node	= val;
		tvItem->m_name	= node->GetName();
		tvItem->m_tooltip = tvItem->m_name;
		tvItem->m_color	= node->GetWireColor();
		tvItem->m_selected= node->Selected();
		tvc->AddItemToTree(tvItem);

		return_protected ((Value *)tvItem);
	}
	else if (val->is_kind_of(class_tag(TreeViewItem)))
	{
		TreeViewItem *tvi = new TreeViewItem ((TreeViewItem *)val);
		tvi->m_tvControl = tvc;
		tvc->AddItemToTree(tvi);

		return_protected ( (Value *)tvi );
	}
	else if (val->is_kind_of(class_tag(MAXTexture)))
	{
		MtlBase* mtl = (MtlBase*)val->to_texmap();

		tvItem->m_node	= val;
		tvItem->m_name	= mtl->GetName();
		tvItem->m_tooltip = tvItem->m_name;
		tvItem->m_color	= RGB(0, 0, 0);
		tvItem->m_selected= false;
		tvc->AddItemToTree(tvItem);

		return_protected ( (Value *)tvItem );
	}
	else if (val->is_kind_of(class_tag(MAXMaterial)))
	{
		Mtl* mtl = (Mtl*)val->to_mtl();

		tvItem->m_node	= val;
		tvItem->m_name	= mtl->GetName();
		tvItem->m_tooltip = tvItem->m_name;
		tvItem->m_color	= mtl->GetDiffuse(0).toRGB();
		tvItem->m_selected= false;
		tvc->AddItemToTree(tvItem);

		return_protected ( (Value *)tvItem );
	}
	else if (val->is_kind_of(class_tag(MAXMultiMaterial)))
	{
		Mtl* mtl = (Mtl*)val->to_mtl();

		tvItem->m_node	= val;
		tvItem->m_name	= mtl->GetName();
		tvItem->m_tooltip = tvItem->m_name;
		tvItem->m_color	= mtl->GetDiffuse(0).toRGB();
		tvItem->m_selected= false;
		tvc->AddItemToTree(tvItem);

		return_protected ( (Value *)tvItem );
	}
	else if (val->is_kind_of(class_tag(String)))
	{
		tvItem->m_node	= val;
		tvItem->m_name	= val->to_string();
		tvItem->m_tooltip = tvItem->m_name;
		tvItem->m_color	= RGB(128, 128, 128);
		tvItem->m_selected= false;
		tvc->AddItemToTree(tvItem);

		return_protected ( (Value *)tvItem );
	}
	else if (val->is_kind_of(class_tag(MAXRootNode)))
	{
 		MAXRootNode* rootNode = (MAXRootNode*)val->to_rootnode();

		tvItem->m_node	= val;
		tvItem->m_name	= _T("Objects");
		tvItem->m_tooltip = tvItem->m_name;
		tvItem->m_color	= RGB(0, 0, 0);
		tvItem->m_selected= false;
		tvc->AddItemToTree(tvItem);

		return_protected ( (Value *)tvItem );
	}
	else if (val->is_kind_of(class_tag(Array)))
	{
		one_typed_value_local(Array* tvItemArray);
		Array* itemArray = (Array*)val;
		vl.tvItemArray = new Array (itemArray->size);
		
		for (int i =0; i < itemArray->size; i++)
		{
			Value* valItem = itemArray->data[i];
			vl.tvItemArray->append(tv_addItem(tvc, valItem, parent));
		}

		return_value(vl.tvItemArray);
	}

	return &undefined;
}

static int CALLBACK Compare_fn(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	TreeViewControl *tvc = (TreeViewControl*)lParamSort;
	TreeViewItem *pTreeviewItem1 = (TreeViewItem*) lParam1;
	TreeViewItem *pTreeviewItem2 = (TreeViewItem*) lParam2;

	Value** arglist;
	value_local_array(arglist, 2);
	arglist[0] = (Value*)pTreeviewItem1;
	arglist[1] = (Value*)pTreeviewItem2;
		
	init_thread_locals();
	push_alloc_frame();
	one_value_local(arg);

	vl.arg = tvc->m_user_compare_fn->apply(arglist, 2);
	//Value* resultValue = tvc->m_user_compare_fn->apply(arglist, 2);
	int result = vl.arg->to_int();

	pop_value_locals();
	pop_alloc_frame();
	pop_value_local_array(arglist);

	return result;
}

static int CALLBACK DecreaseCompare_fn(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	TreeViewControl *tvc = (TreeViewControl*)lParamSort;
	TreeViewItem *pTreeviewItem1 = (TreeViewItem*) lParam1;
	TreeViewItem *pTreeviewItem2 = (TreeViewItem*) lParam2;

	Value** arglist;
	value_local_array(arglist, 2);
	arglist[0] = (Value*)pTreeviewItem1;
	arglist[1] = (Value*)pTreeviewItem2;

	init_thread_locals();
	push_alloc_frame();
	one_value_local(arg);

	vl.arg = tvc->m_user_compare_fn->apply(arglist, 2);
	int result = vl.arg->to_int() * -1;

	pop_value_locals();
	pop_alloc_frame();
	pop_value_local_array(arglist);

	return result;
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
TreeViewControl::TreeViewControl(Value* name, Value* caption, Value** keyparms, int keyparm_count)
	: RolloutControl(name, caption, keyparms, keyparm_count)  
	{ 
		tag = class_tag(TreeViewControl); 
		
		m_TreeHWnd		= NULL;
		m_containerHWnd	= NULL;
		m_TreeViewImagesHwnd = NULL;
		m_items			= new Array(0);
		m_dragging		= FALSE;
		m_multiselection = FALSE;
		m_dragEnabled	 = TRUE;

		m_user_compare_fn = NULL;
		m_data			= &undefined;
		
		m_syncSelection = FALSE;
		m_ItemEditing	= &undefined;	
		m_hEdit			= NULL;
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
TreeViewControl::~TreeViewControl()
{
	if(m_data) DeleteObject(m_data);
	if(m_ItemEditing) DeleteObject(m_ItemEditing);
	if(m_user_compare_fn) DeleteObject(m_user_compare_fn);
	if(m_items) DeleteObject(m_items);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		void ListBoxPlusControl::gc_trace()
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------
void TreeViewControl::gc_trace()
{
	RolloutControl::gc_trace();

	if (m_items && m_items->is_not_marked())
		m_items->gc_trace();

	if (m_data && m_data->is_not_marked())
		m_data->gc_trace();

	if (m_ItemEditing && m_ItemEditing->is_not_marked())
		m_ItemEditing->gc_trace();

	if (m_user_compare_fn && m_user_compare_fn->is_not_marked())
		m_user_compare_fn->gc_trace();
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
void TreeViewControl::add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y)
{
	caption = caption->eval();

	TCHAR *text = caption->eval()->to_string();
	control_ID = next_id();
	parent_rollout = ro;

	// items property
	Array *items = (Array*)control_param(items);
	if (items == (Array*)&unsupplied)
		items = new Array (0);
	type_check(items, Array, _T("items:"));

	// showTree property
	BOOL showTree = TRUE;
	Value *val = control_param(showTree);
	if(val != &unsupplied)
		showTree = val->to_bool();

	m_multiselection = FALSE;
	val = control_param(multiselection);
	if(val != &unsupplied)
		m_multiselection = val->to_bool();

	long treeFlat3D = WS_EX_CLIENTEDGE;
	val = control_param(flat2D);
	if(val == &true_value)
		treeFlat3D = 0;

	m_bkcolor = GetSysColor( COLOR_BTNFACE );
	val = control_param(bkColor);
	if(val != &unsupplied)
		m_bkcolor = val->to_colorref();
	
	this->m_selectedColor = LightenColour(this->m_bkcolor, 0.75f);
	val = control_param(selectedColor);
	if(val != &unsupplied)
		this->m_selectedColor = val->to_colorref();

	val = control_param(useSelectedColor);
	if(val != &unsupplied)
		this->m_useSelectedColor = val->to_bool();

	BOOL editLabels = FALSE;
	val = control_param(editLabels);
	if(val == &true_value)
		editLabels = TRUE;

	BOOL customTooltips = FALSE;
	val = control_param(customtooltips);
	if(val == &true_value)
		customTooltips = TRUE;

	layout_data pos;
	setup_layout(ro, &pos, current_y);

  	process_layout_params(ro, &pos, current_y);

	// container window
	m_containerHWnd = CreateWindow(
		TREECTRL_WINDOWCLASS,
		_T(""),
		WS_VISIBLE | WS_CHILD ,
		pos.left, pos.top, pos.width, pos.height,
   		parent, (HMENU)control_ID, g_hInst, this);

	// tree window
	long treeProperties = WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_BORDER;
	if (showTree)
		treeProperties |= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT ;// | TVS_EDITLABELS | TVS_SHOWSELALWAYS;
	
	if (editLabels)
		treeProperties |= TVS_EDITLABELS;
	
	if (customTooltips)
		treeProperties |= TVS_INFOTIP;

	m_TreeHWnd = CreateWindowEx(treeFlat3D, WC_TREEVIEW, _T(""),  //"SysTreeView32", _T(""), 
        treeProperties,
        0, 0, pos.width, pos.height,
         m_containerHWnd, (HMENU) control_ID, g_hInst, NULL); 

  	TreeView_SetItemHeight(m_TreeHWnd, ICON_SIZE);


	if (customTooltips)
	{
		HWND hToolTip = TreeView_GetToolTips(m_TreeHWnd);

		static TOOLINFO ti;
		memset(&ti, 0, sizeof(TOOLINFO));
		SendMessage(hToolTip, TTM_GETTOOLINFO, 0, (LPARAM)&ti);
		ti.uFlags = TTF_ABSOLUTE | TTF_TRACK;
		SendMessage(hToolTip, TTM_SETTOOLINFO, 0, (LPARAM)&ti);
	}
	// create an ampty imagelist
	m_TreeViewImagesHwnd = ImageList_Create(ICON_SIZE, ICON_SIZE, ILC_COLOR24 | ILC_MASK, 0, 10);
	TreeView_SetImageList(m_TreeHWnd, m_TreeViewImagesHwnd, TVSIL_NORMAL);
	TreeView_SetImageList(m_TreeHWnd, m_TreeViewImagesHwnd, TVSIL_STATE);
	TreeView_DeleteAllItems(m_TreeHWnd);
	TreeView_SetBkColor(m_TreeHWnd,m_bkcolor); //GetSysColor( COLOR_BTNFACE ));
	TreeView_SetTextColor(m_TreeHWnd,GetSysColor(COLOR_BTNTEXT));

	for (int i = 0; i < items->size; i++)
		tv_addItem(this, items->data[i]);

	SendDlgItemMessage(m_TreeHWnd, control_ID, WM_SETFONT, (WPARAM)ro->font, 0L);
	SendDlgItemMessage(parent, control_ID, WM_SETFONT, (WPARAM)ro->font, 0L);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		Value* TreeViewControl::get_property(Value** arg_list, int count)
//
//	Remarks:
// 		
//	Parameters:
//		
//	Return Value:
//		
//--------------------------------------------------------------------------------------------------------------------------------------------------
Value* TreeViewControl::get_property(Value** arg_list, int count)
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
	else if(prop == n_hwnd)
	{
		if(parent_rollout && parent_rollout->page)
			return IntegerPtr::intern( reinterpret_cast<INT64>( m_TreeHWnd ) );
		else
			return &undefined;
	}
	else if (prop == n_items)
	{
		if(parent_rollout && parent_rollout->page)
			return m_items;
		else
			return &undefined;
	}
	else if (prop == n_dragEnabled)
	{
		return (m_dragEnabled ? &true_value : &false_value);
	}
	else if (prop == n_multiselection)
	{
		return (m_multiselection ? &true_value : &false_value);
	}
	else if (prop == n_bkColor)
	{
		return ColorValue::intern(AColor(m_bkcolor));
	}
	else if ( prop == n_selectedColor ) {
		return ColorValue::intern(AColor(this->m_selectedColor));
	}
	else if ( prop == n_useSelectedColor ) {
		return (this->m_useSelectedColor) ? &true_value : &false_value;
	}
	else if (prop == n_data)
	{
		return m_data;
	}
	else if (prop == n_editLabels)
	{
		return (m_editLabels ? &true_value : &false_value);
	}
	else if (prop == n_syncSelection)
		return (m_syncSelection ? &true_value : &false_value);

	return RolloutControl::get_property(arg_list, count);
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
Value* TreeViewControl::set_property(Value** arg_list, int count)
{
	Value* val = arg_list[0];
	Value* prop = arg_list[1];

	if(prop == n_width)
	{
		if(parent_rollout && parent_rollout->page)
		{
			int width = val->to_int();
			HWND	hwnd = GetDlgItem(parent_rollout->page, control_ID);
			RECT	rect;
			GetWindowRect(hwnd, &rect);
			MapWindowPoints(NULL, parent_rollout->page,	(LPPOINT)&rect, 2);
			SetWindowPos(hwnd, NULL, rect.left, rect.top, width, rect.bottom-rect.top, SWP_NOZORDER);
		}
	}
	else if(prop == n_height)
	{
		if(parent_rollout && parent_rollout->page)
		{
			int height = val->to_int();
			HWND	hwnd = GetDlgItem(parent_rollout->page, control_ID);
			RECT	rect;
			GetWindowRect(hwnd, &rect);
			MapWindowPoints(NULL, parent_rollout->page,	(LPPOINT)&rect, 2);
			SetWindowPos(hwnd, NULL, rect.left, rect.top, rect.right-rect.left, height, SWP_NOZORDER);
		}
	}
	else if(prop == n_hwnd)
	{
		throw RuntimeError(_T("You cannot change hwnd property."));
	}
	else if (prop == n_items)
	{
		type_check(val, Array, _T("items:"));
		Array *items = (Array*)val;
		m_items = new Array(items ->size);
		if (parent_rollout != NULL && parent_rollout->page != NULL)
		{
			HWND tree_view = GetDlgItem(parent_rollout->page, control_ID);
			TreeView_DeleteAllItems(tree_view);

			for (int i = 0; i < items->size; i++)
				tv_addItem(this, items->data[i]);
		}
	}
	else if (prop == n_text || prop == n_caption) // not displayed
	{
		TCHAR *text = val->to_string(); // will throw error if not convertable
		caption = val->get_heap_ptr();
	}
	else if (prop == n_dragEnabled)
	{
		m_dragEnabled = val->to_bool();
	}
	else if (prop == n_multiselection)
	{
		m_multiselection = val->to_bool();
	}
	else if (prop == n_imageList)
	{
		TCHAR *iconFilename = val->to_string();
		//m_TreeViewImagesHwnd = ImageList_Create(16, 16, ILC_COLOR24 | ILC_MASK, 5, 0);
		LoadMAXFileIcon(val->to_string(), m_TreeViewImagesHwnd, kBackground, FALSE);
		TreeView_SetImageList(m_TreeHWnd, m_TreeViewImagesHwnd, TVSIL_NORMAL);
	}
	else if (prop == n_overlayList)
	{
		Array *items = (Array*)val;
		for (int i = 0; i < items->size; i++)
			ImageList_SetOverlayImage (m_TreeViewImagesHwnd, items->data[i]->to_int(), i+1);
	}
	else if (prop == n_bkColor)
	{
		m_bkcolor = val->to_colorref();
		TreeView_SetBkColor(m_TreeHWnd,m_bkcolor);
	}
	else if ( prop == n_selectedColor ) {
		this->m_selectedColor = val->to_colorref();
		if ( this->m_useSelectedColor )
			TreeView_SetBkColor( this->m_TreeHWnd, this->m_bkcolor );
	}
	else if ( prop == n_useSelectedColor ) {
		this->m_useSelectedColor = val->to_bool();
	}
	else if (prop == n_data)
	{
		m_data = val;
	}
	else if (prop == n_editLabels)
	{
		m_editLabels = val->to_bool();

		LONG_PTR  style = GetWindowLongPtr(m_TreeHWnd, GWL_STYLE);
		if (m_editLabels)
			style |= TVS_EDITLABELS;
		else
			style &= ~TVS_EDITLABELS;
		SetWindowLong (m_TreeHWnd, GWL_STYLE, style);
	}
	else if (prop == n_syncSelection)
		m_syncSelection = val->to_bool();
	else
		return RolloutControl::set_property(arg_list, count);

	return val;
}

Value* TreeViewControl::get_props_vf( Value** arg_list, int count )
{
	one_typed_value_local(Array* result);
	vl.result = new Array (0);

	vl.result->append( n_width );
	vl.result->append( n_height );
	vl.result->append( n_hwnd );
	vl.result->append( n_items );
	vl.result->append( n_dragEnabled );
	vl.result->append( n_multiselection );
	vl.result->append( n_bkColor );
	vl.result->append( n_selectedColor );
	vl.result->append( n_useSelectedColor );
	vl.result->append( n_data );
	vl.result->append( n_editLabels );
	vl.result->append( n_syncSelection );

	vl.result->append( n_imageList );
	vl.result->append( n_overlayList );
	vl.result->append( n_caption );
	vl.result->append( n_text );

	return_value(vl.result); 
}

Value* TreeViewControl::show_props_vf( Value** arg_list, int count )
{
	return ( get_props_vf( arg_list, count ) );
}
Value* TreeViewControl::show_methods_vf(Value** arg_list, int count)
{
	one_typed_value_local(Array* result);
	vl.result = new Array (0);
	return_value(vl.result); 
}

Value* TreeViewControl::show_events_vf(Value** arg_list, int count)
{
	one_typed_value_local(Array* result);
	vl.result = new Array (0);

	vl.result->append( n_beginDrag );
	vl.result->append( n_endDrag );
	vl.result->append( n_iconPressed );
	vl.result->append( n_buttonPressed );
	vl.result->append( n_indentPressed );
	vl.result->append( n_stateIconPressed );
	vl.result->append( n_rightAreaPressed );
	vl.result->append( n_mousemove );
	vl.result->append( n_mousedown );
	vl.result->append( n_mouseup );
	vl.result->append( n_dblclick );
	vl.result->append( n_rdblclick );
	vl.result->append( n_rclick );
	vl.result->append( n_focusChanged );

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
BOOL TreeViewControl::handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
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
void TreeViewControl::set_enable()
{
	if (parent_rollout != NULL && parent_rollout->page != NULL)
	{
		HWND ctrl = GetDlgItem(parent_rollout->page, control_ID);
		if(ctrl) 
		{	
			EnableWindow(ctrl, enabled);
			InvalidateRect(ctrl, NULL, TRUE);	// LAM - defect 302171
		}
	}
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
void TreeViewControl::Invalidate()
{
	if (m_containerHWnd == NULL) return;

	RECT rect;
	GetClientRect(m_containerHWnd , &rect);
	MapWindowPoints(m_containerHWnd , parent_rollout->page, (POINT*)&rect, 2);
	InvalidateRect(parent_rollout->page, &rect, TRUE);
	InvalidateRect(m_containerHWnd , NULL, TRUE);
}

//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		HTREEITEM TreeViewControl::AddItemToTree(Value *val)
//
//	Remarks:
// 		adds items to a tree-view control
//	Parameters:
//		hwndTV	- handle to the tree-view control. 
//		lpszItem - text of the item to add. 
//		nLevel	- level at which to add the item. 
//	Return Value:
//		Returns the handle to the newly added item. 
//--------------------------------------------------------------------------------------------------------------------------------------------------
Value *TreeViewControl::AddItemToTree(TreeViewItem* tvitem)
{ 
 	HTREEITEM hNewItem;
	TVINSERTSTRUCT tvins; 

	TVITEM tvi; 
    tvi.mask			= TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE; 
    tvi.pszText			= tvitem->m_name; 
    tvi.cchTextMax		= lstrlen(tvitem->m_name); 
	tvi.iImage			= tvitem->m_iconImage; 
	tvi.iSelectedImage	= tvitem->m_iconSelectedImage; 
	tvi.lParam			= (LPARAM) tvitem; 
	tvi.state			= tvitem->m_selected ? TVIS_SELECTED : 0;

    tvins.item			= tvi; 
    tvins.hInsertAfter	= TVI_LAST; 
    // Set the parent item based on the specified level. 
	if (tvitem->m_parent != NULL)
		tvins.hParent = tvitem->m_parent->m_tvi; 
	else
		tvins.hParent = TVI_ROOT;
 
    // Add the item to the tree-view control. 
    hNewItem = (HTREEITEM) SendMessage(m_TreeHWnd, TVM_INSERTITEM, 0, (LPARAM) (LPTVINSERTSTRUCT) &tvins); 
	
    tvi.mask = TVIF_PARAM; 
	tvi.hItem = hNewItem;
	TreeView_GetItem(m_TreeHWnd, &tvi);
	
	if (tvi.lParam)
	{
		TreeViewItem *tvitem = (TreeViewItem*)tvi.lParam;
		tvitem->m_tvi		= hNewItem;
		tvitem->m_tvControl = this;
		TreeView_SetItem(m_TreeHWnd, &tvi);
	}

	m_items->append(tvitem);

    return &true_value; 
} 






LRESULT CALLBACK TreeViewControl::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
 	LRESULT result = 0;

	// Get back at our stored class pointer
	TreeViewControl *tvc = DLGetWindowLongPtr<TreeViewControl*>(hWnd, GWLP_USERDATA);

	CharStream* out = thread_local(current_stdout); 
	if(tvc == NULL && message != WM_CREATE)
		return DefWindowProc(hWnd, message, wParam, lParam);

	switch ( message ) 
	{
		case WM_CREATE:
		{
			//out->printf(_T("WM_CREATE\n"));

			LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
			tvc = (TreeViewControl*)lpcs->lpCreateParams;
			SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)tvc);
			SetFocus(tvc->m_TreeHWnd);
			break;
		}

		case WM_SIZE:
		{
 			//out->printf(_T("WM_SIZE\n"));
			SetWindowPos(tvc->m_TreeHWnd, NULL, 0, 0 , LOWORD(lParam),HIWORD(lParam), SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE);

			break;
		}

		case WM_NOTIFY:
		{
	 		long lResult = tvc->handleNotify(hWnd, (int) wParam, (LPNMHDR) lParam);
			return lResult;
		}

		case WM_MOUSEMOVE:
		{
			tvc->run_event_handler(tvc->parent_rollout, n_mousemove, NULL, 0);

			if (tvc->m_dragging) 
			{ 
				tvc->m_Pos = MAKEPOINTS(lParam);
 				ImageList_DragMove(tvc->m_Pos.x-5, tvc->m_Pos.y-7); // where to draw the drag from
				ImageList_DragShowNolock(FALSE);
				tvc->m_tvht.pt.x = tvc->m_Pos.x; // the highlight items should be as the same points as the drag
				tvc->m_tvht.pt.y = tvc->m_Pos.y; //
				if(tvc->m_hitTarget = (HTREEITEM)SendMessage(tvc->m_TreeHWnd,TVM_HITTEST,NULL,(LPARAM)&(tvc->m_tvht))) // if there is a hit
					SendMessage(tvc->m_TreeHWnd,TVM_SELECTITEM,TVGN_DROPHILITE,(LPARAM)tvc->m_hitTarget);   // highlight it
			
			    ImageList_DragShowNolock(TRUE); 
			} 
		}
		break;
		
		case WM_LBUTTONDOWN: 
		{
 			ReleaseCapture(); 
			//SendMessage(tvc->m_hWnd,WM_NCLBUTTONDOWN,HTCAPTION,0);
		}
		break;

        case WM_LBUTTONUP:
        {
 			init_thread_locals();
			push_alloc_frame();
			one_value_local(arg);

            if (tvc->m_dragging)  
            {
				ImageList_DragLeave(tvc->m_TreeHWnd);
                ImageList_EndDrag();
                 HTREEITEM hItem = (HTREEITEM)SendMessage(tvc->m_TreeHWnd,TVM_GETNEXTITEM,TVGN_DROPHILITE,0);
				
				if (hItem)
				{
					SendMessage(tvc->m_TreeHWnd,TVM_SELECTITEM,TVGN_CARET,(LPARAM)hItem);
					SendMessage(tvc->m_TreeHWnd,TVM_SELECTITEM,TVGN_DROPHILITE,0);

					ReleaseCapture();
				//   ShowCursor(TRUE); 
					tvc->m_dragging = FALSE;
				
					TreeViewItem *pItemData = (TreeViewItem*) tvc->GetData(hItem);
					if (pItemData)
					{
						vl.arg = (Value*) pItemData;
						tvc->run_event_handler(tvc->parent_rollout, n_endDrag, &vl.arg, 1);
						tvc->Invalidate();
					}
				}
			}

			pop_value_locals();
			pop_alloc_frame();
		}
        break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

long TreeViewControl::handleNotify(HWND hWndDlg, int nIDCtrl, LPNMHDR pNMHDR)
{
	CharStream* out = thread_local(current_stdout); 
	
	if(pNMHDR->code == NM_LDOWN) 
		return this->LButtonDown();

	if(pNMHDR->code == NM_CLICK)
		return this->LButtonUp();

	if(pNMHDR->code == NM_DBLCLK)
		return this->LButtonDblClk();

	if(pNMHDR->code == NM_RDBLCLK)
		return this->RButtonDblClk();

	if(pNMHDR->code == NM_RCLICK)
		return this->RButtonDown();

	if(pNMHDR->code == NM_SETFOCUS)
		return this->FocusChanged();

	if (pNMHDR->code == NM_CUSTOMDRAW)
	{			
 		LPNMTVCUSTOMDRAW pNMTVCD = (LPNMTVCUSTOMDRAW) pNMHDR;
		HWND hWndTreeView = pNMHDR->hwndFrom;
		return handleCustomDraw(hWndTreeView, pNMTVCD);
	}

	if (pNMHDR->code == TVN_BEGINLABELEDIT)
	{			
 		LPNMTVCUSTOMDRAW pNMTVCD = (LPNMTVCUSTOMDRAW) pNMHDR;
		HWND hWndTreeView = pNMHDR->hwndFrom;
		return handleBeginEditLabel(hWndTreeView, pNMTVCD);
	}


	if (pNMHDR->code == TVN_ENDLABELEDIT)
	{			
 		LPNMTVCUSTOMDRAW pNMTVCD = (LPNMTVCUSTOMDRAW) pNMHDR;
		HWND hWndTreeView = pNMHDR->hwndFrom;
		return handleEndEditLabel(hWndTreeView, pNMTVCD);
	}

	if(pNMHDR->code == TVN_BEGINDRAG)
	{
		if (m_dragEnabled)
		{
 			LPNMTREEVIEW pNMTV = (LPNMTREEVIEW) pNMHDR;
			HWND hWndTreeView = pNMHDR->hwndFrom;
			handleBeginDrag(hWndTreeView, pNMTV);
			SetCapture(hWndDlg); 
			m_dragging = TRUE;
		}
	}

	if (pNMHDR->code == TVN_GETINFOTIP)
	{			
 		NMTVGETINFOTIP* pTVTipInfo = (NMTVGETINFOTIP*) pNMHDR;
		return handleCustomTooltip(pTVTipInfo);
	}

	
	return 0;
}

Value* TreeViewControl::GetData(HTREEITEM hItem)
{
	TVITEM tvItem;
	tvItem.hItem = hItem;
	tvItem.mask = TVIF_PARAM | TVIF_HANDLE;				
	TreeView_GetItem(m_TreeHWnd, &tvItem);

	TreeViewItem *pItemData= (TreeViewItem*) tvItem.lParam;
	if (pItemData)
		return pItemData;

	return &undefined;
}

long TreeViewControl::handleBeginDrag(HWND hWndTreeView, LPNMTREEVIEW pNMTV)
{
	CharStream* out = thread_local(current_stdout); 

 	if (pNMTV==NULL)
		return -1;

	HIMAGELIST hImg;
	LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW) pNMTV;
	hImg = TreeView_CreateDragImage(hWndTreeView, lpnmtv->itemNew.hItem);
 	ImageList_BeginDrag(hImg, 0, 0, 0);
	ImageList_DragEnter(hWndTreeView,lpnmtv->ptDrag.x,lpnmtv->ptDrag.y);
	//ShowCursor(FALSE); 

	init_thread_locals();
	push_alloc_frame();
	one_value_local(arg);

	TreeViewItem *pItemData = (TreeViewItem*) GetData(lpnmtv->itemNew.hItem);
	if (pItemData)
	{
		vl.arg = (Value*) pItemData;
		run_event_handler(parent_rollout, n_beginDrag, &vl.arg, 1);
		Invalidate();
	}

	pop_value_locals();
	pop_alloc_frame();

	return 0;
}
long TreeViewControl::handleCustomTooltip(NMTVGETINFOTIP* pTVTipInfo)
{
	LPARAM itemData = (DWORD) pTVTipInfo->lParam;
	TreeViewItem *pItemData = (TreeViewItem*) itemData;
	if (pItemData)
		strcpy(pTVTipInfo->pszText, (LPCTSTR) pItemData->m_tooltip);

	return 0;
}

Value* TreeViewControl::HitTest(HWND hWndTreeView, UINT flags = TVHT_ONITEMLABEL)
{
	TVHITTESTINFO tvhti;  // hit test information
	TVITEM tvItem;
	HTREEITEM hItem;

	tvhti.flags = flags; //TVHT_NOWHERE;
	GetCursorPos(&tvhti.pt);
	ScreenToClient(hWndTreeView, &tvhti.pt);

	if(hItem = TreeView_HitTest(hWndTreeView, &tvhti))
	{
		// Only select folder items for drop targets
		tvItem.hItem = hItem;
		tvItem.mask = TVIF_PARAM | TVIF_HANDLE;

		if ((tvhti.flags & flags) && TreeView_GetItem(hWndTreeView, &tvItem))
			if (tvItem.lParam)
				return (Value*) tvItem.lParam;
	}

	return &undefined;
}
LRESULT TreeViewControl::LButtonDown(void)
{
 	init_thread_locals();
	push_alloc_frame();
	one_value_local(arg);

	TreeViewItem *pItemData = (TreeViewItem*) HitTest(m_TreeHWnd, TVHT_ONITEM);
	if (pItemData && pItemData->m_enabled)
	{
        vl.arg = (Value*) pItemData;
		run_event_handler(parent_rollout, n_mousedown, &vl.arg, 1);
		Invalidate();
	}

	pop_value_locals();
	pop_alloc_frame();
	

	return 0;
}


LRESULT TreeViewControl::LButtonUp(void)
{
 	init_thread_locals();
	push_alloc_frame();
	one_value_local(arg);

	TreeViewItem *pItemData = (TreeViewItem*) HitTest(m_TreeHWnd, TVHT_ONITEMICON);
	if (is_treeviewitem(pItemData) && pItemData->m_enabled)
	{
        vl.arg = (Value*) pItemData;
		run_event_handler(parent_rollout, n_iconPressed, &vl.arg, 1);
	//	run_event_handler(parent_rollout, n_mouseup, &vl.arg, 1);
		Invalidate();
	}

	pItemData = (TreeViewItem*) HitTest(m_TreeHWnd, TVHT_ONITEMBUTTON);
	if (is_treeviewitem(pItemData) && pItemData->m_enabled)
	{
        vl.arg = (Value*) pItemData;
		run_event_handler(parent_rollout, n_buttonPressed, &vl.arg, 1);
	//	run_event_handler(parent_rollout, n_mouseup, &vl.arg, 1);
		Invalidate();
	}

	pItemData = (TreeViewItem*) HitTest(m_TreeHWnd, TVHT_ONITEMINDENT);
	if (is_treeviewitem(pItemData) && pItemData->m_enabled)
	{
        vl.arg = (Value*) pItemData;
		run_event_handler(parent_rollout, n_indentPressed, &vl.arg, 1);
	//	run_event_handler(parent_rollout, n_mouseup, &vl.arg, 1);
		Invalidate();
	}

	pItemData = (TreeViewItem*) HitTest(m_TreeHWnd, TVHT_ONITEMSTATEICON);
	if (is_treeviewitem(pItemData) && pItemData->m_enabled)
	{
        vl.arg = (Value*) pItemData;
		run_event_handler(parent_rollout, n_stateIconPressed, &vl.arg, 1);
	//	run_event_handler(parent_rollout, n_mouseup, &vl.arg, 1);
		Invalidate();
	}

	pItemData = (TreeViewItem*) HitTest(m_TreeHWnd, TVHT_ONITEMRIGHT);
	if (is_treeviewitem(pItemData) && pItemData->m_enabled)
	{
        vl.arg = (Value*) pItemData;
		run_event_handler(parent_rollout, n_rightAreaPressed, &vl.arg, 1);
	//	run_event_handler(parent_rollout, n_mouseup, &vl.arg, 1);
		Invalidate();
	}

	pItemData = (TreeViewItem*) HitTest(m_TreeHWnd, TVHT_ONITEM);
	if (is_treeviewitem(pItemData) && pItemData->m_enabled)
	{
        vl.arg = (Value*) pItemData;
		
		// is there is no multiselection clear the selection except the current item that inverses its selection
		if (!m_multiselection)
		{
			for (int i = 0; i < m_items->size; i++)
			{
				TreeViewItem* item = (TreeViewItem*) m_items->data[i];
				if (item != pItemData)
					item->m_selected = FALSE;
				//else
				//	item->m_selected = TRUE; //!item->m_selected;
			}
		}
		else
		{
			// multiselection means we have to process the ctrl and shift keys
			
			//pItemData->m_selected = TRUE; //!pItemData->m_selected;
		}

		run_event_handler(parent_rollout, n_mouseup, &vl.arg, 1);
		Invalidate();
	}

	

	pop_value_locals();
	pop_alloc_frame();
	

	return 0;
}

LRESULT TreeViewControl::LButtonDblClk(void)
{
 	init_thread_locals();
	push_alloc_frame();
	one_value_local(arg);

	TreeViewItem *pItemData = (TreeViewItem*) HitTest(m_TreeHWnd, TVHT_ONITEM);
	if (pItemData && pItemData->m_enabled)
	{
		LONG_PTR  style = GetWindowLongPtr(m_TreeHWnd, GWL_STYLE);
		if (style & TVS_EDITLABELS)
		{
			TreeView_EditLabel(m_TreeHWnd, pItemData->m_tvi);
		}
		else
		{
			vl.arg = (Value*) pItemData;
			run_event_handler(parent_rollout, n_dblclick, &vl.arg, 1);
		}
		Invalidate();
	}

	pop_value_locals();
	pop_alloc_frame();
	

	return 0;
}

LRESULT TreeViewControl::RButtonDblClk(void)
{
 	init_thread_locals();
	push_alloc_frame();
	one_value_local(arg);

	TreeViewItem *pItemData = (TreeViewItem*) HitTest(m_TreeHWnd, TVHT_ONITEM);
	if (pItemData && pItemData->m_enabled)
	{
        vl.arg = (Value*) pItemData;
		run_event_handler(parent_rollout, n_rdblclick, &vl.arg, 1);
		Invalidate();
	}

	pop_value_locals();
	pop_alloc_frame();
	

	return 0;
}

LRESULT TreeViewControl::RButtonDown(void)
{
 	init_thread_locals();
	push_alloc_frame();
	one_value_local(arg);

	TreeViewItem *pItemData = (TreeViewItem*) HitTest(m_TreeHWnd, TVHT_ONITEM);
	if (pItemData && pItemData->m_enabled)
	{
        vl.arg = (Value*) pItemData;
		run_event_handler(parent_rollout, n_rclick, &vl.arg, 1);
		Invalidate();
	}

	pop_value_locals();
	pop_alloc_frame();
	

	return 0;
}

LRESULT TreeViewControl::FocusChanged(void)
{
 	init_thread_locals();
	push_alloc_frame();
	one_value_local(arg);

	TreeViewItem *pItemData = (TreeViewItem*) HitTest(m_TreeHWnd, TVHT_ONITEM);
	if (pItemData && pItemData->m_enabled)
	{
        vl.arg = (Value*) pItemData;
		run_event_handler(parent_rollout, n_focusChanged, &vl.arg, 1);
		Invalidate();
	}

	pop_value_locals();
	pop_alloc_frame();

	return 0;
}


long TreeViewControl::handleCustomDraw(HWND hWndTreeView, LPNMTVCUSTOMDRAW pNMTVCD)
{
	CharStream* out = thread_local(current_stdout); 

 	if (pNMTVCD==NULL)
		return -1;

 //	out->printf(_T("pNMTVCD->nmcd.dwDrawStage %d \n"), pNMTVCD->nmcd.dwDrawStage);
	switch (pNMTVCD->nmcd.dwDrawStage)
	{ 
		case CDDS_PREPAINT:
		{/*
			HTREEITEM hItem = (HTREEITEM) pNMTVCD->nmcd.dwItemSpec;
            TVITEM tvi = { 0 };
            tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
            tvi.hItem = hItem;
            TreeView_GetItem(hWndTreeView, &tvi);
			
			if (tvi.lParam)
			{
				TreeViewItem *pItemData= (TreeViewItem*) tvi.lParam;
				if (pItemData)
				{
				//	if (tvi.pszText != pItemData->m_name)
					{
						tvi.pszText = pItemData->m_name;
						tvi.cchTextMax = lstrlen(pItemData->m_name); 
						TreeView_SetItem(hWndTreeView, &tvi);
					}
					out->printf(_T("CDDS_PREPAINT %s %s\n"), tvi.pszText, pItemData->m_name);
				}
			}*/

   			return (CDRF_NOTIFYSUBITEMDRAW);
		}

		case CDDS_ITEMPREPAINT:
		{
			//out->printf(_T("CDDS_ITEMPREPAINT %d \n"), hWndTreeView);

			HDC hdc = pNMTVCD->nmcd.hdc;
			HTREEITEM hItem = (HTREEITEM) pNMTVCD->nmcd.dwItemSpec;
			
            TVITEM tvi = { 0 };
            tvi.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
            tvi.hItem = hItem;
            TreeView_GetItem(hWndTreeView, &tvi);
			
			if (tvi.lParam)
			{
 				TreeViewItem *pItemData= (TreeViewItem*) tvi.lParam;
				
				pNMTVCD->clrText = pItemData->m_color;
 				pNMTVCD->clrTextBk = m_bkcolor; //GetSysColor( COLOR_BTNFACE );

 				if (pItemData)
				{
					COLORREF itemNotSelected	= m_bkcolor; 
					COLORREF itemSelected		= ( this->m_useSelectedColor ) ? this->m_selectedColor : LightenColour(pItemData->m_color, 0.75f);
					pNMTVCD->clrTextBk = pItemData->m_selected ? itemSelected : itemNotSelected;

					if (pItemData->m_selected)
					{
						RECT rcTarget;
						TreeView_GetItemRect(hWndTreeView, hItem, &rcTarget, 0);

						RECT rcTargetText;
						TreeView_GetItemRect(hWndTreeView, hItem, &rcTargetText, 1);

						if (pItemData->m_iconImage > 0)
							rcTarget.left = rcTargetText.left + ICON_SIZE;

						HBRUSH bkBrush = CreateSolidBrush( pNMTVCD->clrTextBk );
						SelectObject(hdc, bkBrush);
						FillRect(hdc, &rcTarget, bkBrush);
//						Rectangle(hdc, rcTarget.left, rcTarget.top, rcTarget.right, rcTarget.bottom);
						DeleteObject(bkBrush);

					}

				}
			}

			return (CDRF_NOTIFYPOSTPAINT | CDRF_NEWFONT);
		}

		case CDDS_ITEMPOSTPAINT:
		{
			//out->printf(_T("CDDS_ITEMPOSTPAINT %d \n"), hWndTreeView);
			/*
            HTREEITEM hItem = (HTREEITEM) pNMTVCD->nmcd.dwItemSpec;
            TVITEM tvi = { 0 };
            tvi.mask = TVIF_HANDLE | TVIF_PARAM;
            tvi.hItem = hItem;
            TreeView_GetItem(hWndTreeView, &tvi);

			if (tvi.lParam)
			{
				TreeViewItem *pItemData= (TreeViewItem*) tvi.lParam;
				if (pItemData)
				{
					if (pItemData->m_selected)
					{
						RECT rcTarget;
						TreeView_GetItemRect(hWndTreeView, hItem, &rcTarget, 0);

						RECT rcTargetText;
						TreeView_GetItemRect(hWndTreeView, hItem, &rcTargetText, 1);

						if (pItemData->m_iconImage > 0)
							rcTarget.left = rcTargetText.left;

						HBRUSH bkBrush = CreateSolidBrush( DarkenColour(pItemData->m_color, 0.25f));
						SelectObject(pNMTVCD->nmcd.hdc, bkBrush);
		 				FrameRect(pNMTVCD->nmcd.hdc, &rcTarget, bkBrush);
						DeleteObject(bkBrush);
					}
				}
			}
			*/
			return CDRF_DODEFAULT;
		}

		default:
			break;
	}

	return 0;
} 

long TreeViewControl::handleBeginEditLabel(HWND hWndTreeView, LPNMTVCUSTOMDRAW pNMTVCD)
{
	HTREEITEM target = TreeView_GetSelection(m_TreeHWnd);
	m_ItemEditing = GetData(target);
	m_hEdit = TreeView_GetEditControl(m_TreeHWnd);

	if (m_ItemEditing != &undefined)
	{
		TreeViewItem* pTvItem = (TreeViewItem*) m_ItemEditing;
		if (!pTvItem->m_enabled)
		{
			m_hEdit = NULL;
			m_ItemEditing = &undefined;
			TreeView_EndEditLabelNow(m_TreeHWnd, TRUE);
			return 0;
		}

 		init_thread_locals();
		push_alloc_frame();
		one_value_local(arg);

        vl.arg = m_ItemEditing;
		Value* isEditable = &undefined;
		Value* handler = get_event_handler(n_beginEditLabel);
		if ( handler != &undefined )
			isEditable = handler->apply(&vl.arg, 1);

		if (isEditable == &false_value)
		{
			m_hEdit = NULL;
			m_ItemEditing = &undefined;
			pop_value_locals();
			pop_alloc_frame();
			return 0;			
		}

		if (isEditable->is_kind_of(class_tag(String)))
			SetWindowText(m_hEdit, isEditable->to_string());

		//run_event_handler(parent_rollout, n_beginEditLabel, &vl.arg, 1);

		pop_value_locals();
		pop_alloc_frame();

		Invalidate();


	}
	else
	{
		m_hEdit = NULL;
		m_ItemEditing = &undefined;
	}

	return 0;
}

long TreeViewControl::handleEndEditLabel(HWND hWndTreeView, LPNMTVCUSTOMDRAW pNMTVCD)
{
	TreeViewItem* pItemData = (TreeViewItem*) m_ItemEditing;
	if (m_ItemEditing == &undefined)
		return FALSE;

	char itemText[256] = "";
	GetWindowText(m_hEdit, itemText, sizeof(itemText)); 

 	init_thread_locals();
	push_alloc_frame();
	two_value_locals(arg, text);

	vl.arg = m_ItemEditing;
	vl.text = new String(itemText);

	Value* isChanged = &true_value;
	Value* handler = get_event_handler(n_endEditLabel);
	if ( handler != &undefined )
		isChanged = handler->apply(&vl.arg, 2);

	//run_event_handler(parent_rollout, n_endEditLabel, &vl.arg, 2);

	Invalidate();

	pop_value_locals();
	pop_alloc_frame();

	m_ItemEditing = &undefined;

	return ((isChanged == &true_value) ? TRUE: FALSE);
}



//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		Value *TreeViewControl::CopyItem( TreeViewItem *pTreeviewItem , TreeViewItem *pParentTreeviewItem, HTREEITEM htiAfter = TVI_LAST )
//
//	Remarks:
// 		Copies an item to a new location
//	Parameters:
//		pTreeviewItem        - TreeviewItem to be copied
//		pParentTreeviewItem  - Parent for new item
//		htiAfter             - Item after which the new item should be created
//	Return Value:
//		TreeviewItem of the new item
//--------------------------------------------------------------------------------------------------------------------------------------------------
Value *TreeViewControl::CopyItem( TreeViewItem *pTreeviewItem , TreeViewItem *pParentTreeviewItem, HTREEITEM htiAfter = TVI_LAST )
{
	TreeViewItem *pTVItem = new TreeViewItem( pTreeviewItem );

	pTVItem->m_parent  = pParentTreeviewItem;
	pTVItem->m_tvControl = pParentTreeviewItem->m_tvControl;
	
    return tv_addItem(pTVItem->m_tvControl, pTVItem, pParentTreeviewItem);
}


//--------------------------------------------------------------------------------------------------------------------------------------------------
//	Prototype: 
//		Value *TreeViewControl::CopyBranch( TreeViewItem *pTreeviewItem , TreeViewItem *pParentTreeviewItem, HTREEITEM htiAfter = TVI_LAST )
//
//	Remarks:
// 		Copies all items in a branch to a new location
//	Parameters:
//		pTreeviewItem        - The node that starts the branch
//		pParentTreeviewItem  - Parent for new item
//		deleteOld			 - if true deletes the old branch
//		htiAfter             - Item after which the new item should be created
//	Return Value:
//		The new branch node
//--------------------------------------------------------------------------------------------------------------------------------------------------
Value *TreeViewControl::CopyBranch( TreeViewItem *pTreeviewItem , TreeViewItem *pParentTreeviewItem, BOOL deleteOld = FALSE, HTREEITEM htiAfter = TVI_LAST )
{
	push_alloc_frame();

	TreeViewControl *srcTvc = (TreeViewControl*) pTreeviewItem->m_tvControl;
	Value *pValueItem = CopyItem( pTreeviewItem, pParentTreeviewItem, htiAfter );
	TreeViewItem *pNewTreeviewItem = (TreeViewItem *)pValueItem ;//CopyItem( pTreeviewItem, pParentTreeviewItem, htiAfter );

	HTREEITEM hChild = TreeView_GetChild(srcTvc->m_TreeHWnd, pTreeviewItem->m_tvi);

	while( hChild != NULL)
	{
		// recursively transfer all the sibling items
		TVITEM tviChild; 
		tviChild.mask = TVIF_PARAM | TVIF_HANDLE; 
		tviChild.hItem = hChild;
 		BOOL success = TreeView_GetItem(srcTvc->m_TreeHWnd, &tviChild);

		if (success)
		{
			TreeViewItem *pChildTreeviewItem = (TreeViewItem*)tviChild.lParam;
			CopyBranch( pChildTreeviewItem , pNewTreeviewItem, deleteOld = deleteOld, htiAfter = htiAfter );
		}

		hChild = TreeView_GetNextSibling(srcTvc->m_TreeHWnd, hChild);
	}

	// delete the old branch if choosen
	if (deleteOld)
		TreeView_DeleteItem(srcTvc->m_TreeHWnd, pTreeviewItem->m_tvi);

	pop_alloc_frame();

	return pValueItem;//hNewItem;*/
} 

Value* TreeViewControl::get_vf(Value** arg_list, int count)
{
	int index = arg_list[0]->to_int();

	if (index <= m_items->size)
        return m_items->data[index+1];

	return &undefined;
}

/* ------------------- TreeViewItem Value class instance ----------------------- */






// ============================================================================
#include "definsfn.h"

visible_class_instance(TreeViewItem, "TreeItem");

// ============================================================================
TreeViewItem::TreeViewItem()
{
	tag					= class_tag(TreeViewItem);
	m_name				= _T("");
	m_tooltip			= _T("");
	m_node				= NULL;
	m_parent			= NULL;
	m_tvi				= (HTREEITEM) TVI_FIRST;
	m_enabled			= TRUE;
	m_selected			= TRUE;
	m_color				= RGB(0,  0, 0);
	m_iconImage			= 0;
	m_iconSelectedImage = 0;
	m_iconState			= 0;
	m_children			= new Array(0);
	m_data				= &undefined;
	m_extraData			= &undefined;
}

TreeViewItem::TreeViewItem(TreeViewItem *tvi)
{
	tag					= class_tag(TreeViewItem);
	m_name				= tvi->m_name;
	m_tooltip			= tvi->m_tooltip;
	m_node				= tvi->m_node;
	m_parent			= tvi->m_parent;
	m_tvi				= tvi->m_tvi;
	m_enabled			= tvi->m_enabled;
	m_selected			= tvi->m_selected;
	m_color				= tvi->m_color;
	m_iconImage			= tvi->m_iconImage;
	m_iconSelectedImage = tvi->m_iconSelectedImage;
	m_iconState			= tvi->m_iconState;
	m_children			= tvi->m_children;
	m_data				= tvi->m_data;
	m_extraData			= tvi->m_extraData;
}

// ============================================================================
void TreeViewItem::collect()
{
	if(m_node) DeleteObject(m_node);
	if(m_parent) DeleteObject(m_parent);
	if(m_children) DeleteObject(m_children);
	if(m_tvControl) DeleteObject(m_tvControl);
	if(m_data) DeleteObject(m_data);
	if(m_extraData) DeleteObject(m_extraData);
	//delete this;
}


// ============================================================================
void TreeViewItem::sprin1(CharStream* s)
{
	s->printf(_T("<TreeViewItem:%s>"), (TCHAR*)m_name);
}


// ============================================================================
void TreeViewItem::gc_trace()
{
	if (m_children && m_children->is_not_marked())
		m_children->gc_trace();

	if (m_node && m_node->is_not_marked())
		m_node->gc_trace();

	if (m_parent && m_parent->is_not_marked())
		m_parent->gc_trace();

	if (m_tvControl && m_tvControl->is_not_marked())
		m_tvControl->gc_trace();

	if (m_data && m_data->is_not_marked())
		m_data->gc_trace();

	if (m_extraData && m_extraData->is_not_marked())
		m_extraData->gc_trace();

	Value::gc_trace();
}


BOOL TreeViewItem::isVisible( void )
{
	// we are visible if all of our parents are visible
	HTREEITEM hParent = TreeView_GetParent( this->m_tvControl->m_TreeHWnd, this->m_tvi );
	if (hParent == NULL)
		return TRUE;

	BOOL isVisible = ( TreeView_GetItemState( this->m_tvControl->m_TreeHWnd, hParent, TVIS_EXPANDED) & TVIS_EXPANDED ) ? TRUE : FALSE;
	isVisible &= this->m_parent->isVisible();

	return isVisible;
}

BOOL TreeViewItem::isCollapsed( void )
{
	// we are collapsed if we are collapsed or any of our parents are collapsed
	BOOL isCollapsed = ( TreeView_GetItemState( this->m_tvControl->m_TreeHWnd, this->m_tvi, TVIS_EXPANDED) & TVIS_EXPANDED ) ? FALSE : TRUE;

	HTREEITEM hParent = TreeView_GetParent( this->m_tvControl->m_TreeHWnd, this->m_tvi );
	if (hParent != NULL)
		isCollapsed |= this->m_parent->isCollapsed();

	return isCollapsed;
}

BOOL TreeViewItem::collapse( void )
{
	BOOL success = TreeView_Expand( this->m_tvControl->m_TreeHWnd, this->m_tvi, TVE_COLLAPSE);
	return success;
}

BOOL TreeViewItem::toggleExpand( void )
{
	BOOL success = TreeView_Expand( this->m_tvControl->m_TreeHWnd, this->m_tvi, TVE_TOGGLE);
	return success;
}

BOOL TreeViewItem::expand( void )
{
	BOOL success = TreeView_Expand( this->m_tvControl->m_TreeHWnd, this->m_tvi, TVE_EXPAND);
	return success;
}

BOOL TreeViewItem::ensureVisible( void )
{
	BOOL success = TreeView_EnsureVisible( this->m_tvControl->m_TreeHWnd, this->m_tvi );
	return success;
}

Value* TreeViewItem::getSiblings ( void )
{
	TreeViewControl *tvc = this->m_tvControl;
	TreeViewItem *pTreeviewItem = this;
	
	one_typed_value_local(Array* siblings_array);
	vl.siblings_array = new Array (0);

	HTREEITEM hParent = TreeView_GetParent(tvc->m_TreeHWnd, pTreeviewItem->m_tvi);

	if (hParent == NULL)
		return_value(vl.siblings_array);

	HTREEITEM hSibling = TreeView_GetChild(tvc->m_TreeHWnd, hParent);

	while( hSibling != NULL)
	{
		// recursively transfer all the sibling items
		TVITEM tviSibling; 
		tviSibling.mask = TVIF_PARAM | TVIF_HANDLE; 
		tviSibling.hItem = hSibling;
 		BOOL success = TreeView_GetItem(tvc->m_TreeHWnd, &tviSibling);

		if (success)
		{
			TreeViewItem *pSiblingTreeviewItem = (TreeViewItem*)tviSibling.lParam;
			if ( pSiblingTreeviewItem->m_tvi != pTreeviewItem->m_tvi )
				vl.siblings_array->append((Value *)pSiblingTreeviewItem);
		}

		hSibling = TreeView_GetNextSibling(tvc->m_TreeHWnd, hSibling );
	}

	return_value(vl.siblings_array);
}

Value* TreeViewItem::getChildren ( BOOL recursive )
{
	TreeViewControl *tvc = this->m_tvControl;
	TreeViewItem *pTreeviewItem = this;

	one_typed_value_local(Array* children_array);
	vl.children_array = new Array (0);

	HTREEITEM hChild = TreeView_GetChild(tvc->m_TreeHWnd, pTreeviewItem->m_tvi);

	while( hChild != NULL)
	{
		// recursively transfer all the sibling items
		TVITEM tviChild; 
		tviChild.mask = TVIF_PARAM | TVIF_HANDLE; 
		tviChild.hItem = hChild;
 		BOOL success = TreeView_GetItem(tvc->m_TreeHWnd, &tviChild);

		if (success)
		{
			TreeViewItem *pChildTreeviewItem = (TreeViewItem*)tviChild.lParam;
			vl.children_array->append((Value *)pChildTreeviewItem);

			if (recursive)
				vl.children_array->join( pChildTreeviewItem->getChildren( recursive ) );
		}

		hChild = TreeView_GetNextSibling(tvc->m_TreeHWnd, hChild );
	}

	 return_value(vl.children_array);
}

Value* TreeViewItem::getParent ( void )
{
	TreeViewControl *tvc = this->m_tvControl;
	TreeViewItem *pTreeviewItem = this;
	
	Value *parentItem = &undefined;
	HTREEITEM hParent = TreeView_GetParent(tvc->m_TreeHWnd, pTreeviewItem->m_tvi);

	if (hParent == NULL)
		return parentItem;

	// recursively transfer all the sibling items
	TVITEM tviParent; 
	tviParent.mask = TVIF_PARAM | TVIF_HANDLE; 
	tviParent.hItem = hParent;
 	BOOL success = TreeView_GetItem(tvc->m_TreeHWnd, &tviParent);

	if (success)
	{
		TreeViewItem *pParentTreeviewItem = (TreeViewItem*)tviParent.lParam;
		parentItem = (Value *)pParentTreeviewItem;
	}

	return parentItem;
}

Value* TreeViewItem::editLabel ( void )
{
	TreeViewControl *tvc = this->m_tvControl;
	TreeViewItem *pTreeviewItem = this;
	
	TreeView_EnsureVisible(tvc->m_TreeHWnd, pTreeviewItem->m_tvi);
	TreeView_Select(tvc->m_TreeHWnd, pTreeviewItem->m_tvi, TVGN_FIRSTVISIBLE);
	TreeView_EditLabel(tvc->m_TreeHWnd, pTreeviewItem->m_tvi);

	return &true_value;
}

Value* TreeViewItem::endEditLabel ( void )
{
	TreeViewControl *tvc = this->m_tvControl;
	TreeView_EndEditLabelNow(tvc->m_TreeHWnd, TRUE);

	return &true_value;
}

Value* TreeViewItem::get_property(Value** arg_list, int count)
{
	Value* prop = arg_list[0];

	if (prop == n_name)
		return new String(this->m_name);

	else if (prop == n_tooltip)
		return new String(this->m_tooltip);

	else if (prop == n_node)
		return m_node;

	else if (prop == n_parent)
		return m_parent ? (Value *)m_parent : &undefined;

	else if (prop == n_children)
		return ( this->getChildren( FALSE ) );

	else if (prop == n_enabled)
		return ((this->m_enabled) ? &true_value : &false_value);

	else if (prop == n_selected)
		return ((this->m_selected) ? &true_value : &false_value);

	else if (prop == n_iconImage)
		return Integer::intern(m_iconImage);

	else if (prop == n_iconSelectedImage)
		return Integer::intern(m_iconSelectedImage);

	else if (prop == n_iconState)
		return Integer::intern(m_iconState);
	
	else if(prop == n_hwnd)
	{
		if(this->m_tvi != NULL)
			return IntegerPtr::intern( reinterpret_cast<INT64>(m_tvi) );
		else
			return &undefined;
	}

	else if (prop == n_color)
		return ColorValue::intern(AColor(this->m_color));

	else if (prop == n_treeViewControl)
		return m_tvControl;
	
	else if (prop == n_data)
		return m_data;

	else if (prop == n_extraData)
		return m_extraData;

	else if (prop == n_ensureVisible  )
		return ( NEW_GENERIC_METHOD( ensureVisible ) );

	else if (prop == n_expand  )
		return ( NEW_GENERIC_METHOD( expand ) );

	else if (prop == n_collapse  )
		return ( NEW_GENERIC_METHOD( collapse ) );

	else if (prop == n_toggleExpand  )
		return ( NEW_GENERIC_METHOD( toggleExpand ) );

	else if (prop == n_siblings  )
		return ( this->getSiblings() );

	else if (prop == n_isVisible  )
		return ( NEW_GENERIC_METHOD( isVisible ) );

	else if (prop == n_isCollapsed  )
		return ( NEW_GENERIC_METHOD( isCollapsed ) );

	else if (prop == n_editLabel  )
		return ( NEW_GENERIC_METHOD( editLabel ) );

	else if (prop == n_endEditLabel  )
		return ( NEW_GENERIC_METHOD( endEditLabel ) );

	/*

	def_struct_primitive( tv_deleteAllItems,treeview, "deleteAllItems");
	def_struct_primitive( tv_deleteItem,	treeview, "deleteItem");
	def_struct_primitive( tv_addItem,		treeview, "addItem");
	def_struct_primitive( tv_sortBy,		treeview, "sortBy");
	def_struct_primitive( tv_sortBy2,		treeview, "sortBy2");
	def_struct_primitive( tv_copyItem,		treeview, "copyItem");
	def_struct_primitive( tv_copyBranch,	treeview, "copyBranch");

	def_struct_primitive( tv_setParent, treeview, "setParent");

	def_struct_primitive( tv_iterate,		treeview, "iterate");
	def_struct_primitive( tv_addNodeHierarchy, treeview, "addNodeHierarchy");
	def_struct_primitive( tv_getItemsByNode, treeview, "getItemsByNode");
	def_struct_primitive( tv_editLabel, treeview, "editLabel");
	def_struct_primitive( tv_endEditLabel, treeview, "endEditLabel");
	def_struct_primitive( tv_updateWindow, treeview, "update");*/

	return &undefined;
}

Value* TreeViewItem::applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc )
{
	if ( methodID == n_ensureVisible )
	{
		check_arg_count ( n_ensureVisible, 0, count);
		BOOL success = this->ensureVisible();
		return success ? &true_value : &false_value;
	}
	else if ( methodID == n_expand )
	{
		check_arg_count ( n_expand, 0, count);
		BOOL success = this->expand();
		return success ? &true_value : &false_value;
	}	
	else if ( methodID == n_collapse )
	{
		check_arg_count ( n_collapse, 0, count);
		BOOL success = this->collapse();
		return success ? &true_value : &false_value;
	}	
	else if ( methodID == n_toggleExpand )
	{
		check_arg_count ( n_toggleExpand, 0, count);
		BOOL success = this->toggleExpand();
		return success ? &true_value : &false_value;
	}	
	else if ( methodID == n_children )
	{

	}	
	else if ( methodID == n_isVisible )
	{
		check_arg_count ( n_isVisible, 0, count);
		BOOL isVisible = this->isVisible();
		return (isVisible ? &true_value : &false_value);
	}	
	else if ( methodID == n_isCollapsed )
	{
		check_arg_count ( n_isCollapsed, 0, count);
		BOOL isCollapsed = this->isCollapsed();
		return (isCollapsed ? &true_value : &false_value);
	}	

	else if ( methodID == n_editLabel )
	{
		check_arg_count ( n_editLabel, 0, count);
		return ( this->editLabel() );
	}	

	else if ( methodID == n_endEditLabel )
	{
		check_arg_count ( n_endEditLabel, 0, count);
		return ( this->endEditLabel() );
	}	

	return &ok;
}

Value* TreeViewItem::set_property(Value** arg_list, int count)
{
	Value* val = arg_list[0];
	Value* prop = arg_list[1];

	if (prop == n_name)
	{
 		m_name = val->to_string();

		TVITEM tvItem; 

		tvItem.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
		tvItem.hItem = this->m_tvi;
		tvItem.pszText = this->m_name;
		tvItem.lParam = (LPARAM) this;

		BOOL success = TreeView_SetItem(this->m_tvControl->m_TreeHWnd, &tvItem);
 		m_tooltip = m_name;
		this->m_tvControl->Invalidate();
		/*UpdateWindow(m_tvControl->m_containerHWnd);
		UpdateWindow(m_tvControl->m_TreeHWnd);*/
		return success ? &true_value : &false_value;
	}
	else if (prop == n_tooltip)
	{
 		m_tooltip = val->to_string();
		this->m_tvControl->Invalidate();
		return &true_value;
	}
	else if (prop == n_node)
	{
		this->m_node = val;
		//throw RuntimeError (GetString(IDS_CANNOT_DIRECTLY_SET_NODE), val);
	}
	/*else if (prop == n_parent)
	{
		if (val != &unsupplied && val != &undefined)
		{
			TreeViewItem *pParentTreeviewItem = (TreeViewItem*) val;

			if (is_treeviewitem(pParentTreeviewItem))
			{
				//this->m_parent = pParentTreeviewItem;
				this->m_tvControl->SetParent(this, pParentTreeviewItem);
			}
			else
				this->m_tvControl->SetParent(this, NULL);
		}
		else
			this->m_tvControl->SetParent(this, NULL);

		return &true_value;
	}*/
	else if (prop == n_children)
	{
		throw RuntimeError (GetString(IDS_CANNOT_DIRECTLY_SET_CHILDREN_ARRAY), val);
	}
	else if (prop == n_enabled)
	{
		this->m_enabled = val->to_bool();
		return &true_value;
	}
	else if (prop == n_selected)
	{
		if (m_enabled)
		{
			this->m_selected = val->to_bool();

		/*	if (m_tvControl->m_syncSelection && is_node(m_node))
			{
				INode* theNode = m_node->to_node();

				Interface* ip = GetCOREInterface();
	
				if (m_selected)
					ip->SelectNode (theNode, 0);
				else
					ip->DeSelectNode (theNode);
			}*/
		}
		return &true_value;
	}
	else if (prop == n_iconImage)
	{
		m_iconImage = val->to_int();

		TVITEM tvItem; 
		tvItem.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_IMAGE; 

		tvItem.hItem = this->m_tvi;
 		tvItem.iImage = this->m_iconImage;
		tvItem.lParam = (LPARAM) this;

		BOOL success = TreeView_SetItem(this->m_tvControl->m_TreeHWnd, &tvItem);

		return success ? &true_value : &false_value;
	}
	else if (prop == n_iconSelectedImage)
	{
		m_iconSelectedImage = val->to_int();

		TVITEM tvItem; 
		tvItem.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_SELECTEDIMAGE; 

		tvItem.hItem = this->m_tvi;
		tvItem.iSelectedImage = this->m_iconSelectedImage;
		tvItem.lParam = (LPARAM) this;

		BOOL success = TreeView_SetItem(this->m_tvControl->m_TreeHWnd, &tvItem);

		return success ? &true_value : &false_value;
	}
	else if (prop == n_iconState)
	{
		m_iconState = val->to_int();

		TVITEM tvItem; 
		tvItem.mask = TVIF_HANDLE | TVIF_PARAM | TVIF_STATE; 
		tvItem.stateMask = TVIS_OVERLAYMASK;
		tvItem.state = (INDEXTOOVERLAYMASK(m_iconState));
		tvItem.hItem = this->m_tvi;
		tvItem.lParam = (LPARAM) this;

		BOOL success = TreeView_SetItem(this->m_tvControl->m_TreeHWnd, &tvItem);

		return success ? &true_value : &false_value;
	}
	else if (prop == n_hwnd) 
	{
		throw RuntimeError (GetString(IDS_CANNOT_DIRECTLY_SET_HWND), val);
	}
	else if (prop == n_color)
	{
		this->m_color = val->to_colorref();
		return &true_value;
	}
	else if (prop == n_treeViewControl) 
	{
		throw RuntimeError (GetString(IDS_CANNOT_DIRECTLY_SET_TVCONTROL), val);
	}
	else if (prop == n_data)
	{
		m_data = val;
	}
	else if (prop == n_extraData)
	{
		m_extraData = val;
	}


	return &true_value;
}

Value* TreeViewItem::get_props_vf( Value** arg_list, int count )
{
	one_typed_value_local(Array* result);
	vl.result = new Array (0);

	vl.result->append( n_name );
	vl.result->append( n_tooltip );
	vl.result->append( n_node );
	vl.result->append( n_parent );
	vl.result->append( n_children );
	vl.result->append( n_enabled );
	vl.result->append( n_selected );
	vl.result->append( n_iconImage );
	vl.result->append( n_iconSelectedImage );
	vl.result->append( n_iconState );
	vl.result->append( n_hwnd );
	vl.result->append( n_color );
	vl.result->append( n_treeViewControl );
	vl.result->append( n_data );
	vl.result->append( n_extraData );

	return_value(vl.result); 
}

Value* TreeViewItem::show_props_vf( Value** arg_list, int count )
{
	return ( get_props_vf( arg_list, count ) );
}

Value* TreeViewItem::show_methods_vf(Value** arg_list, int count)
{
	one_typed_value_local(Array* result);
	vl.result = new Array (0);
	vl.result->append( n_ensureVisible );
	vl.result->append( n_expand );
	vl.result->append( n_collapse );
	vl.result->append( n_toggleExpand );
	vl.result->append( n_isVisible );
	vl.result->append( n_isCollapsed );
	vl.result->append( n_editLabel );
	vl.result->append( n_endEditLabel );

	return_value(vl.result); 
}

Value* TreeViewItem::show_events_vf(Value** arg_list, int count)
{
	one_typed_value_local(Array* result);
	vl.result = new Array (0);
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
void TreeViewInit()
{
	#include "defimpfn.h"
	def_name ( hwnd )
	def_name ( additem )
	def_name ( deleteitem )
	def_name ( deleteAllItems )
	def_name ( expand )
	def_name ( collapse )
	def_name ( toggleExpand )
	def_name ( ensureVisible )
	def_name ( showTree )
	def_name ( beginDrag )
	def_name ( endDrag )
	def_name ( imageList )
	def_name ( treeViewControl )
	def_name ( iconSelectedImage )
	def_name ( iconImage )
	def_name ( iconState )
	def_name ( overlayList )

	def_name ( multiselection )
	def_name ( dragEnabled )
		
	def_name ( iconPressed )
	def_name ( buttonPressed )
	def_name ( indentPressed )
	def_name ( stateIconPressed )
	def_name ( rightAreaPressed )

	def_name ( sortBy )
	
	def_name ( flat2D )
	def_name ( bkColor )
	def_name ( selectedColor )
	def_name ( useSelectedColor )
	def_name ( editLabels )
	def_name ( preBeginEditLabel )
	def_name ( beginEditLabel )
	def_name ( endEditLabel )
	def_name ( focusChanged )

	def_name ( data )
	def_name ( extraData )

	def_name ( syncSelection )
	def_name ( recursive )

	def_name ( decrease )
	def_name ( rootNodes )
	def_name ( rdblclick )

	def_name ( tooltip )
	def_name ( customtooltips )

	def_name ( siblings )
	def_name ( isCollapsed )
	def_name ( editLabel )

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
		wcex.lpfnWndProc   = TreeViewControl::WndProc;
		wcex.lpszClassName = TREECTRL_WINDOWCLASS;
		wcex.cbSize        = sizeof(WNDCLASSEX);
		wcex.hIconSm       = NULL;

		if(!RegisterClassEx(&wcex))
			return;
		registered = TRUE;
	}

	// initialize variables to hold names used as keyword parameters
	install_rollout_control(Name::intern("TreeView"), TreeViewControl::create);

}


// define the new primitives using macros from SDK
#include "defextfn.h"
#include "definsfn.h"

def_struct_primitive( tv_ensureVisible, treeview, "ensureVisible");
def_struct_primitive( tv_expand,		treeview, "expand");
def_struct_primitive( tv_collapse,		treeview, "collapse");
def_struct_primitive( tv_toggleExpand,	treeview, "toggleExpand");
def_struct_primitive( tv_deleteAllItems,treeview, "deleteAllItems");
def_struct_primitive( tv_deleteItem,	treeview, "deleteItem");
def_struct_primitive( tv_addItem,		treeview, "addItem");
def_struct_primitive( tv_sortBy,		treeview, "sortBy");
def_struct_primitive( tv_sortBy2,		treeview, "sortBy2");
def_struct_primitive( tv_copyItem,		treeview, "copyItem");
def_struct_primitive( tv_copyBranch,	treeview, "copyBranch");
def_struct_primitive( tv_getChildren,	treeview, "getChildren");
def_struct_primitive( tv_getSiblings,	treeview, "getSiblings");
def_struct_primitive( tv_getParent,		treeview, "getParent");
def_struct_primitive( tv_setParent, treeview, "setParent");
def_struct_primitive( tv_iterate,		treeview, "iterate");
def_struct_primitive( tv_isVisible,		treeview, "isVisible");
def_struct_primitive( tv_isCollapsed,	treeview, "isCollapsed");
def_struct_primitive( tv_addNodeHierarchy, treeview, "addNodeHierarchy");
def_struct_primitive( tv_getItemsByNode, treeview, "getItemsByNode");
def_struct_primitive( tv_editLabel, treeview, "editLabel");
def_struct_primitive( tv_endEditLabel, treeview, "endEditLabel");
def_struct_primitive( tv_updateWindow, treeview, "update");


Value *tv_ensureVisible_cf(Value** arg_list, int count)
{
	check_arg_count (tv_ensureVisible, 2, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];

	if (is_treeviewitem(arg_list[1]))
	{
		TreeViewItem *pTreeviewItem = (TreeViewItem*) arg_list[1];
		BOOL success = TreeView_EnsureVisible(tvc->m_TreeHWnd, pTreeviewItem->m_tvi);
		return success ? &true_value : &false_value;
	}

	if (is_array(arg_list[1]))
	{
		BOOL success = TRUE;
		Array *tvItemArray = (Array*) arg_list[1];

		for (int i = 0; i < tvItemArray->size; i++)
		{
			TreeViewItem *pTreeviewItem = (TreeViewItem*) tvItemArray->data[i];
			success = success && TreeView_EnsureVisible(tvc->m_TreeHWnd, pTreeviewItem->m_tvi);
		}

		return success ? &true_value : &false_value; 
	}

	return &ok;
}

Value *tv_expand_cf(Value** arg_list, int count)
{
	check_arg_count (tv_expand, 2, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];

	if (is_treeviewitem(arg_list[1]))
	{
		TreeViewItem *pTreeviewItem = (TreeViewItem*) arg_list[1];
		BOOL success = TreeView_Expand(pTreeviewItem->m_tvControl->m_TreeHWnd, pTreeviewItem->m_tvi, TVE_EXPAND);
		return success ? &true_value : &false_value;
	}

	if (is_array(arg_list[1]))
	{
		BOOL success = TRUE;
		Array *tvItemArray = (Array*) arg_list[1];

		for (int i = 0; i < tvItemArray->size; i++)
		{
			TreeViewItem *pTreeviewItem = (TreeViewItem*) tvItemArray->data[i];
			success = success && TreeView_Expand(pTreeviewItem->m_tvControl->m_TreeHWnd, pTreeviewItem->m_tvi, TVE_EXPAND);
		}

		return success ? &true_value : &false_value; 
	}

	return &ok;
}

Value *tv_collapse_cf(Value** arg_list, int count)
{
	check_arg_count (tv_collapse, 2, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];

	if (is_treeviewitem(arg_list[1]))
	{
		TreeViewItem *pTreeviewItem = (TreeViewItem*) arg_list[1];
		BOOL success = TreeView_Expand(tvc->m_TreeHWnd, pTreeviewItem->m_tvi, TVE_COLLAPSE);
		return success ? &true_value : &false_value;
	}

	if (is_array(arg_list[1]))
	{
		BOOL success = TRUE;
		Array *tvItemArray = (Array*) arg_list[1];

		for (int i = 0; i < tvItemArray->size; i++)
		{
			TreeViewItem *pTreeviewItem = (TreeViewItem*) tvItemArray->data[i];
			success = success && TreeView_Expand(tvc->m_TreeHWnd, pTreeviewItem->m_tvi, TVE_COLLAPSE);
		}

		return success ? &true_value : &false_value; 
	}

	return &ok;
}

Value *tv_toggleExpand_cf(Value** arg_list, int count)
{
	check_arg_count (tv_toggleExpand, 2, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];

	if (is_treeviewitem(arg_list[1]))
	{
		TreeViewItem *pTreeviewItem = (TreeViewItem*) arg_list[1];
		BOOL success = TreeView_Expand(tvc->m_TreeHWnd, pTreeviewItem->m_tvi, TVE_TOGGLE);
		return success ? &true_value : &false_value;
	}

	if (is_array(arg_list[1]))
	{
		BOOL success = TRUE;
		Array *tvItemArray = (Array*) arg_list[1];

		for (int i = 0; i < tvItemArray->size; i++)
		{
			TreeViewItem *pTreeviewItem = (TreeViewItem*) tvItemArray->data[i];
			success = success && TreeView_Expand(tvc->m_TreeHWnd, pTreeviewItem->m_tvi, TVE_TOGGLE);
		}

		return success ? &true_value : &false_value; 
	}

	return &ok;
}

Value *tv_deleteAllItems_cf(Value** arg_list, int count)
{
	check_arg_count (tv_deleteAllItems, 1, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	TreeView_DeleteAllItems(tvc->m_TreeHWnd);

	// DGH May 16, 2006 : No clue why is crashing when File->Reset ... but I suspect is has something to do with not clearing the items
	/*for (int i = 0 ; i < tvc->m_items->size; i++)
		DeleteObject(tvc->m_items->data[i]);*/

	return &ok;
}

Value *tv_deleteItem_cf(Value** arg_list, int count)
{
	check_arg_count (tv_deleteItem, 2, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	TreeViewItem *pTreeviewItem = (TreeViewItem*) arg_list[1];
	
	if (is_treeviewitem(pTreeviewItem) && is_treeviewcontrol(tvc))
	{
		TreeView_DeleteItem(tvc->m_TreeHWnd,pTreeviewItem->m_tvi);
		

		Array *itemsArray = new Array(0);
		
		for (int i = 0 ; i < tvc->m_items->size; i++)
			if (tvc->m_items->data[i] != (Value*) pTreeviewItem)
				itemsArray->append(tvc->m_items->data[i]);
			// DGH May 16, 2006 : No clue why is crashing when File->Reset ... but I suspect is has something to do with not clearing the items
			/*else
                DeleteObject(tvc->m_items->data[i]);*/

		tvc->m_items = itemsArray;
	}

	return &ok;
}

Value *tv_addItem_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys (tv_addItem, 2, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	Value* parent = key_arg(parent);

	return tv_addItem(tvc, arg_list[1], parent);
}

Array* tv_addNodeHierarchy(TreeViewControl *tvc, Value* nodeValue, Value* parent, Value* showTree)
{
	one_typed_value_local(Array* items);
	vl.items = new Array(0);

	Value* newItem = tv_addItem(tvc, nodeValue, parent);
	vl.items->append(newItem);

	INode* node = NULL;

	if (nodeValue->is_kind_of(class_tag(MAXRootNode)))
		node = nodeValue->to_rootnode();
	else
		node = nodeValue->to_node();

	if (!node)
		return_value(vl.items);

	int childrenCount = node->NumberOfChildren();	
	for (int i = 0; i < childrenCount ; i++)
	{
		INode* childNode = node->GetChildNode(i);
		Value* childValueNode = new MAXNode(childNode);

		if (showTree->to_bool())
			vl.items->join(tv_addNodeHierarchy(tvc, childValueNode, newItem, &true_value));
		else
			vl.items->join(tv_addNodeHierarchy(tvc, childValueNode, parent, &false_value));
	}

	return_value(vl.items);
}

Value *tv_addNodeHierarchy_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys (tv_addItem, 2, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	INode* node = arg_list[1]->to_node();
	Value* parent = key_arg(parent);
	Value* showTree = key_arg(showTree);
	
	Array* items = tv_addNodeHierarchy(tvc, arg_list[1], parent, showTree);
	return items;
}

Value *tv_copyItem_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys (tv_copyItem, 2, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	TreeViewItem *pTreeviewItem = (TreeViewItem*) arg_list[1];
	
	Value* parent = key_arg(parent);
	TreeViewItem *pParentTVItem = (TreeViewItem*) parent;

	return tvc->CopyItem(pTreeviewItem, pParentTVItem);
}

Value *tv_copyBranch_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys (tv_copyItem, 3, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	TreeViewItem *pTreeviewItem = (TreeViewItem*) arg_list[1];
	TreeViewItem *pParentTVItem = (TreeViewItem*) arg_list[2];
	tvc->CopyBranch(pTreeviewItem, pParentTVItem, FALSE);

	return &true_value;
}


Value *tv_iterateThrough(TreeViewItem *pTreeviewItem , Value* user_fn )
{
	Value* arglist[1] = { (Value*)pTreeviewItem };

	if (user_fn->is_kind_of(class_tag(Array)))
	{
		Array* items = (Array*)user_fn;
		items->append((Value*)pTreeviewItem);
	}
	else
		user_fn->apply(arglist, 1);

	TreeViewControl *tvc = (TreeViewControl*) pTreeviewItem->m_tvControl;

	HTREEITEM hChild = TreeView_GetChild(tvc->m_TreeHWnd, pTreeviewItem->m_tvi);

	if (hChild != NULL)
	{
		TVITEM tviChild; 
		tviChild.mask = TVIF_PARAM | TVIF_HANDLE; 
		tviChild.hItem = hChild;
 		BOOL success = TreeView_GetItem(tvc->m_TreeHWnd, &tviChild);

		if (success)
		{
			TreeViewItem *pChildTreeviewItem = (TreeViewItem*)tviChild.lParam;
			tv_iterateThrough(pChildTreeviewItem, user_fn);
		}
	}

	HTREEITEM hSibling = TreeView_GetNextSibling(tvc->m_TreeHWnd, pTreeviewItem->m_tvi);
    if (hSibling != NULL)
	{
		TVITEM tviSibling; 
		tviSibling.mask = TVIF_PARAM | TVIF_HANDLE; 
		tviSibling.hItem = hSibling;
 		BOOL success = TreeView_GetItem(tvc->m_TreeHWnd, &tviSibling);

		if (success)
		{
			TreeViewItem *pSiblingTreeviewItem = (TreeViewItem*)tviSibling.lParam;
			tv_iterateThrough(pSiblingTreeviewItem, user_fn);
		}		
	}

	return &true_value;
}
/*
Value *tv_selectBranch_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(tv_sortBy, 2, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	TreeViewItem *pTreeviewItem = (TreeViewItem*) arg_list[1];

	HTREEITEM hChild = TreeView_GetChild(tvc->m_TreeHWnd, pTreeviewItem->m_tvi);

	if (hChild != NULL)
	{
		TVITEM tviChild; 
		tviChild.mask = TVIF_PARAM | TVIF_HANDLE; 
		tviChild.hItem = hChild;
 		BOOL success = TreeView_GetItem(tvc->m_TreeHWnd, &tviChild);

		if (success)
		{
			TreeViewItem *pChildTreeviewItem = (TreeViewItem*)tviChild.lParam;
			tv_iterateThrough(pChildTreeviewItem, user_fn);
		}
	}	
}*/

Value *tv_sortBy_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(tv_sortBy, 2, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	if (!is_function(arg_list[1]))
		throw RuntimeError (GetString(IDS_SORT_ARG_NOT_FN2), arg_list[1] );

	tvc->m_user_compare_fn = arg_list[1];
	
	Value* decreaseSort = key_arg_or_default(decrease, &false_value);
	Value* rootNodesValue = key_arg(rootNodes);

	// we sort the entire treeview so we have to get all the root nodes in order to sort their children too
	Array* rootNodes = new Array(0);
	if (rootNodesValue != &unsupplied)
		rootNodes = (Array*) rootNodesValue;

 	for (int i = 0; i < tvc->m_items->size; i++)
	{
		TreeViewItem *pTreeviewItem = (TreeViewItem*) tvc->m_items->data[i];
		if (pTreeviewItem->m_parent == NULL)
			rootNodes->append(pTreeviewItem);
	}

	BOOL success = TRUE;
	for (int i = 0; i < rootNodes->size; i++)
	{
		TreeViewItem *pTreeviewItem = (TreeViewItem*) rootNodes->data[i];
		if (is_treeviewitem(pTreeviewItem ))
		{
			HTREEITEM HtiChild = TreeView_GetChild(tvc->m_TreeHWnd,pTreeviewItem->m_tvi);

			TVSORTCB sort;
			sort.hParent = HtiChild ? pTreeviewItem->m_tvi : NULL;//pTreeviewItem->m_tvi;
			sort.lParam = (LPARAM)tvc;
			sort.lpfnCompare = (decreaseSort == &false_value) ? Compare_fn : DecreaseCompare_fn;
			
			success = success && TreeView_SortChildrenCB(tvc->m_TreeHWnd, &sort, TRUE);
		}
	}

	// we need to modify the items property to match the new sorted tree
	TVITEM tviRoot; 
	tviRoot.mask = TVIF_PARAM | TVIF_HANDLE; 
	tviRoot.hItem = TreeView_GetRoot(tvc->m_TreeHWnd);
 	BOOL successRoot = TreeView_GetItem(tvc->m_TreeHWnd, &tviRoot);

	if (successRoot)
	{
		TreeViewItem *pStartTreeviewItem = (TreeViewItem*)tviRoot.lParam;
		Array *pItemsArray = new Array(0);
		tv_iterateThrough(pStartTreeviewItem, pItemsArray);
		tvc->m_items = pItemsArray;
	}		

	return success ? &true_value : &false_value;
}

Value *tv_sortBy2_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys(tv_sortBy, 3, count);

	BOOL success = FALSE;

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	if (!is_function(arg_list[1]))
		throw RuntimeError (GetString(IDS_SORT_ARG_NOT_FN2), arg_list[1] );
	tvc->m_user_compare_fn = arg_list[1];
	Value* decreaseSort = key_arg_or_default(decrease, &false_value);

	TreeViewItem *pTreeviewItem = (TreeViewItem*) arg_list[2];
	if (is_treeviewitem(pTreeviewItem ))
	{
		TVSORTCB sort;
		sort.hParent = pTreeviewItem->m_tvi;
		sort.lParam = (LPARAM)tvc;
		sort.lpfnCompare = (decreaseSort == &false_value) ? Compare_fn : DecreaseCompare_fn;
		
		success = TreeView_SortChildrenCB(tvc->m_TreeHWnd, &sort, TRUE);
	}

	return success ? &true_value : &false_value;
}

Value *tv_getSiblings_cf(Value** arg_list, int count)
{
	check_arg_count (tv_getSiblings, 2, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	TreeViewItem *pTreeviewItem = (TreeViewItem*) arg_list[1];

	return ( pTreeviewItem->getSiblings() );
}

Value *tv_getChildren_cf(Value** arg_list, int count)
{
	check_arg_count (tv_getChildren, 3, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	TreeViewItem *pTreeviewItem = (TreeViewItem*) arg_list[1];
	BOOL recursive = arg_list[2]->to_bool();

	return ( pTreeviewItem->getChildren( recursive ) );
}


Value *tv_getParent_cf(Value** arg_list, int count)
{
	check_arg_count (tv_getParent, 2, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	TreeViewItem *pTreeviewItem = (TreeViewItem*) arg_list[1];

	return pTreeviewItem->getParent();
}

Value *tv_isVisible_cf(Value** arg_list, int count)
{
	check_arg_count (tv_isVisible, 2, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	TreeViewItem *pTreeviewItem = (TreeViewItem*) arg_list[1];
	
	BOOL isVisible = pTreeviewItem->isVisible();
	return (isVisible ? &true_value : &false_value);
}


Value *tv_isCollapsed_cf(Value** arg_list, int count)
{
	check_arg_count (tv_isCollapsed, 2, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	TreeViewItem *pTreeviewItem = (TreeViewItem*) arg_list[1];
	
	BOOL isCollapsed = pTreeviewItem->isCollapsed();
	return (isCollapsed ? &true_value : &false_value);
}


Value *tv_iterate_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys (tv_iterate, 2, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	if (!is_function(arg_list[1]))
		throw RuntimeError (GetString(IDS_SORT_ARG_NOT_FN2), arg_list[1] );

	Value* user_fn = arg_list[1];
	Value* startItem = key_arg(start);

	if (startItem == &unsupplied)
	{
		TVITEM tviSibling; 
		tviSibling.mask = TVIF_PARAM | TVIF_HANDLE; 
		tviSibling.hItem = TreeView_GetRoot(tvc->m_TreeHWnd);
 		BOOL success = TreeView_GetItem(tvc->m_TreeHWnd, &tviSibling);

		if (success)
		{
			TreeViewItem *pStartTreeviewItem = (TreeViewItem*)tviSibling.lParam;
			tv_iterateThrough(pStartTreeviewItem, user_fn);
		}		
	}
	else
	{
		TreeViewItem *pStartTreeviewItem = (TreeViewItem*)startItem;
		tv_iterateThrough(pStartTreeviewItem, user_fn);
	}

	return &undefined;
}

Value *tv_getItemsByNode_cf(Value** arg_list, int count)
{
	check_arg_count (tv_getItemsByNode, 2, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	Array* nodeArray = (Array*) arg_list[1];
	
	one_typed_value_local(Array* result);
	vl.result = new Array(0);

	for (int i = 0; i < tvc->m_items->size; i++)
	{
		TreeViewItem *pTreeviewItem = (TreeViewItem*) tvc->m_items->data[i];

		for (int j = 0; j < nodeArray->size; j++)
		{
			if (pTreeviewItem->m_node == nodeArray->data[i])
				vl.result->append(tvc->m_items->data[i]);
		}
	}

	return_value(vl.result);
}

Value *tv_editLabel_cf(Value** arg_list, int count)
{
	check_arg_count (tv_editLabel, 2, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	TreeViewItem *pTreeviewItem = (TreeViewItem*) arg_list[1];
	
	pTreeviewItem->ensureVisible();
	TreeView_Select(tvc->m_TreeHWnd, pTreeviewItem->m_tvi, TVGN_FIRSTVISIBLE);
	pTreeviewItem->editLabel();

	return &true_value;
}

Value *tv_endEditLabel_cf(Value** arg_list, int count)
{
	check_arg_count (tv_endEditLabel, 1, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	TreeView_EndEditLabelNow(tvc->m_TreeHWnd, TRUE);

	return &true_value;
}

Value *tv_updateWindow_cf(Value** arg_list, int count)
{
	check_arg_count (tv_updateWindow, 1, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	tvc->InvalidateUI();
	tvc->Invalidate();
	UpdateWindow(tvc->m_containerHWnd);

	return &true_value;
}

Value *tv_setParent_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys (tv_copyItem, 3, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	TreeViewItem *pTreeviewItem = (TreeViewItem*) arg_list[1];
	TreeViewItem *pParentTVItem = (TreeViewItem*) arg_list[2];
	if ( pTreeviewItem != pParentTVItem && pParentTVItem != pTreeviewItem->m_parent )
		return tvc->CopyBranch(pTreeviewItem, pParentTVItem, TRUE);
	else
		return &false_value;
}



/*Value *tv_setTooltip_cf(Value** arg_list, int count)
{
	check_arg_count_with_keys (tv_setTooltip, 2, count);

	TreeViewControl *tvc = (TreeViewControl*) arg_list[0];
	TreeViewItem *pTreeviewItem = (TreeViewItem*) arg_list[1];

	HWND hToolTip = TreeView_GetToolTips(tvc->m_TreeHWnd);

	static TOOLINFO ti;

	memset(&ti, 0, sizeof(TOOLINFO));
	ti.cbSize = sizeof(TOOLINFO);
	ti.hwnd = hToolTip;
	ti.uId = 1;
	ti.lpszText = (LPSTR)(pTreeviewItem->m_name);

	SendMessage(hToolTip, TTM_SETTOOLINFO, 0, (LPARAM)ti);


	return &true_value;
}*/

