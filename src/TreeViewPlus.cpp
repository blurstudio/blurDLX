#ifdef __MAXSCRIPT_2012__
#include "maxscript\maxscript.h"
#include "maxscript\maxwrapper\mxsobjects.h"
#include "maxscript\foundation\numbers.h"
#include "maxscript\foundation\colors.h"
#include "maxscript\foundation\structs.h"
#include "maxscript\compiler\parser.h"

#include "maxscript\UI\rollouts.h"

#include "maxscript\maxwrapper\mxsmaterial.h"
#else
#include "MAXScrpt.h"
#include "MAXObj.h"
#include "Numbers.h"
#include "ColorVal.h"
#include "Structs.h"
#include "Parser.h"

#include "rollouts.h"

#include "MAXMats.h"
#endif
#include "MaxIcon.h"

extern HINSTANCE		g_hInst;
extern COLORREF			DarkenColour(COLORREF col, double factor);
extern COLORREF			LightenColour(COLORREF col, double factor);
extern INode*			_get_valid_node(MAXNode* _node, TCHAR* errmsg);

#include "resource.h"

#ifdef ScripterExport
	#undef ScripterExport
#endif
#define ScripterExport __declspec( dllexport )

#include "GenericMethod.h"
#include "TreeViewPlus.h"

//----------------------------------------------------------------------------------------------------------------
//													GLOBALS
//----------------------------------------------------------------------------------------------------------------

#define				GAP_SIZE					5
#define				ICON_SIZE					16
#define				TREECTRL_WINDOWCLASS		_T("TREEPLUSCTRL_WINDOWCLASS")

static WNDPROC		lpfnWndProc = NULL;			// original wndproc for the tree view

#ifdef __MAXSCRIPT_2012__
#include "maxscript\macros\define_external_functions.h"
#include "maxscript\macros\define_instantiation_functions.h"
#else
// external name definitions
#include "defextfn.h"
// internal name definitions
#include "definsfn.h"
#endif

#define				n_addChild					(Name::intern(_T("addChild")))
#define				n_addItem					(Name::intern(_T("addItem")))
#define				n_autoCollapse				(Name::intern(_T("autoCollapse")))

#define				n_beginDrag					(Name::intern(_T("beginDrag")))
#define				n_bkColor					(Name::intern(_T("bkColor")))

#define				n_control					(Name::intern(_T("control")))
#define				n_customToolTips			(Name::intern(_T("customToolTips")))

#define				n_data						(Name::intern(_T("data")))
#define				n_decrease					(Name::intern(_T("decrease")))
#define				n_deleteChildren			(Name::intern(_T("deleteChildren")))
#define				n_deleteItem				(Name::intern(_T("deleteItem")))
#define				n_deleteAllItems			(Name::intern(_T("deleteAllItems")))
#define				n_deselectAll				(Name::intern(_T("deselectAll")))
#define				n_dragEnabled				(Name::intern(_T("dragEnabled")))

#define				n_editLabel					(Name::intern(_T("editLabel")))
#define				n_editLabels				(Name::intern(_T("editLabels")))
#define				n_endDrag					(Name::intern(_T("endDrag")))
#define				n_ensureVisible				(Name::intern(_T("ensureVisible")))
#define				n_extraData					(Name::intern(_T("extraData")))

#define				n_flat2D					(Name::intern(_T("flat2D")))

#define				n_hasChild					(Name::intern(_T("hasChild")))
#define				n_hwnd						(Name::intern(_T("hwnd")))

#define				n_iconImage					(Name::intern(_T("iconImage")))
#define				n_iconSelectedImage			(Name::intern(_T("iconSelectedImage")))
#define				n_iconState					(Name::intern(_T("iconState")))
#define				n_isCollapsed				(Name::intern(_T("isCollapsed")))
#define				n_itemByData				(Name::intern(_T("itemByData")))
#define				n_imageList					(Name::intern(_T("imageList")))

#define				n_keydown					(Name::intern(_T("keydown")))

#define				n_mouseover					(Name::intern(_T("mouseover")))
#define				n_multiSelection			(Name::intern(_T("multiSelection")))

#define				n_overlayList				(Name::intern(_T("overlayList")))

#define				n_recursive					(Name::intern(_T("recursive")))
#define				n_remove					(Name::intern(_T("remove")))
#define				n_repaint					(Name::intern(_T("repaint")))
#define				n_rootNodes					(Name::intern(_T("rootNodes")))

#define				n_selectedColor				(Name::intern(_T("selectedColor")))
#define				n_selectedItems				(Name::intern(_T("selectedItems")))
#define				n_setCollapsed				(Name::intern(_T("setCollapsed")))
#define				n_setCompareMethod			(Name::intern(_T("setCompareMethod")))
#define				n_setParent					(Name::intern(_T("setParent")))
#define				n_showTree					(Name::intern(_T("showTree")))
#define				n_siblings					(Name::intern(_T("siblings")))
#define				n_sort						(Name::intern(_T("sort")))
#define				n_sortBy					(Name::intern(_T("sortBy")))
#define				n_sortingEnabled			(Name::intern(_T("sortingEnabled")))
#define				n_sortingInverted			(Name::intern(_T("sortingInverted")))
#define				n_syncSelection				(Name::intern(_T("syncSelection")))

#define				n_toggleCollapsed			(Name::intern(_T("toggleCollapsed")))
#define				n_toolTip					(Name::intern(_T("toolTip")))

#define				n_useSelectedColor			(Name::intern(_T("useSelectedColor")))

// event names
#define				n_beginEditLabel			(Name::intern(_T("beginEditLabel")))
#define				n_buttonPressed				(Name::intern(_T("buttonPressed")))
#define				n_dblclick					(Name::intern(_T("dblclick")))
#define				n_endEditLabel				(Name::intern(_T("endEditLabel")))
#define				n_focusChanged				(Name::intern(_T("focusChanged")))
#define				n_iconPressed				(Name::intern(_T("iconPressed")))
#define				n_indentPressed				(Name::intern(_T("indentPressed")))
#define				n_mousedown					(Name::intern(_T("mousedown")))
#define				n_mouseup					(Name::intern(_T("mouseup")))
#define				n_preBeginEditLabel			(Name::intern(_T("preBeginEditLabel")))
#define				n_rclick					(Name::intern(_T("rclick")))
#define				n_rdblclick					(Name::intern(_T("rdblclick")))
#define				n_rightAreaPressed			(Name::intern(_T("rightAreaPressed")))
#define				n_stateIconPressed			(Name::intern(_T("stateIconPressed")))

// Keep for compatibility
#define				n_collapse					(Name::intern(_T("collapse")))
#define				n_expand					(Name::intern(_T("expand")))
#define				n_toggleExpand				(Name::intern(_T("toggleExpand")))

//----------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------

static int CALLBACK		CompareItems( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort ) {
	TreeViewPlus* ctrl			= (TreeViewPlus*) lParamSort;
	TreeViewItemPlus* item01	= (TreeViewItemPlus*) lParam1;
	TreeViewItemPlus* item02	= (TreeViewItemPlus*) lParam2;

	int result = 0;
	if ( ctrl->compareMethod() && is_function( ctrl->compareMethod() ) ) {
		Value** args;
		value_local_array( args, 2 );
		args[0] = item01;
		args[1] = item02;
		
		init_thread_locals();
		push_alloc_frame();
		one_value_local( arg );
		
		vl.arg = ctrl->compareMethod()->apply( args, 2 );
		result = vl.arg->to_int();
		pop_value_locals();
		pop_alloc_frame();
		pop_value_local_array( args );
	}
	else
		result = ( item01->name() > item02->name() ) ? 1 : -1;

	if ( ctrl->isSortingInverted() )
		result = -result;

	return result;
}

//----------------------------------------------------------------------------------------------------------------
//											TREEVIEWPLUS
//----------------------------------------------------------------------------------------------------------------
visible_class_instance( TreeViewPlus, S_TREEVIEW );

// Constructor
TreeViewPlus::TreeViewPlus( Value* name, Value* caption, Value** keyparms, int keyparm_count ) : AppliedControl( name, caption, keyparms, keyparm_count ) {
	this->tag					= class_tag( TreeViewPlus );

	// Initialize members
	this->_amAutoCollapsible	= TRUE;
	this->_amDragging			= FALSE;
	this->_compareMethod		= NULL;
	this->_containerWindow		= NULL;
	this->_data					= &undefined;
	this->_dragEnabled			= TRUE;
	this->_editWindow			= NULL;
	this->_invertSort			= FALSE;
	this->_itemEditing			= &undefined;
	this->_multiselection		= FALSE;
	this->_images				= NULL;	
	
	// Build Root
	this->_root					= new TreeViewItemPlus( this );
	this->_root->setControl( this );
	this->_root->setName( _T( "root" ) );

	this->_sortingEnabled		= FALSE;
	this->_syncSelection		= FALSE;
	this->_treeWindow			= NULL;
	this->_useCustomToolTips	= FALSE;
	this->_useSelectedColor		= FALSE;
}
TreeViewPlus::~TreeViewPlus() {
	if ( this->_data )			DeleteObject( this->_data );
	if ( this->_itemEditing )	DeleteObject( this->_itemEditing );
	if ( this->_compareMethod )	DeleteObject( this->_compareMethod );
	if ( this->_root )			DeleteObject( this->_root );
}

//----------------------------------------------------------------------------------------------------------------
//										TREEVIEWPLUS: Static Methods
//----------------------------------------------------------------------------------------------------------------

