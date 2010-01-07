/*===========================================================================*\
 |  Ishani's stuff for MAX Script R3
 |
 |  FILE:	TreeView.h
 |			Header file for my stuff
 | 
 |  AUTH:   Diego Garcia
 |			Copyright(c) Blur Studios Inc. 2005
 |			All Rights Reserved.
 |
 |  HIST:	Started 04-18-05
 | 
\*===========================================================================*/


// String Definitions
#define s_treeview				_T("TreeViewControl")
//-----


// Window Classes
#define TREEVIEW_WINDOWCLASS   WC_TREEVIEW
//-----


/*========================[ TREE VIEW CONTROL ]===========================*\
\*===========================================================================*/
class TreeViewItem;
class TreeViewControl;

visible_class(TreeViewControl)

class TreeViewControl : public RolloutControl
{
private:
	TVHITTESTINFO	m_tvht; 
	HTREEITEM		m_hitTarget;
	POINTS			m_Pos;
	BOOL			m_dragging;
	BOOL			m_multiselection;
	BOOL			m_dragEnabled;
	COLORREF		m_bkcolor;
	COLORREF		m_selectedColor;
	BOOL			m_useSelectedColor;
	Value*			m_data;
	// label editing 
	BOOL			m_editLabels;
	Value*			m_ItemEditing;  
	HWND			m_hEdit;

public:
	HWND	m_TreeHWnd;
	HWND	m_containerHWnd;
	HIMAGELIST	m_TreeViewImagesHwnd;
	Array*	m_items;
	Value*	m_user_compare_fn;
	BOOL	m_syncSelection;
	
	// Constructor
	TreeViewControl(Value* name, Value* caption, Value** keyparms, int keyparm_count);
	~TreeViewControl();

	// Static methods
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static RolloutControl* create(Value* name, Value* caption, Value** keyparms, int keyparm_count)
	{ return new TreeViewControl(name, caption, keyparms, keyparm_count); }

	// Property methods
	BOOL InitTreeViewImageLists();

	Value* GetData(HTREEITEM hItem);
	long handleNotify(HWND hWndDlg, int nIDCtrl, LPNMHDR pNMHDR);
	long handleCustomDraw(HWND hWndTreeView, LPNMTVCUSTOMDRAW pNMTVCD);	
	long handleBeginDrag(HWND hWndTreeView, LPNMTREEVIEW pNMTV);
	long handleBeginEditLabel(HWND hWndTreeView, LPNMTVCUSTOMDRAW pNMTVCD);
	long handleEndEditLabel(HWND hWndTreeView, LPNMTVCUSTOMDRAW pNMTVCD);
	long handleCustomTooltip(NMTVGETINFOTIP* pTVTipInfo);

	Value *AddItemToTree(TreeViewItem* tvitem);
	Value* HitTest(HWND hWndTreeView, UINT flags);

	Value *CopyItem( TreeViewItem *pTreeviewItem , TreeViewItem *pParentTreeviewItem, HTREEITEM htiAfter);
	//Value *CopyBranch( TreeViewItem *pTreeviewItem , TreeViewItem *pParentTreeviewItem, HTREEITEM htiAfter);
	Value *CopyBranch( TreeViewItem *pTreeviewItem , TreeViewItem *pParentTreeviewItem, BOOL deleteOld , HTREEITEM htiAfter );
	
	LRESULT LButtonUp(void);
	LRESULT LButtonDown(void);
	LRESULT LButtonDblClk(void);
	LRESULT MButtonUp(void);
	LRESULT RButtonDblClk(void);
	LRESULT RButtonDown(void);
	LRESULT FocusChanged(void);

	void	Invalidate();

	// MAXScript event handlers
	         classof_methods(TreeViewControl, RolloutControl);

	// Garbage collection
	void	 collect() { delete this; }
	void	 gc_trace();
#	define	 is_treeviewcontrol(p) ((p)->tag == class_tag(TreeViewControl))
	void     sprin1(CharStream* s) { s->printf(_T("TreeViewControl:%s"), name->to_string()); }

	void     add_control(Rollout *ro, HWND parent, HINSTANCE hInstance, int& current_y);
	LPCTSTR  get_control_class() { return TREEVIEW_WINDOWCLASS; }
	int		 num_controls() { return 1; }
	void     compute_layout(Rollout *ro, layout_data* pos) { pos->width = 90; pos->height = 30;}
	BOOL     handle_message(Rollout *ro, UINT message, WPARAM wParam, LPARAM lParam);

	Value*   get_property(Value** arg_list, int count);
	Value*   set_property(Value** arg_list, int count);

	def_generic (get,		"get");
//	def_generic (put,		"put");

	Value*   addItem(Value** arg_list, int count);

	void     set_enable();

#include "defimpfn.h"
	def_generic		( get_props,	"getPropNames" );
	def_generic		( show_props,	"showProperties" );
	def_visible_generic ( show_methods, "showMethods");
	def_visible_generic ( show_events, "showEvents");
};


visible_class(TreeViewItem)
class TreeViewItem : public AppliedValue
{
public:
	TSTR			m_name;
	TSTR			m_tooltip;
	Value*			m_node;
	TreeViewItem*	m_parent;
	Array*			m_children;
	BOOL			m_enabled;
	BOOL			m_selected;
	int				m_iconImage;
	int				m_iconSelectedImage;
	int				m_iconState;
	HTREEITEM 		m_tvi;
	COLORREF		m_color;
	TreeViewControl*m_tvControl;
	Value*			m_data;
	Value*			m_extraData;
	
	TreeViewItem();
	TreeViewItem(TreeViewItem *tvi);
	BOOL isVisible( void );
	BOOL isCollapsed( void );
	BOOL collapse( void );
	BOOL toggleExpand( void );
	BOOL expand( void );
	BOOL ensureVisible( void );
	Value* getSiblings ( void );
	Value* getChildren ( BOOL recursive = TRUE );
	Value* getParent ( void );
	Value* editLabel ( void );
	Value* endEditLabel ( void );

	classof_methods(TreeViewItem, Value);
	void		collect();
	void		sprin1(CharStream* s);
	void		gc_trace();
#	define		is_treeviewitem(p) ((p)->tag == class_tag(TreeViewItem))

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

	Value*		applyMethod( Value* methodID, Value** arg_list, int count, CallContext* cc );

#include "defimpfn.h"
	def_generic		( get_props,	"getPropNames" );
	def_generic		( show_props,	"showProperties" );
	def_visible_generic ( show_methods, "showMethods");
	def_visible_generic ( show_events, "showEvents");
};