LRESULT CALLBACK	TreeViewPlus::WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam ) {
	LRESULT result = 0;

	// Get back at our stored class pointer
	TreeViewPlus* control = DLGetWindowLongPtr<TreeViewPlus*>(hWnd, GWLP_USERDATA);

	CharStream* out				= thread_local( current_stdout );
	if ( control || msg == WM_CREATE ) {
		switch ( msg ) { 
			case WM_CREATE: {
				LPCREATESTRUCT lpcs = (LPCREATESTRUCT) lParam;
				control				= (TreeViewPlus*) lpcs->lpCreateParams;
				SetWindowLongPtr( hWnd, GWLP_USERDATA, (LONG_PTR) control );
				SetFocus( control->window() );
				break;
			}
			case WM_SIZE: {
				SetWindowPos( control->window(), NULL, 0, 0, LOWORD(lParam), HIWORD(lParam), SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOMOVE);
				break;
			}
			case WM_MOUSEMOVE: {
				control->run_event_handler( control->parent_rollout, n_mousemove, NULL, 0 );
				if ( control->isDragging() ) {
					control->setPoints( MAKEPOINTS(lParam) );
					ImageList_DragMove( control->points().x - 5, control->points().y - 7 );
					ImageList_DragShowNolock(FALSE);
					control->_hitTest.pt.x		= control->points().x;
					control->_hitTest.pt.y		= control->points().y;
					if ( control->_hitTarget = (HTREEITEM)SendMessage( control->window(), TVM_HITTEST,NULL,(LPARAM)&(control->_hitTest)))
						SendMessage( control->window(), TVM_SELECTITEM, TVGN_DROPHILITE,(LPARAM) control->_hitTarget );
					ImageList_DragShowNolock( TRUE );
				}
				break;
			}
			case WM_NOTIFY: {
				result = control->handleNotify( hWnd, (int) wParam, (LPNMHDR) lParam );
				break;
			}
			case WM_LBUTTONDOWN: {
				ReleaseCapture();
				break;
			}
			case WM_LBUTTONUP: {
				if ( control->isDragging() ) {
					one_value_local( arg );
					init_thread_locals();
					push_alloc_frame();
					vl.arg = &undefined;

					ImageList_DragLeave( control->window() );
					ImageList_EndDrag();
					//HTREEITEM hItem = (HTREEITEM)SendMessage( control->window(), TVM_GETNEXTITEM, TVGN_DROPHILITE, 0 );
/*
					if ( hItem ) {
						SendMessage( control->window(), TVM_SELECTITEM, TVGN_CARET, (LPARAM) hItem );
						SendMessage( control->window(), TVM_SELECTITEM, TVGN_DROPHILITE, 0 );
*/
					ReleaseCapture();

					control->_amDragging = FALSE;
					TreeViewItemPlus* item = (TreeViewItemPlus*) control->hitTest( control->window(), TVHT_ONITEM );
					if ( is_treeviewitemplus( item ) )
						vl.arg = item;
					//}
					control->run_event_handler( control->parent_rollout, n_endDrag, &vl.arg, 1 );
					pop_value_locals();
					pop_alloc_frame();
				}
				break;
			}
			default: {
				 result = DefWindowProc( hWnd, msg, wParam, lParam );
			}
		}
	}
	else	result = DefWindowProc( hWnd, msg, wParam, lParam );
	return result;
}


//----------------------------------------------------------------------------------------------------------------
//										TREEVIEWPLUS: Notify Methods
//----------------------------------------------------------------------------------------------------------------

long				TreeViewPlus::handleBeginDrag( HWND hWndTreeView, LPNMTREEVIEW pNMTV ) {
	long result = -1;
	if ( pNMTV ) {
		HIMAGELIST hImg;
		LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW) pNMTV;
		hImg = TreeView_CreateDragImage( hWndTreeView, lpnmtv->itemNew.hItem );
		ImageList_BeginDrag( hImg, 0, 0, 0 );
		ImageList_DragEnter( hWndTreeView, lpnmtv->ptDrag.x, lpnmtv->ptDrag.y );

		push_alloc_frame();
		init_thread_locals();
		one_value_local( arg );

		TreeViewItemPlus* pItemData = (TreeViewItemPlus*) this->data(lpnmtv->itemNew.hItem);
		if ( pItemData ) {
			vl.arg = (Value*) pItemData;
			run_event_handler( this->parent_rollout, n_beginDrag, &vl.arg, 1 );
			this->invalidate();
		}

		pop_value_locals();
		pop_alloc_frame();

		result	= 0;
	}
	return result;
}
long				TreeViewPlus::handleBeginEditLabel( HWND hWndTreeView, LPNMTVCUSTOMDRAW pNMTVCD ) {
	long result = 0;
	if ( this->isLabelEditingEnabled() ) {
		HTREEITEM target	= TreeView_GetSelection( this->window() );
		this->_itemEditing	= this->data(target);
		this->_editWindow	= TreeView_GetEditControl( this->window() );

		if ( this->_itemEditing != &undefined ) {
			TreeViewItemPlus* item = (TreeViewItemPlus*) this->_itemEditing;
			if ( item->isEnabled() ) {
				init_thread_locals();
				push_alloc_frame();
				one_value_local(arg);

				vl.arg				= this->_itemEditing;
				Value* isEditable	= &undefined;
				Value* handler		= get_event_handler( n_beginEditLabel );

				if ( handler != &undefined )
					isEditable		= handler->apply( &vl.arg, 1 );

				if ( isEditable != &false_value ) {
					if ( isEditable->is_kind_of( class_tag(String) ) )
						SetWindowText( this->_editWindow, ((TreeViewItemPlus*) this->_itemEditing)->name() );

					pop_value_locals();
					pop_alloc_frame();
					this->invalidate();
				}
				else {
					this->_editWindow	= NULL;
					this->_itemEditing	= &undefined;
					pop_value_locals();
					pop_alloc_frame();
				}
			}
			else {
				this->_editWindow = NULL;
				this->_itemEditing = &undefined;
				TreeView_EndEditLabelNow( this->window(), TRUE );
			}
		}
		else {
			this->_editWindow	= NULL;
			this->_itemEditing	= &undefined;
		}
	}
	return result;
}
long				TreeViewPlus::handleCustomDraw( HWND hWndTreeView, LPNMTVCUSTOMDRAW pNMTVCD ) {
	long result = -1;
	if ( pNMTVCD ) {
		switch ( pNMTVCD->nmcd.dwDrawStage ) {
			case CDDS_PREPAINT: {
				result = (CDRF_NOTIFYSUBITEMDRAW);
				break;
			}
			case CDDS_ITEMPREPAINT: {
				HDC hdc = pNMTVCD->nmcd.hdc;

				HTREEITEM hItem = (HTREEITEM) pNMTVCD->nmcd.dwItemSpec;
				TVITEM tvi		= { 0 };
				tvi.mask		= TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT | TVIF_STATE;
				tvi.hItem		= hItem;
				TreeView_GetItem( hWndTreeView, &tvi );

				if ( tvi.lParam ) {
					TreeViewItemPlus* item	= (TreeViewItemPlus*) tvi.lParam;

					pNMTVCD->clrText	= item->color();

					if ( item ) {
						COLORREF itemSelected		= ( this->isUsingSelectedColor() ) ? this->selectedColor() : LightenColour( item->color(), 0.75f );
						pNMTVCD->clrTextBk			= ( item->isSelected() ) ? itemSelected : this->bkColor();

						if ( item->isSelected() ) {
							RECT rcTarget;
							TreeView_GetItemRect( hWndTreeView, hItem, &rcTarget, 0 );

							RECT rcTargetText;
							TreeView_GetItemRect( hWndTreeView, hItem, &rcTargetText, 1 );

							if ( item->iconImage() > 0 ) rcTarget.left = rcTarget.left + ICON_SIZE;

							HBRUSH bkBrush = CreateSolidBrush( pNMTVCD->clrTextBk );
							SelectObject( hdc, bkBrush );
							FillRect( hdc, &rcTarget, bkBrush );
							DeleteObject( bkBrush );
						}
					}
				}
				result = CDRF_NOTIFYPOSTPAINT | CDRF_NEWFONT;
				break;
			}
			case CDDS_ITEMPOSTPAINT: {
				result = CDRF_DODEFAULT;
				break;
			}
			default: break;
		}
	}
	return result;
}
long				TreeViewPlus::handleCustomTooltip( NMTVGETINFOTIP* pTVTipInfo ) {
	LPARAM itemData = (DWORD) pTVTipInfo->lParam;
	TreeViewItemPlus* pItemData = (TreeViewItemPlus*) itemData;
	// TODO: Fix Custom Tooltip
	// if ( pItemData ) strcpy( pTVTipInfo->pszText, (LPCTSTR) pItemData->_toolTip );
	return 0;
}
long				TreeViewPlus::handleEndEditLabel( HWND hWndTreeView, LPNMTVCUSTOMDRAW pNMTVCD ) {
	TreeViewItemPlus* item = (TreeViewItemPlus*) this->_itemEditing;
	long result = 0;
	if ( this->_itemEditing != &undefined ) {
		char itemText[256] = "";
		GetWindowText( this->_editWindow, itemText, sizeof( itemText ) );

		Value* isChanged	= &true_value;

		push_alloc_frame();
		Value** args;
		value_local_array( args, 2 );
		args[0] = this->_itemEditing;
		args[1] = new String( itemText );
		
		Value* handler		= get_event_handler( n_endEditLabel );
		if ( handler != &undefined )
			isChanged		= handler->apply( args, 2 );

		pop_value_local_array( args );
		pop_alloc_frame();

		this->invalidate();

		this->_itemEditing = &undefined;
		result = ((isChanged == &true_value) ? 1 : 0 );
	}
	return result;
}
long				TreeViewPlus::handleNotify( HWND hWndDlg, int nIDCtrl, LPNMHDR pNMHDR ) {
	long	result	= 0;
	if ( pNMHDR->code == NM_LDOWN )			result = this->LButtonDown();
	else if ( pNMHDR->code == NM_CLICK )	result = this->LButtonClick();
	else if ( pNMHDR->code == NM_DBLCLK )	result = this->LButtonDblClk();
	else if ( pNMHDR->code == NM_RCLICK )	result = this->RButtonClick();
	else if ( pNMHDR->code == NM_SETFOCUS )	result = this->FocusChanged();
	else if ( pNMHDR->code == TVN_KEYDOWN )	result = this->KeyDown( ((LPNMTVKEYDOWN) pNMHDR)->wVKey );
	else if ( pNMHDR->code == NM_CUSTOMDRAW ) {
		LPNMTVCUSTOMDRAW pNMTVCD	= (LPNMTVCUSTOMDRAW) pNMHDR;
		HWND hWndTreeView			= pNMHDR->hwndFrom;
		result						= this->handleCustomDraw( hWndTreeView, pNMTVCD );
	}
	else if ( pNMHDR->code == TVN_BEGINLABELEDIT ) {
		LPNMTVCUSTOMDRAW pNMTVCD	= (LPNMTVCUSTOMDRAW) pNMHDR;
		HWND hWndTreeView			= pNMHDR->hwndFrom;
		result						= this->handleBeginEditLabel( hWndTreeView, pNMTVCD );
	}
	else if ( pNMHDR->code == TVN_ENDLABELEDIT ) {
		LPNMTVCUSTOMDRAW pNMTVCD	= (LPNMTVCUSTOMDRAW) pNMHDR;
		HWND hWndTreeView			= pNMHDR->hwndFrom;
		result						= this->handleEndEditLabel( hWndTreeView, pNMTVCD );
	}
	else if ( pNMHDR->code == TVN_BEGINDRAG ) {
		if ( this->isDragEnabled() ) {
			LPNMTREEVIEW pNMTV		= (LPNMTREEVIEW) pNMHDR;
			HWND hWndTreeView		= pNMHDR->hwndFrom;
			result					= this->handleBeginDrag( hWndTreeView, pNMTV );
			SetCapture( hWndDlg );
			this->_amDragging		= TRUE;
		}
	}
	else if ( pNMHDR->code == TVN_GETINFOTIP ) {
		NMTVGETINFOTIP* pTVTipInfo	= (NMTVGETINFOTIP*) pNMHDR;
		result						= this->handleCustomTooltip( pTVTipInfo );
	}
	return result;
}

//----------------------------------------------------------------------------------------------------------------
//										TREEVIEWPLUS: Member Methods
//----------------------------------------------------------------------------------------------------------------
BOOL				TreeViewPlus::addChild( TreeViewItemPlus* child )		{ return ( this->root() ) ? this->root()->addChild( child ) : FALSE; }
Value*				TreeViewPlus::addItem( Value* item, BOOL recursive )	{ return ( this->root() ) ? this->root()->addItem( item, recursive ) : &undefined; }
COLORREF			TreeViewPlus::bkColor()									{ return this->_bkColor; }
HWND				TreeViewPlus::containerWindow()							{ return this->_containerWindow; }
Value*				TreeViewPlus::compareMethod()							{ return this->_compareMethod; }
Value*				TreeViewPlus::data()									{ return this->_data; }
Value*				TreeViewPlus::data( HTREEITEM hItem )			{
	TVITEM tvItem;
	tvItem.hItem	= hItem;
	tvItem.mask		= TVIF_PARAM | TVIF_HANDLE;
	TreeView_GetItem( this->window(), &tvItem );

	TreeViewItemPlus* pItemData = (TreeViewItemPlus*) tvItem.lParam;
	if ( pItemData )
		return pItemData;
	return &undefined;
}
void				TreeViewPlus::deleteAllItems()							{ this->_root->deleteChildren(); }
void				TreeViewPlus::deselectAll()							{ 
	this->root()->setSelected( FALSE, TRUE, FALSE );
	this->repaint();
}
HWND				TreeViewPlus::editWindow()								{ return this->_editWindow; }
void				TreeViewPlus::endEditLabel()							{ TreeView_EndEditLabelNow( this->window(), TRUE ); }
int					TreeViewPlus::height()									{ return (int)this->size().y; }
Value*				TreeViewPlus::hitTest( HWND hWndTreeView, UINT flags ) {
	TVHITTESTINFO	hitInfo;
	TVITEM			tvItem;
	HTREEITEM		hItem;

	hitInfo.flags = flags;
	GetCursorPos( &hitInfo.pt );

	ScreenToClient( hWndTreeView, &hitInfo.pt );
	if ( hItem = TreeView_HitTest( hWndTreeView, &hitInfo ) ) {
		tvItem.hItem		= hItem;
		tvItem.mask			= TVIF_PARAM | TVIF_HANDLE;
		if ( (hitInfo.flags & flags) && TreeView_GetItem( hWndTreeView, &tvItem ) ) {
			if ( tvItem.lParam ) {
				return (Value*) tvItem.lParam;
			}
		}
	}
	return &undefined;
}
void				TreeViewPlus::invalidate() {
	if ( this->containerWindow() ) {
		RECT rect;
		GetClientRect( this->containerWindow(), &rect );
		MapWindowPoints( this->containerWindow(), this->parent_rollout->page, (POINT*)&rect, 2 );
		InvalidateRect( this->parent_rollout->page, &rect, TRUE );
		InvalidateRect( this->containerWindow(), NULL, TRUE );
	}
}
BOOL				TreeViewPlus::isAutoCollapsible()						{ return this->_amAutoCollapsible; }
BOOL				TreeViewPlus::isDragging()								{ return ( this->_dragEnabled && this->_amDragging ); }
BOOL				TreeViewPlus::isDragEnabled()							{ return this->_dragEnabled; }
BOOL				TreeViewPlus::isLabelEditingEnabled()					{ return this->_labelEditingEnabled; }
BOOL				TreeViewPlus::isMultiSelection()						{ return this->_multiselection; }
BOOL				TreeViewPlus::isSelectionSynced()						{ return this->_syncSelection; }
BOOL				TreeViewPlus::isSortingEnabled()						{ return this->_sortingEnabled; }
BOOL				TreeViewPlus::isSortingInverted()						{ return this->_invertSort; }
BOOL				TreeViewPlus::isUsingCustomToolTips()					{ return this->_useCustomToolTips; }
BOOL				TreeViewPlus::isUsingSelectedColor()					{ return this->_useSelectedColor; }
Value*				TreeViewPlus::itemByData( Value* data )					{ return this->root()->itemByData( data, TRUE ); }
Array*				TreeViewPlus::itemsByNode( Value* node )				{ return this->root()->itemsByNode( node, TRUE ); }
POINTS				TreeViewPlus::points()									{ return this->_points; }
BOOL				TreeViewPlus::repaint()								{ 
	BOOL success = FALSE;
	if ( this->containerWindow() ) {
		this->InvalidateUI();
		this->invalidate();
		UpdateWindow( this->containerWindow() );
		success = TRUE;
	}
	return success;
}
TreeViewItemPlus*	TreeViewPlus::root()									{ return this->_root; }
COLORREF			TreeViewPlus::selectedColor()							{ return this->_selectedColor; }
Array*				TreeViewPlus::selectedItems()						{
	//init_thread_locals();
	//push_alloc_frame();
	one_typed_value_local(Array* out);
	vl.out				= new Array(0);
	Array* children		= this->root()->children(TRUE);
	for (int i = 0; i < children->size; i++ ) {
		if ( ((TreeViewItemPlus*) children->data[i])->isSelected() )
			vl.out->append( children->data[i] );
	}

	//pop_alloc_frame();
	return_value( vl.out );
}
BOOL				TreeViewPlus::setBkColor( COLORREF newBkColor ) {
	this->_bkColor = newBkColor;
	if ( this->window() )
		TreeView_SetBkColor( this->window(), this->_bkColor );
	return TRUE;
}
void				TreeViewPlus::setAutoCollapsible( BOOL state )			{ this->_amAutoCollapsible = state; }
void				TreeViewPlus::setCompareMethod( Value* method )		{ 
	this->_compareMethod = method; 
	if ( this->isSortingEnabled() )
		this->sort();
}
void				TreeViewPlus::setData( Value* data )					{ this->_data = data; }
void				TreeViewPlus::setDragEnabled( BOOL dragEnabled )		{ this->_dragEnabled = dragEnabled; }
void				TreeViewPlus::setLabelEditingEnabled( BOOL state )		{ this->_labelEditingEnabled; }
void				TreeViewPlus::setHeight( int height ) {
	Point2 newSize;
	newSize.x = this->width();
	newSize.y = height;
	this->setSize( newSize );
}
void				TreeViewPlus::setItems( Array* itemList ) {
}
void				TreeViewPlus::setMultiSelection( BOOL state )			{ this->_multiselection = state; }
void				TreeViewPlus::setPoints( POINTS newPoints )				{ this->_points = newPoints; }
void				TreeViewPlus::setSelectedColor( COLORREF newSelColor ) {
	this->_selectedColor = newSelColor;
	if ( this->isUsingSelectedColor() ) this->repaint();
}
void				TreeViewPlus::setSelectionSynced( BOOL syncSel )		{ this->_syncSelection = syncSel; }
void				TreeViewPlus::setSortingEnabled( BOOL state )		{
	this->_sortingEnabled = state;
	if ( state )
		this->_root->sort();
}
void				TreeViewPlus::setSortingInverted( BOOL state )		{
	this->_invertSort = state;
	if ( this->isSortingEnabled() )
		this->sort();
}
void				TreeViewPlus::setSize( Point2 size ) {
	if ( this->parent_rollout && this->parent_rollout->page ) {
		HWND hwnd = GetDlgItem( this->parent_rollout->page, this->control_ID );
		RECT rect;
		GetWindowRect( hwnd, &rect );
		MapWindowPoints( NULL, this->parent_rollout->page, (LPPOINT)&rect, 2 );
		SetWindowPos( hwnd, NULL, rect.left, rect.top, size.x, size.y, SWP_NOZORDER );
	}
}
void				TreeViewPlus::setUseCustomToolTips( BOOL state )		{ this->_useCustomToolTips; }
void				TreeViewPlus::setUseSelectedColor( BOOL useSelColor ) {
	this->_useSelectedColor = useSelColor;
	this->repaint();
}
void				TreeViewPlus::setWidth( int width ) {
	Point2 newSize;
	newSize.x = width;
	newSize.y = this->height();
	this->setSize( newSize );
}
void				TreeViewPlus::sort()									{ this->root()->sort(TRUE); }
Point2				TreeViewPlus::size()		{
	Point2 outSize;
	outSize.x = 0;
	outSize.y = 0;
	if ( this->parent_rollout && this->parent_rollout->page ) {
		HWND hWnd = GetDlgItem( this->parent_rollout->page, this->control_ID );
		RECT rect;
		GetWindowRect( hWnd, &rect );
		MapWindowPoints( NULL, this->parent_rollout->page, (LPPOINT)&rect, 2 );
		outSize.x = rect.right - rect.left;
		outSize.y = rect.bottom - rect.top;
	}
	return outSize;
}
int					TreeViewPlus::width()									{ return (int)this->size().x; }
HWND				TreeViewPlus::window()									{ return this->_treeWindow; }

//----------------------------------------------------------------------------------------------------------------
//									TREEVIEWPLUS: Event Methods
//----------------------------------------------------------------------------------------------------------------
LRESULT	 			TreeViewPlus::FocusChanged() {
	init_thread_locals();
	push_alloc_frame();
	one_value_local( arg );
	vl.arg = &undefined;

	TreeViewItemPlus* item = (TreeViewItemPlus*) this->hitTest( this->window(), TVHT_ONITEM );
	if ( is_treeviewitemplus( item ) && item->isEnabled() )
		vl.arg = item;

	run_event_handler( this->parent_rollout, n_focusChanged, &vl.arg, 1 );

	pop_value_locals();
	pop_alloc_frame();

	return 0;
}
LRESULT				TreeViewPlus::KeyDown( WORD key ) {
	init_thread_locals();
	push_alloc_frame();
	one_value_local( arg );

	if ( key < 16 || 18 < key ) {				// Ignore <Shift> <Ctrl> <Alt>, use keyboard.<btn>Pressed to see if those are down
		vl.arg = Integer::intern( key );
		run_event_handler( this->parent_rollout, n_keydown, &vl.arg, 1 );
	}

	pop_value_locals();
	pop_alloc_frame();

	return 0;
}
LRESULT				TreeViewPlus::LButtonDblClk() {
	init_thread_locals();
	push_alloc_frame();
	one_value_local(arg);
	
	vl.arg = &undefined;

	TreeViewItemPlus* item		= (TreeViewItemPlus*) this->hitTest( this->window(), TVHT_ONITEM );

	LONG_PTR style = GetWindowLongPtr( this->window(), GWL_STYLE );
	if (( style & TVS_EDITLABELS ) && this->isLabelEditingEnabled() && is_treeviewitemplus( item ) )
		TreeView_EditLabel( this->window(), item->treeItem() );

	if ( is_treeviewitemplus( item ) && item->isEnabled() ) {
		vl.arg = item;
		if ( !this->isAutoCollapsible() )
			item->toggleCollapsed();
	}

	run_event_handler( this->parent_rollout, n_dblclick, &vl.arg, 1 );
		
	pop_value_locals();
	pop_alloc_frame();

	return 0;
}
LRESULT				TreeViewPlus::LButtonClick() {
	init_thread_locals();
	push_alloc_frame();
	one_value_local( arg );

	TreeViewItemPlus* item;

	// Check for iconPressed Event
	item = (TreeViewItemPlus*) this->hitTest( this->window(), TVHT_ONITEMICON );
	if ( is_treeviewitemplus( item ) && item->isEnabled() ) {
		vl.arg = item;
		run_event_handler( this->parent_rollout, n_iconPressed, &vl.arg, 1 );
		this->invalidate();
	}
	
	// Check for buttonPressed Event
	item = (TreeViewItemPlus*) this->hitTest( this->window(), TVHT_ONITEMBUTTON );
	if ( is_treeviewitemplus( item ) && item->isEnabled() ) {
		vl.arg = item;
		run_event_handler( this->parent_rollout, n_buttonPressed, &vl.arg, 1 );
		this->invalidate();
	}

	// Check for indentPressed Event
	item = (TreeViewItemPlus*) this->hitTest( this->window(), TVHT_ONITEMINDENT );
	if ( is_treeviewitemplus( item ) && item->isEnabled() ) {
		vl.arg = item;
		run_event_handler( this->parent_rollout, n_indentPressed, &vl.arg, 1 );
		this->invalidate();
	}

	// Check for stateIconPressed Event
	item = (TreeViewItemPlus*) this->hitTest( this->window(), TVHT_ONITEMSTATEICON );
	if ( is_treeviewitemplus( item ) && item->isEnabled() ) {
		vl.arg = item;
		run_event_handler( this->parent_rollout, n_stateIconPressed, &vl.arg, 1 );
		this->invalidate();
	}

	// Check for rightAreaPressed Event
	item = (TreeViewItemPlus*) this->hitTest( this->window(), TVHT_ONITEMRIGHT );
	if ( is_treeviewitemplus( item ) && item->isEnabled() ) {
		vl.arg = item;
		run_event_handler( this->parent_rollout, n_rightAreaPressed, &vl.arg, 1 );
		this->invalidate();
	}

	// Check for mouseUp Event
	item = (TreeViewItemPlus*) this->hitTest( this->window(), TVHT_ONITEM );
	if ( is_treeviewitemplus( item ) && item->isEnabled() ) {
		vl.arg = item;

		if ( !this->isMultiSelection() ) {
			this->deselectAll();
			item->setSelected( true );
		}
		
		run_event_handler( this->parent_rollout, n_mouseup, &vl.arg, 1);
		this->invalidate();
	}

	pop_value_locals();
	pop_alloc_frame();
	
	return 0;
}
LRESULT				TreeViewPlus::LButtonDown() {
	init_thread_locals();
	push_alloc_frame();
	one_value_local( arg );

	TreeViewItemPlus* item = (TreeViewItemPlus*) this->hitTest( this->window(), TVHT_ONITEM );
	if ( is_treeviewitemplus( item ) && item->isEnabled() ) {
		vl.arg = item;
		run_event_handler( this->parent_rollout, n_mousedown, &vl.arg, 1 );
		this->invalidate();
	}
	pop_value_locals();
	pop_alloc_frame();
	return 0;
}
LRESULT				TreeViewPlus::RButtonClick() {
	init_thread_locals();
	push_alloc_frame();
	one_value_local( arg );
	vl.arg = &undefined;

	TreeViewItemPlus* item = (TreeViewItemPlus*) this->hitTest( this->window(), TVHT_ONITEM );
	if ( is_treeviewitemplus( item ) && item->isEnabled() )
		vl.arg = item;

	run_event_handler( this->parent_rollout, n_rclick, &vl.arg, 1 );

	pop_value_locals();
	pop_alloc_frame();

	return 0;
}

//----------------------------------------------------------------------------------------------------------------
//									TREEVIEWPLUS: MAXScript Methods
//----------------------------------------------------------------------------------------------------------------
void				TreeViewPlus::collect() { delete this; }
void				TreeViewPlus::gc_trace() {
	RolloutControl::gc_trace();

	if ( this->_data			&& this->_data->is_not_marked() )			this->_data->gc_trace();
	if ( this->_itemEditing		&& this->_itemEditing->is_not_marked() )	this->_itemEditing->gc_trace();
	if ( this->_compareMethod	&& this->_compareMethod->is_not_marked() )	this->_compareMethod->gc_trace();
	if ( this->_root			&& this->_root->is_not_marked() )			this->_root->gc_trace();
}
void				TreeViewPlus::add_control( Rollout* ro, HWND parent, HINSTANCE hInstance, int& current_y ) {
	this->caption			= this->caption->eval();

	TCHAR* text				= this->caption->eval()->to_string();
	this->control_ID		= this->next_id();
	this->parent_rollout	= ro;

	//-------------------------------------------------------------------------------

	// Load Build Properties
	Value* param;

	// showTree Param
	BOOL showTree			= TRUE;
	param					= control_param( showTree );
	if ( param != &unsupplied )	showTree	= param->to_bool();

	// multiSelection Param
	param					= control_param( multiSelection );
	if ( param != &unsupplied )	this->setMultiSelection( param->to_bool() );

	// flat2d Param
	long flat2D				= WS_EX_CLIENTEDGE;
	param					= control_param( flat2D );
	if ( param == &true_value )	flat2D = 0;

	// bkColor Param
	param					= control_param( bkColor );
	if ( param != &unsupplied )	this->setBkColor( param->to_colorref() );
	else						this->setBkColor( GetSysColor( COLOR_BTNFACE ) );

	// selectedColor Param
	param					= control_param( selectedColor );
	if ( param != &unsupplied )	this->setSelectedColor( param->to_colorref() );
	else						this->setSelectedColor( LightenColour( this->bkColor(), 0.75f ) );

	// useSelectedColor Param
	param					= control_param( useSelectedColor );
	if ( param != &unsupplied )	this->setUseSelectedColor( param->to_bool() );

	// editLabels Param
	param					= control_param( editLabels );
	if ( param != &unsupplied )	this->setLabelEditingEnabled( param->to_bool() );

	// customToolTips Param
	param					= control_param( customToolTips );
	if ( param != &unsupplied )	this->setUseCustomToolTips( param->to_bool() );

	//-------------------------------------------------------------------------------

	// Build Widget
	layout_data pos;
	setup_layout( ro, &pos, current_y );
	process_layout_params( ro, &pos, current_y );

	// Container Window
	this->_containerWindow	= CreateWindow(
		TREECTRL_WINDOWCLASS,
		_T(""),
		WS_VISIBLE | WS_CHILD,
		pos.left, pos.top, pos.width, pos.height,
		parent, (HMENU)this->control_ID, g_hInst, this );

	// Tree Window
	long									treeProperties		= WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_BORDER;
	if ( showTree )							treeProperties		|= TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT;
	if ( this->isLabelEditingEnabled() )	treeProperties		|= TVS_EDITLABELS;
	if ( this->isUsingCustomToolTips() )	treeProperties		|= TVS_INFOTIP;

	this->_treeWindow		= CreateWindowEx( flat2D, WC_TREEVIEW, _T( "" ),
		treeProperties,
		0, 0, pos.width, pos.height,
		this->_containerWindow, (HMENU) this->control_ID, g_hInst, NULL );

	TreeView_SetItemHeight( this->_treeWindow, ICON_SIZE );

	// Set Properties
	if ( this->isUsingCustomToolTips() ) {
		HWND hToolTip = TreeView_GetToolTips( this->window() );
		static TOOLINFO info;
		memset( &info, 0, sizeof(TOOLINFO) );
		SendMessage( hToolTip, TTM_GETTOOLINFO, 0, (LPARAM) &info );
		info.uFlags = TTF_ABSOLUTE | TTF_TRACK;
		SendMessage( hToolTip, TTM_SETTOOLINFO, 0, (LPARAM) &info );
	}
	this->_images = ImageList_Create( ICON_SIZE, ICON_SIZE, ILC_COLOR24 | ILC_MASK, 0, 10 );
	TreeView_SetImageList( this->window(), this->_images, TVSIL_NORMAL );
	TreeView_SetImageList( this->window(), this->_images, TVSIL_STATE );
	TreeView_DeleteAllItems( this->window() );
	TreeView_SetBkColor( this->window(), this->bkColor() );
	TreeView_SetTextColor( this->window(), GetSysColor( COLOR_BTNTEXT ) );

	// Finish Widget
	SendDlgItemMessage( this->_treeWindow, this->control_ID, WM_SETFONT, (WPARAM)ro->font, 0L );
	SendDlgItemMessage( parent, this->control_ID, WM_SETFONT, (WPARAM)ro->font, 0L );
}
Value*				TreeViewPlus::applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc ) {
	one_value_local( result );

	if ( methodID == n_addItem ) {
		check_arg_count_with_keys( addItem, 1, count );
		BOOL recursive	= false;
		Value* param	= key_arg( recursive );
		if ( param != &unsupplied )
			recursive	= param->to_bool();
		return this->addItem( arg_list[0], recursive );
	}
	else if ( methodID == n_deleteAllItems ) {
		check_arg_count( deleteAllItems, 0, count );
		this->deleteAllItems();
	}
	else if ( methodID == n_deselectAll ) {
		check_arg_count( deselectAll, 0, count );
		this->deselectAll();
	}
	else if ( methodID == n_itemByData ) {
		check_arg_count( itemByData, 1, count );
		return this->itemByData( arg_list[0] );
	}
	else if ( methodID == n_repaint ) {
		check_arg_count( repaint, 0, count );
		this->repaint();
	}
	else if ( methodID == n_setCompareMethod ) {
		check_arg_count( setCompareMethod, 1, count );
		if ( is_function( arg_list[0] ) ) {
			this->setCompareMethod( arg_list[0] );
			return &true_value;
		}
		return &false_value;
	}
	else if ( methodID == n_selectedItems ) {
		check_arg_count( selectedItems, 0, count );
		return this->selectedItems();
	}
	else if ( methodID == n_sort ) {
		check_arg_count_with_keys( sort, 0, count );
		Value* invert = key_arg_or_default( invert, &false_value );

		// Define the sort direction
		if ( invert == &true_value )
			this->setSortingInverted( TRUE );
		else
			this->setSortingInverted( FALSE );

		// Setting the inverted setting would auto-sort if sorting is enabled
		if ( !this->isSortingEnabled() )
			this->sort();
	}

	return &ok;
}
Value*				TreeViewPlus::get_property( Value** arg_list, int count ) {
	Value*		prop = arg_list[0]->eval();

	if		( prop == n_addItem )			{ return NEW_CONTROL_METHOD( addItem ); }
	else if ( prop == n_autoCollapse )		{ return ( this->isAutoCollapsible() ) ? &true_value : &false_value; }
	else if ( prop == n_bkColor )			{ return ColorValue::intern( AColor( this->bkColor() ) ); }
	else if ( prop == n_data )				{ return this->data(); }
	else if ( prop == n_deleteAllItems )	{ return NEW_CONTROL_METHOD( deleteAllItems ); }
	else if ( prop == n_deselectAll )		{ return NEW_CONTROL_METHOD( deselectAll ); }
	else if ( prop == n_dragEnabled )		{ return ( this->isDragEnabled() )				? &true_value : &false_value; }
	else if ( prop == n_hwnd )				{ return ( this->parent_rollout && this->parent_rollout->page ) ? IntegerPtr::intern( reinterpret_cast<INT64>( this->_treeWindow ) ) : &undefined; }
	else if ( prop == n_height )			{ return Integer::intern( this->height() ); }
	else if ( prop == n_editLabels )		{ return ( this->isLabelEditingEnabled() )		? &true_value : &false_value; }
	else if ( prop == n_itemByData )		{ return NEW_CONTROL_METHOD( itemByData ); }
	else if ( prop == n_items )				{ return ( this->root() ) ? this->root()->children() : new Array(0); }
	else if ( prop == n_multiSelection )	{ return ( this->isMultiSelection() )			? &true_value : &false_value; }
	else if ( prop == n_repaint )			{ return NEW_CONTROL_METHOD( repaint ); }
	else if ( prop == n_root )				{ return this->root(); }
	else if ( prop == n_selectedColor )		{ return ColorValue::intern( AColor( this->selectedColor() ) ); }
	else if ( prop == n_selectedItems )		{ return NEW_CONTROL_METHOD( selectedItems ); }
	else if ( prop == n_setCompareMethod )	{ return NEW_CONTROL_METHOD( setCompareMethod ); }
	else if ( prop == n_sort )				{ return NEW_CONTROL_METHOD( sort ); }
	else if ( prop == n_sortingEnabled )	{ return ( this->isSortingEnabled())			? &true_value : &false_value; }
	else if ( prop == n_sortingInverted )	{ return ( this->isSortingInverted() )			? &true_value : &false_value; }
	else if ( prop == n_syncSelection )		{ return ( this->isSelectionSynced() )			? &true_value : &false_value; }
	else if ( prop == n_useSelectedColor )	{ return ( this->isUsingSelectedColor() )		? &true_value : &false_value; }
	else if ( prop == n_width )				{ return Integer::intern( this->width() );}
	else									{ return RolloutControl::get_property( arg_list, count ); }

	return &undefined;
}
BOOL				TreeViewPlus::handle_message( Rollout* ro, UINT message, WPARAM wParam, LPARAM lParam )	{ return FALSE; }
void				TreeViewPlus::set_enable() {
	if ( this->parent_rollout != NULL && this->parent_rollout->page != NULL ) {
		HWND ctrl = GetDlgItem( this->parent_rollout->page, this->control_ID );
		if ( ctrl ) {
			EnableWindow( ctrl, this->enabled );
			InvalidateRect( ctrl, NULL, TRUE );
		}
	}
}
Value*				TreeViewPlus::set_property( Value** arg_list, int count ) {
	Value* val	= arg_list[0]->eval();
	Value* prop	= arg_list[1]->eval();

	if		( prop == n_autoCollapse )				{ this->setAutoCollapsible( val->to_bool() ); }
	else if ( prop == n_bkColor )					{ this->setBkColor( val->to_colorref() ); }
	else if ( prop == n_text || prop == n_caption )	{ }
	else if ( prop == n_data )						{ this->setData( val ); }
	else if ( prop == n_dragEnabled )				{ this->setDragEnabled( val->to_bool() ); }
	else if ( prop == n_editLabels )				{
		this->_labelEditingEnabled = val->to_bool();
		LONG_PTR style = GetWindowLongPtr( this->window(), GWL_STYLE );
		if ( this->_labelEditingEnabled )
			style |= TVS_EDITLABELS;
		else
			style &= ~TVS_EDITLABELS;
		SetWindowLong( this->window(), GWL_STYLE, style );
	}
	else if ( prop == n_hwnd )						{ throw RuntimeError( _T( "You cannot change the hwnd property." ) ); }
	else if ( prop == n_height )					{ this->setHeight( val->to_int() ); }
	else if ( prop == n_imageList )					{
		TCHAR* iconFileName = val->to_string();
		LoadMAXFileIcon( iconFileName, this->_images, kBackground, FALSE );
		TreeView_SetImageList( this->window(), this->_images, TVSIL_NORMAL );
	}
	else if ( prop == n_items )						{ this->setItems( (Array*) val ); }
	else if ( prop == n_multiSelection )			{ this->setMultiSelection( val->to_bool() ); }
	else if ( prop == n_overlayList )				{ 
		Array* items = (Array*) val;
		for (int i = 0; i < items->size; i++)
			ImageList_SetOverlayImage( this->_images, items->data[i]->to_int(), i+1 );
	}
	else if ( prop == n_selectedColor )				{ this->setSelectedColor( val->to_colorref() ); }
	else if ( prop == n_sortingEnabled )			{ this->setSortingEnabled( val->to_bool() ); }
	else if ( prop == n_sortingInverted )			{ this->setSortingInverted( val->to_bool() ); }
	else if ( prop == n_syncSelection )				{ this->setSelectionSynced( val->to_bool() ); }
	else if ( prop == n_useSelectedColor )			{ this->setUseSelectedColor( val->to_bool() ); }
	else if ( prop == n_width )						{ this->setWidth( val->to_int() ); }
	else											{ RolloutControl::set_property( arg_list, count ); }

	return &ok;
}

//----------------------------------------------------------------------------------------------------------------
//											TREEVIEWITEMPLUS
//----------------------------------------------------------------------------------------------------------------

visible_class_instance( TreeViewItemPlus, "TreeItemPlus" );

// Constructor
TreeViewItemPlus::TreeViewItemPlus( TreeViewPlus* control ) {
	this->tag					= class_tag( TreeViewItemPlus );
	this->_amEnabled			= TRUE;
	this->_amSelected			= FALSE;
	this->_control				= control;
	this->_children				= new Array(0);
	this->_data					= &undefined;
	this->_extraData			= &undefined;
	this->_iconImage			= 0;
	this->_iconSelectedImage	= -1;
	this->_iconState			= 0;
	this->_name					= _T("");
	this->_node					= &undefined;
	this->_parent				= NULL;
	this->_color				= RGB(0,0,0);
	this->_toolTip				= _T("");
	this->_treeItem				= NULL;
}
TreeViewItemPlus::TreeViewItemPlus( TreeViewPlus* control, TreeViewItemPlus* item ) {
	this->tag					= class_tag( TreeViewItemPlus );
	this->_amEnabled			= item->_amEnabled;
	this->_amSelected			= item->_amSelected;
	this->_children				= item->_children;
	this->_data					= item->_data;
	this->_extraData			= item->_extraData;
	this->_iconImage			= item->_iconImage;
	this->_iconSelectedImage	= item->_iconSelectedImage;
	this->_iconState			= item->_iconState;
	this->_name					= item->_name;
	this->_node					= item->_node;
	this->_parent				= item->_parent;
	this->_color				= item->_color;
	this->_toolTip				= item->_toolTip;
	this->_treeItem				= item->_treeItem;
	this->_control				= control;
}
TreeViewItemPlus::~TreeViewItemPlus() {}

//----------------------------------------------------------------------------------------------------------------
//									TREEVIEWITEMPLUS: MAXScript Methods
//----------------------------------------------------------------------------------------------------------------

BOOL				TreeViewItemPlus::addChild( TreeViewItemPlus* child ) {
	BOOL success = FALSE;
	if ( !( child == this || child->hasChild( this, TRUE ) ) ) {
		if ( child->_parent != this ) {
			if ( child->remove(FALSE) ) {
				child->_parent = this;
				this->_children->append( child );

				if ( !child->isInitialized() )			// Child item does not exist in tree yet
					child->addToTree( this->treeItem() );

				if ( this->control()->isSortingEnabled() )
					this->sort(FALSE);

				success = TRUE;
			}
		}
	}
	return success;
}
Value*				TreeViewItemPlus::addItem( Value* item, BOOL recursive ) {
	// Add String
	if ( item->is_kind_of( class_tag( String ) ) ) {
		TreeViewItemPlus* child	= new TreeViewItemPlus( this->control() );
		child->setNode( item );
		child->setName( _T( item->to_string() ) );
		if ( this->addChild( child ) )
			return child;
	}

	// Add MAXNode || MAXRootNode
	else if ( item->is_kind_of( class_tag( MAXNode ) ) || item->is_kind_of( class_tag( MAXRootNode ) ) ) {
		TreeViewItemPlus* child = new TreeViewItemPlus( this->control() );
		child->setNode( item );

		INode* node;
		if ( item->is_kind_of( class_tag( MAXNode ) ) ) {
			node = item->to_node();
			child->setName( node->GetName() );
			child->setColor( node->GetWireColor() );
			child->setSelected( node->Selected() );
		}
		else {
			node = item->to_rootnode();
			child->setName( _T( "Objects" ) );
		}
		if ( this->addChild( child ) ) {
			if ( recursive ) {
				int childCount	= node->NumberOfChildren();
				for (int i = 0; i < childCount; i++ ) {
					Value* childNode = new MAXNode( node->GetChildNode(i) );
					child->addItem( childNode, TRUE );
				}
			}
			return child;
		}
		return &undefined;
	}

	// Add TreeViewItemPlus
	else if ( item->is_kind_of( class_tag(TreeViewItemPlus) ) ) {
		TreeViewItemPlus* child	= new TreeViewItemPlus( this->control(), (TreeViewItemPlus*) item );
		if ( this->addChild( child ) )
			return child;
		return &undefined;
	}

	// Add MAXTexture
	else if ( item->is_kind_of( class_tag( MAXTexture ) ) || item->is_kind_of( class_tag( MAXMultiMaterial ) ) ) {
		MtlBase* mtl				= (MtlBase*) item->to_texmap();
		TreeViewItemPlus* child		= new TreeViewItemPlus( this->control() );
		child->setNode( item );
		child->setName( _T( mtl->GetName() ) );
		if ( this->addChild( child ) )
			return child;
		return &undefined;
	}

	// Add MAXMaterial || MAXMultiMaterial
	else if ( item->is_kind_of( class_tag( MAXMaterial ) ) || item->is_kind_of( class_tag( MAXMultiMaterial ) ) ) {
		Mtl* mtl					= (Mtl*) item->to_mtl();
		TreeViewItemPlus* child		= new TreeViewItemPlus( this->control() );
		child->setNode( item );
		child->setName( _T( mtl->GetName() ) );
		child->setColor( mtl->GetDiffuse(0).toRGB() );
		if ( this->addChild( child ) )
			return child;
		return &undefined;
	}
	
	// Add Array
	else if( item->is_kind_of( class_tag( Array ) ) ) {
		one_typed_value_local( Array* out );
		vl.out				= new Array(0);
		Array* itemList		= (Array*) item;
		for ( int i = 0; i < itemList->size; i++ ) {
			Value* result = this->addItem( itemList->data[i] );
			if ( result != &undefined )
				vl.out->append( result );
		}
		return_value( vl.out );
	}

	return &undefined;
}
void				TreeViewItemPlus::addToTree( HTREEITEM parent ) {
	TVINSERTSTRUCT insert;
	TVITEM item;
	item.mask			= TVIF_HANDLE | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE;
	item.pszText		= LPSTR( this->name() );
	item.cchTextMax		= lstrlen( this->name() );
	item.iImage			= this->iconImage();
	item.iSelectedImage	= this->iconSelectedImage();
	item.lParam			= (LPARAM) this;
	item.state			= this->isSelected() ? TVIS_SELECTED : 0;

	insert.item			= item;
	insert.hInsertAfter	= TVI_LAST;

	// Set the parent item based on the specified level
	if ( parent )
		insert.hParent	= parent;
	else
		insert.hParent	= TVI_ROOT;

	// Add the item to the tree-view control
	HTREEITEM hNewItem	= (HTREEITEM) SendMessage( this->control()->window(), TVM_INSERTITEM, 0, (LPARAM) (LPTVINSERTSTRUCT) &insert );

	item.mask			= TVIF_PARAM;
	item.hItem			= hNewItem;

	TreeView_GetItem( this->control()->window(), &item );

	if ( item.lParam ) {
		TreeViewItemPlus* tvItem	= (TreeViewItemPlus*) item.lParam;
		tvItem->_treeItem			= hNewItem;
		tvItem->setName( tvItem->name() );
		TreeView_SetItem( this->control()->window(), &item ); 
	}

	for (int i = 0; i < this->_children->size; i++ )
		((TreeViewItemPlus*) this->_children->data[i])->addToTree( this->treeItem() );
}
Array*				TreeViewItemPlus::children( BOOL recursive ) {
	if ( recursive ) {
		init_thread_locals();
		push_alloc_frame();
		one_typed_value_local( Array* out );
		vl.out = new Array(0);
		for ( int i = 0; i < this->_children->size; i++ ) {
			vl.out->append( this->_children->data[i] );
			vl.out->join( ((TreeViewItemPlus*) this->_children->data[i])->children(TRUE) );
		}
		pop_alloc_frame();
		return_value( vl.out );
	}
	return this->_children;
}
COLORREF			TreeViewItemPlus::color()				{ return this->_color; }
TreeViewPlus*		TreeViewItemPlus::control()				{ return this->_control; }
Value*				TreeViewItemPlus::data()				{ return this->_data; }
void				TreeViewItemPlus::deleteChildren()		{
	for ( int i = this->_children->size - 1; i >= 0; i-- )
		((TreeViewItemPlus*) this->_children->data[i])->remove( FALSE );
}
void				TreeViewItemPlus::deselectAll( BOOL recursive )		{
	Array* childList = this->children( recursive );
	for ( int i = 0; i < childList->size; i++ )
		((TreeViewItemPlus*)childList->data[i])->setSelected( FALSE );
}
void				TreeViewItemPlus::editLabel() {
	if ( this->isInitialized() ) {
		this->ensureVisible();
		this->setSelected( TRUE );
		TreeView_EditLabel( this->control()->window(), this->treeItem() );
	}
}
void				TreeViewItemPlus::endEditLabel() {
	if ( this->isInitialized() )
		TreeView_EndEditLabelNow( this->control()->window(), TRUE );
}
BOOL				TreeViewItemPlus::ensureVisible() {
	return (TreeView_EnsureVisible( this->control()->window(), this->treeItem() ));
}
Value*				TreeViewItemPlus::extraData()			{ return this->_extraData; }
Value*				TreeViewItemPlus::gt_vf( Value** arg_list, int count ) {
	check_arg_count( __gt__, 1, count );
	return (CompareItems( (LPARAM) this, (LPARAM) arg_list[0], (LPARAM) this->control() ) == 1 ) ? &true_value : &false_value;
}
BOOL				TreeViewItemPlus::hasChild( TreeViewItemPlus* item, BOOL recursive ) {
	BOOL success = FALSE;
	for (int i = 0; i < this->_children->size; i++ ) {
		if ( this->_children->data[i] == item ) {
			success = TRUE;
			break;
		}
		if ( recursive ) {
			if ( ((TreeViewItemPlus*) this->_children->data[i])->hasChild( item, TRUE ) ) {
				success = TRUE;
				break;
			}
		}
	}
	return success;
}
int					TreeViewItemPlus::iconImage()			{ return this->_iconImage; }
int					TreeViewItemPlus::iconSelectedImage()	{ return (this->_iconSelectedImage == -1) ? this->_iconImage : this->_iconSelectedImage; }
int					TreeViewItemPlus::iconState()			{ return this->_iconState; }
BOOL				TreeViewItemPlus::isCollapsed()		{
	// Collapsed if this item is collapsed or any ancestors are collapsed
	BOOL amCollapsed = FALSE;
	if ( !this->isRoot() ) {
		amCollapsed = (TreeView_GetItemState( this->control()->window(), this->treeItem(), TVIS_EXPANDED ) & TVIS_EXPANDED) ? FALSE : TRUE;
		if ( !amCollapsed && this->parent() )
			amCollapsed = this->parent()->isCollapsed();
	}
	return amCollapsed;
}
BOOL				TreeViewItemPlus::isEnabled()			{ 
	return ( this->_amEnabled && this->parent() ) ? this->parent()->isEnabled() : this->_amEnabled; 
}
BOOL				TreeViewItemPlus::isInitialized()		{ return (this->_treeItem != NULL); }
BOOL				TreeViewItemPlus::isRoot()				{ return ( this->control()->root() == this ); }
BOOL				TreeViewItemPlus::isSelected()			{ return this->_amSelected; }
BOOL				TreeViewItemPlus::isVisible()			{ return (this->parent()) ? !this->parent()->isCollapsed() : TRUE; }
Value*				TreeViewItemPlus::itemByData( Value* data, BOOL recursive ) {
	Array* childList = this->children(recursive);
	Value* out = &undefined;
	for (int i = 0; i < childList->size; i++ ) {
		if ( ((TreeViewItemPlus*) childList->data[i] )->data() == data ) {
			out = childList->data[i];
			break;
		}
	}
	return out;
}
Array*				TreeViewItemPlus::itemsByNode( Value* node, BOOL recursive ) {
	one_typed_value_local( Array* out );
	vl.out = new Array(0);

	for (int i = 0; i < this->_children->size; i++ ) {
		if ( this->_children->data[i] == node )
			vl.out->append( this->_children->data[i] );
	}
	return_value( vl.out );
}
Value*				TreeViewItemPlus::lt_vf( Value** arg_list, int count ) {
	check_arg_count( __lt__, 1, count );
	return (CompareItems( (LPARAM) this, (LPARAM) arg_list[0], (LPARAM) this->control() ) == -1) ? &true_value : &false_value;
}
TSTR				TreeViewItemPlus::name()				{ return this->_name; }
Value*				TreeViewItemPlus::node()				{ return this->_node; }
TreeViewItemPlus*	TreeViewItemPlus::parent()				{ return this->_parent; }
BOOL				TreeViewItemPlus::remove( BOOL repaintUi ) {
	// Remove from the Ui
	if ( this->_treeItem ) {
		TreeView_DeleteItem( this->control()->window(), this->_treeItem );
		this->_treeItem = NULL;
		if ( repaintUi )
			this->control()->repaint();
	}
	// Remove from the data hierarchy
	if ( this->_parent ) {
		Array* oldChildren	= this->_parent->_children;
		Array* newChildren	= new Array(0);
		for (int i = 0; i < oldChildren->size; i++ ) {
			if ( oldChildren->data[i] != this )
				newChildren->append( oldChildren->data[i] );
		}
		this->_parent->_children	= newChildren;
	}
	return TRUE;
}
TreeViewItemPlus*	TreeViewItemPlus::root()				{ return this->control()->root(); }
BOOL				TreeViewItemPlus::setCollapsed( BOOL state ) {
	BOOL success = FALSE;
	if ( state ) {
		success = TreeView_Expand( this->control()->window(), this->treeItem(), TVE_COLLAPSE );
	}
	else {
		success = TreeView_Expand( this->control()->window(), this->treeItem(), TVE_EXPAND );
	}
	return success;
}
BOOL				TreeViewItemPlus::setColor( COLORREF clr ) {
	this->_color = clr;
	if ( this->isInitialized() )
		this->control()->repaint();
	return TRUE;
}
BOOL				TreeViewItemPlus::setControl( TreeViewPlus* ctrl ) { 
	BOOL success = FALSE;
	if ( !this->_control ) {
		this->_control = ctrl; 
		success = TRUE;
	}
	return success;
}
BOOL				TreeViewItemPlus::setData( Value* dataValue )	{ 
	this->_data	= dataValue; 
	return TRUE;
}
BOOL				TreeViewItemPlus::setEnabled( BOOL state ) {
	this->_amEnabled = state;
	return (this->isEnabled());
}
BOOL				TreeViewItemPlus::setExtraData( Value* extraDat ) {
	this->_extraData = extraDat;
	return TRUE;
}
BOOL				TreeViewItemPlus::setIconImage( int index ) {
	BOOL success = TRUE;
	if ( this->isInitialized() ) {
		TVITEM tvItem;
		tvItem.mask			= TVIF_HANDLE | TVIF_PARAM | TVIF_IMAGE;
		tvItem.hItem		= this->treeItem();
		tvItem.iImage		= index;
		tvItem.lParam		= (LPARAM) this;

		success				= TreeView_SetItem( this->control()->window(), &tvItem );
	}

	if ( success ) 
		this->_iconImage	= index;

	return success;
}
BOOL				TreeViewItemPlus::setIconSelectedImage( int index ) {
	BOOL success = TRUE;
	if ( this->isInitialized() ) {
		TVITEM tvItem;
		tvItem.mask					= TVIF_HANDLE | TVIF_PARAM | TVIF_SELECTEDIMAGE;
		tvItem.hItem				= this->treeItem();
		tvItem.iSelectedImage		= index;
		tvItem.lParam				= (LPARAM) this;
		success						= TreeView_SetItem( this->control()->window(), &tvItem );
	}

	if ( success )
		this->_iconSelectedImage	= index;

	return success;
}
BOOL				TreeViewItemPlus::setIconState( int index ) {
	BOOL success = TRUE;
	if ( this->isInitialized() ) {
		TVITEM tvItem;
		tvItem.mask					= TVIF_HANDLE | TVIF_PARAM |TVIF_STATE;
		tvItem.stateMask			= TVIS_OVERLAYMASK;
		tvItem.state				= (INDEXTOOVERLAYMASK( index ) );
		tvItem.hItem				= this->treeItem();
		tvItem.lParam				= (LPARAM) this;

		success						= TreeView_SetItem( this->control()->window(), &tvItem );
	}
	if ( success )
		this->_iconState			= index;

	return success;
}
BOOL				TreeViewItemPlus::setName( TSTR newName )		{ 
	BOOL success = TRUE;
	if ( this->isInitialized() ) {
		TVITEM tvItem;
		tvItem.mask		= TVIF_HANDLE | TVIF_PARAM | TVIF_TEXT;
		tvItem.hItem	= this->_treeItem;
		tvItem.pszText	= newName;
		tvItem.lParam	= (LPARAM) this;

		success	= TreeView_SetItem( this->control()->window(), &tvItem );
		this->control()->invalidate();
	}

	if ( success )
		this->_name	= newName;

	return success;
}
BOOL				TreeViewItemPlus::setNode( Value* node ) {
	this->_node = node;
	return TRUE;
}
BOOL				TreeViewItemPlus::setParent( TreeViewItemPlus* parentItem ) {
	BOOL success = FALSE;
	if ( parentItem && is_treeviewitemplus( parentItem ) ) {
		success = parentItem->addChild( this );
	}
	return success;
}
BOOL				TreeViewItemPlus::setToolTip( TSTR tip ) {
	this->_toolTip = tip;
	this->control()->invalidate();
	return TRUE;
}
BOOL				TreeViewItemPlus::setSelected( BOOL state, BOOL recursive, BOOL updateUi )		{
	if ( recursive ) {
		for ( int i = 0; i < this->_children->size; i++ )
			((TreeViewItemPlus*) this->_children->data[i])->setSelected( state, recursive, FALSE );
	}
	if ( this->isEnabled() ) {
		this->_amSelected = state;
		if ( updateUi && this->isInitialized() )
			this->control()->repaint();
	}
	return this->_amSelected;
}
Array*				TreeViewItemPlus::siblings() {
	init_thread_locals();
	one_typed_value_local( Array* out );

	push_alloc_frame();

	vl.out	= new Array(0);

	if ( this->_parent ) {
		for ( int i = 0; i < this->_parent->_children->size; i++ )
			if ( this->_parent->_children->data[i] != this )
				vl.out->append( this->_parent->_children->data[i] );
	}

	pop_alloc_frame();

	return_value( vl.out );
}
void				TreeViewItemPlus::sort( BOOL recursive ) {
	// Sort Tree Items
	BOOL success = TRUE;
	for (int i = 0; i < this->_children->size; i++) {
		TreeViewItemPlus* item = (TreeViewItemPlus*) this->_children->data[i];
		if ( is_treeviewitemplus( item ) ) {
			HTREEITEM hChild	= TreeView_GetChild( this->control()->window(), this->treeItem() );

			TVSORTCB sort;
			sort.hParent		= hChild ? this->treeItem() : NULL;
			sort.lParam			= (LPARAM) this->control();
			sort.lpfnCompare	= CompareItems;

			success				= success && TreeView_SortChildrenCB( this->control()->window(), &sort, TRUE );
		}
	}

	// Sort Data Items
	this->_children->sort();

	if ( recursive ) {
		for (int i = 0; i < this->_children->size; i++ )
			((TreeViewItemPlus*) this->_children->data[i])->sort(recursive);
	}
}
BOOL				TreeViewItemPlus::toggleCollapsed() {
	return TreeView_Expand( this->control()->window(), this->treeItem(), TVE_TOGGLE );
}
TSTR				TreeViewItemPlus::toolTip()				{ return this->_toolTip; }
HTREEITEM			TreeViewItemPlus::treeItem()			{ return this->_treeItem; }


//----------------------------------------------------------------------------------------------------------------
//									TREEVIEWITEMPLUS: MAXScript Methods
//----------------------------------------------------------------------------------------------------------------
void				TreeViewItemPlus::sprin1( CharStream* s ) { s->printf( _T("<TreeViewItemPlus:%s>"), (TCHAR*) this->_name ); }
void				TreeViewItemPlus::collect() {
	if ( this->_node )		DeleteObject( this->_node );
	if ( this->_extraData )	DeleteObject( this->_extraData );
	if ( this->_data )		DeleteObject( this->_data );
	if ( this->_children )	DeleteObject( this->_children );
	delete this;
}
void				TreeViewItemPlus::gc_trace() {
	Value::gc_trace();

	if ( this->_data		&& this->_data->is_not_marked() )		this->_data->gc_trace();
	if ( this->_extraData	&& this->_extraData->is_not_marked() )	this->_extraData->gc_trace();
	if ( this->_node		&& this->_node->is_not_marked() )		this->_node->gc_trace();
	if ( this->_children	&& this->_children->is_not_marked() )	this->_children->gc_trace();
}
Value*				TreeViewItemPlus::applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc ) {
	if ( methodID == n_addChild ) {
		check_arg_count( addChild, 1, count );
		return ( this->addChild( (TreeViewItemPlus*) arg_list[0] ) ) ? &true_value : &false_value;
	}
	else if ( methodID == n_addItem ) {
		check_arg_count_with_keys( addItem, 1, count );
		BOOL recursive	= false;
		Value* param	= key_arg( recursive );
		if ( param != &unsupplied )
			recursive	= param->to_bool();
		return this->addItem( arg_list[0], recursive );
	}
	else if ( methodID == n_collapse ) {
		check_arg_count( collapse, 0, count );
		return ( this->setCollapsed( TRUE ) ) ? &true_value : &false_value;
	}
	else if ( methodID == n_deleteChildren ) {
		check_arg_count( deleteChildren, 0, count );
		this->deleteChildren();
	}
	else if ( methodID == n_editLabel ) {
		check_arg_count( editLabel, 0, count );
		this->editLabel();
	}
	else if ( methodID == n_endEditLabel ) {
		check_arg_count( endEditLabel, 0, count );
		this->endEditLabel();
	}
	else if ( methodID == n_ensureVisible ) {
		check_arg_count( ensureVisible, 0, count );
		return ( this->ensureVisible() ) ? &true_value : &false_value;
	}
	else if ( methodID == n_expand ) {
		check_arg_count( expand, 0, count );
		return ( this->setCollapsed( FALSE ) ) ? &true_value : &false_value;
	}
	else if ( methodID == n_hasChild ) {
		check_arg_count( hasChild, 1, count );
		return ( this->hasChild( (TreeViewItemPlus*) arg_list[0] ) ) ? &true_value : &false_value;
	}
	else if ( methodID == n_isCollapsed ) {
		check_arg_count( isCollapsed, 0, count );
		return ( this->isCollapsed() ) ? &true_value : &false_value;
	}
	else if ( methodID == n_isVisible ) {
		check_arg_count( isVisible, 0, count );
		return ( this->isVisible() ) ? &true_value : &false_value;
	}
	else if ( methodID == n_itemByData ) {
		check_arg_count( itemByData, 1, count );
		return this->itemByData( arg_list[0] );
	}
	else if ( methodID == n_remove ) {
		check_arg_count( remove, 0, count );
		return ( this->remove() ) ? &true_value : &false_value;
	}
	else if ( methodID == n_setCollapsed ) {
		check_arg_count( setCollapsed, 1, count );
		return ( this->setCollapsed( arg_list[0]->to_bool() ) ) ? &true_value : &false_value;
	}
	else if ( methodID == n_setParent ) {
		check_arg_count( setParent, 1, count );
		if ( arg_list[0] == &undefined )
			return ( this->setParent( this->control()->root() ) )		? &true_value : &false_value;
		if ( is_treeviewitemplus( arg_list[0] ) ) {
			return ( this->setParent( (TreeViewItemPlus*) arg_list[0] ) )	? &true_value : &false_value;
		}
		return &false_value;
	}
	else if ( methodID == n_toggleCollapsed ) {
		check_arg_count( toggleCollapsed, 0, count );
		return ( this->toggleCollapsed() ) ? &true_value : &false_value;
	}
	else if ( methodID == n_toggleExpand ) {
		check_arg_count( toggleExpand, 0, count );
		return ( this->toggleCollapsed() ) ? &true_value : &false_value;
	}

	return &ok;
}
Value*				TreeViewItemPlus::get_property( Value** arg_list, int count ) {
	Value* prop = arg_list[0];

	if ( prop == n_addChild )				return NEW_GENERIC_METHOD( addChild );
	else if ( prop == n_addItem )			return NEW_GENERIC_METHOD( addItem );
	else if ( prop == n_children )			return this->children(FALSE);
	else if ( prop == n_collapse )			return NEW_GENERIC_METHOD( collapse );
	else if ( prop == n_color )				return ColorValue::intern( AColor( this->color() ) );
	else if ( prop == n_control )	{
		if ( this->control() )
			return this->control();
		return &undefined;
	}
	else if ( prop == n_data )				return this->data();
	else if ( prop == n_deleteChildren )	return NEW_GENERIC_METHOD( deleteChildren );
	else if ( prop == n_editLabel )			return NEW_GENERIC_METHOD( editLabel );
	else if ( prop == n_enabled )			return (this->isEnabled()) ? &true_value : &false_value;
	else if ( prop == n_endEditLabel )		return NEW_GENERIC_METHOD( endEditLabel );
	else if ( prop == n_ensureVisible )		return NEW_GENERIC_METHOD( ensureVisible );
	else if ( prop == n_expand )			return NEW_GENERIC_METHOD( expand );
	else if ( prop == n_extraData )			return this->extraData();
	else if ( prop == n_hwnd )				return (this->_treeItem) ? IntegerPtr::intern( reinterpret_cast<INT64> (this->_treeItem) ) : &undefined;
	else if ( prop == n_hasChild )			return NEW_GENERIC_METHOD( hasChild );
	else if ( prop == n_iconImage )			return Integer::intern( this->iconImage() );
	else if ( prop == n_iconSelectedImage )	return Integer::intern( this->iconSelectedImage() );
	else if ( prop == n_iconState )			return Integer::intern( this->iconState() );
	else if ( prop == n_isCollapsed )		return NEW_GENERIC_METHOD( isCollapsed );
	else if ( prop == n_isVisible )			return NEW_GENERIC_METHOD( isVisible );
	else if ( prop == n_itemByData )		return NEW_GENERIC_METHOD( itemByData );
	else if ( prop == n_setCollapsed )		return NEW_GENERIC_METHOD( setCollapsed );
	else if ( prop == n_setParent )			return NEW_GENERIC_METHOD( setParent );
	else if ( prop == n_node )				return this->node();
	else if ( prop == n_name )				return ( new String( this->name() ) );
	else if ( prop == n_parent )			{
		if ( this->parent() )
			return this->parent();
		else
			return &undefined;
	}
	else if ( prop == n_remove )			return NEW_GENERIC_METHOD( remove );
	else if ( prop == n_selected )			return (this->isSelected()) ? &true_value : &false_value;
	else if ( prop == n_siblings )			return this->siblings();
	else if ( prop == n_toggleCollapsed )	return NEW_GENERIC_METHOD( toggleCollapsed );
	else if ( prop == n_toggleExpand )		return NEW_GENERIC_METHOD( toggleExpand );
	else if ( prop == n_toolTip )			return ( new String( this->toolTip() ) );
	else									throw RuntimeError( "#%s is not a valid property of TreeViewItemPlus", prop->to_string() );

	return &ok;
}

Value*				TreeViewItemPlus::set_property( Value** arg_list, int count ) {
	Value* val	= arg_list[0]->eval();
	Value* prop	= arg_list[1];

	if ( prop == n_color )					return ( this->setColor( val->to_colorref() ) )		? &true_value : &false_value;
	else if ( prop == n_data )				return ( this->setData( val ) )						? &true_value : &false_value;
	else if ( prop == n_enabled )			return ( this->setEnabled( val->to_bool() ) )			? &true_value : &false_value;
	else if ( prop == n_extraData )			return ( this->setExtraData( val ) )					? &true_value : &false_value;
	else if ( prop == n_iconImage )			return ( this->setIconImage( val->to_int() ) )			? &true_value : &false_value;
	else if ( prop == n_iconSelectedImage )	return ( this->setIconSelectedImage( val->to_int() ) )	? &true_value : &false_value;
	else if ( prop == n_name )				return ( this->setName( val->to_string() ) )			? &true_value : &false_value;
	else if ( prop == n_node )				return ( this->setNode( val ) )						? &true_value : &false_value;
	else if ( prop == n_selected )			{
		if ( !this->control()->isMultiSelection() )
			this->control()->deselectAll();
		return ( this->setSelected( val->to_bool() ) )			? &true_value : &false_value;
	}
	else if ( prop == n_toolTip )			return ( this->setToolTip( val->to_string() ) )		? &true_value : &false_value;
	else									throw RuntimeError( _T( "#% is not a valid property of TreeViewItemPlus" ), prop->to_string() );

	return &ok;
}

//----------------------------------------------------------------------------------------------------------------
//									TREEVIEWPLUS: Initialize
//----------------------------------------------------------------------------------------------------------------

void TreeViewPlusInit() {
	static BOOL registered = FALSE;
	if ( !registered ) {
		WNDCLASSEX wcex;
		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.hInstance		= g_hInst;
		wcex.hIcon			= NULL;
		wcex.hCursor		= LoadCursor( NULL, IDC_ARROW );
		wcex.hbrBackground	= (HBRUSH)GetStockObject( HOLLOW_BRUSH );
		wcex.lpszMenuName	= NULL;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= 0;
		wcex.lpfnWndProc	= TreeViewPlus::WndProc;
		wcex.lpszClassName	= TREECTRL_WINDOWCLASS;
		wcex.cbSize			= sizeof(WNDCLASSEX);
		wcex.hIconSm		= NULL;

		if ( RegisterClassEx( &wcex ) )
			registered		= TRUE;
	}
	if ( registered )
		install_rollout_control( Name::intern( "TreeViewPlus" ), TreeViewPlus::create );
}